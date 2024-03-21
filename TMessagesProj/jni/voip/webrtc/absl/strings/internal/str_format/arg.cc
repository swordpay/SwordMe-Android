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

// POSIX spec:
//   http://pubs.opengroup.org/onlinepubs/009695399/functions/fprintf.html
//
#include "absl/strings/internal/str_format/arg.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <string>
#include <type_traits>

#include "absl/base/port.h"
#include "absl/strings/internal/str_format/float_conversion.h"
#include "absl/strings/numbers.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace str_format_internal {
namespace {

void ReducePadding(string_view s, size_t *capacity) {
  *capacity = Excess(s.size(), *capacity);
}

void ReducePadding(size_t n, size_t *capacity) {
  *capacity = Excess(n, *capacity);
}

template <typename T>
struct MakeUnsigned : std::make_unsigned<T> {};
template <>
struct MakeUnsigned<absl::int128> {
  using type = absl::uint128;
};
template <>
struct MakeUnsigned<absl::uint128> {
  using type = absl::uint128;
};

template <typename T>
struct IsSigned : std::is_signed<T> {};
template <>
struct IsSigned<absl::int128> : std::true_type {};
template <>
struct IsSigned<absl::uint128> : std::false_type {};

// Call one of the PrintAs* routines after construction once.
// Use with_neg_and_zero/without_neg_or_zero/is_negative to access the results.
class IntDigits {
 public:


  template <typename T>
  void PrintAsOct(T v) {
    static_assert(!IsSigned<T>::value, "");
    char *p = storage_ + sizeof(storage_);
    do {
      *--p = static_cast<char>('0' + (static_cast<size_t>(v) & 7));
      v >>= 3;
    } while (v);
    start_ = p;
    size_ = static_cast<size_t>(storage_ + sizeof(storage_) - p);
  }


  template <typename T>
  void PrintAsDec(T v) {
    static_assert(std::is_integral<T>::value, "");
    start_ = storage_;
    size_ = static_cast<size_t>(numbers_internal::FastIntToBuffer(v, storage_) -
                                storage_);
  }

  void PrintAsDec(int128 v) {
    auto u = static_cast<uint128>(v);
    bool add_neg = false;
    if (v < 0) {
      add_neg = true;
      u = uint128{} - u;
    }
    PrintAsDec(u, add_neg);
  }

  void PrintAsDec(uint128 v, bool add_neg = false) {


    char *p = storage_ + sizeof(storage_);
    do {
      p -= 2;
      numbers_internal::PutTwoDigits(static_cast<size_t>(v % 100), p);
      v /= 100;
    } while (v);
    if (p[0] == '0') {

      ++p;
    }
    if (add_neg) {
      *--p = '-';
    }
    size_ = static_cast<size_t>(storage_ + sizeof(storage_) - p);
    start_ = p;
  }


  template <typename T>
  void PrintAsHexLower(T v) {
    static_assert(!IsSigned<T>::value, "");
    char *p = storage_ + sizeof(storage_);

    do {
      p -= 2;
      constexpr const char* table = numbers_internal::kHexTable;
      std::memcpy(p, table + 2 * (static_cast<size_t>(v) & 0xFF), 2);
      if (sizeof(T) == 1) break;
      v >>= 8;
    } while (v);
    if (p[0] == '0') {

      ++p;
    }
    start_ = p;
    size_ = static_cast<size_t>(storage_ + sizeof(storage_) - p);
  }


  template <typename T>
  void PrintAsHexUpper(T v) {
    static_assert(!IsSigned<T>::value, "");
    char *p = storage_ + sizeof(storage_);

    do {
      *--p = "0123456789ABCDEF"[static_cast<size_t>(v) & 15];
      v >>= 4;
    } while (v);
    start_ = p;
    size_ = static_cast<size_t>(storage_ + sizeof(storage_) - p);
  }


  string_view with_neg_and_zero() const { return {start_, size_}; }


  string_view without_neg_or_zero() const {
    static_assert('-' < '0', "The check below verifies both.");
    size_t advance = start_[0] <= '0' ? 1 : 0;
    return {start_ + advance, size_ - advance};
  }

  bool is_negative() const { return start_[0] == '-'; }

 private:
  const char *start_;
  size_t size_;

