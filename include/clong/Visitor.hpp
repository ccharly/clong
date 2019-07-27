#ifndef CLONG_VISITOR_HPP
#define CLONG_VISITOR_HPP

#include <clong/config.hpp>
#include <clong/clang.hpp>
#include <clong/Node.hpp>
#include <clong/PrettyPrinter.hpp>
#include <clong/Context.hpp>

namespace clong {

/// The clang's visitor that visits each nodes
class Visitor : public clang::RecursiveASTVisitor<Visitor> {

  clang::CompilerInstance& m_ci;
  Context& m_ctxt;

  public:
  Visitor(clang::CompilerInstance& ci, Context& ctxt)
    : m_ci(ci), m_ctxt(ctxt) {
  }

  public:
  bool VisitNamespaceDecl(clang::NamespaceDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitClassTemplateDecl(clang::ClassTemplateDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_node(decl);
    m_ctxt.mark_as_visited(decl->getTemplatedDecl());
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_function_node(decl);
    return true;
  }

  bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_function_node(decl);
    m_ctxt.mark_as_visited(decl->getTemplatedDecl());
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_node(decl);
    return true;
  }

  bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
    CLONG_LOG(debug, log::colored(decl));
    m_ctxt.register_node(decl);
    return true;
  }
};


}

#endif
