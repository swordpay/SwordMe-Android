// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
//
// Note that we only have partial C++11 support yet.

#include "absl/debugging/internal/demangle.h"

#include <cstdint>
#include <cstdio>
#include <limits>

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {

typedef struct {
  const char *abbrev;
  const char *real_name;

  int arity;
} AbbrevPair;

static const AbbrevPair kOperatorList[] = {

    {"nw", "new", 0},
    {"na", "new[]", 0},

    {"dl", "delete", 1},
    {"da", "delete[]", 1},

    {"ps", "+", 1},  // "positive"
    {"ng", "-", 1},  // "negative"
    {"ad", "&", 1},  // "address-of"
    {"de", "*", 1},  // "dereference"
    {"co", "~", 1},

    {"pl", "+", 2},
    {"mi", "-", 2},
    {"ml", "*", 2},
    {"dv", "/", 2},
    {"rm", "%", 2},
    {"an", "&", 2},
    {"or", "|", 2},
    {"eo", "^", 2},
    {"aS", "=", 2},
    {"pL", "+=", 2},
    {"mI", "-=", 2},
    {"mL", "*=", 2},
    {"dV", "/=", 2},
    {"rM", "%=", 2},
    {"aN", "&=", 2},
    {"oR", "|=", 2},
    {"eO", "^=", 2},
    {"ls", "<<", 2},
    {"rs", ">>", 2},
    {"lS", "<<=", 2},
    {"rS", ">>=", 2},
    {"eq", "==", 2},
    {"ne", "!=", 2},
    {"lt", "<", 2},
    {"gt", ">", 2},
    {"le", "<=", 2},
    {"ge", ">=", 2},
    {"nt", "!", 1},
    {"aa", "&&", 2},
    {"oo", "||", 2},
    {"pp", "++", 1},
    {"mm", "--", 1},
    {"cm", ",", 2},
    {"pm", "->*", 2},
    {"pt", "->", 0},  // Special syntax
    {"cl", "()", 0},  // Special syntax
    {"ix", "[]", 2},
    {"qu", "?", 3},
    {"st", "sizeof", 0},  // Special syntax
    {"sz", "sizeof", 1},  // Not a real operator name, but used in expressions.
    {nullptr, nullptr, 0},
};

//
// Invariant: only one- or two-character type abbreviations here.
static const AbbrevPair kBuiltinTypeList[] = {
    {"v", "void", 0},
    {"w", "wchar_t", 0},
    {"b", "bool", 0},
    {"c", "char", 0},
    {"a", "signed char", 0},
    {"h", "unsigned char", 0},
    {"s", "short", 0},
    {"t", "unsigned short", 0},
    {"i", "int", 0},
    {"j", "unsigned int", 0},
    {"l", "long", 0},
    {"m", "unsigned long", 0},
    {"x", "long long", 0},
    {"y", "unsigned long long", 0},
    {"n", "__int128", 0},
    {"o", "unsigned __int128", 0},
    {"f", "float", 0},
    {"d", "double", 0},
    {"e", "long double", 0},
    {"g", "__float128", 0},
    {"z", "ellipsis", 0},

    {"De", "decimal128", 0},      // IEEE 754r decimal floating point (128 bits)
    {"Dd", "decimal64", 0},       // IEEE 754r decimal floating point (64 bits)
    {"Dc", "decltype(auto)", 0},
    {"Da", "auto", 0},
    {"Dn", "std::nullptr_t", 0},  // i.e., decltype(nullptr)
    {"Df", "decimal32", 0},       // IEEE 754r decimal floating point (32 bits)
    {"Di", "char32_t", 0},
    {"Du", "char8_t", 0},
    {"Ds", "char16_t", 0},
    {"Dh", "float16", 0},         // IEEE 754r half-precision float (16 bits)
    {nullptr, nullptr, 0},
};

static const AbbrevPair kSubstitutionList[] = {
    {"St", "", 0},
    {"Sa", "allocator", 0},
    {"Sb", "basic_string", 0},

    {"Ss", "string", 0},

    {"Si", "istream", 0},

    {"So", "ostream", 0},

    {"Sd", "iostream", 0},
    {nullptr, nullptr, 0},
};

// frame, so every byte counts.
typedef struct {
  int mangled_idx;                     // Cursor of mangled name.
  int out_cur_idx;                     // Cursor of output string.
  int prev_name_idx;                   // For constructors/destructors.
  unsigned int prev_name_length : 16;  // For constructors/destructors.
  signed int nest_level : 15;          // For nested names.
  unsigned int append : 1;             // Append flag.




} ParseState;

static_assert(sizeof(ParseState) == 4 * sizeof(int),
              "unexpected size of ParseState");

// constant data, data that's intentionally immune to backtracking (steps), or
// data that would never be changed by backtracking anyway (recursion_depth).
//
// Only one copy of this exists for each call to Demangle, so the size of this
// struct is nearly inconsequential.
typedef struct {
  const char *mangled_begin;  // Beginning of input string.
  char *out;                  // Beginning of output string.
  int out_end_idx;            // One past last allowed output character.
  int recursion_depth;        // For stack exhaustion prevention.
  int steps;               // Cap how much work we'll do, regardless of depth.
  ParseState parse_state;  // Backtrackable state copied for most frames.
} State;

namespace {
// Prevent deep recursion / stack exhaustion.
// Also prevent unbounded handling of complex inputs.
class ComplexityGuard {
 public:
  explicit ComplexityGuard(State *state) : state_(state) {
    ++state->recursion_depth;
    ++state->steps;
  }
  ~ComplexityGuard() { --state_->recursion_depth; }



  static constexpr int kRecursionDepthLimit = 256;














  static constexpr int kParseStepsLimit = 1 << 17;

  bool IsTooComplex() const {
    return state_->recursion_depth > kRecursionDepthLimit ||
           state_->steps > kParseStepsLimit;
  }

 private:
  State *state_;
};
}  // namespace

// signal safe.
static size_t StrLen(const char *str) {
  size_t len = 0;
  while (*str != '\0') {
    ++str;
    ++len;
  }
  return len;
}

static bool AtLeastNumCharsRemaining(const char *str, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (str[i] == '\0') {
      return false;
    }
  }
  return true;
}

