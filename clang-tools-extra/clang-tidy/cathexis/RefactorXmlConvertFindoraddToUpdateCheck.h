//===--- RefactorXmlConvertFindoraddToUpdateCheck.h - clang-tidy *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_REFACTORXMLCONVERTFINDORADDTOUPDATECHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_REFACTORXMLCONVERTFINDORADDTOUPDATECHECK_H

#include "../ClangTidyCheck.h"

namespace clang {
namespace tidy {
namespace cathexis {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/cathexis-refactor-xml-convert-findoradd-to-update.html
class RefactorXmlConvertFindoraddToUpdateCheck : public ClangTidyCheck {
public:
  RefactorXmlConvertFindoraddToUpdateCheck(StringRef Name,
                                           ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace cathexis
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_REFACTORXMLCONVERTFINDORADDTOUPDATECHECK_H
