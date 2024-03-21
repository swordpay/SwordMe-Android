// Copyright 2020 The Abseil Authors.
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

#ifndef ABSL_STRINGS_INTERNAL_STR_FORMAT_CHECKER_H_
#define ABSL_STRINGS_INTERNAL_STR_FORMAT_CHECKER_H_

#include "absl/base/attributes.h"
#include "absl/strings/internal/str_format/arg.h"
#include "absl/strings/internal/str_format/extension.h"


#ifndef ABSL_INTERNAL_ENABLE_FORMAT_CHECKER
// We disable format checker under vscode intellisense compilation.
// See https://github.com/microsoft/vscode-cpptools/issues/3683 for
// more details.
#if ABSL_HAVE_ATTRIBUTE(enable_if) && !defined(__native_client__) && \
    !defined(__INTELLISENSE__)
#define ABSL_INTERNAL_ENABLE_FORMAT_CHECKER 1
#endif  // ABSL_HAVE_ATTRIBUTE(enable_if) && !defined(__native_client__) &&

#endif  // ABSL_INTERNAL_ENABLE_FORMAT_CHECKER

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace str_format_internal {

constexpr bool AllOf() { return true; }

template <typename... T>
constexpr bool AllOf(bool b, T... t) {
  return b && AllOf(t...);
}

#ifdef ABSL_INTERNAL_ENABLE_FORMAT_CHECKER

constexpr bool ContainsChar(const char* chars, char c) {
  return *chars == c || (*chars && ContainsChar(chars + 1, c));
}

struct ConvList {
  const FormatConversionCharSet* array;
  int count;



  constexpr FormatConversionCharSet operator[](int i) const {
    return i < count ? array[i] : FormatConversionCharSet{};
  }

  constexpr ConvList without_front() const {
    return count != 0 ? ConvList{array + 1, count - 1} : *this;
  }
};

template <size_t count>
struct ConvListT {

  FormatConversionCharSet list[count ? count : 1];
};

constexpr char GetChar(string_view str, size_t index) {
  return index < str.size() ? str[index] : char{};
}

constexpr string_view ConsumeFront(string_view str, size_t len = 1) {
  return len <= str.size() ? string_view(str.data() + len, str.size() - len)
                           : string_view();
}

constexpr string_view ConsumeAnyOf(string_view format, const char* chars) {
  while (ContainsChar(chars, GetChar(format, 0))) {
    format = ConsumeFront(format);
  }
  return format;
}

constexpr bool IsDigit(char c) { return c >= '0' && c <= '9'; }

// It encapsulates the two return values we need there.
struct Integer {
  string_view format;
  int value;


  constexpr Integer ConsumePositionalDollar() const {
    if (GetChar(format, 0) == '$') {
      return Integer{ConsumeFront(format), value};
    } else {
      return Integer{format, 0};
    }
  }
};

constexpr Integer ParseDigits(string_view format) {
  int value = 0;
  while (IsDigit(GetChar(format, 0))) {
    value = 10 * value + GetChar(format, 0) - '0';
    format = ConsumeFront(format);
  }

  return Integer{format, value};
}

// The parsing also consumes the '$'.
constexpr Integer ParsePositional(string_view format) {
  return ParseDigits(format).ConsumePositionalDollar();
}

// See ConvParser::Run() for post conditions.
class ConvParser {
  constexpr ConvParser SetFormat(string_view format) const {
    return ConvParser(format, args_, error_, arg_position_, is_positional_);
  }

  constexpr ConvParser SetArgs(ConvList args) const {
    return ConvParser(format_, args, error_, arg_position_, is_positional_);
  }

  constexpr ConvParser SetError(bool error) const {
    return ConvParser(format_, args_, error_ || error, arg_position_,
                      is_positional_);
  }

  constexpr ConvParser SetArgPosition(int arg_position) const {
    return ConvParser(format_, args_, error_, arg_position, is_positional_);
  }


  constexpr ConvParser ConsumeNextArg(char conv) const {
    return SetArgs(args_.without_front()).SetError(!Contains(args_[0], conv));
  }



  constexpr ConvParser VerifyPositional(Integer i, char conv) const {
    return SetFormat(i.format).SetError(!Contains(args_[i.value - 1], conv));
  }

  constexpr ConvParser ParseArgPosition(Integer arg) const {
    return SetFormat(arg.format).SetArgPosition(arg.value);
  }

  constexpr ConvParser ParseFlags() const {
    return SetFormat(ConsumeAnyOf(format_, "-+ #0"));
  }



  constexpr ConvParser ParseWidth() const {
    char first_char = GetChar(format_, 0);

    if (IsDigit(first_char)) {
      return SetFormat(ParseDigits(format_).format);
    } else if (first_char == '*') {
      if (is_positional_) {
        return VerifyPositional(ParsePositional(ConsumeFront(format_)), '*');
      } else {
        return SetFormat(ConsumeFront(format_)).ConsumeNextArg('*');
      }
    } else {
      return *this;
    }
  }



  constexpr ConvParser ParsePrecision() const {
    if (GetChar(format_, 0) != '.') {
      return *this;
    } else if (GetChar(format_, 1) == '*') {
      if (is_positional_) {
        return VerifyPositional(ParsePositional(ConsumeFront(format_, 2)), '*');
      } else {
        return SetFormat(ConsumeFront(format_, 2)).ConsumeNextArg('*');
      }
    } else {
      return SetFormat(ParseDigits(ConsumeFront(format_)).format);
    }
  }

  constexpr ConvParser ParseLength() const {
    return SetFormat(ConsumeAnyOf(format_, "lLhjztq"));
  }


  constexpr ConvParser ParseConversion() const {
    char first_char = GetChar(format_, 0);

    if (first_char == 'v' && *(format_.data() - 1) != '%') {
      return SetError(true);
    }

    if (is_positional_) {
      return VerifyPositional({ConsumeFront(format_), arg_position_},
                              first_char);
    } else {
      return ConsumeNextArg(first_char).SetFormat(ConsumeFront(format_));
    }
  }

  constexpr ConvParser(string_view format, ConvList args, bool error,
                       int arg_position, bool is_positional)
      : format_(format),
        args_(args),
        error_(error),
        arg_position_(arg_position),
        is_positional_(is_positional) {}

 public:
  constexpr ConvParser(string_view format, ConvList args, bool is_positional)
      : format_(format),
        args_(args),
        error_(false),
        arg_position_(0),
        is_positional_(is_positional) {}



  constexpr ConvParser Run() const {
    ConvParser parser = *this;

    if (is_positional_) {
      parser = ParseArgPosition(ParsePositional(format_));
    }

    return parser.ParseFlags()
        .ParseWidth()
        .ParsePrecision()
        .ParseLength()
        .ParseConversion();
  }

  constexpr string_view format() const { return format_; }
  constexpr ConvList args() const { return args_; }
  constexpr bool error() const { return error_; }
  constexpr bool is_positional() const { return is_positional_; }

 private:
  string_view format_;


  ConvList args_;
  bool error_;


  int arg_position_;


  bool is_positional_;
};

// See FormatParser::Run().
class FormatParser {
  static constexpr bool FoundPercent(string_view format) {
    return format.empty() ||
           (GetChar(format, 0) == '%' && GetChar(format, 1) != '%');
  }



  static constexpr string_view ConsumeNonPercentInner(string_view format) {
    int limit = 20;
    while (!FoundPercent(format) && limit != 0) {
      size_t len = 0;

      if (GetChar(format, 0) == '%' && GetChar(format, 1) == '%') {
        len = 2;
      } else {
        len = 1;
      }

      format = ConsumeFront(format, len);
      --limit;
    }

    return format;
  }


  static constexpr string_view ConsumeNonPercent(string_view format) {
    while (!FoundPercent(format)) {
      format = ConsumeNonPercentInner(format);
    }

    return format;
  }

  static constexpr bool IsPositional(string_view format) {
    while (IsDigit(GetChar(format, 0))) {
      format = ConsumeFront(format);
    }

    return GetChar(format, 0) == '$';
  }

  constexpr bool RunImpl(bool is_positional) const {



    return (format_.empty() && (is_positional || args_.count == 0)) ||
           (!format_.empty() &&
            ValidateArg(
                ConvParser(ConsumeFront(format_), args_, is_positional).Run()));
  }

  constexpr bool ValidateArg(ConvParser conv) const {
    return !conv.error() && FormatParser(conv.format(), conv.args())
                                .RunImpl(conv.is_positional());
  }

 public:
  constexpr FormatParser(string_view format, ConvList args)
      : format_(ConsumeNonPercent(format)), args_(args) {}




  constexpr bool Run() const {
    return RunImpl(!format_.empty() && IsPositional(ConsumeFront(format_)));
  }

 private:
  string_view format_;



  ConvList args_;
};

template <FormatConversionCharSet... C>
constexpr bool ValidFormatImpl(string_view format) {
  return FormatParser(format,
                      {ConvListT<sizeof...(C)>{{C...}}.list, sizeof...(C)})
      .Run();
}

#endif  // ABSL_INTERNAL_ENABLE_FORMAT_CHECKER

}  // namespace str_format_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_STR_FORMAT_CHECKER_H_