static bool StrPrefix(const char *str, const char *prefix) {
  size_t i = 0;
  while (str[i] != '\0' && prefix[i] != '\0' && str[i] == prefix[i]) {
    ++i;
  }
  return prefix[i] == '\0';  // Consumed everything in "prefix".
}

static void InitState(State* state,
                      const char* mangled,
                      char* out,
                      size_t out_size) {
  state->mangled_begin = mangled;
  state->out = out;
  state->out_end_idx = static_cast<int>(out_size);
  state->recursion_depth = 0;
  state->steps = 0;

  state->parse_state.mangled_idx = 0;
  state->parse_state.out_cur_idx = 0;
  state->parse_state.prev_name_idx = 0;
  state->parse_state.prev_name_length = 0;
  state->parse_state.nest_level = -1;
  state->parse_state.append = true;
}

static inline const char *RemainingInput(State *state) {
  return &state->mangled_begin[state->parse_state.mangled_idx];
}

// at "mangled_idx" position.  It is assumed that "one_char_token" does
// not contain '\0'.
static bool ParseOneCharToken(State *state, const char one_char_token) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (RemainingInput(state)[0] == one_char_token) {
    ++state->parse_state.mangled_idx;
    return true;
  }
  return false;
}

// at "mangled_cur" position.  It is assumed that "two_char_token" does
// not contain '\0'.
static bool ParseTwoCharToken(State *state, const char *two_char_token) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (RemainingInput(state)[0] == two_char_token[0] &&
      RemainingInput(state)[1] == two_char_token[1]) {
    state->parse_state.mangled_idx += 2;
    return true;
  }
  return false;
}

// "char_class" at "mangled_cur" position.
static bool ParseCharClass(State *state, const char *char_class) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (RemainingInput(state)[0] == '\0') {
    return false;
  }
  const char *p = char_class;
  for (; *p != '\0'; ++p) {
    if (RemainingInput(state)[0] == *p) {
      ++state->parse_state.mangled_idx;
      return true;
    }
  }
  return false;
}

static bool ParseDigit(State *state, int *digit) {
  char c = RemainingInput(state)[0];
  if (ParseCharClass(state, "0123456789")) {
    if (digit != nullptr) {
      *digit = c - '0';
    }
    return true;
  }
  return false;
}

static bool Optional(bool /*status*/) { return true; }

typedef bool (*ParseFunc)(State *);
static bool OneOrMore(ParseFunc parse_func, State *state) {
  if (parse_func(state)) {
    while (parse_func(state)) {
    }
    return true;
  }
  return false;
}

// always returns true and must be followed by a termination token or a
// terminating sequence not handled by parse_func (e.g.
// ParseOneCharToken(state, 'E')).
static bool ZeroOrMore(ParseFunc parse_func, State *state) {
  while (parse_func(state)) {
  }
  return true;
}

// set to out_end_idx+1.  The output string is ensured to
// always terminate with '\0' as long as there is no overflow.
static void Append(State *state, const char *const str, const size_t length) {
  for (size_t i = 0; i < length; ++i) {
    if (state->parse_state.out_cur_idx + 1 <
        state->out_end_idx) {  // +1 for '\0'
      state->out[state->parse_state.out_cur_idx++] = str[i];
    } else {

      state->parse_state.out_cur_idx = state->out_end_idx + 1;
      break;
    }
  }
  if (state->parse_state.out_cur_idx < state->out_end_idx) {
    state->out[state->parse_state.out_cur_idx] =
        '\0';  // Terminate it with '\0'
  }
}

static bool IsLower(char c) { return c >= 'a' && c <= 'z'; }

static bool IsAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool IsDigit(char c) { return c >= '0' && c <= '9'; }

// by GCC 4.5.x and later versions (and our locally-modified version of GCC
// 4.4.x) to indicate functions which have been cloned during optimization.
// We treat any sequence (.<alpha>+.<digit>+)+ as a function clone suffix.
// Additionally, '_' is allowed along with the alphanumeric sequence.
static bool IsFunctionCloneSuffix(const char *str) {
  size_t i = 0;
  while (str[i] != '\0') {
    bool parsed = false;

    if (str[i] == '.' && (IsAlpha(str[i + 1]) || str[i + 1] == '_')) {
      parsed = true;
      i += 2;
      while (IsAlpha(str[i]) || str[i] == '_') {
        ++i;
      }
    }
    if (str[i] == '.' && IsDigit(str[i + 1])) {
      parsed = true;
      i += 2;
      while (IsDigit(str[i])) {
        ++i;
      }
    }
    if (!parsed)
      return false;
  }
  return true;  // Consumed everything in "str".
}

static bool EndsWith(State *state, const char chr) {
  return state->parse_state.out_cur_idx > 0 &&
         state->parse_state.out_cur_idx < state->out_end_idx &&
         chr == state->out[state->parse_state.out_cur_idx - 1];
}

static void MaybeAppendWithLength(State *state, const char *const str,
                                  const size_t length) {
  if (state->parse_state.append && length > 0) {


    if (str[0] == '<' && EndsWith(state, '<')) {
      Append(state, " ", 1);
    }


    if (state->parse_state.out_cur_idx < state->out_end_idx &&
        (IsAlpha(str[0]) || str[0] == '_')) {
      state->parse_state.prev_name_idx = state->parse_state.out_cur_idx;
      state->parse_state.prev_name_length = static_cast<unsigned int>(length);
    }
    Append(state, str, length);
  }
}

