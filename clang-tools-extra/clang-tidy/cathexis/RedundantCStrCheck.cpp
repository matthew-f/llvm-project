//===--- RedundantCStrCheck.cpp - clang-tidy ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RedundantCStrCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void RedundantCStrCheck::registerMatchers(MatchFinder *Finder) {

  // I can't work out how to catch this at the core::VarArgs::toString method

  auto param_matcher = forEachArgumentWithParamType(
      callExpr(callee(cxxMethodDecl(hasName("c_str"),
                                    ofClass(hasName("std::basic_string")))))
          .bind("c_str_call"),
      anything());

  auto vastr_matcher =
      callExpr(param_matcher, callee(functionDecl(hasName("VA_STR"))));

  auto log_matcher = cxxMemberCallExpr(
      param_matcher,
      callee(cxxMethodDecl(anyOf(
          (allOf(hasAnyName("log", "logF", "logCF", "rlogF"),
                 ofClass(hasName("core::V2Logger")))),
          (allOf(hasAnyName("doLogF", "componentLog"),
                 ofClass(hasName("ndif::Component")))),
          (allOf(hasAnyName("appendf", "header"),
                 ofClass(hasName("NS_HTML::Page")))),
          (allOf(hasName("addCellf"), ofClass(hasName("NS_HTML::Table")))),
          (allOf(hasName("log"), ofClass(hasName("kutl::ThreadedFileLogger")))),
          (allOf(hasName("appendf"),
                 ofClass(hasName("utl::deprecated::StringBuilder")))),
          (allOf(hasAnyName("quickQueryf", "quickSelectf", "countRecordsf",
                            "retreiveSingleRecordf", "quickInsertf",
                            "quickUpdatef"),
                 ofClass(hasName("Sql_Utils")))),
          (allOf(hasName("addStringf"), ofClass(hasName("Sql_QueryString")))),
          (allOf(hasName("quickQueryf"), ofClass(hasName("NS_PSQL::Utils")))),
          (allOf(hasName("execf"), ofClass(hasName("NS_PSQL::Query")))),
          (allOf(hasAnyName("execf", "selectf", "insertf", "loadVectorf"),
                 ofClass(hasName("NS_LSQL::Query")))),
          (allOf(hasName("addStringf"),
                 ofClass(hasName("NS_LSQL::QueryString")))),
          (allOf(hasName("log"), ofClass(hasName("vdtt::DbJournalLogger")))),
          (allOf(hasName("log"), ofClass(hasName("vdtt::SliceJournalLogger")))),
          (allOf(hasAnyName("log", "debug"),
                 ofClass(hasName("npdrv::Logger")))),
          (allOf(hasAnyName("runQueryf", "runSelectf", "runInsertf"),
                 ofClass(hasName("SqlInterface")))),
          (allOf(hasName("addToHistoryf"),
                 ofClass(hasName("SqlInterfacev200")))),
          (allOf(hasAnyName("log", "debug"), ofClass(hasName("idrv::Driver")))),
          (allOf(hasName("addTextf"), ofClass(hasName("LcdOutputManager")))),
          (allOf(hasName("log"), ofClass(hasName("Controller")))),
          (allOf(hasName("log"), ofClass(hasName("IncomingConnections")))),
          (allOf(hasName("log"), ofClass(hasName("NS_NW::Device")))),
          (allOf(hasName("addHtmlf"), ofClass(hasName("rep::Renderer"))))))));

  Finder->addMatcher(vastr_matcher, this);
  Finder->addMatcher(log_matcher, this);
}

void RedundantCStrCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *c_str_call = Result.Nodes.getNodeAs<CallExpr>("c_str_call");
  if (c_str_call != nullptr) {
    diag(c_str_call->getExprLoc(), "unnecessary c_str()");
  }
}

} // namespace clang::tidy::cathexis