  char storage_[128 / 3 + 1 + 1];
};

// the '#' flag is specified to modify the precision for 'o' conversions.
string_view BaseIndicator(const IntDigits &as_digits,
                          const FormatConversionSpecImpl conv) {

  bool alt = conv.has_alt_flag() ||
             conv.conversion_char() == FormatConversionCharInternal::p;
  bool hex = (conv.conversion_char() == FormatConversionCharInternal::x ||
              conv.conversion_char() == FormatConversionCharInternal::X ||
              conv.conversion_char() == FormatConversionCharInternal::p);



  if (alt && hex && !as_digits.without_neg_or_zero().empty()) {
    return conv.conversion_char() == FormatConversionCharInternal::X ? "0X"
                                                                     : "0x";
  }
  return {};
}

string_view SignColumn(bool neg, const FormatConversionSpecImpl conv) {
  if (conv.conversion_char() == FormatConversionCharInternal::d ||
      conv.conversion_char() == FormatConversionCharInternal::i) {
    if (neg) return "-";
    if (conv.has_show_pos_flag()) return "+";
    if (conv.has_sign_col_flag()) return " ";
  }
  return {};
}

bool ConvertCharImpl(char v,
                     const FormatConversionSpecImpl conv,
                     FormatSinkImpl* sink) {
  size_t fill = 0;
  if (conv.width() >= 0)
    fill = static_cast<size_t>(conv.width());
  ReducePadding(1, &fill);
  if (!conv.has_left_flag()) sink->Append(fill, ' ');
  sink->Append(1, v);
  if (conv.has_left_flag()) sink->Append(fill, ' ');
  return true;
}

bool ConvertIntImplInnerSlow(const IntDigits &as_digits,
                             const FormatConversionSpecImpl conv,
                             FormatSinkImpl *sink) {


  size_t fill = 0;
  if (conv.width() >= 0)
    fill = static_cast<size_t>(conv.width());

  string_view formatted = as_digits.without_neg_or_zero();
  ReducePadding(formatted, &fill);

  string_view sign = SignColumn(as_digits.is_negative(), conv);
  ReducePadding(sign, &fill);

  string_view base_indicator = BaseIndicator(as_digits, conv);
  ReducePadding(base_indicator, &fill);

  bool precision_specified = conv.precision() >= 0;
  size_t precision =
      precision_specified ? static_cast<size_t>(conv.precision()) : size_t{1};

  if (conv.has_alt_flag() &&
      conv.conversion_char() == FormatConversionCharInternal::o) {



    if (formatted.empty() || *formatted.begin() != '0') {
      size_t needed = formatted.size() + 1;
      precision = std::max(precision, needed);
    }
  }

  size_t num_zeroes = Excess(formatted.size(), precision);
  ReducePadding(num_zeroes, &fill);

  size_t num_left_spaces = !conv.has_left_flag() ? fill : 0;
  size_t num_right_spaces = conv.has_left_flag() ? fill : 0;



  if (!precision_specified && conv.has_zero_flag()) {
    num_zeroes += num_left_spaces;
    num_left_spaces = 0;
  }

  sink->Append(num_left_spaces, ' ');
  sink->Append(sign);
  sink->Append(base_indicator);
  sink->Append(num_zeroes, '0');
  sink->Append(formatted);
  sink->Append(num_right_spaces, ' ');
  return true;
}

template <typename T,
          typename std::enable_if<(std::is_integral<T>::value &&
                                   std::is_signed<T>::value) ||
                                      std::is_same<T, int128>::value,
                                  int>::type = 0>
constexpr auto ConvertV(T) {
  return FormatConversionCharInternal::d;
}

template <typename T,
          typename std::enable_if<(std::is_integral<T>::value &&
                                   std::is_unsigned<T>::value) ||
                                      std::is_same<T, uint128>::value,
                                  int>::type = 0>
constexpr auto ConvertV(T) {
  return FormatConversionCharInternal::u;
}

template <typename T>
bool ConvertIntArg(T v, FormatConversionSpecImpl conv, FormatSinkImpl *sink) {
  using U = typename MakeUnsigned<T>::type;
  IntDigits as_digits;

  if (conv.conversion_char() == FormatConversionCharInternal::v) {
    conv.set_conversion_char(ConvertV(T{}));
  }




  switch (static_cast<uint8_t>(conv.conversion_char())) {
    case static_cast<uint8_t>(FormatConversionCharInternal::c):
      return ConvertCharImpl(static_cast<char>(v), conv, sink);

    case static_cast<uint8_t>(FormatConversionCharInternal::o):
      as_digits.PrintAsOct(static_cast<U>(v));
      break;

    case static_cast<uint8_t>(FormatConversionCharInternal::x):
      as_digits.PrintAsHexLower(static_cast<U>(v));
      break;
    case static_cast<uint8_t>(FormatConversionCharInternal::X):
      as_digits.PrintAsHexUpper(static_cast<U>(v));
      break;

    case static_cast<uint8_t>(FormatConversionCharInternal::u):
      as_digits.PrintAsDec(static_cast<U>(v));
      break;

    case static_cast<uint8_t>(FormatConversionCharInternal::d):
    case static_cast<uint8_t>(FormatConversionCharInternal::i):
      as_digits.PrintAsDec(v);
      break;

    case static_cast<uint8_t>(FormatConversionCharInternal::a):
    case static_cast<uint8_t>(FormatConversionCharInternal::e):
    case static_cast<uint8_t>(FormatConversionCharInternal::f):
    case static_cast<uint8_t>(FormatConversionCharInternal::g):
    case static_cast<uint8_t>(FormatConversionCharInternal::A):
    case static_cast<uint8_t>(FormatConversionCharInternal::E):
    case static_cast<uint8_t>(FormatConversionCharInternal::F):
    case static_cast<uint8_t>(FormatConversionCharInternal::G):
      return ConvertFloatImpl(static_cast<double>(v), conv, sink);

    default:
      ABSL_ASSUME(false);
  }

  if (conv.is_basic()) {
    sink->Append(as_digits.with_neg_and_zero());
    return true;
  }
  return ConvertIntImplInnerSlow(as_digits, conv, sink);
}

template <typename T>
bool ConvertFloatArg(T v, FormatConversionSpecImpl conv, FormatSinkImpl *sink) {
  if (conv.conversion_char() == FormatConversionCharInternal::v) {
    conv.set_conversion_char(FormatConversionCharInternal::g);
  }

  return FormatConversionCharIsFloat(conv.conversion_char()) &&
         ConvertFloatImpl(v, conv, sink);
}

inline bool ConvertStringArg(string_view v, const FormatConversionSpecImpl conv,
                             FormatSinkImpl *sink) {
  if (conv.is_basic()) {
    sink->Append(v);
    return true;
  }
  return sink->PutPaddedString(v, conv.width(), conv.precision(),
                               conv.has_left_flag());
}

}  // namespace

