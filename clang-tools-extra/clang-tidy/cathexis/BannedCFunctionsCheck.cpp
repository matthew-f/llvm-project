//===--- BannedCFunctionsCheck.cpp - clang-tidy
//----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "BannedCFunctionsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void BannedCFunctionsCheck::registerMatchers(MatchFinder *Finder) {
  auto matcher =
      callExpr(
          callee(functionDecl(hasAnyName("::asctime", "::ctime", "::gmtime",
                                         "::localtime", "std::stoi",
                                         "std::stol", "std::stoll"))
                     .bind("func")))
          .bind("expr");
  Finder->addMatcher(matcher, this);
}

void BannedCFunctionsCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("expr");
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");

  if (Call == nullptr) {
    return;
  }

  // std::string name = Func->getNameInfo().getQualifiedName().getAsString();
  std::string name = Func->getQualifiedNameAsString();

  std::map<std::string, std::string> funcs = {
      {"asctime", "core::AsCTime"},     //
      {"ctime", "core::CTime"},         //
      {"gmtime", "core::GmTime"},       //
      {"localtime", "core::LocalTime"}, //
      {"std::stoi", "core::Numbers"},   //
      {"std::stol", "core::Numbers"},   //
      {"std::stoll", "core::Numbers"},
  };

  auto match = funcs.find(name);

  if (match == funcs.end()) {
    diag(Call->getExprLoc(), "%0 is banned (no known alternative)") << name;
  } else {
    diag(Call->getExprLoc(), "%0 is banned, use %1") << name << match->second;
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
