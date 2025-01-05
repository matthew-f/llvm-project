//===--- UseAnonymousDefCheck.cpp - clang-tidy-----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UseAnonymousDefCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

void UseAnonymousDefCheck::registerMatchers(MatchFinder *Finder) {

  Finder->addMatcher(cxxRecordDecl(unless(anyOf(hasAncestor(namespaceDecl()),
                                                hasAncestor(functionDecl()))))
                         .bind("decl"),
                     this);
}

void UseAnonymousDefCheck::check(const MatchFinder::MatchResult &Result) {

  const auto *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

  if (decl == nullptr) {
    return;
  }

  if (!decl->isInAnonymousNamespace() && !decl->isCXXClassMember()) {

    std::string filename = decl->getASTContext()
                               .getSourceManager()
                               .getFilename(decl->getLocation())
                               .str();

    if (filename.size() > 4 &&
        filename.substr(filename.size() - 4, 4) == ".cpp") {

      std::string qn = decl->getQualifiedNameAsString();
      // This check is to pickup:
      // namespace NS_TEST
      // {
      //    struct TestFunc;
      // }
      //
      // struct NS_TEST::TestFunc {};

      if (qn.find("::") == std::string::npos) {
        diag(decl->getLocation(), "'%0' should be in an anonymous namespace")
            << qn;
      }
    }
  }
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
