//===--- QtTrWithoutQobjectCheck.cpp - clang-tidy -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QtTrWithoutQobjectCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void QtTrWithoutQobjectCheck::registerMatchers(MatchFinder *Finder) {

  // If the tr function is from QObject, then the Q_OBJECT macro has not been
  // used
  Finder->addMatcher(
      declRefExpr(allOf(hasDeclaration(cxxMethodDecl(
                            allOf(ofClass(hasName("QObject")), hasName("tr")))),
                        hasAncestor(cxxMethodDecl().bind("method"))))
          .bind("declref"),
      this);
}

void QtTrWithoutQobjectCheck::check(const MatchFinder::MatchResult &Result) {

  const auto DeclRef = Result.Nodes.getNodeAs<DeclRefExpr>("declref");
  const auto Method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");

  if (DeclRef && Method) {
    diag(DeclRef->getExprLoc(),
         "tr() used in class '%0' without Q_OBJECT macro")
        << Method->getParent()->getQualifiedNameAsString();
  }
}

} // namespace clang::tidy::cathexis
