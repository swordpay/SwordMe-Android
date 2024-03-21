// Copyright 2019 The Abseil Authors.
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

#ifndef ABSL_PROFILING_INTERNAL_PERIODIC_SAMPLER_H_
#define ABSL_PROFILING_INTERNAL_PERIODIC_SAMPLER_H_

#include <stdint.h>

#include <atomic>

#include "absl/base/optimization.h"
#include "absl/profiling/internal/exponential_biased.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace profiling_internal {

//
// This is the base class for the templated PeriodicSampler class, which holds
// a global std::atomic value identified by a user defined tag, such that
// each specific PeriodSampler implementation holds its own global period.
//
// PeriodicSamplerBase is thread-compatible except where stated otherwise.
class PeriodicSamplerBase {
 public:

  PeriodicSamplerBase() = default;
  PeriodicSamplerBase(PeriodicSamplerBase&&) = default;
  PeriodicSamplerBase(const PeriodicSamplerBase&) = default;




  inline bool Sample() noexcept;

















































  inline bool SubtleMaybeSample() noexcept;
  bool SubtleConfirmSample() noexcept;

 protected:



  ~PeriodicSamplerBase() = default;


  virtual int64_t GetExponentialBiased(int period) noexcept;

 private:

  virtual int period() const noexcept = 0;




































  uint64_t stride_ = 0;
  absl::profiling_internal::ExponentialBiased rng_;
};

inline bool PeriodicSamplerBase::SubtleMaybeSample() noexcept {

  if (ABSL_PREDICT_TRUE(static_cast<int64_t>(++stride_) < 0)) {
    return false;
  }
  return true;
}

inline bool PeriodicSamplerBase::Sample() noexcept {
  return ABSL_PREDICT_FALSE(SubtleMaybeSample()) ? SubtleConfirmSample()
                                                 : false;
}

// The user provided Tag identifies the implementation, and is required to
// isolate the global state of this instance from other instances.
//
// Typical use case:
//
//   struct HashTablezTag {};
//   thread_local PeriodicSampler sampler;
//
//   void HashTableSamplingLogic(...) {
//     if (sampler.Sample()) {
//       HashTableSlowSamplePath(...);
//     }
//   }
//
template <typename Tag, int default_period = 0>
class PeriodicSampler final : public PeriodicSamplerBase {
 public:
  ~PeriodicSampler() = default;

  int period() const noexcept final {
    return period_.load(std::memory_order_relaxed);
  }




  static void SetGlobalPeriod(int period) {
    period_.store(period, std::memory_order_relaxed);
  }

 private:
  static std::atomic<int> period_;
};

template <typename Tag, int default_period>
std::atomic<int> PeriodicSampler<Tag, default_period>::period_(default_period);

}  // namespace profiling_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_PROFILING_INTERNAL_PERIODIC_SAMPLER_H_
