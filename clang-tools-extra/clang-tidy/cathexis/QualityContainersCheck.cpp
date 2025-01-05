//===--- QualityContainersCheck.cpp - clang-tidy --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QualityContainersCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityContainersCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      declaratorDecl(hasType(cxxRecordDecl(
                         anyOf(hasName("std::list"), hasName("std::map")))))
          .bind("decl"),
      this);
}

void QualityContainersCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *decl = Result.Nodes.getNodeAs<DeclaratorDecl>("decl");
  if (decl != nullptr) {

    const auto *cxx = decl->getType().getTypePtr()->getAsCXXRecordDecl();

    std::string type_name;
    if (cxx != nullptr) {
      type_name = cxx->getQualifiedNameAsString();
    } else {
      type_name = "?";
    }

    diag(decl->getLocation(), "check use of '%0' for variable '%1'")
        << type_name << decl->getName();
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
