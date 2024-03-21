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

#include "absl/strings/internal/str_format/float_conversion.h"

#include <string.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <string>

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/optimization.h"
#include "absl/functional/function_ref.h"
#include "absl/meta/type_traits.h"
#include "absl/numeric/bits.h"
#include "absl/numeric/int128.h"
#include "absl/numeric/internal/representation.h"
#include "absl/strings/numbers.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace str_format_internal {

namespace {

using ::absl::numeric_internal::IsDoubleDouble;

// To do so it needs to allocate memory on the stack.
// `StackArray` will allocate memory on the stack in the form of a uint32_t
// array and call the provided callback with said memory.
// It will allocate memory in increments of 512 bytes. We could allocate the
// largest needed unconditionally, but that is more than we need in most of
// cases. This way we use less stack in the common cases.
class StackArray {
  using Func = absl::FunctionRef<void(absl::Span<uint32_t>)>;
  static constexpr size_t kStep = 512 / sizeof(uint32_t);



  static constexpr size_t kNumSteps = 5;



  template <size_t steps>
  ABSL_ATTRIBUTE_NOINLINE static void RunWithCapacityImpl(Func f) {
    uint32_t values[steps * kStep]{};
    f(absl::MakeSpan(values));
  }

 public:
  static constexpr size_t kMaxCapacity = kStep * kNumSteps;

  static void RunWithCapacity(size_t capacity, Func f) {
    assert(capacity <= kMaxCapacity);
    const size_t step = (capacity + kStep - 1) / kStep;
    assert(step <= kNumSteps);
    switch (step) {
      case 1:
        return RunWithCapacityImpl<1>(f);
      case 2:
        return RunWithCapacityImpl<2>(f);
      case 3:
        return RunWithCapacityImpl<3>(f);
      case 4:
        return RunWithCapacityImpl<4>(f);
      case 5:
        return RunWithCapacityImpl<5>(f);
    }

    assert(false && "Invalid capacity");
  }
};

// the carry.
// Requires: `0 <= carry <= 9`
template <typename Int>
inline char MultiplyBy10WithCarry(Int* v, char carry) {
  using BiggerInt = absl::conditional_t<sizeof(Int) == 4, uint64_t, uint128>;
  BiggerInt tmp =
      10 * static_cast<BiggerInt>(*v) + static_cast<BiggerInt>(carry);
  *v = static_cast<Int>(tmp);
  return static_cast<char>(tmp >> (sizeof(Int) * 8));
}

// Stores the quotient in `*v` and returns the remainder.
// Requires: `0 <= carry <= 9`
inline char DivideBy10WithCarry(uint64_t* v, char carry) {
  constexpr uint64_t divisor = 10;

  constexpr uint64_t chunk_quotient = (uint64_t{1} << 63) / (divisor / 2);
  constexpr uint64_t chunk_remainder = uint64_t{} - chunk_quotient * divisor;

  const uint64_t carry_u64 = static_cast<uint64_t>(carry);
  const uint64_t mod = *v % divisor;
  const uint64_t next_carry = chunk_remainder * carry_u64 + mod;
  *v = *v / divisor + carry_u64 * chunk_quotient + next_carry / divisor;
  return static_cast<char>(next_carry % divisor);
}

using MaxFloatType =
    typename std::conditional<IsDoubleDouble(), double, long double>::type;

// where `v` and `exp` are both positive integers.
// It generates the digits from the left (ie the most significant digit first)
// to allow for direct printing into the sink.
//
// Requires `0 <= exp` and `exp <= numeric_limits<MaxFloatType>::max_exponent`.
class BinaryToDecimal {
  static constexpr size_t ChunksNeeded(int exp) {



    return static_cast<size_t>((128 + exp + 31) / 32 * 11 / 10);
  }

 public:


  static void RunConversion(uint128 v, int exp,
                            absl::FunctionRef<void(BinaryToDecimal)> f) {
    assert(exp > 0);
    assert(exp <= std::numeric_limits<MaxFloatType>::max_exponent);
    static_assert(
        StackArray::kMaxCapacity >=
            ChunksNeeded(std::numeric_limits<MaxFloatType>::max_exponent),
        "");

    StackArray::RunWithCapacity(
        ChunksNeeded(exp),
        [=](absl::Span<uint32_t> input) { f(BinaryToDecimal(input, v, exp)); });
  }

  size_t TotalDigits() const {
    return (decimal_end_ - decimal_start_) * kDigitsPerChunk +
           CurrentDigits().size();
  }

  absl::string_view CurrentDigits() const {
    return absl::string_view(digits_ + kDigitsPerChunk - size_, size_);
  }


  bool AdvanceDigits() {
    if (decimal_start_ >= decimal_end_) return false;

    uint32_t w = data_[decimal_start_++];
    for (size_ = 0; size_ < kDigitsPerChunk; w /= 10) {
      digits_[kDigitsPerChunk - ++size_] = w % 10 + '0';
    }
    return true;
  }

