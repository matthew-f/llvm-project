//===--- QtNolatin1Check.cpp - clang-tidy----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QtNolatin1Check.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QtNolatin1Check::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxMemberCallExpr(callee(cxxMethodDecl(allOf(hasName("toLatin1"),
                                                   ofClass(hasName("QString"))))
                                   .bind("latin1")))
          .bind("latin1_expr"),
      this);
}

void QtNolatin1Check::check(const MatchFinder::MatchResult &Result) {

  const auto *Latin1 = Result.Nodes.getNodeAs<CXXMethodDecl>("latin1");
  const auto *Latin1Expr =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("latin1_expr");

  if (Latin1) {

    diag(Latin1Expr->getExprLoc(), "deprecated, use of toLatin1");
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
