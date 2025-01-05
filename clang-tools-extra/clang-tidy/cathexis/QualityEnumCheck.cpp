//===--- QualityEnumCheck.cpp - clang-tidy --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityEnumCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityEnumCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(enumDecl().bind("enum"), this);
}

void QualityEnumCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *decl = Result.Nodes.getNodeAs<EnumDecl>("enum");

  if (decl->isScopedUsingClassTag()) {
    // fine
    return;
  }

  diag(decl->getLocation(), "prefer class enum for %0") << decl;
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