 private:
  BinaryToDecimal(absl::Span<uint32_t> data, uint128 v, int exp) : data_(data) {

















    size_t after_chunk_index = static_cast<size_t>(exp / 32 + 1);
    decimal_start_ = decimal_end_ = ChunksNeeded(exp);
    const int offset = exp % 32;

    data_[after_chunk_index - 1] = static_cast<uint32_t>(v << offset);
    for (v >>= (32 - offset); v; v >>= 32)
      data_[++after_chunk_index - 1] = static_cast<uint32_t>(v);

    while (after_chunk_index > 0) {



      uint32_t carry = 0;
      for (size_t i = after_chunk_index; i > 0; --i) {
        uint64_t tmp = uint64_t{data_[i - 1]} + (uint64_t{carry} << 32);
        data_[i - 1] = static_cast<uint32_t>(tmp / uint64_t{1000000000});
        carry = static_cast<uint32_t>(tmp % uint64_t{1000000000});
      }

      if (data_[after_chunk_index - 1] == 0)
        --after_chunk_index;

      --decimal_start_;
      assert(decimal_start_ != after_chunk_index - 1);
      data_[decimal_start_] = carry;
    }


    for (uint32_t first = data_[decimal_start_++]; first != 0; first /= 10) {
      digits_[kDigitsPerChunk - ++size_] = first % 10 + '0';
    }
  }

 private:
  static constexpr size_t kDigitsPerChunk = 9;

  size_t decimal_start_;
  size_t decimal_end_;

  char digits_[kDigitsPerChunk];
  size_t size_ = 0;

  absl::Span<uint32_t> data_;
};

// Requires `-exp < 0` and
// `-exp >= limits<MaxFloatType>::min_exponent - limits<MaxFloatType>::digits`.
class FractionalDigitGenerator {
 public:


  static void RunConversion(
      uint128 v, int exp, absl::FunctionRef<void(FractionalDigitGenerator)> f) {
    using Limits = std::numeric_limits<MaxFloatType>;
    assert(-exp < 0);
    assert(-exp >= Limits::min_exponent - 128);
    static_assert(StackArray::kMaxCapacity >=
                      (Limits::digits + 128 - Limits::min_exponent + 31) / 32,
                  "");
    StackArray::RunWithCapacity(
        static_cast<size_t>((Limits::digits + exp + 31) / 32),
        [=](absl::Span<uint32_t> input) {
          f(FractionalDigitGenerator(input, v, exp));
        });
  }

  bool HasMoreDigits() const { return next_digit_ != 0 || after_chunk_index_; }

  bool IsGreaterThanHalf() const {
    return next_digit_ > 5 || (next_digit_ == 5 && after_chunk_index_);
  }

  bool IsExactlyHalf() const { return next_digit_ == 5 && !after_chunk_index_; }

  struct Digits {
    char digit_before_nine;
    size_t num_nines;
  };


  Digits GetDigits() {
    Digits digits{next_digit_, 0};

    next_digit_ = GetOneDigit();
    while (next_digit_ == 9) {
      ++digits.num_nines;
      next_digit_ = GetOneDigit();
    }

    return digits;
  }

 private:

  char GetOneDigit() {
    if (!after_chunk_index_)
      return 0;

    char carry = 0;
    for (size_t i = after_chunk_index_; i > 0; --i) {
      carry = MultiplyBy10WithCarry(&data_[i - 1], carry);
    }

    if (data_[after_chunk_index_ - 1] == 0)
      --after_chunk_index_;
    return carry;
  }

  FractionalDigitGenerator(absl::Span<uint32_t> data, uint128 v, int exp)
      : after_chunk_index_(static_cast<size_t>(exp / 32 + 1)), data_(data) {
    const int offset = exp % 32;

    data_[after_chunk_index_ - 1] = static_cast<uint32_t>(v << (32 - offset));
    v >>= offset;


    for (size_t pos = after_chunk_index_ - 1; v; v >>= 32)
      data_[--pos] = static_cast<uint32_t>(v);

    next_digit_ = GetOneDigit();
  }

  char next_digit_;
  size_t after_chunk_index_;
  absl::Span<uint32_t> data_;
};

int LeadingZeros(uint64_t v) { return countl_zero(v); }
int LeadingZeros(uint128 v) {
  auto high = static_cast<uint64_t>(v >> 64);
  auto low = static_cast<uint64_t>(v);
  return high != 0 ? countl_zero(high) : 64 + countl_zero(low);
}

// The buffer must have an extra digit that is known to not need rounding.
// This is done below by having an extra '0' digit on the left.
void RoundUp(char *p) {
  while (*p == '9' || *p == '.') {
    if (*p == '9') *p = '0';
    --p;
  }
  ++*p;
}

// policy.
void RoundToEven(char *p) {
  if (*p == '.') --p;
  if (*p % 2 == 1) RoundUp(p);
}

// Returns the pointer to the last written digit.
char *PrintIntegralDigitsFromRightFast(uint64_t v, char *p) {
  do {
    *--p = DivideBy10WithCarry(&v, 0) + '0';
  } while (v != 0);
  return p;
}

// Returns the pointer to the last written digit.
char *PrintIntegralDigitsFromRightFast(uint128 v, char *p) {
  auto high = static_cast<uint64_t>(v >> 64);
  auto low = static_cast<uint64_t>(v);

  while (high != 0) {
    char carry = DivideBy10WithCarry(&high, 0);
    carry = DivideBy10WithCarry(&low, carry);
    *--p = carry + '0';
  }
  return PrintIntegralDigitsFromRightFast(low, p);
}

// shifting.
// Performs rounding if necessary to fit within `precision`.
// Returns the pointer to one after the last character written.
char* PrintFractionalDigitsFast(uint64_t v,
                                char* start,
                                int exp,
                                size_t precision) {
  char *p = start;
  v <<= (64 - exp);
  while (precision > 0) {
    if (!v) return p;
    *p++ = MultiplyBy10WithCarry(&v, 0) + '0';
    --precision;
  }

  if (v < 0x8000000000000000) {

  } else if (v > 0x8000000000000000) {

    RoundUp(p - 1);
  } else {
    RoundToEven(p - 1);
  }

  return p;
}

// after shifting.
// Performs rounding if necessary to fit within `precision`.
// Returns the pointer to one after the last character written.
char* PrintFractionalDigitsFast(uint128 v,
                                char* start,
                                int exp,
                                size_t precision) {
  char *p = start;
  v <<= (128 - exp);
  auto high = static_cast<uint64_t>(v >> 64);
  auto low = static_cast<uint64_t>(v);


  while (precision > 0 && low != 0) {
    char carry = MultiplyBy10WithCarry(&low, 0);
    carry = MultiplyBy10WithCarry(&high, carry);

    *p++ = carry + '0';
    --precision;
  }



  while (precision > 0) {
    if (!high) return p;
    *p++ = MultiplyBy10WithCarry(&high, 0) + '0';
    --precision;
  }

  if (high < 0x8000000000000000) {

  } else if (high > 0x8000000000000000 || low != 0) {

    RoundUp(p - 1);
  } else {
    RoundToEven(p - 1);
  }

  return p;
}

struct FormatState {
  char sign_char;
  size_t precision;
  const FormatConversionSpecImpl &conv;
  FormatSinkImpl *sink;


