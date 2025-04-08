//===--- UseDefaultOperatorsCheck.cpp - clang-tidy -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UseDefaultOperatorsCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void UseDefaultOperatorsCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(
          isUserProvided(),            //
          anyOf(hasName("operator=="), //
                hasName("operator<"),  //
                hasName("operator<="), //
                hasName("operator>"),  //
                hasName("operator>=")),
          ofClass(cxxRecordDecl().bind("class")),
          hasParameter(0, parmVarDecl(hasType(references(
                              cxxRecordDecl(equalsBoundNode("class")))))))
          .bind("decl"),
      this);
}

void UseDefaultOperatorsCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *decl = Result.Nodes.getNodeAs<CXXMethodDecl>("decl");
  if (decl == nullptr) {
    return;
  }

  decl = decl->getCanonicalDecl();
  diag(decl->getLocation(), "Use default for %0")
      << decl->getQualifiedNameAsString();
}

} // namespace clang::tidy::cathexis
