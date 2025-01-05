//===--- RefactorXmlConvertFindoraddToUpdateCheck.cpp - clang-tidy --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RefactorXmlConvertFindoraddToUpdateCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

#include <iostream>

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void RefactorXmlConvertFindoraddToUpdateCheck::registerMatchers(
    MatchFinder *Finder) {

  Finder->addMatcher(
      cxxOperatorCallExpr(
          hasOverloadedOperatorName("="),
          has(cxxMemberCallExpr(
                  callee(cxxMethodDecl(hasName("findOrAdd"))),
                  on(hasType(cxxRecordDecl(hasName("NS_XML::SimpleNode")))))
                  .bind("find")))

          .bind("call"),
      this);
}

void RefactorXmlConvertFindoraddToUpdateCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("call");

  clang::SourceRange expr_range(call->getBeginLoc(), call->getEndLoc());

  llvm::StringRef expr = Lexer::getSourceText(
      clang::CharSourceRange(expr_range, false), *Result.SourceManager, {});

  auto arg = call->getArg(1);
  if (arg == nullptr) {
    // assert somehow
    return;
  }

  clang::CharSourceRange arg_source_range(
      {arg->getBeginLoc(), arg->getEndLoc()}, true);

  // diag(call->getOperatorLoc(), "operator is here");

  clang::CharSourceRange remove_range(
      {call->getOperatorLoc().getLocWithOffset(-2), arg->getEndLoc()}, true);

  llvm::StringRef arg_sref = Lexer::getSourceText(
      clang::CharSourceRange({arg->getBeginLoc(), arg->getEndLoc()}, true),
      *Result.SourceManager, {});

  std::string replacement_str = std::string(", ") + arg_sref.str() + ")";

  const auto *find = Result.Nodes.getNodeAs<CXXMemberCallExpr>("find");
  clang::CharSourceRange find_range(
      {find->getExprLoc(), find->getExprLoc().getLocWithOffset(9)}, false);

  // diag(find->getExprLoc(), "findOrAdd expression is here");

  // {
  //   llvm::StringRef ref =
  //       Lexer::getSourceText(find_range, *Result.SourceManager, {});

  //   std::cout << " find |" << ref.str() << "|\n";
  // }

  diag(call->getBeginLoc(), "check call=|%0|")
      << expr //
      << FixItHint::CreateReplacement(find_range, "update")
      << FixItHint::CreateReplacement(remove_range, replacement_str);
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
