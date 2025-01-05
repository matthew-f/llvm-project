//===--- ProtectedClassMemberCheck.cpp - clang-tidy------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ProtectedClassMemberCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void ProtectedClassMemberCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(fieldDecl(isProtected()).bind("field"), this);
}

void ProtectedClassMemberCheck::check(const MatchFinder::MatchResult &Result) {

  const auto Field = Result.Nodes.getNodeAs<FieldDecl>("field");

  if (Field) {
    diag(Field->getLocation(), "Do not use protected member variables");
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
