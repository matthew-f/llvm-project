//===--- QualityLongLambdaCheck.cpp - clang-tidy
//---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QualityLongLambdaCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void QualityLongLambdaCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      lambdaExpr(hasAncestor(decl().bind("decl"))).bind("lambda"), this);
}

void QualityLongLambdaCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *decl = Result.Nodes.getNodeAs<Decl>("decl");
  if (decl == nullptr) {
    return;
  }

  const auto *lambda = Result.Nodes.getNodeAs<LambdaExpr>("lambda");
  if (lambda == nullptr) {
    return;
  }

  auto &sm = decl->getASTContext().getSourceManager();

  unsigned start_line = sm.getSpellingLineNumber(lambda->getBeginLoc());
  unsigned end_line = sm.getSpellingLineNumber(lambda->getEndLoc());
  unsigned num_lines = end_line - start_line + 1;

  if (num_lines > 7) {
    diag(lambda->getBeginLoc(), "Long lambda from %0 to %1 (%2 lines)")
        << start_line << end_line << num_lines;
  }
}
} // namespace clang::tidy::cathexis
