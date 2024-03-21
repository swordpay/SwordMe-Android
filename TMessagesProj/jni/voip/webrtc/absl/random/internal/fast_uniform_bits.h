// Copyright 2017 The Abseil Authors.
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

#ifndef ABSL_RANDOM_INTERNAL_FAST_UNIFORM_BITS_H_
#define ABSL_RANDOM_INTERNAL_FAST_UNIFORM_BITS_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "absl/base/config.h"
#include "absl/meta/type_traits.h"
#include "absl/random/internal/traits.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace random_internal {
// Returns true if the input value is zero or a power of two. Useful for
// determining if the range of output values in a URBG
template <typename UIntType>
constexpr bool IsPowerOfTwoOrZero(UIntType n) {
  return (n == 0) || ((n & (n - 1)) == 0);
}

// zero if that would encompass the entire range of representable values in
// URBG::result_type.
template <typename URBG>
constexpr typename URBG::result_type RangeSize() {
  using result_type = typename URBG::result_type;
  static_assert((URBG::max)() != (URBG::min)(), "URBG range cannot be 0.");
  return ((URBG::max)() == (std::numeric_limits<result_type>::max)() &&
          (URBG::min)() == std::numeric_limits<result_type>::lowest())
             ? result_type{0}
             : ((URBG::max)() - (URBG::min)() + result_type{1});
}

template <typename UIntType>
constexpr UIntType IntegerLog2(UIntType n) {
  return (n <= 1) ? 0 : 1 + IntegerLog2(n >> 1);
}

// `PowerOfTwoVariate(urbg)`.
template <typename URBG>
constexpr size_t NumBits() {
  return RangeSize<URBG>() == 0
             ? std::numeric_limits<typename URBG::result_type>::digits
             : IntegerLog2(RangeSize<URBG>());
}

// If `n == 0`, all bits are set.
template <typename UIntType>
constexpr UIntType MaskFromShift(size_t n) {
  return ((n % std::numeric_limits<UIntType>::digits) == 0)
             ? ~UIntType{0}
             : (UIntType{1} << n) - UIntType{1};
}

// entropy extraction algorithm.
struct SimplifiedLoopTag {};
struct RejectionLoopTag {};

// from a type which conforms to the [rand.req.urbg] concept.
// Parameterized by:
//  `UIntType`: the result (output) type
//
// The std::independent_bits_engine [rand.adapt.ibits] adaptor can be
// instantiated from an existing generator through a copy or a move. It does
// not, however, facilitate the production of pseudorandom bits from an un-owned
// generator that will outlive the std::independent_bits_engine instance.
template <typename UIntType = uint64_t>
class FastUniformBits {
 public:
  using result_type = UIntType;

  static constexpr result_type(min)() { return 0; }
  static constexpr result_type(max)() {
    return (std::numeric_limits<result_type>::max)();
  }

  template <typename URBG>
  result_type operator()(URBG& g);  // NOLINT(runtime/references)

 private:
  static_assert(IsUnsigned<UIntType>::value,
                "Class-template FastUniformBits<> must be parameterized using "
                "an unsigned type.");



  template <typename URBG>
  result_type Generate(URBG& g,  // NOLINT(runtime/references)
                       SimplifiedLoopTag);

  template <typename URBG>
  result_type Generate(URBG& g,  // NOLINT(runtime/references)
                       RejectionLoopTag);
};

template <typename UIntType>
template <typename URBG>
typename FastUniformBits<UIntType>::result_type
FastUniformBits<UIntType>::operator()(URBG& g) {  // NOLINT(runtime/references)



  static_assert((URBG::max)() > (URBG::min)(),
                "URBG::max and URBG::min may not be equal.");

  using tag = absl::conditional_t<IsPowerOfTwoOrZero(RangeSize<URBG>()),
                                  SimplifiedLoopTag, RejectionLoopTag>;
  return Generate(g, tag{});
}

