//===--- PackedStructMissingSizeCheckCheck.cpp - clang-tidy
//---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PackedStructMissingSizeCheckCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void PackedStructMissingSizeCheckCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxRecordDecl(isDefinition(),                          //
                    hasAttr(clang::attr::MaxFieldAlignment), //
                    unless(allOf(isUnion(), hasName("(anonymous)"))),
                    unless(hasDescendant(cxxMethodDecl(
                        anyOf(hasName("internalCheckSizeEquals"),
                              hasName("internalCheckSizeLessOrEqual"))))))
          .bind("decl"),
      this);
}

void PackedStructMissingSizeCheckCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
  diag(decl->getLocation(),
       "'%0' is packed but does not check size (CHECK_THS_SIZE)")
      << decl->getName();
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