static bool MaybeAppendDecimal(State *state, int val) {

  constexpr size_t kMaxLength = 20;
  char buf[kMaxLength];


  if (state->parse_state.append) {


    char *p = &buf[kMaxLength];
    do {  // val=0 is the only input that should write a leading zero digit.
      *--p = static_cast<char>((val % 10) + '0');
      val /= 10;
    } while (p > buf && val != 0);

    Append(state, p, kMaxLength - static_cast<size_t>(p - buf));
  }

  return true;
}

// Returns true so that it can be placed in "if" conditions.
static bool MaybeAppend(State *state, const char *const str) {
  if (state->parse_state.append) {
    size_t length = StrLen(str);
    MaybeAppendWithLength(state, str, length);
  }
  return true;
}

static bool EnterNestedName(State *state) {
  state->parse_state.nest_level = 0;
  return true;
}

static bool LeaveNestedName(State *state, int16_t prev_value) {
  state->parse_state.nest_level = prev_value;
  return true;
}

static bool DisableAppend(State *state) {
  state->parse_state.append = false;
  return true;
}

static bool RestoreAppend(State *state, bool prev_value) {
  state->parse_state.append = prev_value;
  return true;
}

static void MaybeIncreaseNestLevel(State *state) {
  if (state->parse_state.nest_level > -1) {
    ++state->parse_state.nest_level;
  }
}

static void MaybeAppendSeparator(State *state) {
  if (state->parse_state.nest_level >= 1) {
    MaybeAppend(state, "::");
  }
}

static void MaybeCancelLastSeparator(State *state) {
  if (state->parse_state.nest_level >= 1 && state->parse_state.append &&
      state->parse_state.out_cur_idx >= 2) {
    state->parse_state.out_cur_idx -= 2;
    state->out[state->parse_state.out_cur_idx] = '\0';
  }
}

// "mangled_cur" is anonymous namespace.
static bool IdentifierIsAnonymousNamespace(State *state, size_t length) {

  static const char anon_prefix[] = "_GLOBAL__N_";
  return (length > (sizeof(anon_prefix) - 1) &&
          StrPrefix(RemainingInput(state), anon_prefix));
}

static bool ParseMangledName(State *state);
static bool ParseEncoding(State *state);
static bool ParseName(State *state);
static bool ParseUnscopedName(State *state);
static bool ParseNestedName(State *state);
static bool ParsePrefix(State *state);
static bool ParseUnqualifiedName(State *state);
static bool ParseSourceName(State *state);
static bool ParseLocalSourceName(State *state);
static bool ParseUnnamedTypeName(State *state);
static bool ParseNumber(State *state, int *number_out);
static bool ParseFloatNumber(State *state);
static bool ParseSeqId(State *state);
static bool ParseIdentifier(State *state, size_t length);
static bool ParseOperatorName(State *state, int *arity);
static bool ParseSpecialName(State *state);
static bool ParseCallOffset(State *state);
static bool ParseNVOffset(State *state);
static bool ParseVOffset(State *state);
static bool ParseAbiTags(State *state);
static bool ParseCtorDtorName(State *state);
static bool ParseDecltype(State *state);
static bool ParseType(State *state);
static bool ParseCVQualifiers(State *state);
static bool ParseBuiltinType(State *state);
static bool ParseFunctionType(State *state);
static bool ParseBareFunctionType(State *state);
static bool ParseClassEnumType(State *state);
static bool ParseArrayType(State *state);
static bool ParsePointerToMemberType(State *state);
static bool ParseTemplateParam(State *state);
static bool ParseTemplateTemplateParam(State *state);
static bool ParseTemplateArgs(State *state);
static bool ParseTemplateArg(State *state);
static bool ParseBaseUnresolvedName(State *state);
static bool ParseUnresolvedName(State *state);
static bool ParseExpression(State *state);
static bool ParseExprPrimary(State *state);
static bool ParseExprCastValue(State *state);
static bool ParseLocalName(State *state);
static bool ParseLocalNameSuffix(State *state);
static bool ParseDiscriminator(State *state);
static bool ParseSubstitution(State *state, bool accept_std);

// translation of the Itanium C++ ABI defined in BNF with a couple of
// exceptions.
//
// - Support GNU extensions not defined in the Itanium C++ ABI
// - <prefix> and <template-prefix> are combined to avoid infinite loop
// - Reorder patterns to shorten the code
// - Reorder patterns to give greedier functions precedence
//   We'll mark "Less greedy than" for these cases in the code
//
// Each parsing function changes the parse state and returns true on
// success, or returns false and doesn't change the parse state (note:
// the parse-steps counter increases regardless of success or failure).
// To ensure that the parse state isn't changed in the latter case, we
// save the original state before we call multiple parsing functions
// consecutively with &&, and restore it if unsuccessful.  See
// ParseEncoding() as an example of this convention.  We follow the
// convention throughout the code.
//
// Originally we tried to do demangling without following the full ABI
// syntax but it turned out we needed to follow the full syntax to
// parse complicated cases like nested template arguments.  Note that
// implementing a full-fledged demangler isn't trivial (libiberty's
// cp-demangle.c has +4300 lines).
//
// Note that (foo) in <(foo) ...> is a modifier to be ignored.
//
// Reference:
// - Itanium C++ ABI
//   <https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling>

static bool ParseMangledName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  return ParseTwoCharToken(state, "_Z") && ParseEncoding(state);
}

//            ::= <(data) name>
//            ::= <special-name>
static bool ParseEncoding(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;





  if (ParseName(state) && Optional(ParseBareFunctionType(state))) {
    return true;
  }

  if (ParseSpecialName(state)) {
    return true;
  }
  return false;
}

