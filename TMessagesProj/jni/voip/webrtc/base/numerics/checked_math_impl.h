// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_NUMERICS_CHECKED_MATH_IMPL_H_
#define BASE_NUMERICS_CHECKED_MATH_IMPL_H_

#include <stddef.h>
#include <stdint.h>

#include <climits>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

#include "base/numerics/safe_conversions.h"
#include "base/numerics/safe_math_shared_impl.h"

namespace base {
namespace internal {

template <typename T>
constexpr bool CheckedAddImpl(T x, T y, T* result) {
  static_assert(std::is_integral<T>::value, "Type must be integral");


  using UnsignedDst = typename std::make_unsigned<T>::type;
  using SignedDst = typename std::make_signed<T>::type;
  UnsignedDst ux = static_cast<UnsignedDst>(x);
  UnsignedDst uy = static_cast<UnsignedDst>(y);
  UnsignedDst uresult = static_cast<UnsignedDst>(ux + uy);
  *result = static_cast<T>(uresult);


  return (std::is_signed<T>::value)
             ? static_cast<SignedDst>((uresult ^ ux) & (uresult ^ uy)) >= 0
             : uresult >= uy;  // Unsigned is either valid or underflow.
}

template <typename T, typename U, class Enable = void>
struct CheckedAddOp {};

template <typename T, typename U>
struct CheckedAddOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename MaxExponentPromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {

    if (CheckedAddFastOp<T, U>::is_supported)
      return CheckedAddFastOp<T, U>::Do(x, y, result);

    using FastPromotion = typename FastIntegerArithmeticPromotion<T, U>::type;
    using Promotion =
        typename std::conditional<(IntegerBitsPlusSign<FastPromotion>::value >
                                   IntegerBitsPlusSign<intptr_t>::value),
                                  typename BigEnoughPromotion<T, U>::type,
                                  FastPromotion>::type;


    if (BASE_NUMERICS_UNLIKELY(!IsValueInRangeForNumericType<Promotion>(x) ||
                               !IsValueInRangeForNumericType<Promotion>(y))) {
      return false;
    }

    Promotion presult = {};
    bool is_valid = true;
    if (IsIntegerArithmeticSafe<Promotion, T, U>::value) {
      presult = static_cast<Promotion>(x) + static_cast<Promotion>(y);
    } else {
      is_valid = CheckedAddImpl(static_cast<Promotion>(x),
                                static_cast<Promotion>(y), &presult);
    }
    *result = static_cast<V>(presult);
    return is_valid && IsValueInRangeForNumericType<V>(presult);
  }
};

template <typename T>
constexpr bool CheckedSubImpl(T x, T y, T* result) {
  static_assert(std::is_integral<T>::value, "Type must be integral");


  using UnsignedDst = typename std::make_unsigned<T>::type;
  using SignedDst = typename std::make_signed<T>::type;
  UnsignedDst ux = static_cast<UnsignedDst>(x);
  UnsignedDst uy = static_cast<UnsignedDst>(y);
  UnsignedDst uresult = static_cast<UnsignedDst>(ux - uy);
  *result = static_cast<T>(uresult);


  return (std::is_signed<T>::value)
             ? static_cast<SignedDst>((uresult ^ ux) & (ux ^ uy)) >= 0
             : x >= y;
}

template <typename T, typename U, class Enable = void>
struct CheckedSubOp {};

template <typename T, typename U>
struct CheckedSubOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename MaxExponentPromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {

    if (CheckedSubFastOp<T, U>::is_supported)
      return CheckedSubFastOp<T, U>::Do(x, y, result);

    using FastPromotion = typename FastIntegerArithmeticPromotion<T, U>::type;
    using Promotion =
        typename std::conditional<(IntegerBitsPlusSign<FastPromotion>::value >
                                   IntegerBitsPlusSign<intptr_t>::value),
                                  typename BigEnoughPromotion<T, U>::type,
                                  FastPromotion>::type;


    if (BASE_NUMERICS_UNLIKELY(!IsValueInRangeForNumericType<Promotion>(x) ||
                               !IsValueInRangeForNumericType<Promotion>(y))) {
      return false;
    }

    Promotion presult = {};
    bool is_valid = true;
    if (IsIntegerArithmeticSafe<Promotion, T, U>::value) {
      presult = static_cast<Promotion>(x) - static_cast<Promotion>(y);
    } else {
      is_valid = CheckedSubImpl(static_cast<Promotion>(x),
                                static_cast<Promotion>(y), &presult);
    }
    *result = static_cast<V>(presult);
    return is_valid && IsValueInRangeForNumericType<V>(presult);
  }
};

