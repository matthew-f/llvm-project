//===--- WrapperCheck.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "WrapperCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void WrapperCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      callExpr(callee(functionDecl(hasAnyName("::open", "::stat", "::fopen",
                                              "::unlink", "::rename", "::rmdir",
                                              "::chmod"))
                          .bind("func")))
          .bind("expr"),
      this);
}

void WrapperCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("expr");
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");

  if (Call) {
    diag(Call->getExprLoc(), "%0 is wrapped, use W%1")
        << Func << Func->getNameInfo().getName().getAsString();
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