//        ::= <unscoped-template-name> <template-args>
//        ::= <unscoped-name>
//        ::= <local-name>
static bool ParseName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseNestedName(state) || ParseLocalName(state)) {
    return true;
  }








  ParseState copy = state->parse_state;

  if (ParseSubstitution(state, /*accept_std=*/false) &&
      ParseTemplateArgs(state)) {
    return true;
  }
  state->parse_state = copy;


  return ParseUnscopedName(state) && Optional(ParseTemplateArgs(state));
}

//                 ::= St <unqualified-name>
static bool ParseUnscopedName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseUnqualifiedName(state)) {
    return true;
  }

  ParseState copy = state->parse_state;
  if (ParseTwoCharToken(state, "St") && MaybeAppend(state, "std::") &&
      ParseUnqualifiedName(state)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                ::= O // rvalue method reference qualifier
static inline bool ParseRefQualifier(State *state) {
  return ParseCharClass(state, "OR");
}

//                   <unqualified-name> E
//               ::= N [<CV-qualifiers>] [<ref-qualifier>] <template-prefix>
//                   <template-args> E
static bool ParseNestedName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'N') && EnterNestedName(state) &&
      Optional(ParseCVQualifiers(state)) &&
      Optional(ParseRefQualifier(state)) && ParsePrefix(state) &&
      LeaveNestedName(state, copy.nest_level) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

// end up infinite loop.  Hence we merge them to avoid the case.
//
// <prefix> ::= <prefix> <unqualified-name>
//          ::= <template-prefix> <template-args>
//          ::= <template-param>
//          ::= <substitution>
//          ::= # empty
// <template-prefix> ::= <prefix> <(template) unqualified-name>
//                   ::= <template-param>
//                   ::= <substitution>
static bool ParsePrefix(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  bool has_something = false;
  while (true) {
    MaybeAppendSeparator(state);
    if (ParseTemplateParam(state) ||
        ParseSubstitution(state, /*accept_std=*/true) ||
        ParseUnscopedName(state) ||
        (ParseOneCharToken(state, 'M') && ParseUnnamedTypeName(state))) {
      has_something = true;
      MaybeIncreaseNestLevel(state);
      continue;
    }
    MaybeCancelLastSeparator(state);
    if (has_something && ParseTemplateArgs(state)) {
      return ParsePrefix(state);
    } else {
      break;
    }
  }
  return true;
}

//                    ::= <ctor-dtor-name> [<abi-tags>]
//                    ::= <source-name> [<abi-tags>]
//                    ::= <local-source-name> [<abi-tags>]
//                    ::= <unnamed-type-name> [<abi-tags>]
//
// <local-source-name> is a GCC extension; see below.
static bool ParseUnqualifiedName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseOperatorName(state, nullptr) || ParseCtorDtorName(state) ||
      ParseSourceName(state) || ParseLocalSourceName(state) ||
      ParseUnnamedTypeName(state)) {
    return ParseAbiTags(state);
  }
  return false;
}

// <abi-tag>  ::= B <source-name>
static bool ParseAbiTags(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;

  while (ParseOneCharToken(state, 'B')) {
    ParseState copy = state->parse_state;
    MaybeAppend(state, "[abi:");

    if (!ParseSourceName(state)) {
      state->parse_state = copy;
      return false;
    }
    MaybeAppend(state, "]");
  }

  return true;
}

static bool ParseSourceName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  int length = -1;
  if (ParseNumber(state, &length) &&
      ParseIdentifier(state, static_cast<size_t>(length))) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//
// References:
//   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=31775
//   https://gcc.gnu.org/viewcvs?view=rev&revision=124467
static bool ParseLocalSourceName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'L') && ParseSourceName(state) &&
      Optional(ParseDiscriminator(state))) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                     ::= <closure-type-name>
// <closure-type-name> ::= Ul <lambda-sig> E [<(nonnegative) number>] _
// <lambda-sig>        ::= <(parameter) type>+
static bool ParseUnnamedTypeName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;


  int which = -1;

  if (ParseTwoCharToken(state, "Ut") && Optional(ParseNumber(state, &which)) &&
      which <= std::numeric_limits<int>::max() - 2 &&  // Don't overflow.
      ParseOneCharToken(state, '_')) {
    MaybeAppend(state, "{unnamed type#");
    MaybeAppendDecimal(state, 2 + which);
    MaybeAppend(state, "}");
    return true;
  }
  state->parse_state = copy;

  which = -1;
  if (ParseTwoCharToken(state, "Ul") && DisableAppend(state) &&
      OneOrMore(ParseType, state) && RestoreAppend(state, copy.append) &&
      ParseOneCharToken(state, 'E') && Optional(ParseNumber(state, &which)) &&
      which <= std::numeric_limits<int>::max() - 2 &&  // Don't overflow.
      ParseOneCharToken(state, '_')) {
    MaybeAppend(state, "{lambda()#");
    MaybeAppendDecimal(state, 2 + which);
    MaybeAppend(state, "}");
    return true;
  }
  state->parse_state = copy;

  return false;
}

// If "number_out" is non-null, then *number_out is set to the value of the
// parsed number on success.
static bool ParseNumber(State *state, int *number_out) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  bool negative = false;
  if (ParseOneCharToken(state, 'n')) {
    negative = true;
  }
  const char *p = RemainingInput(state);
  uint64_t number = 0;
  for (; *p != '\0'; ++p) {
    if (IsDigit(*p)) {
      number = number * 10 + static_cast<uint64_t>(*p - '0');
    } else {
      break;
    }
  }



  if (negative) {
    number = ~number + 1;
  }
  if (p != RemainingInput(state)) {  // Conversion succeeded.
    state->parse_state.mangled_idx += p - RemainingInput(state);
    if (number_out != nullptr) {

      *number_out = static_cast<int>(number);
    }
    return true;
  }
  return false;
}

