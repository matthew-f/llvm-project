//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LogAssertCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/StringSet.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {
namespace {

//----------------------------------------------------------------------//
bool onlyWhitespaceAndSemicolons(StringRef Text) {
  for (char C : Text) {
    if (llvm::isSpace(C))
      continue;

    if (C == ';')
      continue;

    return false;
  }

  return true;
}

//----------------------------------------------------------------------//
// LogAssertCallbacks
//----------------------------------------------------------------------//
class LogAssertCallbacks : public PPCallbacks {
public:
  LogAssertCallbacks(ClangTidyCheck &Check, SourceManager &SM,
                     const LangOptions &LangOpts)
      : Check(Check), SM(SM), LangOpts(LangOpts) {}

  void MacroExpands(const Token &MacroNameTok, const MacroDefinition &,
                    SourceRange, const MacroArgs *) override;

private:
  void resetPrevious() {
    PreviousEndLoc = {};
    PreviousMacro.clear();
    WaitForLogAfterDebug = false;
  }

private:
  ClangTidyCheck &Check;
  SourceManager &SM;
  const LangOptions &LangOpts;

  std::string PreviousMacro;
  SourceLocation PreviousEndLoc;
  bool WaitForLogAfterDebug = false;
};

//----------------------------------------------------------------------//
void LogAssertCallbacks::MacroExpands(const Token &MacroNameTok,
                                      const MacroDefinition &MD,
                                      SourceRange Range,
                                      const MacroArgs *Args) {
  auto *II = MacroNameTok.getIdentifierInfo();
  if (!II)
    return;

  std::string Current = II->getName().str();

  SourceLocation Loc = MacroNameTok.getLocation();

  if (Current == "CDebug" || Current == "CDebugF") {
    WaitForLogAfterDebug = true;
    return;
  }

  static const llvm::StringSet<> first_macros = {
      "CLogF",
      "CLog",
  };

  static const llvm::StringSet<> second_macros = {
      "CHECKF",
      "CHECKF_M",
  };

  if (WaitForLogAfterDebug && first_macros.contains(Current)) {
    // Debug macros are not 'function-like' and so the args can't be checked
    // Wait for the subsequent expansion of CDebug to CLog
  } else if (!SM.isWrittenInMainFile(Loc))
    return;

  //  llvm::errs() << "Checking '" << Current << "'\n";

  if (!first_macros.contains(Current) && !second_macros.contains(Current)) {
    resetPrevious();
    return;
  }

  if (Args != nullptr && Args->getNumMacroArguments() > 0) {
    const auto *token = Args->getUnexpArgument(0);
    std::string name = std::string(SM.getCharacterData(token->getLocation()),
                                   token->getLength());

    if (name != "INFO" && name != "WARN" && name != "CRIT") {
      resetPrevious();
      return;
    }
  }

  if (PreviousEndLoc.isValid()) {
    CharSourceRange Between =
        CharSourceRange::getCharRange(PreviousEndLoc.getLocWithOffset(1), Loc);
    StringRef Text = Lexer::getSourceText(Between, SM, LangOpts);

    if (!onlyWhitespaceAndSemicolons(Text)) {
      {
        resetPrevious();
        return;
      }
    }
  }

  if (first_macros.contains(PreviousMacro) && second_macros.contains(Current))
    Check.diag(Loc, PreviousMacro + " immediately followed by " + Current);

  PreviousMacro = Current;
  PreviousEndLoc = Range.getEnd();
}
} // namespace

//----------------------------------------------------------------------//
// LogAssertCheck
//----------------------------------------------------------------------//
LogAssertCheck::LogAssertCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {}

void LogAssertCheck::registerPPCallbacks(const SourceManager &,
                                         Preprocessor *PP, Preprocessor *) {
  PP->addPPCallbacks(std::make_unique<LogAssertCallbacks>(
      *this, PP->getSourceManager(), PP->getLangOpts()));
}

} // namespace clang::tidy::cathexis
