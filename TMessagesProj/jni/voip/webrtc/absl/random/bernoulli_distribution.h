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

#ifndef ABSL_RANDOM_BERNOULLI_DISTRIBUTION_H_
#define ABSL_RANDOM_BERNOULLI_DISTRIBUTION_H_

#include <cstdint>
#include <istream>
#include <limits>

#include "absl/base/optimization.h"
#include "absl/random/internal/fast_uniform_bits.h"
#include "absl/random/internal/iostream_state_saver.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

// std::bernoulli_distribution. It guarantees that (given a perfect
// UniformRandomBitGenerator) the acceptance probability is *exactly* equal to
// the given double.
//
// The implementation assumes that double is IEEE754
class bernoulli_distribution {
 public:
  using result_type = bool;

  class param_type {
   public:
    using distribution_type = bernoulli_distribution;

    explicit param_type(double p = 0.5) : prob_(p) {
      assert(p >= 0.0 && p <= 1.0);
    }

    double p() const { return prob_; }

    friend bool operator==(const param_type& p1, const param_type& p2) {
      return p1.p() == p2.p();
    }
    friend bool operator!=(const param_type& p1, const param_type& p2) {
      return p1.p() != p2.p();
    }

   private:
    double prob_;
  };

  bernoulli_distribution() : bernoulli_distribution(0.5) {}

  explicit bernoulli_distribution(double p) : param_(p) {}

  explicit bernoulli_distribution(param_type p) : param_(p) {}

  void reset() {}

  template <typename URBG>
  bool operator()(URBG& g) {  // NOLINT(runtime/references)
    return Generate(param_.p(), g);
  }

  template <typename URBG>
  bool operator()(URBG& g,  // NOLINT(runtime/references)
                  const param_type& param) {
    return Generate(param.p(), g);
  }

  param_type param() const { return param_; }
  void param(const param_type& param) { param_ = param; }

  double p() const { return param_.p(); }

  result_type(min)() const { return false; }
  result_type(max)() const { return true; }

  friend bool operator==(const bernoulli_distribution& d1,
                         const bernoulli_distribution& d2) {
    return d1.param_ == d2.param_;
  }

  friend bool operator!=(const bernoulli_distribution& d1,
                         const bernoulli_distribution& d2) {
    return d1.param_ != d2.param_;
  }

 private:
  static constexpr uint64_t kP32 = static_cast<uint64_t>(1) << 32;

  template <typename URBG>
  static bool Generate(double p, URBG& g);  // NOLINT(runtime/references)

  param_type param_;
};

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,  // NOLINT(runtime/references)
    const bernoulli_distribution& x) {
  auto saver = random_internal::make_ostream_state_saver(os);
  os.precision(random_internal::stream_precision_helper<double>::kPrecision);
  os << x.p();
  return os;
}

template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(
    std::basic_istream<CharT, Traits>& is,  // NOLINT(runtime/references)
    bernoulli_distribution& x) {            // NOLINT(runtime/references)
  auto saver = random_internal::make_istream_state_saver(is);
  auto p = random_internal::read_floating_point<double>(is);
  if (!is.fail()) {
    x.param(bernoulli_distribution::param_type(p));
  }
  return is;
}

template <typename URBG>
bool bernoulli_distribution::Generate(double p,
                                      URBG& g) {  // NOLINT(runtime/references)
  random_internal::FastUniformBits<uint32_t> fast_u32;

  while (true) {















    const uint64_t c = static_cast<uint64_t>(static_cast<int64_t>(p * kP32));
    const uint32_t v = fast_u32(g);




    if (ABSL_PREDICT_TRUE(v != c)) return v < c;




    const double q = static_cast<double>(c) / kP32;
















    const double left = p - q;



    const double here = left * kP32;





    if (here == 0) return false;
    p = here;
  }
}

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_BERNOULLI_DISTRIBUTION_H_
