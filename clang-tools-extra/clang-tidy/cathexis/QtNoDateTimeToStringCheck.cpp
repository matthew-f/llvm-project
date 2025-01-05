//===--- QtNoDateTimeToStringCheck.cpp -
// clang-tidy----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QtNoDateTimeToStringCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QtNoDateTimeToStringCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(cxxMethodDecl(
                     allOf(hasName("toString"),
                           ofClass(hasAnyName("QDateTime", "QDate", "QTime"))))
                     .bind("dtToString")))
          .bind("dtToString_expr"),
      this);
}

void QtNoDateTimeToStringCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *match = Result.Nodes.getNodeAs<CXXMethodDecl>("dtToString");
  const auto *expr =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("dtToString_expr");

  if (match) {

    diag(expr->getExprLoc(),
         "%0() uses system locale, and will use the C-locale in Qt6. "
         "Use QLocale().toString(..) for correctly translated string")
        << match->getQualifiedNameAsString();
    return;
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