template <typename T>
constexpr bool CheckedMulImpl(T x, T y, T* result) {
  static_assert(std::is_integral<T>::value, "Type must be integral");


  using UnsignedDst = typename std::make_unsigned<T>::type;
  using SignedDst = typename std::make_signed<T>::type;
  const UnsignedDst ux = SafeUnsignedAbs(x);
  const UnsignedDst uy = SafeUnsignedAbs(y);
  UnsignedDst uresult = static_cast<UnsignedDst>(ux * uy);
  const bool is_negative =
      std::is_signed<T>::value && static_cast<SignedDst>(x ^ y) < 0;
  *result = is_negative ? 0 - uresult : uresult;



  return uy <= UnsignedDst(!std::is_signed<T>::value || is_negative) ||
         ux <= (std::numeric_limits<T>::max() + UnsignedDst(is_negative)) / uy;
}

template <typename T, typename U, class Enable = void>
struct CheckedMulOp {};

template <typename T, typename U>
struct CheckedMulOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename MaxExponentPromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {

    if (CheckedMulFastOp<T, U>::is_supported)
      return CheckedMulFastOp<T, U>::Do(x, y, result);

    using Promotion = typename FastIntegerArithmeticPromotion<T, U>::type;

    if (BASE_NUMERICS_UNLIKELY((!IsValueInRangeForNumericType<Promotion>(x) ||
                                !IsValueInRangeForNumericType<Promotion>(y)) &&
                               x && y)) {
      return false;
    }

    Promotion presult = {};
    bool is_valid = true;
    if (CheckedMulFastOp<Promotion, Promotion>::is_supported) {

      is_valid = CheckedMulFastOp<Promotion, Promotion>::Do(x, y, &presult);
    } else if (IsIntegerArithmeticSafe<Promotion, T, U>::value) {
      presult = static_cast<Promotion>(x) * static_cast<Promotion>(y);
    } else {
      is_valid = CheckedMulImpl(static_cast<Promotion>(x),
                                static_cast<Promotion>(y), &presult);
    }
    *result = static_cast<V>(presult);
    return is_valid && IsValueInRangeForNumericType<V>(presult);
  }
};

// on signed min/-1.
template <typename T, typename U, class Enable = void>
struct CheckedDivOp {};

template <typename T, typename U>
struct CheckedDivOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename MaxExponentPromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    if (BASE_NUMERICS_UNLIKELY(!y))
      return false;


    using Promotion = typename BigEnoughPromotion<T, U>::type;
    if (BASE_NUMERICS_UNLIKELY(
            (std::is_signed<T>::value && std::is_signed<U>::value &&
             IsTypeInRangeForNumericType<T, Promotion>::value &&
             static_cast<Promotion>(x) ==
                 std::numeric_limits<Promotion>::lowest() &&
             y == static_cast<U>(-1)))) {
      return false;
    }

    if (BASE_NUMERICS_UNLIKELY((!IsValueInRangeForNumericType<Promotion>(x) ||
                                !IsValueInRangeForNumericType<Promotion>(y)) &&
                               x)) {
      return false;
    }

    Promotion presult = Promotion(x) / Promotion(y);
    *result = static_cast<V>(presult);
    return IsValueInRangeForNumericType<V>(presult);
  }
};

template <typename T, typename U, class Enable = void>
struct CheckedModOp {};

template <typename T, typename U>
struct CheckedModOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename MaxExponentPromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    using Promotion = typename BigEnoughPromotion<T, U>::type;
    if (BASE_NUMERICS_LIKELY(y)) {
      Promotion presult = static_cast<Promotion>(x) % static_cast<Promotion>(y);
      *result = static_cast<Promotion>(presult);
      return IsValueInRangeForNumericType<V>(presult);
    }
    return false;
  }
};

template <typename T, typename U, class Enable = void>
struct CheckedLshOp {};

// of bits in the promoted type are undefined. Shifts of negative values
// are undefined. Otherwise it is defined when the result fits.
template <typename T, typename U>
struct CheckedLshOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = T;
  template <typename V>
  static constexpr bool Do(T x, U shift, V* result) {

    if (BASE_NUMERICS_LIKELY(!IsValueNegative(x) &&
                             as_unsigned(shift) <
                                 as_unsigned(std::numeric_limits<T>::digits))) {

      *result = static_cast<V>(as_unsigned(x) << shift);

      return *result >> shift == x;
    }

    return std::is_signed<T>::value && !x &&
           as_unsigned(shift) == as_unsigned(std::numeric_limits<T>::digits);
  }
};

