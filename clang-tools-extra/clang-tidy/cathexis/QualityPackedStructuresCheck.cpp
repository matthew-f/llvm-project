//===--- QualityPackedStructuresCheck.cpp - clang-tidy --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityPackedStructuresCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityPackedStructuresCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      cxxRecordDecl(hasAttr(clang::attr::MaxFieldAlignment)).bind("decl"),
      this);
}

void QualityPackedStructuresCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
  // auto alignment_attr = decl->getAttr<MaxFieldAlignmentAttr>();
  diag(decl->getLocation(), "'%0' is packed") // with %1 byte alignment")
      << decl->getName(); // << alignment_attr->getAlignment();
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
