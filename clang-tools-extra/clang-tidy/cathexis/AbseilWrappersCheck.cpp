//===--- AbseilWrappersCheck.cpp - clang-tidy
//----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AbseilWrappersCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void AbseilWrappersCheck::registerMatchers(MatchFinder *Finder) {

  /*
  auto matcher =
  callExpr(callee(functionDecl(hasAnyName("absl::LocalTimeZone",
    "absl::LoadTimeZone"))
                            .bind("func")))
            .bind("expr");
    Finder->addMatcher(matcher, this);
    */
}

void AbseilWrappersCheck::check(const MatchFinder::MatchResult &Result) {

  /*
const auto *Call = Result.Nodes.getNodeAs<CallExpr>("expr");
const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");

if (Call == nullptr) {
return;
}

std::string name = Func->getNameInfo().getName().getAsString();

std::map<std::string, std::string> funcs = {
{"LocalTimeZone", "core::LocalTimeZone"},
{"LoadTimeZone", "core::LoadTimeZone"},
};

auto match = funcs.find(name);

if (match == funcs.end()) {
diag(Call->getExprLoc(), "%0 is wrapped (no known alternative for %1)")
  << Func << name;
} else {
diag(Call->getExprLoc(), "%0 is wrapped, use %1") << Func << match->second;
}
  */
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