template <typename T, typename U, class Enable = void>
struct CheckedRshOp {};

// of bits in the promoted type are undefined. Otherwise, it is always defined,
// but a right shift of a negative value is implementation-dependent.
template <typename T, typename U>
struct CheckedRshOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = T;
  template <typename V>
  static bool Do(T x, U shift, V* result) {

    if (BASE_NUMERICS_LIKELY(as_unsigned(shift) <
                             IntegerBitsPlusSign<T>::value)) {
      T tmp = x >> shift;
      *result = static_cast<V>(tmp);
      return IsValueInRangeForNumericType<V>(tmp);
    }
    return false;
  }
};

template <typename T, typename U, class Enable = void>
struct CheckedAndOp {};

template <typename T, typename U>
struct CheckedAndOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename std::make_unsigned<
      typename MaxExponentPromotion<T, U>::type>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    result_type tmp = static_cast<result_type>(x) & static_cast<result_type>(y);
    *result = static_cast<V>(tmp);
    return IsValueInRangeForNumericType<V>(tmp);
  }
};

template <typename T, typename U, class Enable = void>
struct CheckedOrOp {};

template <typename T, typename U>
struct CheckedOrOp<T,
                   U,
                   typename std::enable_if<std::is_integral<T>::value &&
                                           std::is_integral<U>::value>::type> {
  using result_type = typename std::make_unsigned<
      typename MaxExponentPromotion<T, U>::type>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    result_type tmp = static_cast<result_type>(x) | static_cast<result_type>(y);
    *result = static_cast<V>(tmp);
    return IsValueInRangeForNumericType<V>(tmp);
  }
};

template <typename T, typename U, class Enable = void>
struct CheckedXorOp {};

template <typename T, typename U>
struct CheckedXorOp<T,
                    U,
                    typename std::enable_if<std::is_integral<T>::value &&
                                            std::is_integral<U>::value>::type> {
  using result_type = typename std::make_unsigned<
      typename MaxExponentPromotion<T, U>::type>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    result_type tmp = static_cast<result_type>(x) ^ static_cast<result_type>(y);
    *result = static_cast<V>(tmp);
    return IsValueInRangeForNumericType<V>(tmp);
  }
};

// but it makes the code much cleaner to use the MathOp wrappers.
template <typename T, typename U, class Enable = void>
struct CheckedMaxOp {};

template <typename T, typename U>
struct CheckedMaxOp<
    T,
    U,
    typename std::enable_if<std::is_arithmetic<T>::value &&
                            std::is_arithmetic<U>::value>::type> {
  using result_type = typename MaxExponentPromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    result_type tmp = IsGreater<T, U>::Test(x, y) ? static_cast<result_type>(x)
                                                  : static_cast<result_type>(y);
    *result = static_cast<V>(tmp);
    return IsValueInRangeForNumericType<V>(tmp);
  }
};

// but it makes the code much cleaner to use the MathOp wrappers.
template <typename T, typename U, class Enable = void>
struct CheckedMinOp {};

template <typename T, typename U>
struct CheckedMinOp<
    T,
    U,
    typename std::enable_if<std::is_arithmetic<T>::value &&
                            std::is_arithmetic<U>::value>::type> {
  using result_type = typename LowestValuePromotion<T, U>::type;
  template <typename V>
  static constexpr bool Do(T x, U y, V* result) {
    result_type tmp = IsLess<T, U>::Test(x, y) ? static_cast<result_type>(x)
                                               : static_cast<result_type>(y);
    *result = static_cast<V>(tmp);
    return IsValueInRangeForNumericType<V>(tmp);
  }
};

// A macro isn't the nicest solution, but it beats rewriting these repeatedly.
#define BASE_FLOAT_ARITHMETIC_OPS(NAME, OP)                              \
  template <typename T, typename U>                                      \
  struct Checked##NAME##Op<                                              \
      T, U,                                                              \
      typename std::enable_if<std::is_floating_point<T>::value ||        \
                              std::is_floating_point<U>::value>::type> { \
    using result_type = typename MaxExponentPromotion<T, U>::type;       \
    template <typename V>                                                \
    static constexpr bool Do(T x, U y, V* result) {                      \
      using Promotion = typename MaxExponentPromotion<T, U>::type;       \
      Promotion presult = x OP y;                                        \
      *result = static_cast<V>(presult);                                 \
      return IsValueInRangeForNumericType<V>(presult);                   \
    }                                                                    \
  };

