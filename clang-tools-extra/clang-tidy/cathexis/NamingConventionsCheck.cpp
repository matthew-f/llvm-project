//===--- NamingConventionsCheck.cpp - clang-tidy---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NamingConventionsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace cathexis {

enum class CaseType {
  Upper,
  Lower,
  Camel,
};

static CaseType GetCaseType(std::string_view in) {
  bool contains_upper = false;
  bool contains_lower = false;

  for (char c : in) {

    if (c >= 'a' && c <= 'z') {
      contains_lower = true;
    }

    if (c >= 'A' && c <= 'Z') {
      contains_upper = true;
    }
  }

  if (contains_upper && !contains_lower) {
    return CaseType::Upper;
  }

  if (contains_lower && !contains_upper) {
    return CaseType::Lower;
  }

  // what about with just numbers? is that possible?
  return CaseType::Camel;
}

//----------------------------------------------------------------------//
static std::string ToUpper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}

//----------------------------------------------------------------------//
NamingConventionsCheck::NamingConventionsCheck(StringRef Name,
                                               ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      CheckRangeBasedForLoops(Options.get("CheckRangeBasedForLoops", true)),
      AllowUpperCaseConst(Options.get("AllowUpperCaseConst", false)),
      AllowCoreEnumException(Options.get("AllowCoreEnumException", false)),
      AllowStorageException(Options.get("AllowStorageException", false)),
      AllowCapitalisedLOGStatics(
          Options.get("AllowCapitalisedLOGStatics", false)) {

  for (const auto &ref :
       utils::options::parseStringList(Options.get("ExceptRecordNames", ""))) {
    ExceptRecordNames.emplace_back(ref);
  }

  for (const auto &ref : utils::options::parseStringList(
           Options.get("AllowCamelCaseFields", ""))) {
    AllowCamelCaseFields.emplace_back(ref);
  }
}

//----------------------------------------------------------------------//
void NamingConventionsCheck::registerMatchers(MatchFinder *Finder) {

  // Global functions should be capitalised
  Finder->addMatcher(
      functionDecl(
          unless(
              anyOf(cxxMethodDecl(),                       //
                    hasName("take_ownership"),             // core_owner
                    hasName("make_owned"),                 // core_owner
                    hasName("final"),                      // core_scopeguard
                    hasName("narrow_cast"),                // core_templates
                    hasName("make_array"),                 // core_stlutils
                    hasName("in_numeric_limits"),          // core_stlutils
                    hasName("check_size_equal"),           // core_includes
                    hasName("check_size_less_or_equal"),   // core_includes
                    hasName("from_strdump"),               // core_strconv
                    hasName("from_strdump_or"),            // core_strconv
                    hasName("to_strdump"),                 // core_strconv
                    hasName("make_unique_qptr"),           // qt_uniqueqptr
                    hasName("swap"),                       //
                    hasName("__builtin_trap"),             //
                    hasName("__builtin_memcpy"),           //
                    hasName("qt_getEnumMetaObject"),       // Qt's Q_ENUM
                    hasName("qt_getEnumName"),             // Qt's Q_ENUM
                    hasName("qHash"),                      //
                    hasName("RunCombineUnordered"),        //
                    hasName("finally"),                    //
                    hasName("ref_ptr"),                    //
                    hasName("flat_hash_set_compare"),      //
                    hasName("flat_hash_set_difference"),   //
                    hasName("flat_hash_set_union"),        //
                    hasName("flat_hash_set_intersection"), //
                    hasName("to_string"),                  //
                    hasName("from_string"),                //
                    hasName("to_serial_estimate"),         //
                    hasName("to_serial"),                  //
                    hasName("write_buf_estimate"),         //
                    hasName("write_buf_dump"),             //
                    hasName("optional_out"),               //
                    hasName("owning_ref_ptr"),             //
                    hasName("has_ToStrDump"),              //
                    hasName("has_FromStrDump"),            //
                    hasName("to_strdump"),                 //
                    hasName("from_strdump"),               //
                    matchesName("^::qInitResources_.*") // Qt's Q_INIT_RESOURCE
                    )))
          .bind("func"),
      this);

  // Member functions should not be capitalised
  Finder->addMatcher(
      functionDecl(cxxMethodDecl(unless(
                       anyOf(cxxConstructorDecl(), cxxDestructorDecl()))))
          .bind("method"),
      this);

  // Class and structure definitions should be capitalised
  Finder->addMatcher(
      cxxRecordDecl(
          allOf(isDefinition(),
                unless(anyOf(isUnion(),                     //
                             hasName("const_iterator"),     //
                             hasName("except"),             //
                             hasName("iterator"),           //
                             hasName("has_input_operator"), // core_strconv
                             hasName("has_output_operator") // core_strconv
                             ))))
          .bind("record"),
      this);

  // Field declaration (members of class/union/struct)
  Finder->addMatcher(fieldDecl().bind("field"), this);

  // Var declaration (variable declaration not that is not a field)
  if (CheckRangeBasedForLoops) {
    Finder->addMatcher(varDecl().bind("var"), this);
  } else {
    Finder->addMatcher(
        varDecl(unless(hasParent(declStmt(hasParent(cxxForRangeStmt())))))
            .bind("var"),
        this);
  }

  // Namespaces (should be lower-case or upper case starting with NS_
  Finder->addMatcher(namespaceDecl().bind("ns"), this);
}

//----------------------------------------------------------------------//
void NamingConventionsCheck::check(const MatchFinder::MatchResult &Result) {

  const auto Func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (Func) {
    if (!Func->isMain() && !Func->isOverloadedOperator()) {
      std::string name = Func->getNameInfo().getName().getAsString();

      if (name[0] < 'A' || name[0] > 'Z') {
        diag(Func->getNameInfo().getLoc(),
             "Global function '%0' should start with a capital letter")
            << name;
      }

      // NOLINTNEXTLINE(cathexis-core-bad-spelling)
      if (containsColour(name)) {
        diag(Func->getNameInfo().getLoc(),
             // NOLINTNEXTLINE(cathexis-core-bad-spelling)
             "Global function '%0' should use 'color' instead of 'colour'")
            << name;
      }
    }

    return;
  }

  const auto Method = Result.Nodes.getNodeAs<FunctionDecl>("method");
  if (Method) {
    std::string name = Method->getNameInfo().getName().getAsString();

    if ((name[0] < 'a' || name[0] > 'z') &&
        !(name.size() > 2 && name.substr(0, 2) == "__")) {

      diag(Method->getNameInfo().getLoc(),
           "Member function '%0' should not start with a capital letter")
          << name;
    }

    // NOLINTNEXTLINE(cathexis-core-bad-spelling)
    if (containsColour(name)) {

      diag(Method->getNameInfo().getLoc(),
           // NOLINTNEXTLINE(cathexis-core-bad-spelling)
           "Member function '%0' should use 'color' instead of 'colour'")
          << name;
    }

    return;
  }

  const auto Record = Result.Nodes.getNodeAs<CXXRecordDecl>("record");
  if (Record) {
    std::string name = Record->getNameAsString();

    if (!name.empty() && (name[0] < 'A' || name[0] > 'Z')) {

      auto match = std::find(ExceptRecordNames.begin(), ExceptRecordNames.end(),
                             Record->getQualifiedNameAsString());

      if (match != ExceptRecordNames.end()) {
        return;
      }

      // std::cout << "name=" << name << "\n";

      std::string type;
      if (Record->isStruct()) {
        type = "struct";
      } else if (Record->isClass()) {
        type = "class";
      } else if (Record->isUnion()) {
        type = "union";
      } else {
        type = "??";
      }
      diag(Record->getLocation(), "%0 '%1' should be capitalised (%2)")
          << type << name << Record->getQualifiedNameAsString();
    }

    return;
  }

  const auto field = Result.Nodes.getNodeAs<FieldDecl>("field");
  if (field != nullptr) {

    if (field->isAnonymousStructOrUnion()) {
      // fine
    } else {

      std::string warning;
      if (!isValidFieldName(&warning, field)) {
        diag(field->getLocation(), "'%0' %1")
            << field->getNameAsString() << warning;
      }
    }

    return;
  }

  const auto var = Result.Nodes.getNodeAs<VarDecl>("var");
  if (var != nullptr) {

    std::string warning;
    if (!isValidVarName(&warning, var)) {
      diag(var->getLocation(), "'%0' %1") << var->getNameAsString() << warning;
    }

    return;
  }

  const auto ns = Result.Nodes.getNodeAs<NamespaceDecl>("ns");
  if (ns) {
    auto name = ns->getNameAsString();

    if (!name.empty()) {
      auto case_type = GetCaseType(name);

      if (case_type == CaseType::Camel ||
          (case_type == CaseType::Upper && name.substr(0, 3) != "NS_")) {
        if (name.size() >= 3 && name.substr(name.size() - 3) == "Cmd") {
          // namespaces ending in Cmd as an exception
        } else {
          diag(ns->getLocation(), "namespace %0 should be lower-case") << ns;
        }
      }
    }

    return;
  }
}

//----------------------------------------------------------------------//
// taken from https://en.cppreference.com/w/cpp/string/basic_string/replace
static std::size_t ReplaceAll(std::string &inout, std::string_view what,
                              std::string_view with) {
  std::size_t count{};
  for (std::string::size_type pos{};
       inout.npos != (pos = inout.find(what.data(), pos, what.length()));
       pos += with.length(), ++count) {
    inout.replace(pos, what.length(), with.data(), with.length());
  }
  return count;
}

//----------------------------------------------------------------------//
bool NamingConventionsCheck::isValidDeclaratorDecl(
    std::string *warning, DeclaratorDeclError *error,
    const DeclaratorDecl *dec) const {

  std::string name = dec->getNameAsString();

  std::string lc_name;
  lc_name.reserve(name.size());
  for (char c : name) {
    if (c >= 'A' && c <= 'Z') {
      lc_name += ('a' + c - 'A');
    } else {
      lc_name += c;
    }
  }

  // NOLINTBEGIN(cathexis-core-bad-spelling)
  if (containsColour(lc_name)) {
    *error = DeclaratorDeclError::Colour;
    *warning = "should use 'color' for 'colour'";
    return false;
  }
  // NOLINTEND(cathexis-core-bad-spelling)

  for (const auto &s : ValidUnits) {
    ReplaceAll(name, std::string("_") + s, "");

    if (std::string_view(name).substr(0, s.size()) == s) {
      name.erase(0, s.size());
    }
  }

  for (const auto &s : InvalidUnits) {
    if (name.find(std::string("_") + s) != std::string::npos) {
      *warning = std::string("should use kB/MB/GB or kb/Mb/Gb or "
                             "KiB/MiB/GiB or Kib/Mib/Gib "
                             "for data sizes, not '") +
                 s + "'";
      *error = DeclaratorDeclError::Units;
      return false;
    }
  }

  auto case_type = GetCaseType(name);

  if (case_type == CaseType::Camel) {
    *error = DeclaratorDeclError::CamelCase;
    *warning = "is camel-case and should use snake-case";
    return false;
  }

  if (case_type == CaseType::Upper) {
    *error = DeclaratorDeclError::UpperCase;
    *warning = "is upper-case and should use snake-case";
    return false;
  }

  return true;
} // namespace cathexis

//----------------------------------------------------------------------//
bool NamingConventionsCheck::isValidFieldName(std::string *warning,
                                              const FieldDecl *field) const {
  std::string name = field->getNameAsString();
  if (name.empty()) {
    return true;
  }

  std::string prefix = name.substr(0, 2);

  if (field->getAccess() == AS_private) {

    if (prefix != "d_" && prefix != "d") {
      *warning = "is a private class/struct member and should start with 'd_'";
      return false;
    }
  } else {

    if (prefix == "d_") {
      *warning =
          "is a not private class/struct member and should not start with 'd_'";
      return false;
    }
  }

  DeclaratorDeclError error;
  if (isValidDeclaratorDecl(warning, &error, field)) {
    return true;
  }

  return (error == DeclaratorDeclError::CamelCase &&
          std::find(AllowCamelCaseFields.begin(), AllowCamelCaseFields.end(),
                    field->getParent()->getQualifiedNameAsString()) !=
              AllowCamelCaseFields.end());
}

//----------------------------------------------------------------------//
bool NamingConventionsCheck::isValidConstexprStaticConstVar(
    const std::string &name, const std::string &prefix,
    const VarDecl *var) const {

  // assert prefix matches name
  // assert constexpr/static const

  if (prefix.size() < 2) {
    return false;
  }

  char p0 = prefix[0];
  char p1 = prefix[1];

  if (p0 == 'k' && ((p1 >= 'A' && p1 <= 'Z') || (p1 >= '0' && p1 <= '9'))) {
    // This could be a bit more thorough
    return true;
  }

  if (AllowUpperCaseConst && GetCaseType(name) == CaseType::Upper) {
    return true;
  }

  if (AllowCoreEnumException && prefix == "e_") {

    const auto *cxx = var->getType().getTypePtr()->getAsCXXRecordDecl();

    if (cxx != nullptr) {
      if (cxx->getQualifiedNameAsString() == "core::Enum") {
        return true;
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------//
bool NamingConventionsCheck::isValidVarName(std::string *warning,
                                            const VarDecl *var) const {

  std::string name = var->getNameAsString();

  if (name.empty()) {
    // This happens for unnamed arguments
    return true;
  }

  std::string prefix = name.substr(0, 2);
  bool is_constexpr = var->isConstexpr();
  bool is_const = var->getType().isConstQualified();

  // std::cout << "checking '" << name << "' is_constexpr=" << is_constexpr
  //           << " is_const=" << is_const << "\n";

  bool expect_camel_case = false;
  bool allow_upper_case = false;

  if (is_constexpr || (is_const && var->getStorageDuration() == SD_Static)) {

    if (!isValidConstexprStaticConstVar(name, prefix, var)) {

      if (is_constexpr) {
        *warning = "is a constexpr and should start with 'k{A-Z}'";
      } else {
        *warning = "is a const variable with static storage duration and "
                   "should start with 'k{A-Z}'";
      }
      // don't use square backets in above message as it messes with the
      // parsing of the warnings
      return false;
    }

    if (AllowUpperCaseConst) {
      allow_upper_case = true;
    }

    expect_camel_case = true;

  } else if (var->getStorageDuration() == SD_Static) {

    if (prefix != "s_") {

      if (AllowCapitalisedLOGStatics && name.substr(0, 4) == "LOG_") {
        allow_upper_case = true;
      } else {
        *warning = "has static storage duration and should start with 's_'";
        return false;
      }
    }
  } else if (var->getStorageDuration() == SD_Thread) {

    if (prefix != "t_") {
      *warning = "has thread storage duration and should start with 't_'";
      return false;
    }
  } else {

    if (prefix == "d_") {
      *warning = "is a not private class/struct member and should not start "
                 "with 'd_'";
      return false;
    }

    if (!AllowStorageException) {
      if (prefix == "s_") {
        *warning =
            "is not a variable with static storage duration (don't use 's_')";
        return false;
      }

      if (prefix == "t_") {
        *warning =
            "is not a variable with thread storage duration (don't use 't_')";
        return false;
      }
    }
  }

  DeclaratorDeclError error;
  if (isValidDeclaratorDecl(warning, &error, var)) {
    return true;
  }

  if (error == DeclaratorDeclError::CamelCase && expect_camel_case) {
    return true;
  }

  if (error == DeclaratorDeclError::UpperCase && allow_upper_case) {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------//
// NOLINTNEXTLINE(cathexis-core-bad-spelling)
bool NamingConventionsCheck::containsColour(const std::string &s) const {
  // NOLINTNEXTLINE(cathexis-core-bad-spelling)
  return ToUpper(s).find("COLOUR") != std::string::npos;
}

} // namespace cathexis
} // namespace tidy
} // namespace clang
