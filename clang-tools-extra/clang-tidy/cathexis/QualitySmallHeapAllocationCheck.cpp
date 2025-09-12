
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "QualitySmallHeapAllocationCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <iostream>
using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void QualitySmallHeapAllocationCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxNewExpr().bind("new"), this);
  Finder->addMatcher(callExpr(callee(functionDecl(hasAnyName(
                                  "::std::make_unique", "::make_shared" ,"::core::make_owned"))))
                         .bind("make"),
                     this);
}

void QualitySmallHeapAllocationCheck::check(
    const MatchFinder::MatchResult &Result) {

  QualType qualType;
  SourceLocation location;

  const auto *make = Result.Nodes.getNodeAs<CallExpr>("make");
  if (make != nullptr) {
    auto type = make->getCallReturnType(*Result.Context);

    const CXXRecordDecl *decl = type.getTypePtr()->getAsCXXRecordDecl();
    if (decl == nullptr) {
      return;
    }

    const auto *templateDecl = dyn_cast<ClassTemplateSpecializationDecl>(decl);
    if (templateDecl == nullptr) {
      return;
    }

    qualType = templateDecl->getTemplateArgs()[0].getAsType();
    location = make->getBeginLoc();
  }

  const auto *newExpr = Result.Nodes.getNodeAs<CXXNewExpr>("new");
  if (newExpr != nullptr) {

    if (newExpr->getNumPlacementArgs() > 0) {
      // Placement new, user probably knows what he's doing
      return;
    }

    if (newExpr->isArray()) {
      return;
    }

    qualType = newExpr->getAllocatedType();
    location = newExpr->getBeginLoc();
  }

  if (qualType.isNull()) {
    return;
  }

  const auto *cxxRecordDecl = qualType.getTypePtr()->getAsCXXRecordDecl();
  if (cxxRecordDecl != nullptr) {

    if (cxxRecordDecl->isPolymorphic()) {
      return;
    }

    if (!cxxRecordDecl->isTriviallyCopyConstructible()) {
      return;
    }

    if (!cxxRecordDecl->isTriviallyCopyable()) {
      return;
    }
  }

  if (qualType->getTypeClass() == Type::TypeClass::TemplateSpecialization) {
    return;
  }

  if (qualType->getTypeClass() == Type::TypeClass::TemplateTypeParm) {
    return;
  }

  const auto typeSize = Result.Context->getTypeSize(qualType) / 8;

  if (typeSize <= 16) {
    diag(location, "Small heap-allocation of '%0' size %1")
        << qualType.getAsString() << typeSize;
  }
}

} // namespace clang::tidy::cathexis
