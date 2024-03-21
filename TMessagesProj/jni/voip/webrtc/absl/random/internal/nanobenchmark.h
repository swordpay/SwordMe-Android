// Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef ABSL_RANDOM_INTERNAL_NANOBENCHMARK_H_
#define ABSL_RANDOM_INTERNAL_NANOBENCHMARK_H_

// prediction hit rates. Uses a robust estimator to summarize the measurements.
// The precision is about 0.2%.
//
// Examples: see nanobenchmark_test.cc.
//
// Background: Microbenchmarks such as http://github.com/google/benchmark
// can measure elapsed times on the order of a microsecond. Shorter functions
// are typically measured by repeating them thousands of times and dividing
// the total elapsed time by this count. Unfortunately, repetition (especially
// with the same input parameter!) influences the runtime. In time-critical
// code, it is reasonable to expect warm instruction/data caches and TLBs,
// but a perfect record of which branches will be taken is unrealistic.
// Unless the application also repeatedly invokes the measured function with
// the same parameter, the benchmark is measuring something very different -
// a best-case result, almost as if the parameter were made a compile-time
// constant. This may lead to erroneous conclusions about branch-heavy
// algorithms outperforming branch-free alternatives.
//
// Our approach differs in three ways. Adding fences to the timer functions
// reduces variability due to instruction reordering, improving the timer
// resolution to about 40 CPU cycles. However, shorter functions must still
// be invoked repeatedly. For more realistic branch prediction performance,
// we vary the input parameter according to a user-specified distribution.
// Thus, instead of VaryInputs(Measure(Repeat(func))), we change the
// loop nesting to Measure(Repeat(VaryInputs(func))). We also estimate the
// central tendency of the measurement samples with the "half sample mode",
// which is more robust to outliers and skewed data than the mean or median.

// distinct flags, avoid #including headers that define functions.

#include <stddef.h>
#include <stdint.h>

#include "absl/base/config.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace random_internal_nanobenchmark {

using FuncInput = size_t;

using FuncOutput = uint64_t;

// arguments or 2) a lambda with capture, in which case the first argument
// is reserved for use by MeasureClosure.
using Func = FuncOutput (*)(const void*, FuncInput);

struct Params {



  static constexpr size_t kTimerSamples = 256;


  size_t precision_divisor = 1024;



  size_t subset_ratio = 2;



  double seconds_per_eval = 4E-3;

  size_t min_samples_per_eval = 7;



  size_t min_mode_samples = 64;

  double target_rel_mad = 0.002;


  size_t max_evals = 9;

  size_t max_measure_retries = 2;

  bool verbose = true;
};

struct Result {
  FuncInput input;

  float ticks;

  float variability;
};

// Reduces noise due to desynchronized socket RDTSC and context switches.
// If "cpu" is negative, pin to the currently running core.
void PinThreadToCPU(const int cpu = -1);

// means the tick counter frequency is independent of CPU throttling or sleep.
// This call may be expensive, callers should cache the result.
double InvariantTicksPerSecond();

// given inputs, shuffled to ensure realistic branch prediction hit rates.
//
// "func" returns a 'proof of work' to ensure its computations are not elided.
// "arg" is passed to Func, or reserved for internal use by MeasureClosure.
// "inputs" is an array of "num_inputs" (not necessarily unique) arguments to
//   "func". The values should be chosen to maximize coverage of "func". This
//   represents a distribution, so a value's frequency should reflect its
//   probability in the real application. Order does not matter; for example, a
//   uniform distribution over [0, 4) could be represented as {3,0,2,1}.
// Returns how many Result were written to "results": one per unique input, or
//   zero if the measurement failed (an error message goes to stderr).
size_t Measure(const Func func, const void* arg, const FuncInput* inputs,
               const size_t num_inputs, Result* results,
               const Params& p = Params());

template <class Closure>
static FuncOutput CallClosure(const void* f, const FuncInput input) {
  return (*reinterpret_cast<const Closure*>(f))(input);
}

// FuncInput -> FuncOutput with a capture list.
template <class Closure>
static inline size_t MeasureClosure(const Closure& closure,
                                    const FuncInput* inputs,
                                    const size_t num_inputs, Result* results,
                                    const Params& p = Params()) {
  return Measure(reinterpret_cast<Func>(&CallClosure<Closure>),
                 reinterpret_cast<const void*>(&closure), inputs, num_inputs,
                 results, p);
}

}  // namespace random_internal_nanobenchmark
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_INTERNAL_NANOBENCHMARK_H_