// hexadecimal string.
static bool ParseFloatNumber(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  const char *p = RemainingInput(state);
  for (; *p != '\0'; ++p) {
    if (!IsDigit(*p) && !(*p >= 'a' && *p <= 'f')) {
      break;
    }
  }
  if (p != RemainingInput(state)) {  // Conversion succeeded.
    state->parse_state.mangled_idx += p - RemainingInput(state);
    return true;
  }
  return false;
}

// using digits and upper case letters
static bool ParseSeqId(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  const char *p = RemainingInput(state);
  for (; *p != '\0'; ++p) {
    if (!IsDigit(*p) && !(*p >= 'A' && *p <= 'Z')) {
      break;
    }
  }
  if (p != RemainingInput(state)) {  // Conversion succeeded.
    state->parse_state.mangled_idx += p - RemainingInput(state);
    return true;
  }
  return false;
}

static bool ParseIdentifier(State *state, size_t length) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (!AtLeastNumCharsRemaining(RemainingInput(state), length)) {
    return false;
  }
  if (IdentifierIsAnonymousNamespace(state, length)) {
    MaybeAppend(state, "(anonymous namespace)");
  } else {
    MaybeAppendWithLength(state, RemainingInput(state), length);
  }
  state->parse_state.mangled_idx += length;
  return true;
}

//                 ::= cv <type>  # (cast)
//                 ::= v  <digit> <source-name> # vendor extended operator
static bool ParseOperatorName(State *state, int *arity) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (!AtLeastNumCharsRemaining(RemainingInput(state), 2)) {
    return false;
  }

  ParseState copy = state->parse_state;
  if (ParseTwoCharToken(state, "cv") && MaybeAppend(state, "operator ") &&
      EnterNestedName(state) && ParseType(state) &&
      LeaveNestedName(state, copy.nest_level)) {
    if (arity != nullptr) {
      *arity = 1;
    }
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'v') && ParseDigit(state, arity) &&
      ParseSourceName(state)) {
    return true;
  }
  state->parse_state = copy;


  if (!(IsLower(RemainingInput(state)[0]) &&
        IsAlpha(RemainingInput(state)[1]))) {
    return false;
  }

  const AbbrevPair *p;
  for (p = kOperatorList; p->abbrev != nullptr; ++p) {
    if (RemainingInput(state)[0] == p->abbrev[0] &&
        RemainingInput(state)[1] == p->abbrev[1]) {
      if (arity != nullptr) {
        *arity = p->arity;
      }
      MaybeAppend(state, "operator");
      if (IsLower(*p->real_name)) {  // new, delete, etc.
        MaybeAppend(state, " ");
      }
      MaybeAppend(state, p->real_name);
      state->parse_state.mangled_idx += 2;
      return true;
    }
  }
  return false;
}

//                ::= TT <type>
//                ::= TI <type>
//                ::= TS <type>
//                ::= TH <type>  # thread-local
//                ::= Tc <call-offset> <call-offset> <(base) encoding>
//                ::= GV <(object) name>
//                ::= T <call-offset> <(base) encoding>
// G++ extensions:
//                ::= TC <type> <(offset) number> _ <(base) type>
//                ::= TF <type>
//                ::= TJ <type>
//                ::= GR <name>
//                ::= GA <encoding>
//                ::= Th <call-offset> <(base) encoding>
//                ::= Tv <call-offset> <(base) encoding>
//
// Note: we don't care much about them since they don't appear in
// stack traces.  The are special data.
static bool ParseSpecialName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'T') && ParseCharClass(state, "VTISH") &&
      ParseType(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "Tc") && ParseCallOffset(state) &&
      ParseCallOffset(state) && ParseEncoding(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "GV") && ParseName(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'T') && ParseCallOffset(state) &&
      ParseEncoding(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "TC") && ParseType(state) &&
      ParseNumber(state, nullptr) && ParseOneCharToken(state, '_') &&
      DisableAppend(state) && ParseType(state)) {
    RestoreAppend(state, copy.append);
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'T') && ParseCharClass(state, "FJ") &&
      ParseType(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "GR") && ParseName(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "GA") && ParseEncoding(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'T') && ParseCharClass(state, "hv") &&
      ParseCallOffset(state) && ParseEncoding(state)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//               ::= v <v-offset> _
static bool ParseCallOffset(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'h') && ParseNVOffset(state) &&
      ParseOneCharToken(state, '_')) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'v') && ParseVOffset(state) &&
      ParseOneCharToken(state, '_')) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

static bool ParseNVOffset(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  return ParseNumber(state, nullptr);
}