BASE_FLOAT_ARITHMETIC_OPS(Add, +)
BASE_FLOAT_ARITHMETIC_OPS(Sub, -)
BASE_FLOAT_ARITHMETIC_OPS(Mul, *)
BASE_FLOAT_ARITHMETIC_OPS(Div, /)

#undef BASE_FLOAT_ARITHMETIC_OPS

// we wrap the underlying value in a specialization in order to hide that detail
// and expose an interface via accessors.
enum NumericRepresentation {
  NUMERIC_INTEGER,
  NUMERIC_FLOATING,
  NUMERIC_UNKNOWN
};

template <typename NumericType>
struct GetNumericRepresentation {
  static const NumericRepresentation value =
      std::is_integral<NumericType>::value
          ? NUMERIC_INTEGER
          : (std::is_floating_point<NumericType>::value ? NUMERIC_FLOATING
                                                        : NUMERIC_UNKNOWN);
};

template <typename T,
          NumericRepresentation type = GetNumericRepresentation<T>::value>
class CheckedNumericState {};

template <typename T>
class CheckedNumericState<T, NUMERIC_INTEGER> {
 private:



  bool is_valid_;
  T value_;

  template <typename Src>
  static constexpr T WellDefinedConversionOrZero(const Src value,
                                                 const bool is_valid) {
    using SrcType = typename internal::UnderlyingType<Src>::type;
    return (std::is_integral<SrcType>::value || is_valid)
               ? static_cast<T>(value)
               : static_cast<T>(0);
  }

 public:
  template <typename Src, NumericRepresentation type>
  friend class CheckedNumericState;

  constexpr CheckedNumericState() : is_valid_(true), value_(0) {}

  template <typename Src>
  constexpr CheckedNumericState(Src value, bool is_valid)
      : is_valid_(is_valid && IsValueInRangeForNumericType<T>(value)),
        value_(WellDefinedConversionOrZero(value, is_valid_)) {
    static_assert(std::is_arithmetic<Src>::value, "Argument must be numeric.");
  }

  template <typename Src>
  constexpr CheckedNumericState(const CheckedNumericState<Src>& rhs)
      : is_valid_(rhs.IsValid()),
        value_(WellDefinedConversionOrZero(rhs.value(), is_valid_)) {}

  template <typename Src>
  constexpr explicit CheckedNumericState(Src value)
      : is_valid_(IsValueInRangeForNumericType<T>(value)),
        value_(WellDefinedConversionOrZero(value, is_valid_)) {}

  constexpr bool is_valid() const { return is_valid_; }
  constexpr T value() const { return value_; }
};

template <typename T>
class CheckedNumericState<T, NUMERIC_FLOATING> {
 private:
  T value_;

  template <typename Src>
  static constexpr T WellDefinedConversionOrNaN(const Src value,
                                                const bool is_valid) {
    using SrcType = typename internal::UnderlyingType<Src>::type;
    return (StaticDstRangeRelationToSrcRange<T, SrcType>::value ==
                NUMERIC_RANGE_CONTAINED ||
            is_valid)
               ? static_cast<T>(value)
               : std::numeric_limits<T>::quiet_NaN();
  }

 public:
  template <typename Src, NumericRepresentation type>
  friend class CheckedNumericState;

  constexpr CheckedNumericState() : value_(0.0) {}

  template <typename Src>
  constexpr CheckedNumericState(Src value, bool is_valid)
      : value_(WellDefinedConversionOrNaN(value, is_valid)) {}

  template <typename Src>
  constexpr explicit CheckedNumericState(Src value)
      : value_(WellDefinedConversionOrNaN(
            value,
            IsValueInRangeForNumericType<T>(value))) {}

  template <typename Src>
  constexpr CheckedNumericState(const CheckedNumericState<Src>& rhs)
      : value_(WellDefinedConversionOrNaN(
            rhs.value(),
            rhs.is_valid() && IsValueInRangeForNumericType<T>(rhs.value()))) {}

  constexpr bool is_valid() const {

    return MustTreatAsConstexpr(value_)
               ? value_ <= std::numeric_limits<T>::max() &&
                     value_ >= std::numeric_limits<T>::lowest()
               : std::isfinite(value_);
  }
  constexpr T value() const { return value_; }
};

}  // namespace internal
}  // namespace base

#endif  // BASE_NUMERICS_CHECKED_MATH_IMPL_H_
