
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "NoNewCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void NoNewCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxNewExpr().bind("new"), this);
}

void NoNewCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *newExpr = Result.Nodes.getNodeAs<CXXNewExpr>("new");
  if (newExpr == nullptr) {
    return;
  }

  if (newExpr->getNumPlacementArgs() > 0) {
    // Placement new, user probably knows what he's doing
    return;
  }

  auto qualType = newExpr->getAllocatedType();
  auto location = newExpr->getBeginLoc();

  const auto *cxxRecordDecl = qualType.getTypePtr()->getAsCXXRecordDecl();
  if (cxxRecordDecl != nullptr) {

    IdentifierInfo &ii = Result.Context->Idents.get("QObject");
    DeclarationName base_name(&ii);

    auto lookup = Result.Context->getTranslationUnitDecl()->lookup(base_name);

    const CXXRecordDecl *qobject_decl = nullptr;
    for (NamedDecl *nd : lookup) {
      if (const auto *rd = dyn_cast<CXXRecordDecl>(nd)) {
        qobject_decl = rd->getDefinition();
        break;
      }
    }

    if (qobject_decl != nullptr && cxxRecordDecl->isDerivedFrom(qobject_decl)) {
      return;
    }
  }

  diag(location, "Avoid 'new' of '%0'") << qualType.getAsString();
}

} // namespace clang::tidy::cathexis
