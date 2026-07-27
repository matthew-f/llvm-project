//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FilesystemPathCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void FilesystemPathCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(cxxMethodDecl(hasName("string"), ofClass(hasName("std::filesystem::path")))))
          .bind("call"),
      this);
}

void FilesystemPathCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CallExpr>("call");

  diag(call->getBeginLoc(), "Avoid std::filesystem::path::string");
}

} // namespace clang::tidy::cathexis
