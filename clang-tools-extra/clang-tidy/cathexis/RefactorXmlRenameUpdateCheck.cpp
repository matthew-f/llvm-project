//===--- RefactorXmlRenameUpdateCheck.cpp - clang-tidy --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RefactorXmlRenameUpdateCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void RefactorXmlRenameUpdateCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxMemberCallExpr(on(hasType(cxxRecordDecl(
                                           hasName("NS_XML::SimpleNode")))),
                                       callee(cxxMethodDecl(hasName("update"))))
                         .bind("call"),
                     this);
}

void RefactorXmlRenameUpdateCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");

  clang::SourceRange source_range(call->getExprLoc(),
                                  call->getExprLoc().getLocWithOffset(5));

  std::string replacement = "findOrAdd";

  diag(call->getExprLoc(), "replace with 'findOrAdd'")
      << FixItHint::CreateReplacement(source_range, replacement);
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