  bool ShouldPrintDot() const { return precision != 0 || conv.has_alt_flag(); }
};

struct Padding {
  size_t left_spaces;
  size_t zeros;
  size_t right_spaces;
};

Padding ExtraWidthToPadding(size_t total_size, const FormatState &state) {
  if (state.conv.width() < 0 ||
      static_cast<size_t>(state.conv.width()) <= total_size) {
    return {0, 0, 0};
  }
  size_t missing_chars = static_cast<size_t>(state.conv.width()) - total_size;
  if (state.conv.has_left_flag()) {
    return {0, 0, missing_chars};
  } else if (state.conv.has_zero_flag()) {
    return {0, missing_chars, 0};
  } else {
    return {missing_chars, 0, 0};
  }
}

void FinalPrint(const FormatState& state,
                absl::string_view data,
                size_t padding_offset,
                size_t trailing_zeros,
                absl::string_view data_postfix) {
  if (state.conv.width() < 0) {

    if (state.sign_char != '\0') state.sink->Append(1, state.sign_char);
    state.sink->Append(data);
    state.sink->Append(trailing_zeros, '0');
    state.sink->Append(data_postfix);
    return;
  }

  auto padding =
      ExtraWidthToPadding((state.sign_char != '\0' ? 1 : 0) + data.size() +
                              data_postfix.size() + trailing_zeros,
                          state);

  state.sink->Append(padding.left_spaces, ' ');
  if (state.sign_char != '\0') state.sink->Append(1, state.sign_char);

  state.sink->Append(data.substr(0, padding_offset));
  state.sink->Append(padding.zeros, '0');
  state.sink->Append(data.substr(padding_offset));
  state.sink->Append(trailing_zeros, '0');
  state.sink->Append(data_postfix);
  state.sink->Append(padding.right_spaces, ' ');
}

// type.
// Prints `v*2^exp` with the options from `state`.
template <typename Int>
void FormatFFast(Int v, int exp, const FormatState &state) {
  constexpr int input_bits = sizeof(Int) * 8;

  static constexpr size_t integral_size =
      /* in case we need to round up an extra digit */ 1 +
      /* decimal digits for uint128 */ 40 + 1;
  char buffer[integral_size + /* . */ 1 + /* max digits uint128 */ 128];
  buffer[integral_size] = '.';
  char *const integral_digits_end = buffer + integral_size;
  char *integral_digits_start;
  char *const fractional_digits_start = buffer + integral_size + 1;
  char *fractional_digits_end = fractional_digits_start;

  if (exp >= 0) {
    const int total_bits = input_bits - LeadingZeros(v) + exp;
    integral_digits_start =
        total_bits <= 64
            ? PrintIntegralDigitsFromRightFast(static_cast<uint64_t>(v) << exp,
                                               integral_digits_end)
            : PrintIntegralDigitsFromRightFast(static_cast<uint128>(v) << exp,
                                               integral_digits_end);
  } else {
    exp = -exp;

    integral_digits_start = PrintIntegralDigitsFromRightFast(
        exp < input_bits ? v >> exp : 0, integral_digits_end);


    integral_digits_start[-1] = '0';

    fractional_digits_end =
        exp <= 64 ? PrintFractionalDigitsFast(v, fractional_digits_start, exp,
                                              state.precision)
                  : PrintFractionalDigitsFast(static_cast<uint128>(v),
                                              fractional_digits_start, exp,
                                              state.precision);

    if (integral_digits_start[-1] != '0') --integral_digits_start;
  }

  size_t size =
      static_cast<size_t>(fractional_digits_end - integral_digits_start);


  if (!state.ShouldPrintDot()) --size;
  FinalPrint(state, absl::string_view(integral_digits_start, size),
             /*padding_offset=*/0,
             state.precision - static_cast<size_t>(fractional_digits_end -
                                                   fractional_digits_start),
             /*data_postfix=*/"");
}

// `exp > 0`.
// Prints `v*2^exp` with the options from `state`.
// This one is guaranteed to not have fractional digits, so we don't have to
// worry about anything after the `.`.
void FormatFPositiveExpSlow(uint128 v, int exp, const FormatState &state) {
  BinaryToDecimal::RunConversion(v, exp, [&](BinaryToDecimal btd) {
    const size_t total_digits =
        btd.TotalDigits() + (state.ShouldPrintDot() ? state.precision + 1 : 0);

    const auto padding = ExtraWidthToPadding(
        total_digits + (state.sign_char != '\0' ? 1 : 0), state);

    state.sink->Append(padding.left_spaces, ' ');
    if (state.sign_char != '\0')
      state.sink->Append(1, state.sign_char);
    state.sink->Append(padding.zeros, '0');

    do {
      state.sink->Append(btd.CurrentDigits());
    } while (btd.AdvanceDigits());

    if (state.ShouldPrintDot())
      state.sink->Append(1, '.');
    state.sink->Append(state.precision, '0');
    state.sink->Append(padding.right_spaces, ' ');
  });
}

// `exp < 0`.
// Prints `v*2^exp` with the options from `state`.
// This one is guaranteed to be < 1.0, so we don't have to worry about integral
// digits.
void FormatFNegativeExpSlow(uint128 v, int exp, const FormatState &state) {
  const size_t total_digits =
      /* 0 */ 1 + (state.ShouldPrintDot() ? state.precision + 1 : 0);
  auto padding =
      ExtraWidthToPadding(total_digits + (state.sign_char ? 1 : 0), state);
  padding.zeros += 1;
  state.sink->Append(padding.left_spaces, ' ');
  if (state.sign_char != '\0') state.sink->Append(1, state.sign_char);
  state.sink->Append(padding.zeros, '0');

  if (state.ShouldPrintDot()) state.sink->Append(1, '.');

  size_t digits_to_go = state.precision;

  FractionalDigitGenerator::RunConversion(
      v, exp, [&](FractionalDigitGenerator digit_gen) {

        if (state.precision == 0) return;



        while (digits_to_go > 0 && digit_gen.HasMoreDigits()) {
          auto digits = digit_gen.GetDigits();


          if (digits.num_nines + 1 < digits_to_go) {

            state.sink->Append(1, digits.digit_before_nine + '0');
            state.sink->Append(digits.num_nines, '9');
            digits_to_go -= digits.num_nines + 1;

          } else {


            bool round_up = false;
            if (digits.num_nines + 1 > digits_to_go) {

              round_up = true;
            } else {

              if (digit_gen.IsGreaterThanHalf()) {
                round_up = true;
              } else if (digit_gen.IsExactlyHalf()) {

                round_up =
                    digits.num_nines != 0 || digits.digit_before_nine % 2 == 1;
              }
            }

            if (round_up) {
              state.sink->Append(1, digits.digit_before_nine + '1');
              --digits_to_go;

            } else {
              state.sink->Append(1, digits.digit_before_nine + '0');
              state.sink->Append(digits_to_go - 1, '9');
              digits_to_go = 0;
            }
            return;
          }
        }
      });

  state.sink->Append(digits_to_go, '0');
  state.sink->Append(padding.right_spaces, ' ');
}

template <typename Int>
void FormatF(Int mantissa, int exp, const FormatState &state) {
  if (exp >= 0) {
    const int total_bits =
        static_cast<int>(sizeof(Int) * 8) - LeadingZeros(mantissa) + exp;


    if (ABSL_PREDICT_FALSE(total_bits > 128)) {
      return FormatFPositiveExpSlow(mantissa, exp, state);
    }
  } else {


    if (ABSL_PREDICT_FALSE(exp < -128)) {
      return FormatFNegativeExpSlow(mantissa, -exp, state);
    }
  }
  return FormatFFast(mantissa, exp, state);
}

// bits 4-7.
template <typename Int>
uint8_t GetNibble(Int n, size_t nibble_index) {
  constexpr Int mask_low_nibble = Int{0xf};
  int shift = static_cast<int>(nibble_index * 4);
  n &= mask_low_nibble << shift;
  return static_cast<uint8_t>((n >> shift) & 0xf);
}

// if overflow, false otherwise.
template <typename Int>
bool IncrementNibble(size_t nibble_index, Int* n) {
  constexpr size_t kShift = sizeof(Int) * 8 - 1;
  constexpr size_t kNumNibbles = sizeof(Int) * 8 / 4;
  Int before = *n >> kShift;






  *n += ((nibble_index >= kNumNibbles)
             ? 0
             : (Int{1} << static_cast<int>(nibble_index * 4)));
  Int after = *n >> kShift;
  return (before && !after) || (nibble_index >= kNumNibbles);
}

template <typename Int>
Int MaskUpToNibbleInclusive(size_t nibble_index) {
  constexpr size_t kNumNibbles = sizeof(Int) * 8 / 4;
  static const Int ones = ~Int{0};
  ++nibble_index;
  return ones >> static_cast<int>(
                     4 * (std::max(kNumNibbles, nibble_index) - nibble_index));
}

template <typename Int>
Int MaskUpToNibbleExclusive(size_t nibble_index) {
  return nibble_index == 0 ? 0 : MaskUpToNibbleInclusive<Int>(nibble_index - 1);
}

template <typename Int>
Int MoveToNibble(uint8_t nibble, size_t nibble_index) {
  return Int{nibble} << static_cast<int>(4 * nibble_index);
}

//
// In the hex representation we keep a single hex digit to the left of the dot.
// However, the question as to how many bits of the mantissa should be put into
// that hex digit in theory is arbitrary, but in practice it is optimal to
// choose based on the size of the mantissa. E.g., for a `double`, there are 53
// mantissa bits, so that means that we should put 1 bit to the left of the dot,
// thereby leaving 52 bits to the right, which is evenly divisible by four and
// thus all fractional digits represent actual precision. For a `long double`,
// on the other hand, there are 64 bits of mantissa, thus we can use all four
// bits for the initial hex digit and still have a number left over (60) that is
// a multiple of four. Once again, the goal is to have all fractional digits
// represent real precision.
template <typename Float>
constexpr size_t HexFloatLeadingDigitSizeInBits() {
  return std::numeric_limits<Float>::digits % 4 > 0
             ? static_cast<size_t>(std::numeric_limits<Float>::digits % 4)
             : size_t{4};
}

// representations. E.g. when rounding 0x1.ab800000 to a precision of .2
// ("%.2a") glibc will round up because it rounds toward the even number (since
// 0xb is an odd number, it will round up to 0xc). However, when rounding at a
// point that is not followed by 800000..., it disregards the parity and rounds
// up if > 8 and rounds down if < 8.
template <typename Int>
bool HexFloatNeedsRoundUp(Int mantissa,
                          size_t final_nibble_displayed,
                          uint8_t leading) {



  if (final_nibble_displayed == 0) {
    return false;
  }
  size_t rounding_nibble_idx = final_nibble_displayed - 1;
  constexpr size_t kTotalNibbles = sizeof(Int) * 8 / 4;
  assert(final_nibble_displayed <= kTotalNibbles);
  Int mantissa_up_to_rounding_nibble_inclusive =
      mantissa & MaskUpToNibbleInclusive<Int>(rounding_nibble_idx);
  Int eight = MoveToNibble<Int>(8, rounding_nibble_idx);
  if (mantissa_up_to_rounding_nibble_inclusive != eight) {
    return mantissa_up_to_rounding_nibble_inclusive > eight;
  }

  uint8_t round_if_odd = (final_nibble_displayed == kTotalNibbles)
                             ? leading
                             : GetNibble(mantissa, final_nibble_displayed);
  return round_if_odd % 2 == 1;
}

// implementation in order to avoid templatizing that function by the Float
// type.
struct HexFloatTypeParams {
  template <typename Float>
  explicit HexFloatTypeParams(Float)
      : min_exponent(std::numeric_limits<Float>::min_exponent - 1),
        leading_digit_size_bits(HexFloatLeadingDigitSizeInBits<Float>()) {
    assert(leading_digit_size_bits >= 1 && leading_digit_size_bits <= 4);
  }

