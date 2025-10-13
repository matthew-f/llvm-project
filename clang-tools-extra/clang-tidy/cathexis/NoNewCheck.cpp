
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

    if (isDerivedFrom(Result, cxxRecordDecl, {"QObject"}) ||
				isDerivedFrom(Result, cxxRecordDecl, {"QLayoutItem"}) ||
				isDerivedFrom(Result, cxxRecordDecl, {"QListWidgetItem"}) ||
				isDerivedFrom(Result, cxxRecordDecl, {"QStandardItem"}) ||
				isDerivedFrom(Result, cxxRecordDecl, {"QTreeWidgetItem"}) ||
				isDerivedFrom(Result, cxxRecordDecl, {"QTableWidgetItem"}) ||
        isDerivedFrom(Result, cxxRecordDecl, {"qt3", "Q3ListViewItem"})) {
      return;
    }
  }

  diag(location, "Avoid 'new' of '%0'") << qualType.getAsString();
}

bool NoNewCheck::isDerivedFrom(const MatchFinder::MatchResult &Result,
                               const CXXRecordDecl *decl,
                               const std::vector<std::string> &parts) const {

  // This walks the AST through the namespaces (!!)
  DeclContext *context = Result.Context->getTranslationUnitDecl();
  for (size_t i = 0; i < parts.size(); ++i) {
    auto res =
        context->lookup(DeclarationName(&Result.Context->Idents.get(parts[i])));
    if (res.empty()) {
      return false;
    }

    context = dyn_cast<DeclContext>(res.front());
    if (context == nullptr) {
      return false;
    }
  }

  const auto *cxxRecordDecl = dyn_cast<CXXRecordDecl>(context);
  if (cxxRecordDecl == nullptr) {
    return false;
  }

  const CXXRecordDecl *baseDecl = cxxRecordDecl->getDefinition();

  return baseDecl != nullptr &&
         (decl == baseDecl || decl->isDerivedFrom(baseDecl));
}

} // namespace clang::tidy::cathexis
