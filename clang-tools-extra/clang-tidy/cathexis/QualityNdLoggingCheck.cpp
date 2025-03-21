//===--- QualityNdLoggingCheck.cpp - clang-tidy ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QualityNdLoggingCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void QualityNdLoggingCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxMemberCallExpr(
          unless(anyOf(isExpandedFromMacro("CDebugF"),
                       isExpandedFromMacro("CDebugf"),
                       isExpandedFromMacro("CDebug"))),
          on(hasType(cxxRecordDecl(hasName("core::V2Logger")))),
          callee(cxxMethodDecl(anyOf(hasName("log"),   //
                                     hasName("logF"),  //
                                     hasName("logCF"), //
                                     hasName("rlogF")))),
          // hasDescendant(declRefExpr(hasDeclaration(enumConstantDecl(
          //     anyOf(hasName("INFO"), hasName("WARN"), hasName("CRIT")))))),
          hasAncestor(cxxMethodDecl(
              ofClass(isDerivedFrom(hasName("ndif::Component"))))))
          .bind("call"),
      this);
}

void QualityNdLoggingCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");
  if (call != nullptr) {
    diag(call->getBeginLoc(),
         "Use ndif::Component::logDebug/log/logNorm/logInfo/logWarn");
  }
}

} // namespace clang::tidy::cathexis