static bool ParseVOffset(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseNumber(state, nullptr) && ParseOneCharToken(state, '_') &&
      ParseNumber(state, nullptr)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

// <base-class-type>
//                  ::= D0 | D1 | D2
// # GCC extensions: "unified" constructor/destructor.  See
// #
// https://github.com/gcc-mirror/gcc/blob/7ad17b583c3643bd4557f29b8391ca7ef08391f5/gcc/cp/mangle.c#L1847
//                  ::= C4 | D4
static bool ParseCtorDtorName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'C')) {
    if (ParseCharClass(state, "1234")) {
      const char *const prev_name =
          state->out + state->parse_state.prev_name_idx;
      MaybeAppendWithLength(state, prev_name,
                            state->parse_state.prev_name_length);
      return true;
    } else if (ParseOneCharToken(state, 'I') && ParseCharClass(state, "12") &&
               ParseClassEnumType(state)) {
      return true;
    }
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'D') && ParseCharClass(state, "0124")) {
    const char *const prev_name = state->out + state->parse_state.prev_name_idx;
    MaybeAppend(state, "~");
    MaybeAppendWithLength(state, prev_name,
                          state->parse_state.prev_name_length);
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                                   # member access (C++0x)
//            ::= DT <expression> E  # decltype of an expression (C++0x)
static bool ParseDecltype(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;

  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'D') && ParseCharClass(state, "tT") &&
      ParseExpression(state) && ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

//        ::= P <type>   # pointer-to
//        ::= R <type>   # reference-to
//        ::= O <type>   # rvalue reference-to (C++0x)
//        ::= C <type>   # complex pair (C 2000)
//        ::= G <type>   # imaginary (C 2000)
//        ::= U <source-name> <type>  # vendor extended type qualifier
//        ::= <builtin-type>
//        ::= <function-type>
//        ::= <class-enum-type>  # note: just an alias for <name>
//        ::= <array-type>
//        ::= <pointer-to-member-type>
//        ::= <template-template-param> <template-args>
//        ::= <template-param>
//        ::= <decltype>
//        ::= <substitution>
//        ::= Dp <type>          # pack expansion of (C++0x)
//        ::= Dv <num-elems> _   # GNU vector extension
//
static bool ParseType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;













  if (ParseCVQualifiers(state)) {
    const bool result = ParseType(state);
    if (!result) state->parse_state = copy;
    return result;
  }
  state->parse_state = copy;




  if (ParseCharClass(state, "OPRCG")) {
    const bool result = ParseType(state);
    if (!result) state->parse_state = copy;
    return result;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "Dp") && ParseType(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'U') && ParseSourceName(state) &&
      ParseType(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseBuiltinType(state) || ParseFunctionType(state) ||
      ParseClassEnumType(state) || ParseArrayType(state) ||
      ParsePointerToMemberType(state) || ParseDecltype(state) ||

      ParseSubstitution(state, /*accept_std=*/false)) {
    return true;
  }

  if (ParseTemplateTemplateParam(state) && ParseTemplateArgs(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTemplateParam(state)) {
    return true;
  }

  if (ParseTwoCharToken(state, "Dv") && ParseNumber(state, nullptr) &&
      ParseOneCharToken(state, '_')) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

// We don't allow empty <CV-qualifiers> to avoid infinite loop in
// ParseType().
static bool ParseCVQualifiers(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  int num_cv_qualifiers = 0;
  num_cv_qualifiers += ParseOneCharToken(state, 'r');
  num_cv_qualifiers += ParseOneCharToken(state, 'V');
  num_cv_qualifiers += ParseOneCharToken(state, 'K');
  return num_cv_qualifiers > 0;
}

//                ::= u <source-name>
//                ::= Dd, etc.  # two-character builtin types
//
// Not supported:
//                ::= DF <number> _ # _FloatN (N bits)
//
static bool ParseBuiltinType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  const AbbrevPair *p;
  for (p = kBuiltinTypeList; p->abbrev != nullptr; ++p) {

    if (p->abbrev[1] == '\0') {
      if (ParseOneCharToken(state, p->abbrev[0])) {
        MaybeAppend(state, p->real_name);
        return true;
      }
    } else if (p->abbrev[2] == '\0' && ParseTwoCharToken(state, p->abbrev)) {
      MaybeAppend(state, p->real_name);
      return true;
    }
  }

  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'u') && ParseSourceName(state)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                                           exception-specification (e.g.,
//                                           noexcept, throw())
//                   ::= DO <expression> E # computed (instantiation-dependent)
//                                           noexcept
//                   ::= Dw <type>+ E      # dynamic exception specification
//                                           with instantiation-dependent types
static bool ParseExceptionSpec(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;

  if (ParseTwoCharToken(state, "Do")) return true;

  ParseState copy = state->parse_state;
  if (ParseTwoCharToken(state, "DO") && ParseExpression(state) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;
  if (ParseTwoCharToken(state, "Dw") && OneOrMore(ParseType, state) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

static bool ParseFunctionType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (Optional(ParseExceptionSpec(state)) && ParseOneCharToken(state, 'F') &&
      Optional(ParseOneCharToken(state, 'Y')) && ParseBareFunctionType(state) &&
      Optional(ParseOneCharToken(state, 'O')) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

static bool ParseBareFunctionType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  DisableAppend(state);
  if (OneOrMore(ParseType, state)) {
    RestoreAppend(state, copy.append);
    MaybeAppend(state, "()");
    return true;
  }
  state->parse_state = copy;
  return false;
}

static bool ParseClassEnumType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  return ParseName(state);
}

//              ::= A [<(dimension) expression>] _ <(element) type>
static bool ParseArrayType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'A') && ParseNumber(state, nullptr) &&
      ParseOneCharToken(state, '_') && ParseType(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'A') && Optional(ParseExpression(state)) &&
      ParseOneCharToken(state, '_') && ParseType(state)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

static bool ParsePointerToMemberType(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'M') && ParseType(state) && ParseType(state)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                  ::= T <parameter-2 non-negative number> _
static bool ParseTemplateParam(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseTwoCharToken(state, "T_")) {
    MaybeAppend(state, "?");  // We don't support template substitutions.
    return true;
  }

  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'T') && ParseNumber(state, nullptr) &&
      ParseOneCharToken(state, '_')) {
    MaybeAppend(state, "?");  // We don't support template substitutions.
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                           ::= <substitution>
static bool ParseTemplateTemplateParam(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  return (ParseTemplateParam(state) ||

          ParseSubstitution(state, /*accept_std=*/false));
}

static bool ParseTemplateArgs(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  DisableAppend(state);
  if (ParseOneCharToken(state, 'I') && OneOrMore(ParseTemplateArg, state) &&
      ParseOneCharToken(state, 'E')) {
    RestoreAppend(state, copy.append);
    MaybeAppend(state, "<>");
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                 ::= <expr-primary>
//                 ::= J <template-arg>* E        # argument pack
//                 ::= X <expression> E
static bool ParseTemplateArg(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'J') && ZeroOrMore(ParseTemplateArg, state) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;




































































  if (ParseLocalSourceName(state) && Optional(ParseTemplateArgs(state))) {
    copy = state->parse_state;
    if (ParseExprCastValue(state) && ParseOneCharToken(state, 'E')) {
      return true;
    }
    state->parse_state = copy;
    return true;
  }


  if (ParseType(state) || ParseExprPrimary(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'X') && ParseExpression(state) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                   ::= <decltype>
//                   ::= <substitution>
static inline bool ParseUnresolvedType(State *state) {

  return (ParseTemplateParam(state) && Optional(ParseTemplateArgs(state))) ||
         ParseDecltype(state) || ParseSubstitution(state, /*accept_std=*/false);
}

static inline bool ParseSimpleId(State *state) {



  return ParseSourceName(state) && Optional(ParseTemplateArgs(state));
}

//                        ::= on <operator-name> [<template-args>]
//                        ::= dn <destructor-name>
static bool ParseBaseUnresolvedName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;

  if (ParseSimpleId(state)) {
    return true;
  }

  ParseState copy = state->parse_state;
  if (ParseTwoCharToken(state, "on") && ParseOperatorName(state, nullptr) &&
      Optional(ParseTemplateArgs(state))) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "dn") &&
      (ParseUnresolvedType(state) || ParseSimpleId(state))) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

//                   ::= sr <unresolved-type> <base-unresolved-name>
//                   ::= srN <unresolved-type> <unresolved-qualifier-level>+ E
//                         <base-unresolved-name>
//                   ::= [gs] sr <unresolved-qualifier-level>+ E
//                         <base-unresolved-name>
static bool ParseUnresolvedName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;

  ParseState copy = state->parse_state;
  if (Optional(ParseTwoCharToken(state, "gs")) &&
      ParseBaseUnresolvedName(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "sr") && ParseUnresolvedType(state) &&
      ParseBaseUnresolvedName(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "sr") && ParseOneCharToken(state, 'N') &&
      ParseUnresolvedType(state) &&
      OneOrMore(/* <unresolved-qualifier-level> ::= */ ParseSimpleId, state) &&
      ParseOneCharToken(state, 'E') && ParseBaseUnresolvedName(state)) {
    return true;
  }
  state->parse_state = copy;

  if (Optional(ParseTwoCharToken(state, "gs")) &&
      ParseTwoCharToken(state, "sr") &&
      OneOrMore(/* <unresolved-qualifier-level> ::= */ ParseSimpleId, state) &&
      ParseOneCharToken(state, 'E') && ParseBaseUnresolvedName(state)) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

//              ::= <2-ary operator-name> <expression> <expression>
//              ::= <3-ary operator-name> <expression> <expression> <expression>
//              ::= cl <expression>+ E
//              ::= cp <simple-id> <expression>* E # Clang-specific.
//              ::= cv <type> <expression>      # type (expression)
//              ::= cv <type> _ <expression>* E # type (expr-list)
//              ::= st <type>
//              ::= <template-param>
//              ::= <function-param>
//              ::= <expr-primary>
//              ::= dt <expression> <unresolved-name> # expr.name
//              ::= pt <expression> <unresolved-name> # expr->name
//              ::= sp <expression>         # argument pack expansion
//              ::= sr <type> <unqualified-name> <template-args>
//              ::= sr <type> <unqualified-name>
// <function-param> ::= fp <(top-level) CV-qualifiers> _
//                  ::= fp <(top-level) CV-qualifiers> <number> _
//                  ::= fL <number> p <(top-level) CV-qualifiers> _
//                  ::= fL <number> p <(top-level) CV-qualifiers> <number> _
static bool ParseExpression(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseTemplateParam(state) || ParseExprPrimary(state)) {
    return true;
  }

  ParseState copy = state->parse_state;

  if (ParseTwoCharToken(state, "cl") && OneOrMore(ParseExpression, state) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;


  if (ParseTwoCharToken(state, "cp") && ParseSimpleId(state) &&
      ZeroOrMore(ParseExpression, state) && ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "fp") && Optional(ParseCVQualifiers(state)) &&
      Optional(ParseNumber(state, nullptr)) && ParseOneCharToken(state, '_')) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "fL") && Optional(ParseNumber(state, nullptr)) &&
      ParseOneCharToken(state, 'p') && Optional(ParseCVQualifiers(state)) &&
      Optional(ParseNumber(state, nullptr)) && ParseOneCharToken(state, '_')) {
    return true;
  }
  state->parse_state = copy;








  if (ParseTwoCharToken(state, "cv")) {
    if (ParseType(state)) {
      ParseState copy2 = state->parse_state;
      if (ParseOneCharToken(state, '_') && ZeroOrMore(ParseExpression, state) &&
          ParseOneCharToken(state, 'E')) {
        return true;
      }
      state->parse_state = copy2;
      if (ParseExpression(state)) {
        return true;
      }
    }
  } else {





    int arity = -1;
    if (ParseOperatorName(state, &arity) &&
        arity > 0 &&  // 0 arity => disabled.
        (arity < 3 || ParseExpression(state)) &&
        (arity < 2 || ParseExpression(state)) &&
        (arity < 1 || ParseExpression(state))) {
      return true;
    }
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "st") && ParseType(state)) {
    return true;
  }
  state->parse_state = copy;

  if ((ParseTwoCharToken(state, "dt") || ParseTwoCharToken(state, "pt")) &&
      ParseExpression(state) && ParseType(state)) {
    return true;
  }
  state->parse_state = copy;



  if (ParseTwoCharToken(state, "ds") && ParseExpression(state) &&
      ParseExpression(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseTwoCharToken(state, "sp") && ParseExpression(state)) {
    return true;
  }
  state->parse_state = copy;

  return ParseUnresolvedName(state);
}

