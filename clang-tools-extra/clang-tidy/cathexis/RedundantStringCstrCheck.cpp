//===--- RedundantStringCstrCheck.cpp - clang-tidy
//------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RedundantStringCstrCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void RedundantStringCstrCheck::registerMatchers(MatchFinder *Finder) {

  // I can't work out how to catch this at the core::VarArgs::toString method

  auto param_matcher = forEachArgumentWithParam(
      callExpr(callee(cxxMethodDecl(hasName("c_str"),
                                    ofClass(hasName("std::basic_string")))))
          .bind("c_str_call"),
      parmVarDecl().bind("parm"));

  auto vastr_matcher =
      callExpr(param_matcher, callee(functionDecl(hasName("VA_STR"))))
          .bind("caller");

  auto log_matcher =
      cxxMemberCallExpr(
          param_matcher,
          callee(cxxMethodDecl(anyOf(
              (allOf(hasAnyName("log", "logF", "logCF", "rlogF"),
                     ofClass(hasName("core::V2Logger")))),
              (allOf(hasAnyName("doLogF", "componentLog"),
                     ofClass(hasName("ndif::Component")))),
              (allOf(hasAnyName("appendf", "header"),
                     ofClass(hasName("NS_HTML::Page")))),
              (allOf(hasName("addCellf"), ofClass(hasName("NS_HTML::Table")))),
              (allOf(hasName("log"),
                     ofClass(hasName("kutl::ThreadedFileLogger")))),
              (allOf(hasName("appendf"),
                     ofClass(hasName("utl::deprecated::StringBuilder")))),
              (allOf(hasAnyName("quickQueryf", "quickSelectf", "countRecordsf",
                                "retreiveSingleRecordf", "quickInsertf",
                                "quickUpdatef"),
                     ofClass(hasName("Sql_Utils")))),
              (allOf(hasName("addStringf"),
                     ofClass(hasName("Sql_QueryString")))),
              (allOf(hasName("quickQueryf"),
                     ofClass(hasName("NS_PSQL::Utils")))),
              (allOf(hasName("execf"), ofClass(hasName("NS_PSQL::Query")))),
              (allOf(hasAnyName("execf", "selectf", "insertf", "loadVectorf"),
                     ofClass(hasName("NS_LSQL::Query")))),
              (allOf(hasName("addStringf"),
                     ofClass(hasName("NS_LSQL::QueryString")))),
              (allOf(hasName("log"),
                     ofClass(hasName("vdtt::DbJournalLogger")))),
              (allOf(hasName("log"),
                     ofClass(hasName("vdtt::SliceJournalLogger")))),
              (allOf(hasAnyName("log", "debug"),
                     ofClass(hasName("npdrv::Logger")))),
              (allOf(hasAnyName("runQueryf", "runSelectf", "runInsertf"),
                     ofClass(hasName("SqlInterface")))),
              (allOf(hasName("addToHistoryf"),
                     ofClass(hasName("SqlInterfacev200")))),
              (allOf(hasAnyName("log", "debug"),
                     ofClass(hasName("idrv::Driver")))),
              (allOf(hasName("addTextf"),
                     ofClass(hasName("LcdOutputManager")))),
              (allOf(hasName("log"), ofClass(hasName("Controller")))),
              (allOf(hasName("log"), ofClass(hasName("IncomingConnections")))),
              (allOf(hasName("log"), ofClass(hasName("NS_NW::Device")))),
              (allOf(hasName("addHtmlf"),
                     ofClass(hasName("rep::Renderer"))))))))
          .bind("caller");

  Finder->addMatcher(vastr_matcher, this);
  Finder->addMatcher(log_matcher, this);
}

void RedundantStringCstrCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *c_str_call = Result.Nodes.getNodeAs<CallExpr>("c_str_call");
  const auto *parm = Result.Nodes.getNodeAs<ParmVarDecl>("parm");
  const auto *caller = Result.Nodes.getNodeAs<CallExpr>("caller");

  if (c_str_call != nullptr && parm != nullptr && caller != nullptr) {

    const auto *callee = caller->getDirectCallee();
    std::string callee_name;
    if (callee != nullptr) {
      callee_name = callee->getQualifiedNameAsString();
    }

    unsigned index = parm->getFunctionScopeIndex();
    if ((index == 1 && (callee_name == "core::V2Logger::logF" ||
                        callee_name == "core::V2Logger::log")) ||
        (index == 0 && callee_name == "VA_STR")) {
      // These are the format strings
      return;
    }

    diag(c_str_call->getExprLoc(), "unnecessary c_str() at index %0, callee=%1")
        << index << callee_name;
  }
}

} // namespace clang::tidy::cathexis
