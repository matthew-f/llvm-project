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

namespace {

// A narrow (`char`) string literal whose contents are entirely ASCII. The
// literal's bytes are known here, at analysis time, so no runtime check is
// involved. Narrow literals are the only kind reachable from the matchers
// below -- `u8"..."` is `char8_t` in C++20 and so never satisfies the
// string-like argument constraint -- but the char width is still verified
// because getString() requires a single-byte encoding.
AST_MATCHER(StringLiteral, isAsciiOnly) {
  return Node.getCharByteWidth() == 1 && !Node.containsNonAscii();
}

} // namespace

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

  // A pure-ASCII narrow literal is exempt everywhere: `path p("/etc")` and
  // `p /= "subdir"` are idiomatic and carry no encoding hazard, because the
  // literal's bytes are known here and are identical under every source
  // encoding. A non-ASCII literal is a different matter -- its bytes depend
  // on the encoding of the source file, which is exactly the ambiguity path
  // exists to resolve -- so those are still diagnosed. Note that this is
  // decided by the literal's value, not its spelling: "\xc3\xa9" is written
  // in ASCII but does not produce ASCII bytes, and is diagnosed.
  auto isExemptLiteral = stringLiteral(isAsciiOnly());

  // The argument shape every matcher below diagnoses: string-like, and not
  // an exempt literal.
  auto isDiagnosedStringArg = expr(isPathStringArg, unless(isExemptLiteral));

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
                   hasArgument(0, ignoringParenImpCasts(isDiagnosedStringArg)))
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
                       hasInitializer(isDiagnosedStringArg))
                   .bind("pathVar")),
      this);

  // path's mutating operators (`/=`, `+=`, `=`) each have a templated
  // `Source` overload, so `p /= str` binds directly to
  // `operator/=(const std::string &)` -- no path is constructed and the
  // matchers above see nothing. The free `operator/(const path &, const
  // path &)` does convert, but implicitly, so it is not spelled in source
  // either. Match the argument instead of the construction.
  //
  // Argument 0 is the left operand in both shapes (the object for a member
  // operator, the lhs for the free `operator/`), so requiring it to be a
  // path is what scopes this to filesystem operators; argument 1 is the
  // right operand in both.
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               cxxOperatorCallExpr(
                   hasAnyOverloadedOperatorName("/=", "+=", "/", "="),
                   hasArgument(0, expr(hasType(cxxRecordDecl(
                                      hasName("std::filesystem::path"))))),
                   hasArgument(1, ignoringParenImpCasts(isDiagnosedStringArg)))
                   .bind("pathOp")),
      this);

  // The named equivalents of the operators above, which take the same
  // templated `Source` argument.
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               cxxMemberCallExpr(
                   callee(cxxMethodDecl(
                       ofClass(hasName("std::filesystem::path")),
                       hasAnyName("append", "concat", "assign",
                                  "replace_filename", "replace_extension"))),
                   hasArgument(0, ignoringParenImpCasts(isDiagnosedStringArg)))
                   .bind("pathMemberOp")),
      this);
}

void FilesystemPathCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *call = Result.Nodes.getNodeAs<CallExpr>("call")) {
    diag(call->getBeginLoc(), "Do not use std::filesystem::path::string()");
    return;
  }

  if (const auto *conv =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("pathToString")) {
    diag(conv->getBeginLoc(),
         "Avoid implicit conversion of std::filesystem::path to std::string. "
         "This will not build on all platforms");
    return;
  }

  if (const auto *ctor = Result.Nodes.getNodeAs<CXXConstructExpr>("pathCtor")) {
    diag(ctor->getBeginLoc(),
         "Do not construct std::filesystem::path from a std::string, "
         "std::string_view, or char pointer. Use "
         "core::filesystem::PathFromString or a u8 string literal");
    return;
  }

  if (const auto *var = Result.Nodes.getNodeAs<VarDecl>("pathVar")) {
    diag(var->getBeginLoc(),
         "Do not construct std::filesystem::path from a std::string, "
         "std::string_view, or char pointer. Use "
         "core::filesystem::PathFromString or a u8 string literal");
    return;
  }

  if (const auto *op = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("pathOp")) {
    diag(op->getOperatorLoc(),
         "Do not combine std::filesystem::path with a std::string, "
         "std::string_view, or char pointer via '%0'")
        << getOperatorSpelling(op->getOperator());
    return;
  }

  if (const auto *call =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("pathMemberOp")) {
    diag(call->getExprLoc(),
         "Do not combine std::filesystem::path with a std::string, "
         "std::string_view, or char pointer via '%0'")
        << call->getMethodDecl()->getName();
  }
}

} // namespace clang::tidy::cathexis