//                ::= L <type> <(value) float> E
//                ::= L <mangled-name> E
//                // A bug in g++'s C++ ABI version 2 (-fabi-version=2).
//                ::= LZ <encoding> E
//
// Warning, subtle: the "bug" LZ production above is ambiguous with the first
// production where <type> starts with <local-name>, which can lead to
// exponential backtracking in two scenarios:
//
// - When whatever follows the E in the <local-name> in the first production is
//   not a name, we backtrack the whole <encoding> and re-parse the whole thing.
//
// - When whatever follows the <local-name> in the first production is not a
//   number and this <expr-primary> may be followed by a name, we backtrack the
//   <name> and re-parse it.
//
// Moreover this ambiguity isn't always resolved -- for example, the following
// has two different parses:
//
//   _ZaaILZ4aoeuE1x1EvE
//   => operator&&<aoeu, x, E, void>
//   => operator&&<(aoeu::x)(1), void>
//
// To resolve this, we just do what GCC's demangler does, and refuse to parse
// casts to <local-name> types.
static bool ParseExprPrimary(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;


  if (ParseTwoCharToken(state, "LZ")) {
    if (ParseEncoding(state) && ParseOneCharToken(state, 'E')) {
      return true;
    }

    state->parse_state = copy;
    return false;
  }

  if (ParseOneCharToken(state, 'L') && ParseType(state) &&
      ParseExprCastValue(state)) {
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'L') && ParseMangledName(state) &&
      ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

static bool ParseExprCastValue(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;



  ParseState copy = state->parse_state;
  if (ParseNumber(state, nullptr) && ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;

  if (ParseFloatNumber(state) && ParseOneCharToken(state, 'E')) {
    return true;
  }
  state->parse_state = copy;

  return false;
}

//              ::= Z <(function) encoding> E s [<discriminator>]
//
// Parsing a common prefix of these two productions together avoids an
// exponential blowup of backtracking.  Parse like:
//   <local-name> := Z <encoding> E <local-name-suffix>
//   <local-name-suffix> ::= s [<discriminator>]
//                       ::= <name> [<discriminator>]

static bool ParseLocalNameSuffix(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;

  if (MaybeAppend(state, "::") && ParseName(state) &&
      Optional(ParseDiscriminator(state))) {
    return true;
  }



  if (state->parse_state.append) {
    state->out[state->parse_state.out_cur_idx - 2] = '\0';
  }

  return ParseOneCharToken(state, 's') && Optional(ParseDiscriminator(state));
}

static bool ParseLocalName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'Z') && ParseEncoding(state) &&
      ParseOneCharToken(state, 'E') && ParseLocalNameSuffix(state)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

static bool ParseDiscriminator(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, '_') && ParseNumber(state, nullptr)) {
    return true;
  }
  state->parse_state = copy;
  return false;
}

