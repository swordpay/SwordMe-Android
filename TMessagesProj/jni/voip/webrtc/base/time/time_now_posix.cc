// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time/time.h"

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#if defined(OS_ANDROID) && !defined(__LP64__)
#include <time64.h>
#endif
#include <unistd.h>

#include "base/logging.h"
#include "base/numerics/safe_math.h"
#include "base/time/time_override.h"
#include "build/build_config.h"

// non-POSIX implementation is used for sampling the system clocks.
#if defined(OS_FUCHSIA) || defined(OS_MACOSX)
#error "This implementation is for POSIX platforms other than Fuchsia or Mac."
#endif

namespace {

int64_t ConvertTimespecToMicros(const struct timespec& ts) {


  if (sizeof(ts.tv_sec) <= 4 && sizeof(ts.tv_nsec) <= 8) {
    int64_t result = ts.tv_sec;
    result *= base::Time::kMicrosecondsPerSecond;
    result += (ts.tv_nsec / base::Time::kNanosecondsPerMicrosecond);
    return result;
  }
  base::CheckedNumeric<int64_t> result(ts.tv_sec);
  result *= base::Time::kMicrosecondsPerSecond;
  result += (ts.tv_nsec / base::Time::kNanosecondsPerMicrosecond);
  return result.ValueOrDie();
}

// microsecond timebase. Minimum requirement is MONOTONIC_CLOCK to be supported
// on the system. FreeBSD 6 has CLOCK_MONOTONIC but defines
// _POSIX_MONOTONIC_CLOCK to -1.
#if (defined(OS_POSIX) && defined(_POSIX_MONOTONIC_CLOCK) && \
     _POSIX_MONOTONIC_CLOCK >= 0) ||                         \
    defined(OS_BSD) || defined(OS_ANDROID)
int64_t ClockNow(clockid_t clk_id) {
  struct timespec ts;
  CHECK(clock_gettime(clk_id, &ts) == 0);
  return ConvertTimespecToMicros(ts);
}
#else  // _POSIX_MONOTONIC_CLOCK
#error No usable tick clock function on this platform.
#endif  // _POSIX_MONOTONIC_CLOCK

}  // namespace

namespace base {


namespace subtle {
Time TimeNowIgnoringOverride() {
  struct timeval tv;
  struct timezone tz = {0, 0};  // UTC
  CHECK(gettimeofday(&tv, &tz) == 0);



  return Time() + TimeDelta::FromMicroseconds(
                      (tv.tv_sec * Time::kMicrosecondsPerSecond + tv.tv_usec) +
                      Time::kTimeTToMicrosecondsOffset);
}

Time TimeNowFromSystemTimeIgnoringOverride() {

  return TimeNowIgnoringOverride();
}
}  // namespace subtle


namespace subtle {
TimeTicks TimeTicksNowIgnoringOverride() {
  return TimeTicks() + TimeDelta::FromMicroseconds(ClockNow(CLOCK_MONOTONIC));
}
}  // namespace subtle

TimeTicks::Clock TimeTicks::GetClock() {
  return Clock::LINUX_CLOCK_MONOTONIC;
}

bool TimeTicks::IsHighResolution() {
  return true;
}

bool TimeTicks::IsConsistentAcrossProcesses() {
  return true;
}


namespace subtle {
ThreadTicks ThreadTicksNowIgnoringOverride() {
#if (defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME >= 0)) || \
    defined(OS_ANDROID)
  return ThreadTicks() +
         TimeDelta::FromMicroseconds(ClockNow(CLOCK_THREAD_CPUTIME_ID));
#else
  NOTREACHED();
  return ThreadTicks();
#endif
}
}  // namespace subtle

}  // namespace base
