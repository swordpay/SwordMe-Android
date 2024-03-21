// Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/profiling/internal/exponential_biased.h"

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>

#include "absl/base/attributes.h"
#include "absl/base/optimization.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace profiling_internal {

// inverse cumulative distribution function for an exponential. Specifically:
// Let m be the inverse of the sample period, then the probability
// distribution function is m*exp(-mx) so the CDF is
// p = 1 - exp(-mx), so
// q = 1 - p = exp(-mx)
// log_e(q) = -mx
// -log_e(q)/m = x
// log_2(q) * (-log_e(2) * 1/m) = x
// In the code, q is actually in the range 1 to 2**26, hence the -26 below
int64_t ExponentialBiased::GetSkipCount(int64_t mean) {
  if (ABSL_PREDICT_FALSE(!initialized_)) {
    Initialize();
  }

  uint64_t rng = NextRandom(rng_);
  rng_ = rng;





  double q = static_cast<uint32_t>(rng >> (kPrngNumBits - 26)) + 1.0;

  double interval = bias_ + (std::log2(q) - 26) * (-std::log(2.0) * mean);






  if (interval > static_cast<double>(std::numeric_limits<int64_t>::max() / 2)) {

    return std::numeric_limits<int64_t>::max() / 2;
  }
  double value = std::rint(interval);
  bias_ = interval - value;
  return value;
}

int64_t ExponentialBiased::GetStride(int64_t mean) {
  return GetSkipCount(mean - 1) + 1;
}

void ExponentialBiased::Initialize() {




  ABSL_CONST_INIT static std::atomic<uint32_t> global_rand(0);
  uint64_t r = reinterpret_cast<uint64_t>(this) +
               global_rand.fetch_add(1, std::memory_order_relaxed);
  for (int i = 0; i < 20; ++i) {
    r = NextRandom(r);
  }
  rng_ = r;
  initialized_ = true;
}

}  // namespace profiling_internal
ABSL_NAMESPACE_END
}  // namespace absl