  int min_exponent;
  size_t leading_digit_size_bits;
};

// by manipulating (incrementing) the mantissa, that way we can later print the
// mantissa digits by iterating through them in the same way regardless of
// whether a rounding happened.
template <typename Int>
void FormatARound(bool precision_specified, const FormatState &state,
                  uint8_t *leading, Int *mantissa, int *exp) {
  constexpr size_t kTotalNibbles = sizeof(Int) * 8 / 4;

  size_t final_nibble_displayed =
      precision_specified
          ? (std::max(kTotalNibbles, state.precision) - state.precision)
          : 0;
  if (HexFloatNeedsRoundUp(*mantissa, final_nibble_displayed, *leading)) {

    bool overflow = IncrementNibble(final_nibble_displayed, mantissa);
    *leading += (overflow ? 1 : 0);
    if (ABSL_PREDICT_FALSE(*leading > 15)) {



      *leading = 1;
      *mantissa = 0;
      *exp += 4;
    }
  }


  if (precision_specified) {
    *mantissa &= ~MaskUpToNibbleExclusive<Int>(final_nibble_displayed);
  }
}

template <typename Int>
void FormatANormalize(const HexFloatTypeParams float_traits, uint8_t *leading,
                      Int *mantissa, int *exp) {
  constexpr size_t kIntBits = sizeof(Int) * 8;
  static const Int kHighIntBit = Int{1} << (kIntBits - 1);
  const size_t kLeadDigitBitsCount = float_traits.leading_digit_size_bits;


  while (*mantissa && !(*mantissa & kHighIntBit)) {
    if (ABSL_PREDICT_FALSE(*exp - 1 < float_traits.min_exponent)) {
      *mantissa >>= (float_traits.min_exponent - *exp);
      *exp = float_traits.min_exponent;
      return;
    }
    *mantissa <<= 1;
    --*exp;
  }


  *leading = static_cast<uint8_t>(
      *mantissa >> static_cast<int>(kIntBits - kLeadDigitBitsCount));
  *exp -= (*mantissa != 0) ? static_cast<int>(kLeadDigitBitsCount) : *exp;
  *mantissa <<= static_cast<int>(kLeadDigitBitsCount);
}

template <typename Int>
void FormatA(const HexFloatTypeParams float_traits, Int mantissa, int exp,
             bool uppercase, const FormatState &state) {

  constexpr size_t kIntBits = sizeof(Int) * 8;
  constexpr size_t kTotalNibbles = sizeof(Int) * 8 / 4;

  const bool precision_specified = state.conv.precision() >= 0;

  exp += kIntBits;  // make all digits fractional digits.


  uint8_t leading = 0;
  FormatANormalize(float_traits, &leading, &mantissa, &exp);



  FormatARound(precision_specified, state, &leading, &mantissa, &exp);



  constexpr size_t kBufSizeForHexFloatRepr =
      2                                                // 0x
      + std::numeric_limits<MaxFloatType>::digits / 4  // number of hex digits
      + 1                                              // round up
      + 1;                                             // "." (dot)
  char digits_buffer[kBufSizeForHexFloatRepr];
  char *digits_iter = digits_buffer;
  const char *const digits =
      static_cast<const char *>("0123456789ABCDEF0123456789abcdef") +
      (uppercase ? 0 : 16);

  *digits_iter++ = '0';
  *digits_iter++ = uppercase ? 'X' : 'x';

  *digits_iter++ = digits[leading];



  if ((precision_specified && state.precision > 0) ||
      (!precision_specified && mantissa > 0) || state.conv.has_alt_flag()) {
    *digits_iter++ = '.';
  }

  size_t digits_emitted = 0;
  while (mantissa > 0) {
    *digits_iter++ = digits[GetNibble(mantissa, kTotalNibbles - 1)];
    mantissa <<= 4;
    ++digits_emitted;
  }
  size_t trailing_zeros = 0;
  if (precision_specified) {
    assert(state.precision >= digits_emitted);
    trailing_zeros = state.precision - digits_emitted;
  }
  auto digits_result = string_view(
      digits_buffer, static_cast<size_t>(digits_iter - digits_buffer));

  constexpr size_t kBufSizeForExpDecRepr =
      numbers_internal::kFastToBufferSize  // requred for FastIntToBuffer
      + 1                                  // 'p' or 'P'
      + 1;                                 // '+' or '-'
  char exp_buffer[kBufSizeForExpDecRepr];
  exp_buffer[0] = uppercase ? 'P' : 'p';
  exp_buffer[1] = exp >= 0 ? '+' : '-';
  numbers_internal::FastIntToBuffer(exp < 0 ? -exp : exp, exp_buffer + 2);

  FinalPrint(state,
             digits_result,                        // 0xN.NNN...
             2,                                    // offset of any padding
             static_cast<size_t>(trailing_zeros),  // remaining mantissa padding
             exp_buffer);                          // exponent
}

char *CopyStringTo(absl::string_view v, char *out) {
  std::memcpy(out, v.data(), v.size());
  return out + v.size();
}

template <typename Float>
bool FallbackToSnprintf(const Float v, const FormatConversionSpecImpl &conv,
                        FormatSinkImpl *sink) {
  int w = conv.width() >= 0 ? conv.width() : 0;
  int p = conv.precision() >= 0 ? conv.precision() : -1;
  char fmt[32];
  {
    char *fp = fmt;
    *fp++ = '%';
    fp = CopyStringTo(FormatConversionSpecImplFriend::FlagsToString(conv), fp);
    fp = CopyStringTo("*.*", fp);
    if (std::is_same<long double, Float>()) {
      *fp++ = 'L';
    }
    *fp++ = FormatConversionCharToChar(conv.conversion_char());
    *fp = 0;
    assert(fp < fmt + sizeof(fmt));
  }
  std::string space(512, '\0');
  absl::string_view result;
  while (true) {
    int n = snprintf(&space[0], space.size(), fmt, w, p, v);
    if (n < 0) return false;
    if (static_cast<size_t>(n) < space.size()) {
      result = absl::string_view(space.data(), static_cast<size_t>(n));
      break;
    }
    space.resize(static_cast<size_t>(n) + 1);
  }
  sink->Append(result);
  return true;
}

//   or std::numeric_limits<__uint128_t>::digits10
constexpr size_t kMaxFixedPrecision = 39;

constexpr size_t kBufferLength = /*sign*/ 1 +
                                 /*integer*/ kMaxFixedPrecision +
                                 /*point*/ 1 +
                                 /*fraction*/ kMaxFixedPrecision +
                                 /*exponent e+123*/ 5;

struct Buffer {
  void push_front(char c) {
    assert(begin > data);
    *--begin = c;
  }
  void push_back(char c) {
    assert(end < data + sizeof(data));
    *end++ = c;
  }
  void pop_back() {
    assert(begin < end);
    --end;
  }