//                ::= S <seq-id> _
//                ::= St, etc.
//
// "St" is special in that it's not valid as a standalone name, and it *is*
// allowed to precede a name without being wrapped in "N...E".  This means that
// if we accept it on its own, we can accept "St1a" and try to parse
// template-args, then fail and backtrack, accept "St" on its own, then "1a" as
// an unqualified name and re-parse the same template-args.  To block this
// exponential backtracking, we disable it with 'accept_std=false' in
// problematic contexts.
static bool ParseSubstitution(State *state, bool accept_std) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseTwoCharToken(state, "S_")) {
    MaybeAppend(state, "?");  // We don't support substitutions.
    return true;
  }

  ParseState copy = state->parse_state;
  if (ParseOneCharToken(state, 'S') && ParseSeqId(state) &&
      ParseOneCharToken(state, '_')) {
    MaybeAppend(state, "?");  // We don't support substitutions.
    return true;
  }
  state->parse_state = copy;

  if (ParseOneCharToken(state, 'S')) {
    const AbbrevPair *p;
    for (p = kSubstitutionList; p->abbrev != nullptr; ++p) {
      if (RemainingInput(state)[0] == p->abbrev[1] &&
          (accept_std || p->abbrev[1] != 't')) {
        MaybeAppend(state, "std");
        if (p->real_name[0] != '\0') {
          MaybeAppend(state, "::");
          MaybeAppend(state, p->real_name);
        }
        ++state->parse_state.mangled_idx;
        return true;
      }
    }
  }
  state->parse_state = copy;
  return false;
}

// or version suffix.  Returns true only if all of "mangled_cur" was consumed.
static bool ParseTopLevelMangledName(State *state) {
  ComplexityGuard guard(state);
  if (guard.IsTooComplex()) return false;
  if (ParseMangledName(state)) {
    if (RemainingInput(state)[0] != '\0') {

      if (IsFunctionCloneSuffix(RemainingInput(state))) {
        return true;
      }


      if (RemainingInput(state)[0] == '@') {
        MaybeAppend(state, RemainingInput(state));
        return true;
      }
      return false;  // Unconsumed suffix.
    }
    return true;
  }
  return false;
}

static bool Overflowed(const State *state) {
  return state->parse_state.out_cur_idx >= state->out_end_idx;
}

bool Demangle(const char* mangled, char* out, size_t out_size) {
  State state;
  InitState(&state, mangled, out, out_size);
  return ParseTopLevelMangledName(&state) && !Overflowed(&state) &&
         state.parse_state.out_cur_idx > 0;
}

}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl
