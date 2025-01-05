//===--- BugproneBufDataCheck.cpp - clang-tidy-----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "BugproneBufDataCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void BugproneBufDataCheck::registerMatchers(MatchFinder *Finder) {
  // clang-format off
  // |   `-CXXStaticCastExpr 0x83f6a98 <col:11, col:46> 'const char *' static_cast<const char *> <BitCast>
	// |     `-CXXMemberCallExpr 0x83f6a40 <col:36, col:45> 'const void *'
	// |       `-MemberExpr 0x83f6a08 <col:36, col:40> '<bound member function type>' .data 0x8359f90
	// |         `-ImplicitCastExpr 0x83f6a68 <col:36> 'const GSharedBuf' lvalue <NoOp>
	// |           `-DeclRefExpr 0x83f69e0 <col:36> 'GSharedBuf' lvalue Var 0x83f68c8 'buf' 'GSharedBuf'
  // clang-format on

  Finder->addMatcher(
      cxxStaticCastExpr(has(cxxMemberCallExpr(
                            on(hasType(cxxRecordDecl(anyOf(
                                hasName("GUniqueBuf"), hasName("GSharedBuf"),
                                hasName("GConstBuf"), hasName("GBufView"))))),
                            callee(cxxMethodDecl(anyOf(
                                hasName("data"), hasName("dataAt"),
                                hasName("workPtr"), hasName("workPtrAt")))))))
          .bind("cast"),
      this);
}

void BugproneBufDataCheck::check(const MatchFinder::MatchResult &Result) {

  const auto Cast = Result.Nodes.getNodeAs<CXXStaticCastExpr>("cast");

  if (Cast) {
    diag(Cast->getOperatorLoc(), "use core::buf::Cast");
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
