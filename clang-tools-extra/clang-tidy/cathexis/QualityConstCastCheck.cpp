//===--- QualityConstCastCheck.cpp - clang-tidy ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityConstCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityConstCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxConstCastExpr().bind("cast"), this);
}

void QualityConstCastCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *cast = Result.Nodes.getNodeAs<CXXConstCastExpr>("cast");
  diag(cast->getBeginLoc(), "check const_cast");
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