  char &back() {
    assert(begin < end);
    return end[-1];
  }

  char last_digit() const { return end[-1] == '.' ? end[-2] : end[-1]; }

  size_t size() const { return static_cast<size_t>(end - begin); }

  char data[kBufferLength];
  char *begin;
  char *end;
};

enum class FormatStyle { Fixed, Precision };

// Otherwise, return false.
template <typename Float>
bool ConvertNonNumericFloats(char sign_char, Float v,
                             const FormatConversionSpecImpl &conv,
                             FormatSinkImpl *sink) {
  char text[4], *ptr = text;
  if (sign_char != '\0') *ptr++ = sign_char;
  if (std::isnan(v)) {
    ptr = std::copy_n(
        FormatConversionCharIsUpper(conv.conversion_char()) ? "NAN" : "nan", 3,
        ptr);
  } else if (std::isinf(v)) {
    ptr = std::copy_n(
        FormatConversionCharIsUpper(conv.conversion_char()) ? "INF" : "inf", 3,
        ptr);
  } else {
    return false;
  }

  return sink->PutPaddedString(
      string_view(text, static_cast<size_t>(ptr - text)), conv.width(), -1,
      conv.has_left_flag());
}

// It will carry over and potentially overflow. 'exp' will be adjusted in that
// case.
template <FormatStyle mode>
void RoundUp(Buffer *buffer, int *exp) {
  char *p = &buffer->back();
  while (p >= buffer->begin && (*p == '9' || *p == '.')) {
    if (*p == '9') *p = '0';
    --p;
  }

  if (p < buffer->begin) {
    *p = '1';
    buffer->begin = p;
    if (mode == FormatStyle::Precision) {
      std::swap(p[1], p[2]);  // move the .
      ++*exp;
      buffer->pop_back();
    }
  } else {
    ++*p;
  }
}

void PrintExponent(int exp, char e, Buffer *out) {
  out->push_back(e);
  if (exp < 0) {
    out->push_back('-');
    exp = -exp;
  } else {
    out->push_back('+');
  }

  if (exp > 99) {
    out->push_back(static_cast<char>(exp / 100 + '0'));
    out->push_back(static_cast<char>(exp / 10 % 10 + '0'));
    out->push_back(static_cast<char>(exp % 10 + '0'));
  } else {
    out->push_back(static_cast<char>(exp / 10 + '0'));
    out->push_back(static_cast<char>(exp % 10 + '0'));
  }
}

template <typename Float, typename Int>
constexpr bool CanFitMantissa() {
  return
#if defined(__clang__) && !defined(__SSE3__)


      (!std::is_same<Float, long double>::value ||
       !std::is_same<Int, uint64_t>::value) &&
#endif
      std::numeric_limits<Float>::digits <= std::numeric_limits<Int>::digits;
}

template <typename Float>
struct Decomposed {
  using MantissaType =
      absl::conditional_t<std::is_same<long double, Float>::value, uint128,
                          uint64_t>;
  static_assert(std::numeric_limits<Float>::digits <= sizeof(MantissaType) * 8,
                "");
  MantissaType mantissa;
  int exponent;
};

template <typename Float>
Decomposed<Float> Decompose(Float v) {
  int exp;
  Float m = std::frexp(v, &exp);
  m = std::ldexp(m, std::numeric_limits<Float>::digits);
  exp -= std::numeric_limits<Float>::digits;

  return {static_cast<typename Decomposed<Float>::MantissaType>(m), exp};
}

// In Fixed mode, we add a '.' at the end.
// In Precision mode, we add a '.' after the first digit.
template <FormatStyle mode, typename Int>
size_t PrintIntegralDigits(Int digits, Buffer* out) {
  size_t printed = 0;
  if (digits) {
    for (; digits; digits /= 10) out->push_front(digits % 10 + '0');
    printed = out->size();
    if (mode == FormatStyle::Precision) {
      out->push_front(*out->begin);
      out->begin[1] = '.';
    } else {
      out->push_back('.');
    }
  } else if (mode == FormatStyle::Fixed) {
    out->push_front('0');
    out->push_back('.');
    printed = 1;
  }
  return printed;
}

void RemoveExtraPrecision(size_t extra_digits,
                          bool has_leftover_value,
                          Buffer* out,
                          int* exp_out) {

  out->end -= extra_digits;

  bool needs_to_round_up = [&] {


    if (*out->end > '5') return true;
    if (*out->end < '5') return false;
    if (has_leftover_value || std::any_of(out->end + 1, out->end + extra_digits,
                                          [](char c) { return c != '0'; }))
      return true;

    return out->last_digit() % 2 == 1;
  }();

  if (needs_to_round_up) {
    RoundUp<FormatStyle::Precision>(out, exp_out);
  }
}

// This will not include the exponent, which will be returned in 'exp_out' for
// Precision mode.
template <typename Int, typename Float, FormatStyle mode>
bool FloatToBufferImpl(Int int_mantissa,
                       int exp,
                       size_t precision,
                       Buffer* out,
                       int* exp_out) {
  assert((CanFitMantissa<Float, Int>()));

  const int int_bits = std::numeric_limits<Int>::digits;



  out->begin = out->end =
      out->data + 1 + kMaxFixedPrecision + (mode == FormatStyle::Precision);

  if (exp >= 0) {
    if (std::numeric_limits<Float>::digits + exp > int_bits) {

      return false;
    }
    size_t digits_printed = PrintIntegralDigits<mode>(int_mantissa << exp, out);
    size_t digits_to_zero_pad = precision;
    if (mode == FormatStyle::Precision) {
      *exp_out = static_cast<int>(digits_printed - 1);
      if (digits_to_zero_pad < digits_printed - 1) {
        RemoveExtraPrecision(digits_printed - 1 - digits_to_zero_pad, false,
                             out, exp_out);
        return true;
      }
      digits_to_zero_pad -= digits_printed - 1;
    }
    for (; digits_to_zero_pad-- > 0;) out->push_back('0');
    return true;
  }

  exp = -exp;


  if (exp > int_bits - 4) return false;

  const Int mask = (Int{1} << exp) - 1;

  size_t digits_printed = PrintIntegralDigits<mode>(int_mantissa >> exp, out);
  int_mantissa &= mask;

  size_t fractional_count = precision;
  if (mode == FormatStyle::Precision) {
    if (digits_printed == 0) {

      *exp_out = 0;
      if (int_mantissa) {
        while (int_mantissa <= mask) {
          int_mantissa *= 10;
          --*exp_out;
        }
      }
      out->push_front(static_cast<char>(int_mantissa >> exp) + '0');
      out->push_back('.');
      int_mantissa &= mask;
    } else {

      *exp_out = static_cast<int>(digits_printed - 1);
      if (fractional_count < digits_printed - 1) {


        RemoveExtraPrecision(digits_printed - 1 - fractional_count,
                             int_mantissa != 0, out, exp_out);
        return true;
      }
      fractional_count -= digits_printed - 1;
    }
  }

  auto get_next_digit = [&] {
    int_mantissa *= 10;
    char digit = static_cast<char>(int_mantissa >> exp);
    int_mantissa &= mask;
    return digit;
  };

  for (; fractional_count > 0; --fractional_count) {
    out->push_back(get_next_digit() + '0');
  }

  char next_digit = get_next_digit();
  if (next_digit > 5 ||
      (next_digit == 5 && (int_mantissa || out->last_digit() % 2 == 1))) {
    RoundUp<mode>(out, exp_out);
  }

  return true;
}

template <FormatStyle mode, typename Float>
bool FloatToBuffer(Decomposed<Float> decomposed,
                   size_t precision,
                   Buffer* out,
                   int* exp) {
  if (precision > kMaxFixedPrecision) return false;

  if (CanFitMantissa<Float, std::uint64_t>() &&
      FloatToBufferImpl<std::uint64_t, Float, mode>(
          static_cast<std::uint64_t>(decomposed.mantissa), decomposed.exponent,
          precision, out, exp))
    return true;

#if defined(ABSL_HAVE_INTRINSIC_INT128)

  return CanFitMantissa<Float, __uint128_t>() &&
         FloatToBufferImpl<__uint128_t, Float, mode>(
             static_cast<__uint128_t>(decomposed.mantissa), decomposed.exponent,
             precision, out, exp);
#endif
  return false;
}

void WriteBufferToSink(char sign_char, absl::string_view str,
                       const FormatConversionSpecImpl &conv,
                       FormatSinkImpl *sink) {
  size_t left_spaces = 0, zeros = 0, right_spaces = 0;
  size_t missing_chars = 0;
  if (conv.width() >= 0) {
    const size_t conv_width_size_t = static_cast<size_t>(conv.width());
    const size_t existing_chars =
        str.size() + static_cast<size_t>(sign_char != 0);
    if (conv_width_size_t > existing_chars)
      missing_chars = conv_width_size_t - existing_chars;
  }
  if (conv.has_left_flag()) {
    right_spaces = missing_chars;
  } else if (conv.has_zero_flag()) {
    zeros = missing_chars;
  } else {
    left_spaces = missing_chars;
  }

  sink->Append(left_spaces, ' ');
  if (sign_char != '\0') sink->Append(1, sign_char);
  sink->Append(zeros, '0');
  sink->Append(str);
  sink->Append(right_spaces, ' ');
}

template <typename Float>
bool FloatToSink(const Float v, const FormatConversionSpecImpl &conv,
                 FormatSinkImpl *sink) {

  Float abs_v = v;
  char sign_char = 0;
  if (std::signbit(abs_v)) {
    sign_char = '-';
    abs_v = -abs_v;
  } else if (conv.has_show_pos_flag()) {
    sign_char = '+';
  } else if (conv.has_sign_col_flag()) {
    sign_char = ' ';
  }

  if (ConvertNonNumericFloats(sign_char, abs_v, conv, sink)) {
    return true;
  }

  size_t precision =
      conv.precision() < 0 ? 6 : static_cast<size_t>(conv.precision());

  int exp = 0;

  auto decomposed = Decompose(abs_v);

  Buffer buffer;

  FormatConversionChar c = conv.conversion_char();

  if (c == FormatConversionCharInternal::f ||
      c == FormatConversionCharInternal::F) {
    FormatF(decomposed.mantissa, decomposed.exponent,
            {sign_char, precision, conv, sink});
    return true;
  } else if (c == FormatConversionCharInternal::e ||
             c == FormatConversionCharInternal::E) {
    if (!FloatToBuffer<FormatStyle::Precision>(decomposed, precision, &buffer,
                                               &exp)) {
      return FallbackToSnprintf(v, conv, sink);
    }
    if (!conv.has_alt_flag() && buffer.back() == '.') buffer.pop_back();
    PrintExponent(
        exp, FormatConversionCharIsUpper(conv.conversion_char()) ? 'E' : 'e',
        &buffer);
  } else if (c == FormatConversionCharInternal::g ||
             c == FormatConversionCharInternal::G) {
    precision = std::max(precision, size_t{1}) - 1;
    if (!FloatToBuffer<FormatStyle::Precision>(decomposed, precision, &buffer,
                                               &exp)) {
      return FallbackToSnprintf(v, conv, sink);
    }
    if ((exp < 0 || precision + 1 > static_cast<size_t>(exp)) && exp >= -4) {
      if (exp < 0) {


        buffer.begin[1] = *buffer.begin;

        for (; exp < -1; ++exp) *buffer.begin-- = '0';
        *buffer.begin-- = '.';
        *buffer.begin = '0';
      } else if (exp > 0) {


        std::rotate(buffer.begin + 1, buffer.begin + 2, buffer.begin + exp + 2);
      }
      exp = 0;
    }
    if (!conv.has_alt_flag()) {
      while (buffer.back() == '0') buffer.pop_back();
      if (buffer.back() == '.') buffer.pop_back();
    }
    if (exp) {
      PrintExponent(
          exp, FormatConversionCharIsUpper(conv.conversion_char()) ? 'E' : 'e',
          &buffer);
    }
  } else if (c == FormatConversionCharInternal::a ||
             c == FormatConversionCharInternal::A) {
    bool uppercase = (c == FormatConversionCharInternal::A);
    FormatA(HexFloatTypeParams(Float{}), decomposed.mantissa,
            decomposed.exponent, uppercase, {sign_char, precision, conv, sink});
    return true;
  } else {
    return false;
  }

  WriteBufferToSink(
      sign_char,
      absl::string_view(buffer.begin,
                        static_cast<size_t>(buffer.end - buffer.begin)),
      conv, sink);

  return true;
}

}  // namespace

bool ConvertFloatImpl(long double v, const FormatConversionSpecImpl &conv,
                      FormatSinkImpl *sink) {
  if (IsDoubleDouble()) {


    return FallbackToSnprintf(v, conv, sink);
  }

  return FloatToSink(v, conv, sink);
}

bool ConvertFloatImpl(float v, const FormatConversionSpecImpl &conv,
                      FormatSinkImpl *sink) {
  return FloatToSink(static_cast<double>(v), conv, sink);
}

bool ConvertFloatImpl(double v, const FormatConversionSpecImpl &conv,
                      FormatSinkImpl *sink) {
  return FloatToSink(v, conv, sink);
}

}  // namespace str_format_internal
ABSL_NAMESPACE_END
}  // namespace absl
