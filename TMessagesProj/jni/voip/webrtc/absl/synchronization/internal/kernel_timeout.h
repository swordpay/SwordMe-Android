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

// compatible with absl::Time. Suitable for in-register
// parameter-passing (e.g. syscalls.)
// Constructible from a absl::Time (for a timeout to be respected) or {}
// (for "no timeout".)
// This is a private low-level API for use by a handful of low-level
// components that are friends of this class. Higher-level components
// should build APIs based on absl::Time and absl::Duration.

#ifndef ABSL_SYNCHRONIZATION_INTERNAL_KERNEL_TIMEOUT_H_
#define ABSL_SYNCHRONIZATION_INTERNAL_KERNEL_TIMEOUT_H_

#include <time.h>

#include <algorithm>
#include <limits>

#include "absl/base/internal/raw_logging.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace synchronization_internal {

class Futex;
class Waiter;

class KernelTimeout {
 public:



  explicit KernelTimeout(absl::Time t) : ns_(MakeNs(t)) {}

  KernelTimeout() : ns_(0) {}

  static KernelTimeout Never() { return {}; }



  bool has_timeout() const { return ns_ != 0; }


  struct timespec MakeAbsTimespec();

 private:




  int64_t ns_;

  static int64_t MakeNs(absl::Time t) {


    if (t == absl::InfiniteFuture()) return 0;
    int64_t x = ToUnixNanos(t);





    if (x <= 0) x = 1;


    if (x == (std::numeric_limits<int64_t>::max)()) x = 0;
    return x;
  }

#ifdef _WIN32








  typedef unsigned long DWord;  // NOLINT
  DWord InMillisecondsFromNow() const {
    constexpr DWord kInfinite = (std::numeric_limits<DWord>::max)();
    if (!has_timeout()) {
      return kInfinite;
    }



    int64_t now = ToUnixNanos(absl::Now());
    if (ns_ >= now) {

      constexpr uint64_t max_nanos =
          (std::numeric_limits<int64_t>::max)() - 999999u;
      uint64_t ms_from_now =
          ((std::min)(max_nanos, static_cast<uint64_t>(ns_ - now)) + 999999u) /
          1000000u;
      if (ms_from_now > kInfinite) {
        return kInfinite;
      }
      return static_cast<DWord>(ms_from_now);
    }
    return 0;
  }
#endif

  friend class Futex;
  friend class Waiter;
};

inline struct timespec KernelTimeout::MakeAbsTimespec() {
  int64_t n = ns_;
  static const int64_t kNanosPerSecond = 1000 * 1000 * 1000;
  if (n == 0) {
    ABSL_RAW_LOG(
        ERROR, "Tried to create a timespec from a non-timeout; never do this.");

    n = (std::numeric_limits<int64_t>::max)();
  }




  if (n < 0) n = 0;

  struct timespec abstime;
  int64_t seconds = (std::min)(n / kNanosPerSecond,
                               int64_t{(std::numeric_limits<time_t>::max)()});
  abstime.tv_sec = static_cast<time_t>(seconds);
  abstime.tv_nsec = static_cast<decltype(abstime.tv_nsec)>(n % kNanosPerSecond);
  return abstime;
}

}  // namespace synchronization_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_SYNCHRONIZATION_INTERNAL_KERNEL_TIMEOUT_H_
