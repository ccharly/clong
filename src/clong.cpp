#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/CommandLine.h>
#include "clong/clong.hpp"
#include "clong/jekyll.hpp"

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
namespace cl = llvm::cl;
static cl::OptionCategory OptionsCategory("clong options");

// -O <dir>
static cl::opt<std::string> OutputDir("O",
    cl::desc("Specify output directory"), cl::value_desc("dir"), cl::init("_doc"));


namespace clong {

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
    m_ctxt.register_function_node(decl);
    return true;
  }

  bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl* decl) {
    m_ctxt.register_function_node(decl);
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
  std::function<void (Context&)> m_on_end;

  public:
  template <typename OnEnd>
  Action(Context& ctxt, OnEnd on_end)
    : m_ctxt(ctxt), m_on_end(on_end) {
  }

  Action(Context& ctxt)
    : m_ctxt(ctxt), m_on_end([](Context&) {}) {
  }

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::unique_ptr<clang::ASTConsumer>(new Consumer(ci, m_ctxt));
  }

  virtual void EndSourceFileAction() override {
    // Call hook
    m_on_end(m_ctxt);
  }

};

class FrontendActionFactory : public clang::tooling::FrontendActionFactory {
  Context& m_ctxt;
  std::function<void (Context&)> m_on_end;

  public:
  template <typename OnEnd>
  FrontendActionFactory(Context& ctxt, OnEnd on_end)
    : m_ctxt(ctxt), m_on_end(on_end) {
  }

  FrontendActionFactory(Context& ctxt)
    : m_ctxt(ctxt), m_on_end([](Context&) {}) {
  }

  virtual clang::FrontendAction* create() override {
    return new Action(m_ctxt, m_on_end);
  }
};

}

int main(int argc, const char** argv) {
  // CommonOptionsParser constructor will parse arguments and create a
  // CompilationDatabase.  In case of error it will terminate the program.
  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, OptionsCategory);
  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
          OptionsParser.getSourcePathList());

  // Hook called whenever the tool as finished running
  auto on_end = [](clong::Context& ctxt) {
    // Pretty print current parsed nodes
    std::cout << clong::PrettyPrinter::pprint(&ctxt.root());

    // Output to jekyll format
    clong::jekyll::JustTheDocs::write(OutputDir, ctxt);
  };

  // Run the tool
  clong::Context ctxt;
  return Tool.run(std::make_unique<clong::FrontendActionFactory>(ctxt, on_end).get());
}