template <typename UIntType>
template <typename URBG>
typename FastUniformBits<UIntType>::result_type
FastUniformBits<UIntType>::Generate(URBG& g,  // NOLINT(runtime/references)
                                    SimplifiedLoopTag) {



  static_assert(IsPowerOfTwoOrZero(RangeSize<URBG>()),
                "incorrect Generate tag for URBG instance");

  static constexpr size_t kResultBits =
      std::numeric_limits<result_type>::digits;
  static constexpr size_t kUrbgBits = NumBits<URBG>();
  static constexpr size_t kIters =
      (kResultBits / kUrbgBits) + (kResultBits % kUrbgBits != 0);
  static constexpr size_t kShift = (kIters == 1) ? 0 : kUrbgBits;
  static constexpr auto kMin = (URBG::min)();

  result_type r = static_cast<result_type>(g() - kMin);
  for (size_t n = 1; n < kIters; ++n) {
    r = static_cast<result_type>(r << kShift) +
        static_cast<result_type>(g() - kMin);
  }
  return r;
}

template <typename UIntType>
template <typename URBG>
typename FastUniformBits<UIntType>::result_type
FastUniformBits<UIntType>::Generate(URBG& g,  // NOLINT(runtime/references)
                                    RejectionLoopTag) {
  static_assert(!IsPowerOfTwoOrZero(RangeSize<URBG>()),
                "incorrect Generate tag for URBG instance");
  using urbg_result_type = typename URBG::result_type;

















  static constexpr size_t kResultBits =
      std::numeric_limits<result_type>::digits;                      // w
  static constexpr urbg_result_type kUrbgRange = RangeSize<URBG>();  // R
  static constexpr size_t kUrbgBits = NumBits<URBG>();               // m


  static constexpr size_t kA =  // ceil(w/m)
      (kResultBits / kUrbgBits) + ((kResultBits % kUrbgBits) != 0);  // n'

  static constexpr size_t kABits = kResultBits / kA;  // w0'
  static constexpr urbg_result_type kARejection =
      ((kUrbgRange >> kABits) << kABits);  // y0'

  static constexpr size_t kTotalIters =
      ((kUrbgRange - kARejection) <= (kARejection / kA)) ? kA : (kA + 1);  // n

  static constexpr size_t kSmallIters =
      kTotalIters - (kResultBits % kTotalIters);                   // n0
  static constexpr size_t kSmallBits = kResultBits / kTotalIters;  // w0
  static constexpr urbg_result_type kSmallRejection =
      ((kUrbgRange >> kSmallBits) << kSmallBits);  // y0

  static constexpr size_t kLargeBits = kSmallBits + 1;  // w0+1
  static constexpr urbg_result_type kLargeRejection =
      ((kUrbgRange >> kLargeBits) << kLargeBits);  // y1














  static_assert(kResultBits == kSmallIters * kSmallBits +
                                   (kTotalIters - kSmallIters) * kLargeBits,
                "Error in looping constant calculations.");



  static constexpr size_t kSmallShift = kSmallBits % kResultBits;
  static constexpr auto kSmallMask =
      MaskFromShift<urbg_result_type>(kSmallShift);
  static constexpr size_t kLargeShift = kLargeBits % kResultBits;
  static constexpr auto kLargeMask =
      MaskFromShift<urbg_result_type>(kLargeShift);

  static constexpr auto kMin = (URBG::min)();

  result_type s = 0;
  for (size_t n = 0; n < kSmallIters; ++n) {
    urbg_result_type v;
    do {
      v = g() - kMin;
    } while (v >= kSmallRejection);

    s = (s << kSmallShift) + static_cast<result_type>(v & kSmallMask);
  }

  for (size_t n = kSmallIters; n < kTotalIters; ++n) {
    urbg_result_type v;
    do {
      v = g() - kMin;
    } while (v >= kLargeRejection);

    s = (s << kLargeShift) + static_cast<result_type>(v & kLargeMask);
  }
  return s;
}

}  // namespace random_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_INTERNAL_FAST_UNIFORM_BITS_H_