bool ConvertBoolArg(bool v, FormatSinkImpl *sink) {
  if (v) {
    sink->Append("true");
  } else {
    sink->Append("false");
  }
  return true;
}

StringConvertResult FormatConvertImpl(const std::string &v,
                                      const FormatConversionSpecImpl conv,
                                      FormatSinkImpl *sink) {
  return {ConvertStringArg(v, conv, sink)};
}

StringConvertResult FormatConvertImpl(string_view v,
                                      const FormatConversionSpecImpl conv,
                                      FormatSinkImpl *sink) {
  return {ConvertStringArg(v, conv, sink)};
}

ArgConvertResult<FormatConversionCharSetUnion(
    FormatConversionCharSetInternal::s, FormatConversionCharSetInternal::p)>
FormatConvertImpl(const char *v, const FormatConversionSpecImpl conv,
                  FormatSinkImpl *sink) {
  if (conv.conversion_char() == FormatConversionCharInternal::p)
    return {FormatConvertImpl(VoidPtr(v), conv, sink).value};
  size_t len;
  if (v == nullptr) {
    len = 0;
  } else if (conv.precision() < 0) {
    len = std::strlen(v);
  } else {

    len = static_cast<size_t>(std::find(v, v + conv.precision(), '\0') - v);
  }
  return {ConvertStringArg(string_view(v, len), conv, sink)};
}

ArgConvertResult<FormatConversionCharSetInternal::p> FormatConvertImpl(
    VoidPtr v, const FormatConversionSpecImpl conv, FormatSinkImpl *sink) {
  if (!v.value) {
    sink->Append("(nil)");
    return {true};
  }
  IntDigits as_digits;
  as_digits.PrintAsHexLower(v.value);
  return {ConvertIntImplInnerSlow(as_digits, conv, sink)};
}

FloatingConvertResult FormatConvertImpl(float v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertFloatArg(v, conv, sink)};
}
FloatingConvertResult FormatConvertImpl(double v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertFloatArg(v, conv, sink)};
}
FloatingConvertResult FormatConvertImpl(long double v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertFloatArg(v, conv, sink)};
}

CharConvertResult FormatConvertImpl(char v, const FormatConversionSpecImpl conv,
                                    FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
CharConvertResult FormatConvertImpl(signed char v,
                                    const FormatConversionSpecImpl conv,
                                    FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
CharConvertResult FormatConvertImpl(unsigned char v,
                                    const FormatConversionSpecImpl conv,
                                    FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}

IntegralConvertResult FormatConvertImpl(short v,  // NOLINT
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(unsigned short v,  // NOLINT
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(int v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(unsigned v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(long v,  // NOLINT
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(unsigned long v,  // NOLINT
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(long long v,  // NOLINT
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(unsigned long long v,  // NOLINT
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(absl::int128 v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}
IntegralConvertResult FormatConvertImpl(absl::uint128 v,
                                        const FormatConversionSpecImpl conv,
                                        FormatSinkImpl *sink) {
  return {ConvertIntArg(v, conv, sink)};
}

ABSL_INTERNAL_FORMAT_DISPATCH_OVERLOADS_EXPAND_();



}  // namespace str_format_internal

ABSL_NAMESPACE_END
}  // namespace absl
