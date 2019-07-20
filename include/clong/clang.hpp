#ifndef CLONG_CLANG_HPP
#define CLONG_CLANG_HPP

#include <clong/config.hpp>

// Make sure to disable warnings with MSVC locally
#if CLONG_IS_MSVC
#pragma warning(push)
#pragma warning(disable : 4141 4146 4244 4267 4291)
#endif

#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Comment.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/CommandLine.h>

#if CLONG_IS_MSVC
#pragma warning(pop)
#endif

#endif
