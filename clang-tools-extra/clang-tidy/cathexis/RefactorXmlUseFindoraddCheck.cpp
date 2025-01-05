//===--- RefactorXmlUseFindoraddCheck.cpp - clang-tidy
//---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RefactorXmlUseFindoraddCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void RefactorXmlUseFindoraddCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxOperatorCallExpr(
          allOf(hasType(cxxRecordDecl(hasName("NS_XML::SimpleNode"))), //
                unless(hasType(isConstQualified())),                   //
                hasOverloadedOperatorName("[]"),
                hasDescendant(declRefExpr().bind("decl")))),
      this);
}

void RefactorXmlUseFindoraddCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *decl = Result.Nodes.getNodeAs<DeclRefExpr>("decl");

  clang::SourceRange replacement_range(decl->getBeginLoc().getLocWithOffset(1),
                                       decl->getEndLoc());
  LangOptions options;

  llvm::StringRef source =
      Lexer::getSourceText(clang::CharSourceRange(replacement_range, false),
                           *Result.SourceManager, options);

  std::string replacement =
      ".findOrAdd(" + std::string(source.data(), source.size()) + ")";

  diag(decl->getLocation(), "use NS_XML::SimpleNode::findOrAdd")
      << FixItHint::CreateReplacement(decl->getSourceRange(), replacement);
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
