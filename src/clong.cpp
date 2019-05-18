#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/Comment.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/PrettyPrinter.h>
#include <llvm/Support/CommandLine.h>

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory OptionsCategory("clong options");

namespace clong {

struct Node {
  Node* parent;
  std::string comment;
  const clang::Decl* current;
  std::vector<Node*> children;
};

class PrettyPrinter {
  private:
  static bool ends_with(std::string const& str, std::string const& with)
  {
    if (with.size() > str.size()) {
      return false;
    }
    return std::equal(with.rbegin(), with.rend(), str.rbegin());
  }

  public:
  static std::string pprint(const clang::Decl* decl, clang::LangOptions const& lo) {
    clang::PrintingPolicy pp(lo);
    // Skip body pprinting (function + class bodies)
    pp.TerseOutput = true;
    // Create steam and buffer
    std::string s;
    llvm::raw_string_ostream ss(s);
    // Use policy to skip bodies
    if (decl) {
      decl->print(ss, pp);
    } else {
      s = "(null-decl)";
    }
    // Force flush of stream
    s = ss.str();
    // Post-process pprinted buffer
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    // Check for " {}"
    std::string bad_brackets = " {}";
    if (ends_with(s, bad_brackets)) {
      s.erase(s.end() - bad_brackets.length(), s.end());
    }
    return s;
  }

  static std::string pprint(const clang::Decl* decl) {
    clang::LangOptions lo;
    return pprint(decl, lo);
  }

  static std::string ascii_encode(std::string const& s) {
    std::string e;
    for (char c : s) {
      if (c == '\n') {
        e += "\\n";
      } else {
        e += c;
      }
    }
    return e;
  }

  static std::string pprint(const Node* node, int level = 0) {
    std::string p;
    std::string prefix = "- ";
    for (int i = 0; i < level; ++i) {
      prefix += "  ";
    }
    p += prefix + pprint(node->current) + " -- " + ascii_encode(node->comment) + "\n";
    for (auto const* child : node->children) {
      p += pprint(child, level + 1);
    }
    return p;
  }

  static std::string pprint_comments(const clang::comments::Comment* com) {
    using comment_t = clang::comments::TextComment;
    std::string p;
    if (com) {
        if (clang::isa<comment_t>(com)) {
          p += clang::dyn_cast<comment_t>(com)->getText();
          p += "\n";
        }
        for (auto child = com->child_begin(); child != com->child_end(); ++child) {
          p += pprint_comments(*child);
        }
    }
    return p;
  }
};

class Context {
  Node m_root;
  std::unordered_map<const clang::Decl*, std::unique_ptr<Node>> m_decl2node;
  std::unordered_set<const clang::Decl*> m_visited;

  public:
  const Node& root() const { return m_root; }

  clang::comments::FullComment* comments_of(const clang::Decl* decl) const {
    // Extract comment node from decl
    return decl->getASTContext().getLocalCommentForDeclUncached(decl);
  }

  bool has_been_visited(const clang::Decl* decl) const {
    return m_visited.find(decl) != m_visited.end();
  }

  void mark_as_visited(const clang::Decl* decl) {
    m_visited.insert(decl);
  }

  void register_node(const clang::Decl* decl) {
    if (has_been_visited(decl)) {
      return;
    }
    // Register
    m_decl2node.emplace(decl, std::move(std::make_unique<Node>()));
    auto& node = m_decl2node[decl];
    // Visit
    mark_as_visited(decl);
    // Save decl
    node->current = decl;
    // Dump comment
    node->comment = PrettyPrinter::pprint_comments(comments_of(decl));
    // Find parents
    auto walking_decl = decl;
    clang::ASTContext& ast_ctxt = decl->getASTContext();
    while (true) {
      const auto& parents = ast_ctxt.getParents(*walking_decl);
      if (parents.empty()) {
        break;
      }
      walking_decl = parents[0].get<clang::Decl>();
      if (!walking_decl) {
        break;
      }
      // Found a registered parent
      if (has_been_visited(walking_decl)) {
        node->parent = m_decl2node[walking_decl].get();
        break;
      }
    }
    // If no parent found, use root as parent node
    if (!node->parent) {
      node->parent = &m_root;
    }
    // Make sure to add children to the parent
    node->parent->children.push_back(node.get());
  }
};


class Visitor : public clang::RecursiveASTVisitor<Visitor> {

  clang::CompilerInstance& m_ci;
  Context& m_ctxt;

  public:
  Visitor(clang::CompilerInstance& ci, Context& ctxt)
    : m_ci(ci), m_ctxt(ctxt) {
  }

  private:
  public:
  bool VisitNamespaceDecl(clang::NamespaceDecl* decl) {
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitClassTemplateDecl(clang::ClassTemplateDecl* decl) {
    m_ctxt.register_node(decl);
    m_ctxt.mark_as_visited(decl->getTemplatedDecl());
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl* decl) {
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl* decl) {
    m_ctxt.register_node(decl);
    m_ctxt.mark_as_visited(decl->getTemplatedDecl());
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* decl) {
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* decl) {
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
    m_ctxt.register_node(decl);
    return true;
  }
};

class Consumer : public clang::ASTConsumer {
  Visitor m_visitor;

  public:
  Consumer(clang::CompilerInstance& ci, Context& ctxt)
      : m_visitor(ci, ctxt) {
  }

  virtual void HandleTranslationUnit(clang::ASTContext &ctxt) {
    // Traversing the translation unit decl via a RecursiveASTVisitor
    // will visit all nodes in the AST
    m_visitor.TraverseDecl(ctxt.getTranslationUnitDecl());
  }
};

class Action : public clang::ASTFrontendAction {
  Context& m_ctxt;

  public:
  Action(Context& ctxt)
    : m_ctxt(ctxt) {
  }

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &ci, llvm::StringRef) {
    return std::unique_ptr<clang::ASTConsumer>(new Consumer(ci, m_ctxt));
  }
};

class FrontendActionFactory : public clang::tooling::FrontendActionFactory {
  Context& m_ctxt;

  public:
  FrontendActionFactory(Context& ctxt) : m_ctxt(ctxt) {
  }

  virtual clang::FrontendAction* create() override {
    return new Action(m_ctxt);
  }
};

}

int main(int argc, const char** argv) {
  // CommonOptionsParser constructor will parse arguments and create a
  // CompilationDatabase.  In case of error it will terminate the program.
  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, OptionsCategory);
  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
          OptionsParser.getSourcePathList());
  //  context
  clong::Context ctxt;
  if (Tool.run(std::make_unique<clong::FrontendActionFactory>(ctxt).get())) {
    return 1;
  }
  std::cout << clong::PrettyPrinter::pprint(&ctxt.root());
  return 0;
}

