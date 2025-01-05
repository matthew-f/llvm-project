//===--- QualityXmlSuspiciousFindoraddCheck.cpp - clang-tidy
//----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QualityXmlSuspiciousFindoraddCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QualityXmlSuspiciousFindoraddCheck::registerMatchers(MatchFinder *Finder) {

  // This is to pick usage where NS_XML::SimpleNode::update and operator[] is
  // probably the correct choice

  Finder->addMatcher(
      cxxMemberCallExpr(
          on(hasType(cxxRecordDecl(hasName("NS_XML::XNode")))),
          callee(cxxMethodDecl(hasName("findOrAdd"))),
          anyOf(hasAncestor(declStmt(has(varDecl().bind("var")))),

                hasAncestor(cxxOperatorCallExpr().bind("parent_call")),
                hasAncestor(binaryOperator(hasOperatorName("="),
                                           unless(hasLHS(hasType(cxxRecordDecl(
                                               hasName("NS_XML::XNode")))))))))
          .bind("call"),
      this);
}

void QualityXmlSuspiciousFindoraddCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");

  const auto *var = Result.Nodes.getNodeAs<VarDecl>("var");
  if (var != nullptr) {
    std::string name = var->getType().getAsString();
    if (name.find("NS_XML::XNode") != std::string::npos) {
      // This is assigning to a SimpleNode. Don't warn on this
      return;
    }

    diag(call->getBeginLoc(), "check this (varDecl-%0) (%1)")
        << var->getName() << name;
    return;
  }

  const auto *parent_call =
      Result.Nodes.getNodeAs<CXXOperatorCallExpr>("parent_call");
  if (parent_call != nullptr) {
    std::string name = parent_call->getType().getAsString();

    if (name.find("NS_XML::XNode") != std::string::npos) {
      // This is assigning to a XNode. Don't warn on this
      return;
    }

    diag(call->getBeginLoc(), "check this (CXXOperatorCallExpr) (%0)") << name;
    return;
  }

  diag(call->getBeginLoc(), "check this (=operator)");
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
