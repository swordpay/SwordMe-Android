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
//
// -----------------------------------------------------------------------------
// File: uniform_int_distribution.h
// -----------------------------------------------------------------------------
//
// This header defines a class for representing a uniform integer distribution
// over the closed (inclusive) interval [a,b]. You use this distribution in
// combination with an Abseil random bit generator to produce random values
// according to the rules of the distribution.
//
// `absl::uniform_int_distribution` is a drop-in replacement for the C++11
// `std::uniform_int_distribution` [rand.dist.uni.int] but is considerably
// faster than the libstdc++ implementation.

#ifndef ABSL_RANDOM_UNIFORM_INT_DISTRIBUTION_H_
#define ABSL_RANDOM_UNIFORM_INT_DISTRIBUTION_H_

#include <cassert>
#include <istream>
#include <limits>
#include <type_traits>

#include "absl/base/optimization.h"
#include "absl/random/internal/fast_uniform_bits.h"
#include "absl/random/internal/iostream_state_saver.h"
#include "absl/random/internal/traits.h"
#include "absl/random/internal/wide_multiply.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// This distribution produces random integer values uniformly distributed in the
// closed (inclusive) interval [a, b].
//
// Example:
//
//   absl::BitGen gen;
//
//   // Use the distribution to produce a value between 1 and 6, inclusive.
//   int die_roll = absl::uniform_int_distribution<int>(1, 6)(gen);
//
template <typename IntType = int>
class uniform_int_distribution {
 private:
  using unsigned_type =
      typename random_internal::make_unsigned_bits<IntType>::type;

 public:
  using result_type = IntType;

  class param_type {
   public:
    using distribution_type = uniform_int_distribution;

    explicit param_type(
        result_type lo = 0,
        result_type hi = (std::numeric_limits<result_type>::max)())
        : lo_(lo),
          range_(static_cast<unsigned_type>(hi) -
                 static_cast<unsigned_type>(lo)) {

      assert(lo <= hi);
    }

    result_type a() const { return lo_; }
    result_type b() const {
      return static_cast<result_type>(static_cast<unsigned_type>(lo_) + range_);
    }

    friend bool operator==(const param_type& a, const param_type& b) {
      return a.lo_ == b.lo_ && a.range_ == b.range_;
    }

    friend bool operator!=(const param_type& a, const param_type& b) {
      return !(a == b);
    }

   private:
    friend class uniform_int_distribution;
    unsigned_type range() const { return range_; }

    result_type lo_;
    unsigned_type range_;

    static_assert(random_internal::IsIntegral<result_type>::value,
                  "Class-template absl::uniform_int_distribution<> must be "
                  "parameterized using an integral type.");
  };  // param_type

  uniform_int_distribution() : uniform_int_distribution(0) {}

  explicit uniform_int_distribution(
      result_type lo,
      result_type hi = (std::numeric_limits<result_type>::max)())
      : param_(lo, hi) {}

  explicit uniform_int_distribution(const param_type& param) : param_(param) {}




  void reset() {}

  template <typename URBG>
  result_type operator()(URBG& gen) {  // NOLINT(runtime/references)
    return (*this)(gen, param());
  }

  template <typename URBG>
  result_type operator()(
      URBG& gen, const param_type& param) {  // NOLINT(runtime/references)
    return static_cast<result_type>(param.a() + Generate(gen, param.range()));
  }

  result_type a() const { return param_.a(); }
  result_type b() const { return param_.b(); }

  param_type param() const { return param_; }
  void param(const param_type& params) { param_ = params; }

  result_type(min)() const { return a(); }
  result_type(max)() const { return b(); }

  friend bool operator==(const uniform_int_distribution& a,
                         const uniform_int_distribution& b) {
    return a.param_ == b.param_;
  }
  friend bool operator!=(const uniform_int_distribution& a,
                         const uniform_int_distribution& b) {
    return !(a == b);
  }

 private:

  template <typename URBG>
  unsigned_type Generate(URBG& g,  // NOLINT(runtime/references)
                         unsigned_type R);
  param_type param_;
};

// Implementation details follow
// -----------------------------------------------------------------------------
template <typename CharT, typename Traits, typename IntType>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    const uniform_int_distribution<IntType>& x) {
  using stream_type =
      typename random_internal::stream_format_type<IntType>::type;
  auto saver = random_internal::make_ostream_state_saver(os);
  os << static_cast<stream_type>(x.a()) << os.fill()
     << static_cast<stream_type>(x.b());
  return os;
}

template <typename CharT, typename Traits, typename IntType>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,
    uniform_int_distribution<IntType>& x) {
  using param_type = typename uniform_int_distribution<IntType>::param_type;
  using result_type = typename uniform_int_distribution<IntType>::result_type;
  using stream_type =
      typename random_internal::stream_format_type<IntType>::type;

  stream_type a;
  stream_type b;

  auto saver = random_internal::make_istream_state_saver(is);
  is >> a >> b;
  if (!is.fail()) {
    x.param(
        param_type(static_cast<result_type>(a), static_cast<result_type>(b)));
  }
  return is;
}

template <typename IntType>
template <typename URBG>
typename random_internal::make_unsigned_bits<IntType>::type
uniform_int_distribution<IntType>::Generate(
    URBG& g,  // NOLINT(runtime/references)
    typename random_internal::make_unsigned_bits<IntType>::type R) {
  random_internal::FastUniformBits<unsigned_type> fast_bits;
  unsigned_type bits = fast_bits(g);
  const unsigned_type Lim = R + 1;
  if ((R & Lim) == 0) {

    return bits & R;
  }








































  using helper = random_internal::wide_multiply<unsigned_type>;
  auto product = helper::multiply(bits, Lim);





  if (ABSL_PREDICT_FALSE(helper::lo(product) < Lim)) {





    const unsigned_type threshold =
        ((std::numeric_limits<unsigned_type>::max)() - Lim + 1) % Lim;
    while (helper::lo(product) < threshold) {
      bits = fast_bits(g);
      product = helper::multiply(bits, Lim);
    }
  }

  return helper::hi(product);
}

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_UNIFORM_INT_DISTRIBUTION_H_
