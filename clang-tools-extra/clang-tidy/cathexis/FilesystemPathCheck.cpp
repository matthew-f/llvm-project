//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FilesystemPathCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cathexis {

void FilesystemPathCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(cxxMethodDecl(hasName("string"),
                               ofClass(hasName("std::filesystem::path")))))
          .bind("call"),
      this);

  // Match the reverse direction: an implicit conversion of a
  // std::filesystem::path to a string via path's `operator string_type()`
  // conversion operator, e.g. `std::string s = p;`. This is a distinct member
  // (a CXXConversionDecl) from the `string()` method above. Because the
  // conversion is implicit it is not spelled in source, so this uses the
  // default traversal rather than TK_IgnoreUnlessSpelledInSource.
  Finder->addMatcher(cxxMemberCallExpr(callee(cxxConversionDecl(ofClass(
                                           hasName("std::filesystem::path")))))
                         .bind("pathToString"),
                     this);

  // A std::string, std::string_view (char specializations), char pointer, or
  // char array (e.g. a string literal). The std::string overload is the
  // non-template path(const string_type &) constructor; the others are deduced
  // through the templated path(const Source &) constructor. Matching on the
  // argument's type covers all of them.
  auto isPathStringArg = anyOf(
      hasType(hasUnqualifiedDesugaredType(
          recordType(hasDeclaration(classTemplateSpecializationDecl(
              hasAnyName("::std::basic_string", "::std::basic_string_view"),
              hasTemplateArgument(0, refersToType(asString("char")))))))),
      hasType(pointerType(
          pointee(qualType(anyOf(asString("char"), asString("const char")))))),
      hasType(arrayType(hasElementType(
          qualType(anyOf(asString("char"), asString("const char")))))));

  // TK_IgnoreUnlessSpelledInSource restricts matching to constructions the
  // user actually wrote (e.g. `path p(str)`, `path{str}`, or `path(str)`). It
  // hides the implicit path conversions the compiler synthesizes during
  // overload resolution -- for example when a call like `Exists(str)` resolves
  // to a path-taking overload instead of a string-taking wrapper -- which are
  // not something the caller can address at the call site.
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               cxxConstructExpr(
                   hasDeclaration(cxxConstructorDecl(
                       ofClass(hasName("std::filesystem::path")))),
                   hasArgument(0, ignoringParenImpCasts(expr(isPathStringArg))))
                   .bind("pathCtor")),
      this);

  // Copy-initialization (`path p = str;`) is itself an implicit conversion, so
  // the construction is not spelled in source and the matcher above does not
  // see it. Match it via the variable instead: a path whose written
  // initializer is one of the string-like types. Direct- and list-init have a
  // path-typed initializer (the construction) and so are not matched here,
  // avoiding duplicate diagnostics.
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               varDecl(hasType(cxxRecordDecl(hasName("std::filesystem::path"))),
                       hasInitializer(expr(isPathStringArg)))
                   .bind("pathVar")),
      this);
}

void FilesystemPathCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *call = Result.Nodes.getNodeAs<CallExpr>("call");
      call != nullptr) {
    diag(call->getBeginLoc(), "Do not use std::filesystem::path::string");
    return;
  }

  if (const auto *conv =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("pathToString");
      conv != nullptr) {
    diag(conv->getBeginLoc(),
         "Avoid implicit conversion of std::filesystem::path to std::string. "
         "This will not build on all platforms");
    return;
  }

  if (const auto *ctor = Result.Nodes.getNodeAs<CXXConstructExpr>("pathCtor");
      ctor != nullptr) {
    diag(ctor->getBeginLoc(),
         "Do not construct std::filesystem::path from a std::string, "
         "std::string_view, or char pointer. Use "
         "core::filesystem::PathFromString or a u8 string literal");
    return;
  }

  if (const auto *var = Result.Nodes.getNodeAs<VarDecl>("pathVar");
      var != nullptr) {
    diag(var->getBeginLoc(),
         "Do not construct std::filesystem::path from a std::string, "
         "std::string_view, or char pointer. Use "
         "core::filesystem::PathFromString or a u8 string literal");
  }
}

} // namespace clang::tidy::cathexis
