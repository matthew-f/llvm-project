//===--- NamingConventionsCheck.h - clang-tidy-------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_NAMINGCONVENTIONSCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_NAMINGCONVENTIONSCHECK_H

#include "../ClangTidyCheck.h"
#include "../utils/OptionsUtils.h"

namespace clang {
namespace tidy {
namespace cathexis {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/cathexis-naming-conventions.html
class NamingConventionsCheck : public ClangTidyCheck {
public:
  NamingConventionsCheck(StringRef Name, ClangTidyContext *Context);

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

  void storeOptions(ClangTidyOptions::OptionMap &Opts) override {
    Options.store(Opts, "CheckRangeBasedForLoops", CheckRangeBasedForLoops);
  }

private:
  enum DeclaratorDeclError {
    // NOLINTNEXTLINE(cathexis-core-bad-spelling)
    Colour,
    Units,
    CamelCase,
    UpperCase,
  };

  bool isValidDeclaratorDecl(std::string *warning, DeclaratorDeclError *error,
                             const DeclaratorDecl *) const;
  bool isValidFieldName(std::string *warning, const FieldDecl *) const;
  bool isValidConstexprStaticConstVar(const std::string &name,
                                      const std::string &prefix,
                                      const VarDecl *) const;
  bool isValidVarName(std::string *warning, const VarDecl *) const;
  // NOLINTNEXTLINE(cathexis-core-bad-spelling)
  bool containsColour(const std::string &) const;

private:
  const bool CheckRangeBasedForLoops = true;
  const bool AllowUpperCaseConst = false;
  const bool AllowCoreEnumException = false;
  const bool AllowStorageException = false;
  const bool AllowCapitalisedLOGStatics = false;

  std::vector<std::string> ExceptRecordNames;
  std::vector<std::string> AllowCamelCaseFields;

  std::vector<std::string> ValidUnits = {
      "kB",  "MB",  "GB",  //
      "kb",  "Mb",  "Gb",  //
      "KiB", "MiB", "GiB", //
      "Kib", "Mib", "Gib", //
      "Bps",
  };

  std::vector<std::string> InvalidUnits = {
      "KB", "Kb", "mb", "gb", "kib", "mib", "gib",
  };
};

} // namespace cathexis
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CATHEXIS_NAMINGCONVENTIONSCHECK_H
