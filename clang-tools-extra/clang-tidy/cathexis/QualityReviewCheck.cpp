//===--- QualityReviewCheck.cpp - clang-tidy ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QualityReviewCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

//----------------------------------------------------------------------//
void QualityReviewCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      callExpr(callee(cxxMethodDecl(hasAnyName("GConstBuf::makeReferenced",
                                               "GSharedBuf::makeReferenced"))
                          .bind("func")))
          .bind("expr"),
      this);
}

//----------------------------------------------------------------------//
void QualityReviewCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("expr");
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");

  if (Call != nullptr) {
    std::string name = Func->getNameInfo().getName().getAsString();
    diag(Call->getExprLoc(), "review use of %0") << Func;
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
