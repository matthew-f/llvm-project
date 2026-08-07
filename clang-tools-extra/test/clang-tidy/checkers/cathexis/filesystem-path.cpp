// RUN: %check_clang_tidy -std=c++20 %s cathexis-filesystem-path %t

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void Func(const fs::path &);
void Plain(const std::string &);

// Construction from a string literal. Only an ordinary literal carrying
// non-ASCII characters is diagnosed; u8"..." is the recommended spelling.
void construct_from_literal() {
  fs::path p1("abc");
  fs::path p2("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p3(u8"abc☀");
  fs::path p4 = "abc☀";
  // CHECK-MESSAGES: :[[@LINE-1]]:17: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p5{"abc☀"};
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p6 = fs::path("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:26: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p7("\xc3\xa9");
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
}

// Non-ordinary literals state their encoding in their type, but only u8 is
// exempt outright; the others are exempt just when they are pure ASCII.
void construct_from_wide_literal() {
  fs::path w1(L"abc");
  fs::path w2(L"abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path w3(u"abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path w4(U"abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
}

// Construction from a string object, whether or not the conversion is
// spelled in source.
void construct_from_string(std::string s, std::string_view sv,
                           const char *cp, fs::path p) {
  fs::path p1(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p2 = s;
  // CHECK-MESSAGES: :[[@LINE-1]]:17: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p3{s};
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p4(sv);
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p5(cp);
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  fs::path p6(p);
}

// The conversion the compiler synthesizes during overload resolution, which
// is never spelled in source.
void implicit_conversion(std::string s, fs::path p) {
  Func("abc");
  Func("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  Func(u8"abc☀");
  Func(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  Func(p);
  Plain(s);
}

fs::path returns_string(std::string s) { return s; }
// CHECK-MESSAGES: :[[@LINE-1]]:49: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]

fs::path returns_literal() { return "abc☀"; }
// CHECK-MESSAGES: :[[@LINE-1]]:37: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]

struct DefaultMemberInit {
  fs::path m = "abc☀";
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
};

struct MemberInitFromString {
  fs::path m;
  MemberInitFromString(std::string s) : m(s) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:41: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
};

// A conversion inside a function template is reported once, against the
// pattern, not once per instantiation.
template <typename T> void in_template(std::string s) { Func(s); }
// CHECK-MESSAGES: :[[@LINE-1]]:62: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]

void instantiate_template(std::string s) {
  in_template<int>(s);
  in_template<char>(s);
}

// The free operator/ converts its right operand, so it is reported by the
// construction rules above -- exactly once, not once more for the operator.
void free_operator(fs::path p, std::string s) {
  auto r1 = p / "def";
  auto r2 = p / "def☀";
  // CHECK-MESSAGES: :[[@LINE-1]]:17: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  auto r3 = p / u8"def☀";
  auto r4 = p / s;
  // CHECK-MESSAGES: :[[@LINE-1]]:17: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  auto r5 = p / p;
}

// The mutating operators take a templated Source and construct nothing, so
// they are reported in their own right.
void mutating_operators(fs::path p, std::string s, fs::path other) {
  p /= "abc";
  p /= "abc☀";
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via '/=' [cathexis-filesystem-path]
  p /= u8"abc☀";
  p /= s;
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via '/=' [cathexis-filesystem-path]
  p /= other;

  p += "abc";
  p += "abc☀";
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via '+=' [cathexis-filesystem-path]
  p += u8"abc☀";
  p += s;
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via '+=' [cathexis-filesystem-path]
  p += other;

  p = "abc☀";
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via '=' [cathexis-filesystem-path]
  p = s;
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via '=' [cathexis-filesystem-path]
  p = other;
}

// The named equivalents of those operators.
void named_operators(fs::path p, std::string s) {
  p.append("abc");
  p.append("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via 'append' [cathexis-filesystem-path]
  p.append(u8"abc☀");
  p.append(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via 'append' [cathexis-filesystem-path]

  p.concat("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via 'concat' [cathexis-filesystem-path]
  p.concat(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via 'concat' [cathexis-filesystem-path]

  p.assign("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via 'assign' [cathexis-filesystem-path]
  p.assign(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Do not combine std::filesystem::path with a std::string, std::string_view, or char pointer via 'assign' [cathexis-filesystem-path]
}

// replace_filename and replace_extension take a `const path &` rather than a
// Source, so their argument really is converted and is reported once by the
// construction rules -- not a second time as a member operation.
void replacing_members(fs::path p, std::string s) {
  p.replace_filename("abc");
  p.replace_filename("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:22: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  p.replace_filename(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:22: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  p.replace_extension(".txt");
  p.replace_extension(".txt☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:23: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  p.replace_extension(s);
  // CHECK-MESSAGES: :[[@LINE-1]]:23: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
}

// Conversions to string, in both directions.
void to_string(fs::path p) {
  std::string s1 = p.string();
  // CHECK-MESSAGES: :[[@LINE-1]]:20: warning: Do not use std::filesystem::path::string() [cathexis-filesystem-path]
  std::string s2 = p;
  // CHECK-MESSAGES: :[[@LINE-1]]:20: warning: Avoid implicit conversion of std::filesystem::path to std::string. This will not build on all platforms [cathexis-filesystem-path]
}

// Elements of a container of paths, where the conversion is implicit.
void container_elements(std::string s) {
  std::vector<fs::path> v1 = {"abc"};
  std::vector<fs::path> v2 = {"abc☀"};
  // CHECK-MESSAGES: :[[@LINE-1]]:31: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  std::vector<fs::path> v3 = {u8"abc☀"};
  std::vector<fs::path> v4 = {s};
  // CHECK-MESSAGES: :[[@LINE-1]]:31: warning: Do not construct std::filesystem::path from a std::string, std::string_view, or char pointer. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
}

// A literal forwarded into a path container never reaches path's constructor
// as an argument, so the forwarding idioms are recognised directly.
void forwarded_into_container(std::string s) {
  std::vector<fs::path> v;
  v.push_back("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  v.emplace_back("abc");
  v.emplace_back("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:18: warning: Do not pass a non-ASCII string literal to a container of std::filesystem::path, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  v.emplace_back(u8"abc☀");

  std::map<fs::path, int> m;
  m.emplace("abc☀", 1);
  // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: Do not pass a non-ASCII string literal to a container of std::filesystem::path, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]

  auto u = std::make_unique<fs::path>("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:39: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  auto sp = std::make_shared<fs::path>("abc☀");
  // CHECK-MESSAGES: :[[@LINE-1]]:40: warning: Do not construct std::filesystem::path from a non-ASCII string literal, whose bytes depend on the encoding of this source file. Use core::filesystem::PathFromString or a u8 string literal [cathexis-filesystem-path]
  auto os = std::make_unique<std::string>("abc☀");
}
