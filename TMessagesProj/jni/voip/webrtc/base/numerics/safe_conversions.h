// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_NUMERICS_SAFE_CONVERSIONS_H_
#define BASE_NUMERICS_SAFE_CONVERSIONS_H_

#include <stddef.h>

#include <limits>
#include <type_traits>

#include "base/numerics/safe_conversions_impl.h"

#if !defined(__native_client__) && (defined(__ARMEL__) || defined(__arch64__))
#include "base/numerics/safe_conversions_arm_impl.h"
#define BASE_HAS_OPTIMIZED_SAFE_CONVERSIONS (1)
#else
#define BASE_HAS_OPTIMIZED_SAFE_CONVERSIONS (0)
#endif

#if !BASE_NUMERICS_DISABLE_OSTREAM_OPERATORS
#include <ostream>
#endif

namespace base {
namespace internal {

#if !BASE_HAS_OPTIMIZED_SAFE_CONVERSIONS
template <typename Dst, typename Src>
struct SaturateFastAsmOp {
  static const bool is_supported = false;
  static constexpr Dst Do(Src) {

    return CheckOnFailure::template HandleFailure<Dst>();
  }
};
#endif  // BASE_HAS_OPTIMIZED_SAFE_CONVERSIONS
#undef BASE_HAS_OPTIMIZED_SAFE_CONVERSIONS

// eke out better performance than range checking.
template <typename Dst, typename Src, typename Enable = void>
struct IsValueInRangeFastOp {
  static const bool is_supported = false;
  static constexpr bool Do(Src value) {

    return CheckOnFailure::template HandleFailure<bool>();
  }
};

template <typename Dst, typename Src>
struct IsValueInRangeFastOp<
    Dst,
    Src,
    typename std::enable_if<
        std::is_integral<Dst>::value && std::is_integral<Src>::value &&
        std::is_signed<Dst>::value && std::is_signed<Src>::value &&
        !IsTypeInRangeForNumericType<Dst, Src>::value>::type> {
  static const bool is_supported = true;

  static constexpr bool Do(Src value) {


    return value == static_cast<Dst>(value);
  }
};

template <typename Dst, typename Src>
struct IsValueInRangeFastOp<
    Dst,
    Src,
    typename std::enable_if<
        std::is_integral<Dst>::value && std::is_integral<Src>::value &&
        !std::is_signed<Dst>::value && std::is_signed<Src>::value &&
        !IsTypeInRangeForNumericType<Dst, Src>::value>::type> {
  static const bool is_supported = true;

  static constexpr bool Do(Src value) {


    return as_unsigned(value) <= as_unsigned(CommonMax<Src, Dst>());
  }
};

// for the destination type.
template <typename Dst, typename Src>
constexpr bool IsValueInRangeForNumericType(Src value) {
  using SrcType = typename internal::UnderlyingType<Src>::type;
  return internal::IsValueInRangeFastOp<Dst, SrcType>::is_supported
             ? internal::IsValueInRangeFastOp<Dst, SrcType>::Do(
                   static_cast<SrcType>(value))
             : internal::DstRangeRelationToSrcRange<Dst>(
                   static_cast<SrcType>(value))
                   .IsValid();
}

// except that it CHECKs that the specified numeric conversion will not
// overflow or underflow. NaN source will always trigger a CHECK.
template <typename Dst,
          class CheckHandler = internal::CheckOnFailure,
          typename Src>
constexpr Dst checked_cast(Src value) {


  using SrcType = typename internal::UnderlyingType<Src>::type;
  return BASE_NUMERICS_LIKELY((IsValueInRangeForNumericType<Dst>(value)))
             ? static_cast<Dst>(static_cast<SrcType>(value))
             : CheckHandler::template HandleFailure<Dst>();
}

// You may provide your own limits (e.g. to saturated_cast) so long as you
// implement all of the static constexpr member functions in the class below.
template <typename T>
struct SaturationDefaultLimits : public std::numeric_limits<T> {
  static constexpr T NaN() {
    return std::numeric_limits<T>::has_quiet_NaN
               ? std::numeric_limits<T>::quiet_NaN()
               : T();
  }
  using std::numeric_limits<T>::max;
  static constexpr T Overflow() {
    return std::numeric_limits<T>::has_infinity
               ? std::numeric_limits<T>::infinity()
               : std::numeric_limits<T>::max();
  }
  using std::numeric_limits<T>::lowest;
  static constexpr T Underflow() {
    return std::numeric_limits<T>::has_infinity
               ? std::numeric_limits<T>::infinity() * -1
               : std::numeric_limits<T>::lowest();
  }
};

template <typename Dst, template <typename> class S, typename Src>
constexpr Dst saturated_cast_impl(Src value, RangeCheck constraint) {


  return !constraint.IsOverflowFlagSet()
             ? (!constraint.IsUnderflowFlagSet() ? static_cast<Dst>(value)
                                                 : S<Dst>::Underflow())

             : (std::is_integral<Src>::value || !constraint.IsUnderflowFlagSet()
                    ? S<Dst>::Overflow()
                    : S<Dst>::NaN());
}

// for normal signed and unsigned integer ranges. And in the specific case of
// Arm, we can use the optimized saturation instructions.
template <typename Dst, typename Src, typename Enable = void>
struct SaturateFastOp {
  static const bool is_supported = false;
  static constexpr Dst Do(Src value) {

    return CheckOnFailure::template HandleFailure<Dst>();
  }
};

template <typename Dst, typename Src>
struct SaturateFastOp<
    Dst,
    Src,
    typename std::enable_if<std::is_integral<Src>::value &&
                            std::is_integral<Dst>::value &&
                            SaturateFastAsmOp<Dst, Src>::is_supported>::type> {
  static const bool is_supported = true;
  static Dst Do(Src value) { return SaturateFastAsmOp<Dst, Src>::Do(value); }
};

template <typename Dst, typename Src>
struct SaturateFastOp<
    Dst,
    Src,
    typename std::enable_if<std::is_integral<Src>::value &&
                            std::is_integral<Dst>::value &&
                            !SaturateFastAsmOp<Dst, Src>::is_supported>::type> {
  static const bool is_supported = true;
  static Dst Do(Src value) {



    Dst saturated = CommonMaxOrMin<Dst, Src>(
        IsMaxInRangeForNumericType<Dst, Src>() ||
        (!IsMinInRangeForNumericType<Dst, Src>() && IsValueNegative(value)));
    return BASE_NUMERICS_LIKELY(IsValueInRangeForNumericType<Dst>(value))
               ? static_cast<Dst>(value)
               : saturated;
  }
};

// that the specified numeric conversion will saturate by default rather than
// overflow or underflow, and NaN assignment to an integral will return 0.
// All boundary condition behaviors can be overriden with a custom handler.
template <typename Dst,
          template <typename> class SaturationHandler = SaturationDefaultLimits,
          typename Src>
constexpr Dst saturated_cast(Src value) {
  using SrcType = typename UnderlyingType<Src>::type;
  return !IsCompileTimeConstant(value) &&
                 SaturateFastOp<Dst, SrcType>::is_supported &&
                 std::is_same<SaturationHandler<Dst>,
                              SaturationDefaultLimits<Dst>>::value
             ? SaturateFastOp<Dst, SrcType>::Do(static_cast<SrcType>(value))
             : saturated_cast_impl<Dst, SaturationHandler, SrcType>(
                   static_cast<SrcType>(value),
                   DstRangeRelationToSrcRange<Dst, SaturationHandler, SrcType>(
                       static_cast<SrcType>(value)));
}

// it will cause a compile failure if the destination type is not large enough
// to contain any value in the source type. It performs no runtime checking.
template <typename Dst, typename Src>
constexpr Dst strict_cast(Src value) {
  using SrcType = typename UnderlyingType<Src>::type;
  static_assert(UnderlyingType<Src>::is_numeric, "Argument must be numeric.");
  static_assert(std::is_arithmetic<Dst>::value, "Result must be numeric.");






  static_assert(StaticDstRangeRelationToSrcRange<Dst, SrcType>::value ==
                    NUMERIC_RANGE_CONTAINED,
                "The source type is out of range for the destination type. "
                "Please see strict_cast<> comments for more information.");

  return static_cast<Dst>(static_cast<SrcType>(value));
}

template <typename Dst, typename Src, class Enable = void>
struct IsNumericRangeContained {
  static const bool value = false;
};

template <typename Dst, typename Src>
struct IsNumericRangeContained<
    Dst,
    Src,
    typename std::enable_if<ArithmeticOrUnderlyingEnum<Dst>::value &&
                            ArithmeticOrUnderlyingEnum<Src>::value>::type> {
  static const bool value = StaticDstRangeRelationToSrcRange<Dst, Src>::value ==
                            NUMERIC_RANGE_CONTAINED;
};

// wrapping assignment operations in a strict_cast. This class is intended to be
// used for function arguments and return types, to ensure the destination type
// can always contain the source type. This is essentially the same as enforcing
// -Wconversion in gcc and C4302 warnings on MSVC, but it can be applied
// incrementally at API boundaries, making it easier to convert code so that it
// compiles cleanly with truncation warnings enabled.
// This template should introduce no runtime overhead, but it also provides no
// runtime checking of any of the associated mathematical operations. Use
// CheckedNumeric for runtime range checks of the actual value being assigned.
template <typename T>
class StrictNumeric {
 public:
  using type = T;

