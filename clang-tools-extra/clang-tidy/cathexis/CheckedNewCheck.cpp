//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CheckedNewCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void CheckedNewCheck::registerMatchers(MatchFinder *Finder) {
  // Matches variables initialized with `new`
  auto varWithNew = varDecl(hasInitializer(cxxNewExpr())).bind("varWithNew");

  // Case 1: `if (p)`
  auto ifVar = ifStmt(hasCondition(ignoringParenImpCasts(
                          declRefExpr(to(varWithNew)).bind("varUse"))))
                   .bind("ifWithVarNew");

  // Case 2: `if (!p)`
  auto ifNotVar = ifStmt(hasCondition(unaryOperator(
                             hasOperatorName("!"),
                             hasUnaryOperand(ignoringParenImpCasts(
                                 declRefExpr(to(varWithNew)).bind("varUse"))))))
                      .bind("ifWithVarNew");

  // Case 3: `if (p == nullptr)` or `if (nullptr == p)` or if `(p != nullptr)` or `if (nullptr != p)`
  auto ifCompareNull =
      ifStmt(
          hasCondition(binaryOperator(
              anyOf(hasOperatorName("=="), hasOperatorName("!=")),
              anyOf(
                  allOf(hasLHS(ignoringParenImpCasts(
                            declRefExpr(to(varWithNew)).bind("varUse"))),
                        hasRHS(ignoringParenImpCasts(cxxNullPtrLiteralExpr()))),
                  allOf(hasRHS(ignoringParenImpCasts(
                            declRefExpr(to(varWithNew)).bind("varUse"))),
                        hasLHS(
                            ignoringParenImpCasts(cxxNullPtrLiteralExpr())))))))
          .bind("ifWithVarNew");

  Finder->addMatcher(ifVar, this);
  Finder->addMatcher(ifNotVar, this);
  Finder->addMatcher(ifCompareNull, this);
}

void CheckedNewCheck::check(const MatchFinder::MatchResult &Result) {

  if (const auto *If = Result.Nodes.getNodeAs<IfStmt>("ifWithVarNew")) {
    if (const auto *Var = Result.Nodes.getNodeAs<VarDecl>("varWithNew")) {
      diag(If->getIfLoc(),
           "Variable '%0' initialized with 'new' is being checked for null")
          << Var->getName();
    }
  }
}

} // namespace clang::tidy::cathexis
