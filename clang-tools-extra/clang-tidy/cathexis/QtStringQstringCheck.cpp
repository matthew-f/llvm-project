//===--- QtStringQstringCheck.cpp - clang-tidy ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QtStringQstringCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void QtStringQstringCheck::registerMatchers(MatchFinder *Finder) {

  auto matcher =
      cxxConstructExpr(
          allOf(argumentCountIs(1),
                hasDeclaration(namedDecl(hasName("QString")).bind("decl")),
                hasArgument(
                    0, callExpr(callee(cxxMethodDecl(
                                    hasName("c_str"),
                                    ofClass(hasName("std::basic_string")))))
                           .bind("call"))))
          .bind("construct");

  Finder->addMatcher(matcher, this);
}

void QtStringQstringCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CallExpr>("call");
  const auto *construct = Result.Nodes.getNodeAs<CXXConstructExpr>("construct");
  const auto *decl = Result.Nodes.getNodeAs<NamedDecl>("decl");

  if (call != nullptr && construct != nullptr && decl != nullptr) {
    diag(construct->getExprLoc(),
         "std::string::c_str() used to construct QString, "
         "prefer QString::fromStdString")
        << FixItHint::CreateInsertion(construct->getBeginLoc(),
                                      "QString::fromStdString(")
        << FixItHint::CreateReplacement(
               SourceRange(call->getExprLoc().getLocWithOffset(-1),
                           call->getExprLoc().getLocWithOffset(6)),
               ")");
  }
}

} // namespace clang::tidy::cathexis
