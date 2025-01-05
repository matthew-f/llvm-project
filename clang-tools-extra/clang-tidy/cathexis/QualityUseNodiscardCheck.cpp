//===--- QualityUseNodiscardCheck.cpp - clang-tidy ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QualityUseNodiscardCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void QualityUseNodiscardCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      functionDecl(unless(anyOf(hasAttr(clang::attr::WarnUnusedResult),
                                cxxMethodDecl(), //
                                matchesName(".*::operator"))),
                   returns(asString("_Bool")))
          .bind("func"),
      this);

  Finder->addMatcher(
      functionDecl(cxxMethodDecl(unless(hasAttr(clang::attr::WarnUnusedResult)),
                                 returns(asString("_Bool")), //
                                 isPublic()))
          .bind("method"),
      this);
}

void QualityUseNodiscardCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (func != nullptr) {

    func = func->getCanonicalDecl();
    auto location = func->getReturnTypeSourceRange().getBegin();

    diag(location, "check if function '%0' should use NO_DISCARD ")
        << func->getQualifiedNameAsString();
  }

  const auto *method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (method != nullptr && !method->isConst() && !method->isStatic()) {

    method = method->getCanonicalDecl();
    auto location = method->getReturnTypeSourceRange().getBegin();

    diag(location, "check if method '%0' should use NO_DISCARD")
        << method->getQualifiedNameAsString();
  }
}

} // namespace clang::tidy::cathexis
