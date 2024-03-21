// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time/time.h"

#include <cmath>
#include <ios>
#include <limits>
#include <ostream>
#include <sstream>

#include "base/logging.h"
#include "base/macros.h"
#include "base/no_destructor.h"
#include "base/strings/stringprintf.h"
#include "base/third_party/nspr/prtime.h"
#include "base/time/time_override.h"
#include "build/build_config.h"

namespace base {

namespace internal {

TimeNowFunction g_time_now_function = &subtle::TimeNowIgnoringOverride;

TimeNowFunction g_time_now_from_system_time_function =
    &subtle::TimeNowFromSystemTimeIgnoringOverride;

TimeTicksNowFunction g_time_ticks_now_function =
    &subtle::TimeTicksNowIgnoringOverride;

ThreadTicksNowFunction g_thread_ticks_now_function =
    &subtle::ThreadTicksNowIgnoringOverride;

}  // namespace internal


int TimeDelta::InDays() const {
  if (is_max()) {

    return std::numeric_limits<int>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int>::min();
  }
  return static_cast<int>(delta_ / Time::kMicrosecondsPerDay);
}

int TimeDelta::InDaysFloored() const {
  if (is_max()) {

    return std::numeric_limits<int>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int>::min();
  }
  int result = delta_ / Time::kMicrosecondsPerDay;
  int64_t remainder = delta_ - (result * Time::kMicrosecondsPerDay);
  if (remainder < 0) {
    --result;  // Use floor(), not trunc() rounding behavior.
  }
  return result;
}

int TimeDelta::InHours() const {
  if (is_max()) {

    return std::numeric_limits<int>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int>::min();
  }
  return static_cast<int>(delta_ / Time::kMicrosecondsPerHour);
}

int TimeDelta::InMinutes() const {
  if (is_max()) {

    return std::numeric_limits<int>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int>::min();
  }
  return static_cast<int>(delta_ / Time::kMicrosecondsPerMinute);
}

double TimeDelta::InSecondsF() const {
  if (is_max()) {

    return std::numeric_limits<double>::infinity();
  }
  if (is_min()) {

    return -std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(delta_) / Time::kMicrosecondsPerSecond;
}

int64_t TimeDelta::InSeconds() const {
  if (is_max()) {

    return std::numeric_limits<int64_t>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int64_t>::min();
  }
  return delta_ / Time::kMicrosecondsPerSecond;
}

double TimeDelta::InMillisecondsF() const {
  if (is_max()) {

    return std::numeric_limits<double>::infinity();
  }
  if (is_min()) {

    return -std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(delta_) / Time::kMicrosecondsPerMillisecond;
}

int64_t TimeDelta::InMilliseconds() const {
  if (is_max()) {

    return std::numeric_limits<int64_t>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int64_t>::min();
  }
  return delta_ / Time::kMicrosecondsPerMillisecond;
}

int64_t TimeDelta::InMillisecondsRoundedUp() const {
  if (is_max()) {

    return std::numeric_limits<int64_t>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int64_t>::min();
  }
  int64_t result = delta_ / Time::kMicrosecondsPerMillisecond;
  int64_t remainder = delta_ - (result * Time::kMicrosecondsPerMillisecond);
  if (remainder > 0) {
    ++result;  // Use ceil(), not trunc() rounding behavior.
  }
  return result;
}

double TimeDelta::InMicrosecondsF() const {
  if (is_max()) {

    return std::numeric_limits<double>::infinity();
  }
  if (is_min()) {

    return -std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(delta_);
}

int64_t TimeDelta::InNanoseconds() const {
  if (is_max()) {

    return std::numeric_limits<int64_t>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int64_t>::min();
  }
  return delta_ * Time::kNanosecondsPerMicrosecond;
}

std::ostream& operator<<(std::ostream& os, TimeDelta time_delta) {
  return os << time_delta.InSecondsF() << " s";
}


Time Time::Now() {
  return internal::g_time_now_function();
}

Time Time::NowFromSystemTime() {

  return internal::g_time_now_from_system_time_function();
}

Time Time::FromDeltaSinceWindowsEpoch(TimeDelta delta) {
  return Time(delta.InMicroseconds());
}

TimeDelta Time::ToDeltaSinceWindowsEpoch() const {
  return TimeDelta::FromMicroseconds(us_);
}

Time Time::FromTimeT(time_t tt) {
  if (tt == 0)
    return Time();  // Preserve 0 so we can tell it doesn't exist.
  if (tt == std::numeric_limits<time_t>::max())
    return Max();
  return Time(kTimeTToMicrosecondsOffset) + TimeDelta::FromSeconds(tt);
}

time_t Time::ToTimeT() const {
  if (is_null())
    return 0;  // Preserve 0 so we can tell it doesn't exist.
  if (is_max()) {

    return std::numeric_limits<time_t>::max();
  }
  if (is_min()) {

    return std::numeric_limits<time_t>::min();
  }
  if (std::numeric_limits<int64_t>::max() - kTimeTToMicrosecondsOffset <= us_) {
    DLOG(WARNING) << "Overflow when converting base::Time with internal " <<
                     "value " << us_ << " to time_t.";
    return std::numeric_limits<time_t>::max();
  }
  return (us_ - kTimeTToMicrosecondsOffset) / kMicrosecondsPerSecond;
}

Time Time::FromDoubleT(double dt) {
  if (dt == 0 || std::isnan(dt))
    return Time();  // Preserve 0 so we can tell it doesn't exist.
  return Time(kTimeTToMicrosecondsOffset) + TimeDelta::FromSecondsD(dt);
}

double Time::ToDoubleT() const {
  if (is_null())
    return 0;  // Preserve 0 so we can tell it doesn't exist.
  if (is_max()) {

    return std::numeric_limits<double>::infinity();
  }
  if (is_min()) {

    return -std::numeric_limits<double>::infinity();
  }
  return (static_cast<double>(us_ - kTimeTToMicrosecondsOffset) /
          static_cast<double>(kMicrosecondsPerSecond));
}

#if defined(OS_POSIX)
// static
Time Time::FromTimeSpec(const timespec& ts) {
  return FromDoubleT(ts.tv_sec +
                     static_cast<double>(ts.tv_nsec) /
                         base::Time::kNanosecondsPerSecond);
}
#endif

Time Time::FromJsTime(double ms_since_epoch) {


  return Time(kTimeTToMicrosecondsOffset) +
         TimeDelta::FromMillisecondsD(ms_since_epoch);
}

double Time::ToJsTime() const {
  if (is_null()) {

    return 0;
  }
  return ToJsTimeIgnoringNull();
}

double Time::ToJsTimeIgnoringNull() const {
  if (is_max()) {

    return std::numeric_limits<double>::infinity();
  }
  if (is_min()) {

    return -std::numeric_limits<double>::infinity();
  }
  return (static_cast<double>(us_ - kTimeTToMicrosecondsOffset) /
          kMicrosecondsPerMillisecond);
}

Time Time::FromJavaTime(int64_t ms_since_epoch) {
  return base::Time::UnixEpoch() +
         base::TimeDelta::FromMilliseconds(ms_since_epoch);
}

int64_t Time::ToJavaTime() const {
  if (is_null()) {

    return 0;
  }
  if (is_max()) {

    return std::numeric_limits<int64_t>::max();
  }
  if (is_min()) {

    return std::numeric_limits<int64_t>::min();
  }
  return ((us_ - kTimeTToMicrosecondsOffset) /
          kMicrosecondsPerMillisecond);
}

Time Time::UnixEpoch() {
  Time time;
  time.us_ = kTimeTToMicrosecondsOffset;
  return time;
}

Time Time::Midnight(bool is_local) const {
  Exploded exploded;
  Explode(is_local, &exploded);
  exploded.hour = 0;
  exploded.minute = 0;
  exploded.second = 0;
  exploded.millisecond = 0;
  Time out_time;
  if (FromExploded(is_local, exploded, &out_time)) {
    return out_time;
  } else if (is_local) {




    exploded.hour = 1;
    if (FromExploded(is_local, exploded, &out_time))
      return out_time;
  }

  NOTREACHED();
  return Time();
}

bool Time::FromStringInternal(const char* time_string,
                              bool is_local,
                              Time* parsed_time) {
  DCHECK((time_string != nullptr) && (parsed_time != nullptr));

  if (time_string[0] == '\0')
    return false;

  PRTime result_time = 0;
  PRStatus result = PR_ParseTimeString(time_string,
                                       is_local ? PR_FALSE : PR_TRUE,
                                       &result_time);
  if (PR_SUCCESS != result)
    return false;

  result_time += kTimeTToMicrosecondsOffset;
  *parsed_time = Time(result_time);
  return true;
}

bool Time::ExplodedMostlyEquals(const Exploded& lhs, const Exploded& rhs) {
  return lhs.year == rhs.year && lhs.month == rhs.month &&
         lhs.day_of_month == rhs.day_of_month && lhs.hour == rhs.hour &&
         lhs.minute == rhs.minute && lhs.second == rhs.second &&
         lhs.millisecond == rhs.millisecond;
}

bool Time::FromMillisecondsSinceUnixEpoch(int64_t unix_milliseconds,
                                          Time* time) {


  base::CheckedNumeric<int64_t> checked_microseconds_win_epoch =
      unix_milliseconds;
  checked_microseconds_win_epoch *= kMicrosecondsPerMillisecond;
  checked_microseconds_win_epoch += kTimeTToMicrosecondsOffset;
  if (!checked_microseconds_win_epoch.IsValid()) {
    *time = base::Time(0);
    return false;
  }

  *time = Time(checked_microseconds_win_epoch.ValueOrDie());
  return true;
}

int64_t Time::ToRoundedDownMillisecondsSinceUnixEpoch() const {

  int64_t microseconds = us_ - kTimeTToMicrosecondsOffset;

  if (microseconds >= 0) {

    return microseconds / kMicrosecondsPerMillisecond;
  } else {
    return (microseconds + 1) / kMicrosecondsPerMillisecond - 1;
  }
}

std::ostream& operator<<(std::ostream& os, Time time) {
  Time::Exploded exploded;
  time.UTCExplode(&exploded);

  return os << StringPrintf("%04d-%02d-%02d %02d:%02d:%02d.%03d UTC",
                            exploded.year,
                            exploded.month,
                            exploded.day_of_month,
                            exploded.hour,
                            exploded.minute,
                            exploded.second,
                            exploded.millisecond);
}


TimeTicks TimeTicks::Now() {
  return internal::g_time_ticks_now_function();
}

TimeTicks TimeTicks::UnixEpoch() {
  static const base::NoDestructor<base::TimeTicks> epoch([]() {
    return subtle::TimeTicksNowIgnoringOverride() -
           (subtle::TimeNowIgnoringOverride() - Time::UnixEpoch());
  }());
  return *epoch;
}

TimeTicks TimeTicks::SnappedToNextTick(TimeTicks tick_phase,
                                       TimeDelta tick_interval) const {


  TimeDelta interval_offset = (tick_phase - *this) % tick_interval;



  if (!interval_offset.is_zero() && tick_phase < *this)
    interval_offset += tick_interval;
  return *this + interval_offset;
}

std::ostream& operator<<(std::ostream& os, TimeTicks time_ticks) {





  const TimeDelta as_time_delta = time_ticks - TimeTicks();
  return os << as_time_delta.InMicroseconds() << " bogo-microseconds";
}


ThreadTicks ThreadTicks::Now() {
  return internal::g_thread_ticks_now_function();
}

std::ostream& operator<<(std::ostream& os, ThreadTicks thread_ticks) {
  const TimeDelta as_time_delta = thread_ticks - ThreadTicks();
  return os << as_time_delta.InMicroseconds() << " bogo-thread-microseconds";
}


inline bool is_in_range(int value, int lo, int hi) {
  return lo <= value && value <= hi;
}

bool Time::Exploded::HasValidValues() const {
  return is_in_range(month, 1, 12) &&
         is_in_range(day_of_week, 0, 6) &&
         is_in_range(day_of_month, 1, 31) &&
         is_in_range(hour, 0, 23) &&
         is_in_range(minute, 0, 59) &&
         is_in_range(second, 0, 60) &&
         is_in_range(millisecond, 0, 999);
}

}  // namespace base