  constexpr StrictNumeric() : value_(0) {}

  template <typename Src>
  constexpr StrictNumeric(const StrictNumeric<Src>& rhs)
      : value_(strict_cast<T>(rhs.value_)) {}


  template <typename Src>
  constexpr StrictNumeric(Src value)  // NOLINT(runtime/explicit)
      : value_(strict_cast<T>(value)) {}












  template <typename Dst,
            typename std::enable_if<
                IsNumericRangeContained<Dst, T>::value>::type* = nullptr>
  constexpr operator Dst() const {
    return static_cast<typename ArithmeticOrUnderlyingEnum<Dst>::type>(value_);
  }

 private:
  const T value_;
};

template <typename T>
constexpr StrictNumeric<typename UnderlyingType<T>::type> MakeStrictNum(
    const T value) {
  return value;
}

#if !BASE_NUMERICS_DISABLE_OSTREAM_OPERATORS
// Overload the ostream output operator to make logging work nicely.
template <typename T>
std::ostream& operator<<(std::ostream& os, const StrictNumeric<T>& value) {
  os << static_cast<T>(value);
  return os;
}
#endif

#define BASE_NUMERIC_COMPARISON_OPERATORS(CLASS, NAME, OP)              \
  template <typename L, typename R,                                     \
            typename std::enable_if<                                    \
                internal::Is##CLASS##Op<L, R>::value>::type* = nullptr> \
  constexpr bool operator OP(const L lhs, const R rhs) {                \
    return SafeCompare<NAME, typename UnderlyingType<L>::type,          \
                       typename UnderlyingType<R>::type>(lhs, rhs);     \
  }

BASE_NUMERIC_COMPARISON_OPERATORS(Strict, IsLess, <)
BASE_NUMERIC_COMPARISON_OPERATORS(Strict, IsLessOrEqual, <=)
BASE_NUMERIC_COMPARISON_OPERATORS(Strict, IsGreater, >)
BASE_NUMERIC_COMPARISON_OPERATORS(Strict, IsGreaterOrEqual, >=)
BASE_NUMERIC_COMPARISON_OPERATORS(Strict, IsEqual, ==)
BASE_NUMERIC_COMPARISON_OPERATORS(Strict, IsNotEqual, !=)

}  // namespace internal

using internal::as_signed;
using internal::as_unsigned;
using internal::checked_cast;
using internal::strict_cast;
using internal::saturated_cast;
using internal::SafeUnsignedAbs;
using internal::StrictNumeric;
using internal::MakeStrictNum;
using internal::IsValueInRangeForNumericType;
using internal::IsTypeInRangeForNumericType;
using internal::IsValueNegative;

using SizeT = StrictNumeric<size_t>;

}  // namespace base

#endif  // BASE_NUMERICS_SAFE_CONVERSIONS_H_
