//===--- QualityLoggingCheck.cpp - clang-tidy -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityLoggingCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityLoggingCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxMemberCallExpr(
          on(hasType(cxxRecordDecl(hasName("core::V2Logger")))),
          callee(cxxMethodDecl(hasName("log"))),
          hasDescendant(declRefExpr(hasDeclaration(enumConstantDecl(
              anyOf(hasName("INFO"), hasName("WARN"), hasName("CRIT")))))))
          .bind("call"),
      this);
}

void QualityLoggingCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");
  if (call != nullptr) {
    diag(call->getBeginLoc(), "CLog(INFO/WARN/CRIT) called - use CLogF?");
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
