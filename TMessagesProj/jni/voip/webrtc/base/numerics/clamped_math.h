// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_NUMERICS_CLAMPED_MATH_H_
#define BASE_NUMERICS_CLAMPED_MATH_H_

#include <stddef.h>

#include <limits>
#include <type_traits>

#include "base/numerics/clamped_math_impl.h"

namespace base {
namespace internal {

template <typename T>
class ClampedNumeric {
  static_assert(std::is_arithmetic<T>::value,
                "ClampedNumeric<T>: T must be a numeric type.");

 public:
  using type = T;

  constexpr ClampedNumeric() : value_(0) {}

  template <typename Src>
  constexpr ClampedNumeric(const ClampedNumeric<Src>& rhs)
      : value_(saturated_cast<T>(rhs.value_)) {}

  template <typename Src>
  friend class ClampedNumeric;


  template <typename Src>
  constexpr ClampedNumeric(Src value)  // NOLINT(runtime/explicit)
      : value_(saturated_cast<T>(value)) {
    static_assert(std::is_arithmetic<Src>::value, "Argument must be numeric.");
  }


  template <typename Src>
  constexpr ClampedNumeric(
      StrictNumeric<Src> value)  // NOLINT(runtime/explicit)
      : value_(saturated_cast<T>(static_cast<Src>(value))) {}


  template <typename Dst>
  constexpr ClampedNumeric<typename UnderlyingType<Dst>::type> Cast() const {
    return *this;
  }

  template <typename Src>
  constexpr ClampedNumeric& operator+=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator-=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator*=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator/=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator%=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator<<=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator>>=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator&=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator|=(const Src rhs);
  template <typename Src>
  constexpr ClampedNumeric& operator^=(const Src rhs);

  constexpr ClampedNumeric operator-() const {


    return ClampedNumeric<T>(SaturatedNegWrapper(value_));
  }

  constexpr ClampedNumeric operator~() const {
    return ClampedNumeric<decltype(InvertWrapper(T()))>(InvertWrapper(value_));
  }

  constexpr ClampedNumeric Abs() const {


    return ClampedNumeric<T>(SaturatedAbsWrapper(value_));
  }

  template <typename U>
  constexpr ClampedNumeric<typename MathWrapper<ClampedMaxOp, T, U>::type> Max(
      const U rhs) const {
    using result_type = typename MathWrapper<ClampedMaxOp, T, U>::type;
    return ClampedNumeric<result_type>(
        ClampedMaxOp<T, U>::Do(value_, Wrapper<U>::value(rhs)));
  }

  template <typename U>
  constexpr ClampedNumeric<typename MathWrapper<ClampedMinOp, T, U>::type> Min(
      const U rhs) const {
    using result_type = typename MathWrapper<ClampedMinOp, T, U>::type;
    return ClampedNumeric<result_type>(
        ClampedMinOp<T, U>::Do(value_, Wrapper<U>::value(rhs)));
  }



  constexpr ClampedNumeric<typename UnsignedOrFloatForSize<T>::type>
  UnsignedAbs() const {
    return ClampedNumeric<typename UnsignedOrFloatForSize<T>::type>(
        SafeUnsignedAbs(value_));
  }

  constexpr ClampedNumeric& operator++() {
    *this += 1;
    return *this;
  }

  constexpr ClampedNumeric operator++(int) {
    ClampedNumeric value = *this;
    *this += 1;
    return value;
  }

  constexpr ClampedNumeric& operator--() {
    *this -= 1;
    return *this;
  }

  constexpr ClampedNumeric operator--(int) {
    ClampedNumeric value = *this;
    *this -= 1;
    return value;
  }


  template <template <typename, typename, typename> class M,
            typename L,
            typename R>
  static constexpr ClampedNumeric MathOp(const L lhs, const R rhs) {
    using Math = typename MathWrapper<M, L, R>::math;
    return ClampedNumeric<T>(
        Math::template Do<T>(Wrapper<L>::value(lhs), Wrapper<R>::value(rhs)));
  }

  template <template <typename, typename, typename> class M, typename R>
  constexpr ClampedNumeric& MathOp(const R rhs) {
    using Math = typename MathWrapper<M, T, R>::math;
    *this =
        ClampedNumeric<T>(Math::template Do<T>(value_, Wrapper<R>::value(rhs)));
    return *this;
  }

  template <typename Dst>
  constexpr operator Dst() const {
    return saturated_cast<typename ArithmeticOrUnderlyingEnum<Dst>::type>(
        value_);
  }



  constexpr T RawValue() const { return value_; }

 private:
  T value_;


  template <typename Src>
  struct Wrapper {
    static constexpr Src value(Src value) {
      return static_cast<typename UnderlyingType<Src>::type>(value);
    }
  };
};

// or ClampedNumericType.
template <typename T>
constexpr ClampedNumeric<typename UnderlyingType<T>::type> MakeClampedNum(
    const T value) {
  return value;
}

#if !BASE_NUMERICS_DISABLE_OSTREAM_OPERATORS
// Overload the ostream output operator to make logging work nicely.
template <typename T>
std::ostream& operator<<(std::ostream& os, const ClampedNumeric<T>& value) {
  os << static_cast<T>(value);
  return os;
}
#endif

template <template <typename, typename, typename> class M,
          typename L,
          typename R>
constexpr ClampedNumeric<typename MathWrapper<M, L, R>::type> ClampMathOp(
    const L lhs,
    const R rhs) {
  using Math = typename MathWrapper<M, L, R>::math;
  return ClampedNumeric<typename Math::result_type>::template MathOp<M>(lhs,
                                                                        rhs);
}

template <template <typename, typename, typename> class M,
          typename L,
          typename R,
          typename... Args>
constexpr ClampedNumeric<typename ResultType<M, L, R, Args...>::type>
ClampMathOp(const L lhs, const R rhs, const Args... args) {
  return ClampMathOp<M>(ClampMathOp<M>(lhs, rhs), args...);
}

BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Add, +, +=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Sub, -, -=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Mul, *, *=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Div, /, /=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Mod, %, %=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Lsh, <<, <<=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Rsh, >>, >>=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, And, &, &=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Or, |, |=)
BASE_NUMERIC_ARITHMETIC_OPERATORS(Clamped, Clamp, Xor, ^, ^=)
BASE_NUMERIC_ARITHMETIC_VARIADIC(Clamped, Clamp, Max)
BASE_NUMERIC_ARITHMETIC_VARIADIC(Clamped, Clamp, Min)
BASE_NUMERIC_COMPARISON_OPERATORS(Clamped, IsLess, <)
BASE_NUMERIC_COMPARISON_OPERATORS(Clamped, IsLessOrEqual, <=)
BASE_NUMERIC_COMPARISON_OPERATORS(Clamped, IsGreater, >)
BASE_NUMERIC_COMPARISON_OPERATORS(Clamped, IsGreaterOrEqual, >=)
BASE_NUMERIC_COMPARISON_OPERATORS(Clamped, IsEqual, ==)
BASE_NUMERIC_COMPARISON_OPERATORS(Clamped, IsNotEqual, !=)

}  // namespace internal

using internal::ClampedNumeric;
using internal::MakeClampedNum;
using internal::ClampMax;
using internal::ClampMin;
using internal::ClampAdd;
using internal::ClampSub;
using internal::ClampMul;
using internal::ClampDiv;
using internal::ClampMod;
using internal::ClampLsh;
using internal::ClampRsh;
using internal::ClampAnd;
using internal::ClampOr;
using internal::ClampXor;

}  // namespace base

#endif  // BASE_NUMERICS_CLAMPED_MATH_H_
