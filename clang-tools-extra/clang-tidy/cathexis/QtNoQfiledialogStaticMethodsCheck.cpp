//===--- QtNoQfiledialogStaticMethodsCheck.cpp - clang-tidy --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QtNoQfiledialogStaticMethodsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void QtNoQfiledialogStaticMethodsCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(
      callExpr(callee(cxxMethodDecl(hasAnyName("getExistingDirectory",    //
                                               "getExistingDirectoryUrl", //
                                               "getOpenFileName",         //
                                               "getOpenFileNames",        //
                                               "getOpenFileUrl",          //
                                               "getOpenFileUrls",         //
                                               "getSaveFileName",         //
                                               "getSaveFileUrl"),
                                    ofClass(hasName("QFileDialog")))
                          .bind("method")))
          .bind("call"),
      this);
}

void QtNoQfiledialogStaticMethodsCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *call = Result.Nodes.getNodeAs<CallExpr>("call");
  const auto *method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");

  diag(call->getBeginLoc(),
       "Avoid QFileDialog::%0, use NS_QT::FileDialog::%1 instead")
      << method->getName() << method->getName();
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
