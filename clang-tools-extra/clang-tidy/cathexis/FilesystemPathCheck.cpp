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

  // The operators below never construct a path -- they take the literal
  // through a templated Source overload -- so a literal argument to them is
  // only ever seen there: exempt the ASCII ones and diagnose the rest.
  auto isDiagnosedStringArg = expr(isPathStringArg, unless(isExemptLiteral));

  // Constructions from a literal, on the other hand, are all handled by the
  // literal matcher further down, which sees implicit conversions the two
  // construction matchers cannot. Excluding every literal here keeps the two
  // from both reporting the same construction.
  auto isDiagnosedStringObject = expr(isPathStringArg, unless(stringLiteral()));

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
                   hasArgument(0,
                               ignoringParenImpCasts(isDiagnosedStringObject)))
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
                       hasInitializer(isDiagnosedStringObject))
                   .bind("pathVar")),
      this);

  // Everywhere else a literal becomes a path, the conversion is implicit and
  // so invisible to the two matchers above: an element of a `vector<path>`
  // initializer list, an argument to a path-taking function, a `return` in a
  // path-returning function, a default member initializer. Those all do
  // produce a path construction, just not one spelled in source, so this
  // matcher uses the default traversal and keys on the literal rather than
  // on the construction.
  //
  // Matching literals only is what makes the default traversal safe here.
  // The implicit conversions the spelled-in-source matchers deliberately
  // hide are the ones the caller cannot address -- an existing std::string
  // reaching a path parameter -- whereas a literal is always fixable in
  // place by spelling it u8"...". ASCII literals are exempt as everywhere
  // else, so what remains is exactly the encoding-dependent case.
  //
  // Template instantiations are skipped so a literal in a function template
  // is reported once, against the pattern, rather than once per
  // instantiation.
  Finder->addMatcher(
      cxxConstructExpr(
          hasDeclaration(
              cxxConstructorDecl(ofClass(hasName("std::filesystem::path")))),
          hasArgument(0, ignoringParenImpCasts(
                             stringLiteral(unless(isAsciiOnly()))
                                 .bind("pathLiteral"))),
          unless(isInTemplateInstantiation())),
      this);

  // The matcher above sees a construction only where the literal is an
  // argument to path's own constructor. That relationship is broken when the
  // literal is handed to a perfect-forwarding template instead: in
  // `v.emplace_back("x")` or `map<path, int> m = {{"k", 1}}` the path is
  // built deep inside the container or inside std::pair, from a forwarded
  // parameter, and the literal never appears as an argument to a path
  // constructor anywhere the check can see. The three matchers below
  // recognise the forwarding idioms themselves.
  auto isNonAsciiLiteral = stringLiteral(unless(isAsciiOnly()));
  auto refersToPath =
      refersToType(hasDeclaration(cxxRecordDecl(hasName("std::filesystem::path"))));

  // A container whose element type mentions path -- vector<path>,
  // set<path>, map<path, V>, map<K, path> -- receiving a literal through one
  // of the emplace entry points. Which forwarded argument becomes the path
  // is not recoverable from the AST, so this looks for a path anywhere in
  // the container's template arguments and a literal in any argument. That
  // is deliberately approximate: `map<path, string>` taking a non-ASCII
  // literal as a *value* is reported too. Only non-ASCII literals reach
  // here, which keeps the over-reporting rare, and the fix suggested for a
  // genuinely-a-string argument is harmless.
  Finder->addMatcher(
      cxxMemberCallExpr(
          callee(cxxMethodDecl(hasAnyName("emplace", "emplace_back",
                                          "emplace_front", "emplace_hint"))),
          on(expr(hasType(hasUnqualifiedDesugaredType(recordType(hasDeclaration(
              classTemplateSpecializationDecl(
                  hasAnyTemplateArgument(refersToPath)))))))),
          hasAnyArgument(
              ignoringParenImpCasts(isNonAsciiLiteral.bind("forwardedLiteral"))),
          unless(isInTemplateInstantiation())),
      this);

  // std::pair construction, which covers the brace-initialized forms that
  // reach a path container: `map<path, int> m = {{"k", 1}}`, `m.insert({"k",
  // 1})`, and a directly declared `pair<path, int>`. Unlike emplace, a
  // pair's arguments line up positionally with its template arguments, so
  // this matches the literal against the element that is actually the path.
  Finder->addMatcher(
      cxxConstructExpr(
          hasType(hasUnqualifiedDesugaredType(recordType(
              hasDeclaration(classTemplateSpecializationDecl(hasName("std::pair")))))),
          anyOf(allOf(hasType(hasUnqualifiedDesugaredType(
                          recordType(hasDeclaration(classTemplateSpecializationDecl(
                              hasTemplateArgument(0, refersToPath)))))),
                      hasArgument(0, ignoringParenImpCasts(
                                         isNonAsciiLiteral.bind("pathLiteral")))),
                allOf(hasType(hasUnqualifiedDesugaredType(
                          recordType(hasDeclaration(classTemplateSpecializationDecl(
                              hasTemplateArgument(1, refersToPath)))))),
                      hasArgument(1, ignoringParenImpCasts(
                                         isNonAsciiLiteral.bind("pathLiteral"))))),
          unless(isInTemplateInstantiation())),
      this);

  // std::make_unique<path>(...) / std::make_shared<path>(...), where the
  // explicit template argument makes the target type unambiguous.
  Finder->addMatcher(
      callExpr(callee(functionDecl(
                   hasAnyName("::std::make_unique", "::std::make_shared"),
                   hasTemplateArgument(0, refersToPath))),
               hasAnyArgument(
                   ignoringParenImpCasts(isNonAsciiLiteral.bind("pathLiteral"))),
               unless(isInTemplateInstantiation())),
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

  if (const auto *lit =
          Result.Nodes.getNodeAs<StringLiteral>("pathLiteral")) {
    diag(lit->getBeginLoc(),
         "Do not construct std::filesystem::path from a non-ASCII string "
         "literal, whose bytes depend on the encoding of this source file. "
         "Use core::filesystem::PathFromString or a u8 string literal");
    return;
  }

  if (const auto *lit =
          Result.Nodes.getNodeAs<StringLiteral>("forwardedLiteral")) {
    diag(lit->getBeginLoc(),
         "Do not pass a non-ASCII string literal to a container of "
         "std::filesystem::path, whose bytes depend on the encoding of this "
         "source file. Use core::filesystem::PathFromString or a u8 string "
         "literal");
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
