//===--- QualityNoNakedNewCheck.cpp - clang-tidy --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityNoNakedNewCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityNoNakedNewCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxNewExpr(
          unless(hasDescendant(cxxConstructExpr(hasDeclaration(
              cxxConstructorDecl(ofClass(isSameOrDerivedFrom("QObject"))))))))
          .bind("expr"),
      this);
}

void QualityNoNakedNewCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *expr = Result.Nodes.getNodeAs<CXXNewExpr>("expr");

  diag(expr->getBeginLoc(), "no naked new");
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
