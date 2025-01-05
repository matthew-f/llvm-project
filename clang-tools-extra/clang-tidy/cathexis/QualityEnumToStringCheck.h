//===--- QualityEnumToStringCheck.h - clang-tidy-------------------*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_ENUMTOSTRINGCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_ENUMTOSTRINGCHECK_H

#include "../ClangTidyCheck.h"
#include "../utils/OptionsUtils.h"

namespace clang {
namespace tidy {
namespace cathexis {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/cathexis-naming-conventions.html
class QualityEnumToStringCheck : public ClangTidyCheck {
public:
  QualityEnumToStringCheck(StringRef Name, ClangTidyContext *Context);

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
};

} // namespace cathexis
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_ENUMTOSTRINGCHECK_H
