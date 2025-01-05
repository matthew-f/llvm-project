//===--- QualityQtAvoidQdialogExecCheck.cpp - clang-tidy
//---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityQtAvoidQdialogExecCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityQtAvoidQdialogExecCheck::registerMatchers(MatchFinder *Finder) {

  // This could be extended to check for '.exec()' as opposed to '->exec()'
  // Since for now it is just a quality check this will suffice

  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(cxxMethodDecl(hasName("exec"), ofClass(hasName("QDialog")))))
          .bind("call"),
      this);
}

void QualityQtAvoidQdialogExecCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CallExpr>("call");

  diag(call->getBeginLoc(), "Avoid QDialog::exec with QDialogs created on the "
                            "stack. Use NS_QT::unique_qptr::execDialog "
                            "instead");
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
