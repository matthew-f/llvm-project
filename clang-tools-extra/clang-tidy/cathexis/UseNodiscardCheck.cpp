//===--- UseNodiscardCheck.cpp - clang-tidy--------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UseNodiscardCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void UseNodiscardCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      functionDecl(unless(anyOf(hasAttr(clang::attr::WarnUnusedResult),
                                cxxMethodDecl())),
                   returns(asString("_Bool")),
                   anyOf(matchesName("::[lL]oad[a-zA-Z0-9]*$"),
                         matchesName("::[iI]nit[a-zA-Z0-9]*$")))
          .bind("func"),
      this);

  Finder->addMatcher(
      functionDecl(cxxMethodDecl(unless(hasAttr(clang::attr::WarnUnusedResult)),
                                 returns(asString("_Bool")), //
                                 isPublic(),
                                 anyOf(matchesName("::[lL]oad[a-zA-Z0-9]*$"),
                                       matchesName("::[iI]nit[a-zA-Z0-9]*$"),
                                       matchesName("::[pP]ush[a-zA-Z0-9]*$"), //
                                       matchesName("::[pP]op[a-zA-Z0-9]*$"),
                                       matchesName("::[sS]eek[a-zA-Z0-9]*$"))))
          .bind("method"),
      this);
}

void UseNodiscardCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (func != nullptr) {

    func = func->getCanonicalDecl();
    auto location = func->getReturnTypeSourceRange().getBegin();

    diag(location, "function '%0' should use NO_DISCARD")
        << func->getQualifiedNameAsString()
        << FixItHint::CreateInsertion(location, "NO_DISCARD ");
  }

  const auto *method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (method != nullptr && !method->isConst() && !method->isStatic()) {

    method = method->getCanonicalDecl();
    auto location = method->getReturnTypeSourceRange().getBegin();

    diag(location, "method '%0' should use NO_DISCARD")
        << method->getQualifiedNameAsString()
        << FixItHint::CreateInsertion(location, "NO_DISCARD ");
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
