//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_LOGASSERTCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_LOGASSERTCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cathexis {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// https://clang.llvm.org/extra/clang-tidy/checks/cathexis/log-assert.html
class LogAssertCheck : public ClangTidyCheck {
public:
  LogAssertCheck(StringRef Name, ClangTidyContext *Context);

  void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP,
                           Preprocessor *ModuleExpanderPP) override;

  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
};

} // namespace clang::tidy::cathexis

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_LOGASSERTCHECK_H
