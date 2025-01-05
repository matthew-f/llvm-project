//===--- QualityEnumToStringCheck.cpp -
// clang-tidy---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityEnumToStringCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

QualityEnumToStringCheck::QualityEnumToStringCheck(StringRef Name,
                                                   ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {}

void QualityEnumToStringCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      functionDecl(
          allOf(isDefinition(),      //
                parameterCountIs(1), //
                hasParameter(0, hasType(enumDecl())),
                hasReturnTypeLoc(loc(asString("std::string"))),
                unless(anyOf(hasName("ToString"), hasName("ToEnumStrDump")))))
          .bind("func"),
      this);
}

void QualityEnumToStringCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *cxx_method = Result.Nodes.getNodeAs<CXXMethodDecl>("func");
  if (cxx_method != nullptr && !cxx_method->isStatic()) {
    return;
  }

  const auto Func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (Func == nullptr) {
    return;
  }

  std::string name = Func->getNameInfo().getName().getAsString();

  diag(Func->getNameInfo().getLoc(),
       "enum-to-string function '%0' should be called 'ToString'")
      << name;
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
