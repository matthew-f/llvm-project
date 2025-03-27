//===--- UseDefaultEqualsOperatorCheck.cpp - clang-tidy -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UseDefaultEqualsOperatorCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void UseDefaultEqualsOperatorCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(
          isUserProvided(),      //
          hasName("operator=="), //
          ofClass(cxxRecordDecl().bind("class")),
          hasParameter(0, parmVarDecl(hasType(references(
                              cxxRecordDecl(equalsBoundNode("class")))))))
          .bind("decl"),
      this);
}

void UseDefaultEqualsOperatorCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *decl = Result.Nodes.getNodeAs<CXXMethodDecl>("decl");
  if (decl == nullptr) {
    return;
  }

  diag(decl->getLocation(), "Use default for %0") << decl->getName();
}

} // namespace clang::tidy::cathexis
