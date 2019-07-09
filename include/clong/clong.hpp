#ifndef CLONG_CLONG_HPP
#define CLONG_CLONG_HPP

#include <clang/AST/Comment.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/PrettyPrinter.h>

namespace clong {

/// A node definition with it's decl `clang` declaration
struct Node {
  Node* parent;
  std::string comment;
  const clang::Decl* decl;
  std::vector<Node*> children;
};

class PrettyPrinter {
  private:
  static bool ends_with(std::string const& str, std::string const& with) {
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
    p += prefix + pprint(node->decl) + " -- " + ascii_encode(node->comment) + "\n";
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
  std::unordered_set<const Node*> m_functions;

  public:
  const Node& root() const { return m_root; }
  const std::unordered_set<const Node*>& functions() const { return m_functions; }

  clang::comments::FullComment* comments_of(const clang::Decl* decl) const {
    // Extract comment node from decl
    return decl->getASTContext().getLocalCommentForDeclUncached(decl);
  }

  bool has_registered_node(const clang::Decl* decl) const {
    return m_decl2node.find(decl) != m_decl2node.end();
  }

  bool has_been_visited(const clang::Decl* decl) const {
    return m_visited.find(decl) != m_visited.end();
  }

  bool has_been_visited_and_registered(const clang::Decl* decl) const {
    return has_been_visited(decl) && has_registered_node(decl);
  }

  void mark_as_visited(const clang::Decl* decl) {
    m_visited.insert(decl);
  }

  Node* register_node(const clang::Decl* decl) {
    // If already marked as visited, don't go further
    if (has_been_visited(decl)) {
      // If there is a registered node, returns it, otherwise it means that the current decl
      // has been marked as visited and means we cannot register a node to this decl. In this
      // case returns nullptr.
      if (has_registered_node(decl)) {
        return m_decl2node[decl].get();
      } else {
        return nullptr;
      }
    }
    // Visit
    mark_as_visited(decl);
    // Register
    m_decl2node.emplace(decl, std::move(std::make_unique<Node>()));
    auto* node = m_decl2node[decl].get();
    // Save decl
    node->decl = decl;
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
      if (has_been_visited_and_registered(walking_decl)) {
        node->parent = m_decl2node[walking_decl].get();
        break;
      }
    }
    // If no parent found, use root as parent node
    if (!node->parent) {
      node->parent = &m_root;
    }
    // Make sure to add children to the parent
    node->parent->children.push_back(node);
    return node;
  }

  void register_function_node(const clang::FunctionDecl* decl) {
    auto node = register_node(decl);
    // Might be null if the decl has been marked visited but has no registered node
    if (node) {
      m_functions.insert(node);
    }
  }

  void register_function_node(const clang::FunctionTemplateDecl* decl) {
    auto node = register_node(decl);
    // Same here!
    if (node) {
      m_functions.insert(node);
    }
  }
};

}

#endif
