/*
 *  Copyright 2005 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_TIME_UTILS_H_
#define RTC_BASE_TIME_UTILS_H_

#include <stdint.h>
#include <time.h>

#include "rtc_base/checks.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/system_time.h"

namespace rtc {

static const int64_t kNumMillisecsPerSec = INT64_C(1000);
static const int64_t kNumMicrosecsPerSec = INT64_C(1000000);
static const int64_t kNumNanosecsPerSec = INT64_C(1000000000);

static const int64_t kNumMicrosecsPerMillisec =
    kNumMicrosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMillisec =
    kNumNanosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMicrosec =
    kNumNanosecsPerSec / kNumMicrosecsPerSec;

// (see https://tools.ietf.org/html/rfc868), and January 1 00:00 GMT 1970
// epoch. This is useful when converting between the NTP time base and the
// time base used in RTCP reports.
constexpr int64_t kNtpJan1970Millisecs = 2'208'988'800 * kNumMillisecsPerSec;


class ClockInterface {
 public:
  virtual ~ClockInterface() {}
  virtual int64_t TimeNanos() const = 0;
};

//
// Returns the previously set ClockInterface, or nullptr if none is set.
//
// Does not transfer ownership of the clock. SetClockForTesting(nullptr)
// should be called before the ClockInterface is deleted.
//
// This method is not thread-safe; it should only be used when no other thread
// is running (for example, at the start/end of a unit test, or start/end of
// main()).
//
// TODO(deadbeef): Instead of having functions that access this global
// ClockInterface, we may want to pass the ClockInterface into everything
// that uses it, eliminating the need for a global variable and this function.
RTC_EXPORT ClockInterface* SetClockForTesting(ClockInterface* clock);

RTC_EXPORT ClockInterface* GetClockForTesting();

#if defined(WINUWP)
// Synchronizes the current clock based upon an NTP server's epoch in
// milliseconds.
void SyncWithNtp(int64_t time_from_ntp_server_ms);

// system wall clock time upon instatiation. It may also be synchronized using
// the SyncWithNtp() function above. Please note that the clock will most likely
// drift away from the system wall clock time as time goes by.
int64_t WinUwpSystemTimeNanos();
#endif  // defined(WINUWP)

// Useful for timeouts while using a test clock, or for logging.
int64_t SystemTimeMillis();

uint32_t Time32();

RTC_EXPORT int64_t TimeMillis();
// Deprecated. Do not use this in any new code.
inline int64_t Time() {
  return TimeMillis();
}

RTC_EXPORT int64_t TimeMicros();

RTC_EXPORT int64_t TimeNanos();

int64_t TimeAfter(int64_t elapsed);

// timestamps.  The value is negative if 'later' occurs before 'earlier'.
int64_t TimeDiff(int64_t later, int64_t earlier);
int32_t TimeDiff32(uint32_t later, uint32_t earlier);

inline int64_t TimeSince(int64_t earlier) {
  return TimeMillis() - earlier;
}

inline int64_t TimeUntil(int64_t later) {
  return later - TimeMillis();
}

class TimestampWrapAroundHandler {
 public:
  TimestampWrapAroundHandler();

  int64_t Unwrap(uint32_t ts);

 private:
  uint32_t last_ts_;
  int64_t num_wrap_;
};

// seconds from 1970-01-01 00:00 ("epoch"). Don't return time_t since that
// is still 32 bits on many systems.
int64_t TmToSeconds(const tm& tm);

// Useful mainly when producing logs to be correlated with other
// devices, and when the devices in question all have properly
// synchronized clocks.
//
// Note that this function obeys the system's idea about what the time
// is. It is not guaranteed to be monotonic; it will jump in case the
// system time is changed, e.g., by some other process calling
// settimeofday. Always use rtc::TimeMicros(), not this function, for
// measuring time intervals and timeouts.
int64_t TimeUTCMicros();

// See above.
int64_t TimeUTCMillis();

}  // namespace rtc

#endif  // RTC_BASE_TIME_UTILS_H_
