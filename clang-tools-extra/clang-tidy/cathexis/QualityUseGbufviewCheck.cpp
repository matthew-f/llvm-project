//===--- QualityUseGbufviewCheck.cpp - clang-tidy -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityUseGbufviewCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityUseGbufviewCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(functionDecl(matchesName("::[lL]oad[a-zA-Z0-9]*$"),
                                  hasAnyParameter(parmVarDecl().bind("parm")))
                         .bind("func"),
                     this);
}

void QualityUseGbufviewCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  const auto *parm = Result.Nodes.getNodeAs<ParmVarDecl>("parm");

  std::string s = parm->getType().getAsString();

  if (s.find("GSharedBuf") != std::string::npos ||
      s.find("GConstBuf") != std::string::npos ||
      s.find("GUniqueBuf") != std::string::npos) {
    diag(func->getLocation(), "function %0 should use 'GBufView', not %1")
        << func << parm->getType();
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
