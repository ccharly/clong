#ifndef CLONG_VISITOR_HPP
#define CLONG_VISITOR_HPP

#include <clong/config.hpp>
#include <clong/Node.hpp>
#include <clong/PrettyPrinter.hpp>
#include <clong/Context.hpp>
#include <clang/AST/RecursiveASTVisitor.h>

namespace clong {

/// The clang's visitor that visits each nodes
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
    m_ctxt.register_node(decl, m_ctxt.functions());
    return true;
  }

  bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl* decl) {
    m_ctxt.register_node(decl, m_ctxt.functions());
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


}

#endif
