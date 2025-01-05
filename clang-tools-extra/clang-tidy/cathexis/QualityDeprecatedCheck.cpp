//===--- QualityDeprecatedCheck.cpp - clang-tidy --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityDeprecatedCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityDeprecatedCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(callExpr(hasDescendant(implicitCastExpr(
                         hasDescendant(declRefExpr().bind("mem"))))),
                     this);

  Finder->addMatcher(
      declaratorDecl(hasType(cxxRecordDecl(matchesName(".*::deprecated::"))))
          .bind("decl"),
      this);

  Finder->addMatcher(
      callExpr(
          callee(
              functionDecl(matchesName(".*::deprecated::")).bind("func_decl")))
          .bind("func"),
      this);
}

void QualityDeprecatedCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *Mem = Result.Nodes.getNodeAs<DeclRefExpr>("mem");

  if (Mem) {

    std::string name = Mem->getNameInfo().getName().getAsString();
    if (name == "memset" || name == "memcmp" || name == "memcpy" ||
        name == "memmove") {

      diag(Mem->getExprLoc(), "deprecated, use of '%0'") << name;
    }
  }

  const auto *decl = Result.Nodes.getNodeAs<DeclaratorDecl>("decl");
  if (decl) {
    const auto *cxx = decl->getType().getTypePtr()->getAsCXXRecordDecl();

    std::string type_name;
    if (cxx != nullptr) {
      type_name = cxx->getQualifiedNameAsString();
    } else {
      type_name = "?";
    }

    diag(decl->getLocation(), "use of deprecated type '%0' for variable '%1'")
        << type_name << decl->getName();
  }

  const auto *func = Result.Nodes.getNodeAs<CallExpr>("func");
  if (func != nullptr) {
    const auto *func_decl = Result.Nodes.getNodeAs<FunctionDecl>("func_decl");
    diag(func->getBeginLoc(), "use of deprecated function '%0'") << func_decl;
  }
}
} // namespace cathexis
} // namespace tidy
} // namespace clang
