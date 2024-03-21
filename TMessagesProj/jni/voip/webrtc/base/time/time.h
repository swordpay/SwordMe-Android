// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// internally represented as microseconds (s/1,000,000) since the Windows epoch
// (1601-01-01 00:00:00 UTC). System-dependent clock interface routines are
// defined in time_PLATFORM.cc. Note that values for Time may skew and jump
// around as the operating system makes adjustments to synchronize (e.g., with
// NTP servers). Thus, client code that uses the Time class must account for
// this.
//
// TimeDelta represents a duration of time, internally represented in
// microseconds.
//
// TimeTicks and ThreadTicks represent an abstract time that is most of the time
// incrementing, for use in measuring time durations. Internally, they are
// represented in microseconds. They cannot be converted to a human-readable
// time, but are guaranteed not to decrease (unlike the Time class). Note that
// TimeTicks may "stand still" (e.g., if the computer is suspended), and
// ThreadTicks will "stand still" whenever the thread has been de-scheduled by
// the operating system.
//
// All time classes are copyable, assignable, and occupy 64-bits per instance.
// As a result, prefer passing them by value:
//   void MyFunction(TimeDelta arg);
// If circumstances require, you may also pass by const reference:
//   void MyFunction(const TimeDelta& arg);  // Not preferred.
//
// Definitions of operator<< are provided to make these types work with
// DCHECK_EQ() and other log macros. For human-readable formatting, see
// "base/i18n/time_formatting.h".
//
// So many choices!  Which time class should you use?  Examples:
//
//   Time:        Interpreting the wall-clock time provided by a remote system.
//                Detecting whether cached resources have expired. Providing the
//                user with a display of the current date and time. Determining
//                the amount of time between events across re-boots of the
//                machine.
//
//   TimeTicks:   Tracking the amount of time a task runs. Executing delayed
//                tasks at the right time. Computing presentation timestamps.
//                Synchronizing audio and video using TimeTicks as a common
//                reference clock (lip-sync). Measuring network round-trip
//                latency.
//
//   ThreadTicks: Benchmarking how long the current thread has been doing actual
//                work.

#ifndef BASE_TIME_TIME_H_
#define BASE_TIME_TIME_H_

#include <stdint.h>
#include <time.h>

#include <iosfwd>
#include <limits>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/numerics/safe_math.h"
#include "build/build_config.h"

#if defined(OS_FUCHSIA)
#include <zircon/types.h>
#endif

#if defined(OS_MACOSX)
#include <CoreFoundation/CoreFoundation.h>
// Avoid Mac system header macro leak.
#undef TYPE_BOOL
#endif

#if defined(OS_ANDROID)
#include <jni.h>
#endif

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <unistd.h>
#include <sys/time.h>
#endif

#if defined(OS_WIN)
#include "base/gtest_prod_util.h"
#include "base/win/windows_types.h"
#endif

namespace ABI {
namespace Windows {
namespace Foundation {
struct DateTime;
}  // namespace Foundation
}  // namespace Windows
}  // namespace ABI

namespace base {

class PlatformThreadHandle;
class TimeDelta;

// time classes and functions.  Please use the math operators defined in the
// time classes instead.
namespace time_internal {

// as infinity and will always saturate the return value (infinity math applies
// if |value| also is at either limit of its spectrum). The int64_t argument and
// return value are in terms of a microsecond timebase.
BASE_EXPORT constexpr int64_t SaturatedAdd(int64_t value, TimeDelta delta);
BASE_EXPORT constexpr int64_t SaturatedSub(int64_t value, TimeDelta delta);

}  // namespace time_internal


class BASE_EXPORT TimeDelta {
 public:
  constexpr TimeDelta() : delta_(0) {}






  static constexpr TimeDelta FromDays(int days);
  static constexpr TimeDelta FromHours(int hours);
  static constexpr TimeDelta FromMinutes(int minutes);
  static constexpr TimeDelta FromSeconds(int64_t secs);
  static constexpr TimeDelta FromMilliseconds(int64_t ms);
  static constexpr TimeDelta FromMicroseconds(int64_t us);
  static constexpr TimeDelta FromNanoseconds(int64_t ns);
  static constexpr TimeDelta FromSecondsD(double secs);
  static constexpr TimeDelta FromMillisecondsD(double ms);
  static constexpr TimeDelta FromMicrosecondsD(double us);
  static constexpr TimeDelta FromNanosecondsD(double ns);
#if defined(OS_WIN)
  static TimeDelta FromQPCValue(LONGLONG qpc_value);


  static TimeDelta FromFileTime(FILETIME ft);
  static TimeDelta FromWinrtDateTime(ABI::Windows::Foundation::DateTime dt);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  static TimeDelta FromTimeSpec(const timespec& ts);
#endif
#if defined(OS_FUCHSIA)
  static TimeDelta FromZxDuration(zx_duration_t nanos);
#endif






  static constexpr TimeDelta FromInternalValue(int64_t delta) {
    return TimeDelta(delta);
  }



  static constexpr TimeDelta Max();



  static constexpr TimeDelta Min();






  constexpr int64_t ToInternalValue() const { return delta_; }

  constexpr TimeDelta magnitude() const {



    const int64_t mask = delta_ >> (sizeof(delta_) * 8 - 1);
    return TimeDelta((delta_ + mask) ^ mask);
  }

  constexpr bool is_zero() const { return delta_ == 0; }

  constexpr bool is_max() const {
    return delta_ == std::numeric_limits<int64_t>::max();
  }
  constexpr bool is_min() const {
    return delta_ == std::numeric_limits<int64_t>::min();
  }

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
  struct timespec ToTimeSpec() const;
#endif
#if defined(OS_FUCHSIA)
  zx_duration_t ToZxDuration() const;
#endif
#if defined(OS_WIN)
  ABI::Windows::Foundation::DateTime ToWinrtDateTime() const;
#endif










  int InDays() const;
  int InDaysFloored() const;
  int InHours() const;
  int InMinutes() const;
  double InSecondsF() const;
  int64_t InSeconds() const;
  double InMillisecondsF() const;
  int64_t InMilliseconds() const;
  int64_t InMillisecondsRoundedUp() const;
  constexpr int64_t InMicroseconds() const { return delta_; }
  double InMicrosecondsF() const;
  int64_t InNanoseconds() const;

  constexpr TimeDelta operator+(TimeDelta other) const {
    return TimeDelta(time_internal::SaturatedAdd(delta_, other));
  }
  constexpr TimeDelta operator-(TimeDelta other) const {
    return TimeDelta(time_internal::SaturatedSub(delta_, other));
  }

  constexpr TimeDelta& operator+=(TimeDelta other) {
    return *this = (*this + other);
  }
  constexpr TimeDelta& operator-=(TimeDelta other) {
    return *this = (*this - other);
  }
  constexpr TimeDelta operator-() const {
    if (is_max()) {
      return Min();
    }
    if (is_min()) {
      return Max();
    }
    return TimeDelta(-delta_);
  }

  template <typename T>
  constexpr TimeDelta operator*(T a) const {
    CheckedNumeric<int64_t> rv(delta_);
    rv *= a;
    if (rv.IsValid())
      return TimeDelta(rv.ValueOrDie());

    if ((delta_ < 0) ^ (a < 0))
      return TimeDelta(std::numeric_limits<int64_t>::min());
    return TimeDelta(std::numeric_limits<int64_t>::max());
  }
  template <typename T>
  constexpr TimeDelta operator/(T a) const {
    CheckedNumeric<int64_t> rv(delta_);
    rv /= a;
    if (rv.IsValid())
      return TimeDelta(rv.ValueOrDie());


    if ((delta_ < 0) ^ (a <= 0))
      return TimeDelta(std::numeric_limits<int64_t>::min());
    return TimeDelta(std::numeric_limits<int64_t>::max());
  }
  template <typename T>
  constexpr TimeDelta& operator*=(T a) {
    return *this = (*this * a);
  }
  template <typename T>
  constexpr TimeDelta& operator/=(T a) {
    return *this = (*this / a);
  }

  constexpr int64_t operator/(TimeDelta a) const {
    if (a.delta_ == 0) {
      return delta_ < 0 ? std::numeric_limits<int64_t>::min()
                        : std::numeric_limits<int64_t>::max();
    }
    if (is_max()) {
      if (a.delta_ < 0) {
        return std::numeric_limits<int64_t>::min();
      }
      return std::numeric_limits<int64_t>::max();
    }
    if (is_min()) {
      if (a.delta_ > 0) {
        return std::numeric_limits<int64_t>::min();
      }
      return std::numeric_limits<int64_t>::max();
    }
    if (a.is_max()) {
      return 0;
    }
    return delta_ / a.delta_;
  }

  constexpr TimeDelta operator%(TimeDelta a) const {
    if (a.is_min() || a.is_max()) {
      return TimeDelta(delta_);
    }
    return TimeDelta(delta_ % a.delta_);
  }
  TimeDelta& operator%=(TimeDelta other) { return *this = (*this % other); }

  constexpr bool operator==(TimeDelta other) const {
    return delta_ == other.delta_;
  }
  constexpr bool operator!=(TimeDelta other) const {
    return delta_ != other.delta_;
  }
  constexpr bool operator<(TimeDelta other) const {
    return delta_ < other.delta_;
  }
  constexpr bool operator<=(TimeDelta other) const {
    return delta_ <= other.delta_;
  }
  constexpr bool operator>(TimeDelta other) const {
    return delta_ > other.delta_;
  }
  constexpr bool operator>=(TimeDelta other) const {
    return delta_ >= other.delta_;
  }

 private:
  friend constexpr int64_t time_internal::SaturatedAdd(int64_t value,
                                                       TimeDelta delta);
  friend constexpr int64_t time_internal::SaturatedSub(int64_t value,
                                                       TimeDelta delta);



  constexpr explicit TimeDelta(int64_t delta_us) : delta_(delta_us) {}

  static constexpr TimeDelta FromDouble(double value);


  static constexpr TimeDelta FromProduct(int64_t value, int64_t positive_value);

  int64_t delta_;
};

template <typename T>
constexpr TimeDelta operator*(T a, TimeDelta td) {
  return td * a;
}

BASE_EXPORT std::ostream& operator<<(std::ostream& os, TimeDelta time_delta);

// use one of the time subclasses instead, and only reference the public
// TimeBase members via those classes.
namespace time_internal {

constexpr int64_t SaturatedAdd(int64_t value, TimeDelta delta) {


  if (delta.is_max()) {
    CHECK_GT(value, std::numeric_limits<int64_t>::min());
    return std::numeric_limits<int64_t>::max();
  } else if (delta.is_min()) {
    CHECK_LT(value, std::numeric_limits<int64_t>::max());
    return std::numeric_limits<int64_t>::min();
  }

  return base::ClampAdd(value, delta.delta_);
}

constexpr int64_t SaturatedSub(int64_t value, TimeDelta delta) {


  if (delta.is_max()) {
    CHECK_LT(value, std::numeric_limits<int64_t>::max());
    return std::numeric_limits<int64_t>::min();
  } else if (delta.is_min()) {
    CHECK_GT(value, std::numeric_limits<int64_t>::min());
    return std::numeric_limits<int64_t>::max();
  }

  return base::ClampSub(value, delta.delta_);
}


// classes. Each subclass provides for strong type-checking to ensure
// semantically meaningful comparison/math of time values from the same clock
// source or timeline.
template<class TimeClass>
class TimeBase {
 public:
  static constexpr int64_t kHoursPerDay = 24;
  static constexpr int64_t kSecondsPerMinute = 60;
  static constexpr int64_t kSecondsPerHour = 60 * kSecondsPerMinute;
  static constexpr int64_t kMillisecondsPerSecond = 1000;
  static constexpr int64_t kMillisecondsPerDay =
      kMillisecondsPerSecond * 60 * 60 * kHoursPerDay;
  static constexpr int64_t kMicrosecondsPerMillisecond = 1000;
  static constexpr int64_t kMicrosecondsPerSecond =
      kMicrosecondsPerMillisecond * kMillisecondsPerSecond;
  static constexpr int64_t kMicrosecondsPerMinute = kMicrosecondsPerSecond * 60;
  static constexpr int64_t kMicrosecondsPerHour = kMicrosecondsPerMinute * 60;
  static constexpr int64_t kMicrosecondsPerDay =
      kMicrosecondsPerHour * kHoursPerDay;
  static constexpr int64_t kMicrosecondsPerWeek = kMicrosecondsPerDay * 7;
  static constexpr int64_t kNanosecondsPerMicrosecond = 1000;
  static constexpr int64_t kNanosecondsPerSecond =
      kNanosecondsPerMicrosecond * kMicrosecondsPerSecond;





  constexpr bool is_null() const { return us_ == 0; }

  constexpr bool is_max() const {
    return us_ == std::numeric_limits<int64_t>::max();
  }
  constexpr bool is_min() const {
    return us_ == std::numeric_limits<int64_t>::min();
  }


  static constexpr TimeClass Max() {
    return TimeClass(std::numeric_limits<int64_t>::max());
  }

  static constexpr TimeClass Min() {
    return TimeClass(std::numeric_limits<int64_t>::min());
  }






  constexpr int64_t ToInternalValue() const { return us_; }






  constexpr TimeDelta since_origin() const {
    return TimeDelta::FromMicroseconds(us_);
  }

  constexpr TimeClass& operator=(TimeClass other) {
    us_ = other.us_;
    return *(static_cast<TimeClass*>(this));
  }

  constexpr TimeDelta operator-(TimeClass other) const {
    return TimeDelta::FromMicroseconds(us_ - other.us_);
  }

  constexpr TimeClass operator+(TimeDelta delta) const {
    return TimeClass(time_internal::SaturatedAdd(us_, delta));
  }
  constexpr TimeClass operator-(TimeDelta delta) const {
    return TimeClass(time_internal::SaturatedSub(us_, delta));
  }

  constexpr TimeClass& operator+=(TimeDelta delta) {
    return static_cast<TimeClass&>(*this = (*this + delta));
  }
  constexpr TimeClass& operator-=(TimeDelta delta) {
    return static_cast<TimeClass&>(*this = (*this - delta));
  }

  constexpr bool operator==(TimeClass other) const { return us_ == other.us_; }
  constexpr bool operator!=(TimeClass other) const { return us_ != other.us_; }
  constexpr bool operator<(TimeClass other) const { return us_ < other.us_; }
  constexpr bool operator<=(TimeClass other) const { return us_ <= other.us_; }
  constexpr bool operator>(TimeClass other) const { return us_ > other.us_; }
  constexpr bool operator>=(TimeClass other) const { return us_ >= other.us_; }

 protected:
  constexpr explicit TimeBase(int64_t us) : us_(us) {}

  int64_t us_;
};

}  // namespace time_internal

template <class TimeClass>
inline constexpr TimeClass operator+(TimeDelta delta, TimeClass t) {
  return t + delta;
}


// monotonically non-decreasing and are subject to large amounts of skew.
// Time is stored internally as microseconds since the Windows epoch (1601).
class BASE_EXPORT Time : public time_internal::TimeBase<Time> {
 public:





  static constexpr int64_t kTimeTToMicrosecondsOffset =
      INT64_C(11644473600000000);

#if defined(OS_WIN)



  static constexpr int64_t kQPCOverflowThreshold = INT64_C(0x8637BD05AF7);
#endif

// for values passed to FromUTCExploded() and FromLocalExploded(). Those
// functions will return false if passed values outside these limits. The limits
// are inclusive, meaning that the API should support all dates within a given
// limit year.
#if defined(OS_WIN)
  static constexpr int kExplodedMinYear = 1601;
  static constexpr int kExplodedMaxYear = 30827;
#elif defined(OS_IOS) && !__LP64__
  static constexpr int kExplodedMinYear = std::numeric_limits<int>::min();
  static constexpr int kExplodedMaxYear = std::numeric_limits<int>::max();
#elif defined(OS_MACOSX)
  static constexpr int kExplodedMinYear = 1902;
  static constexpr int kExplodedMaxYear = std::numeric_limits<int>::max();
#elif defined(OS_ANDROID)



  static constexpr int kExplodedMinYear = 1902;
  static constexpr int kExplodedMaxYear = std::numeric_limits<int>::max();
#else
  static constexpr int kExplodedMinYear =
      (sizeof(time_t) == 4 ? 1902 : std::numeric_limits<int>::min());
  static constexpr int kExplodedMaxYear =
      (sizeof(time_t) == 4 ? 2037 : std::numeric_limits<int>::max());
#endif



  struct BASE_EXPORT Exploded {
    int year;          // Four digit year "2007"
    int month;         // 1-based month (values 1 = January, etc.)
    int day_of_week;   // 0-based day of week (0 = Sunday, etc.)
    int day_of_month;  // 1-based day of month (1-31)
    int hour;          // Hour within the current day (0-23)
    int minute;        // Minute within the current hour (0-59)
    int second;        // Second within the current minute (0-59 plus leap

    int millisecond;   // Milliseconds within the current second (0-999)



    bool HasValidValues() const;
  };

  constexpr Time() : TimeBase(0) {}

  static Time UnixEpoch();



  static Time Now();




  static Time NowFromSystemTime();











  static Time FromDeltaSinceWindowsEpoch(TimeDelta delta);
  TimeDelta ToDeltaSinceWindowsEpoch() const;

  static Time FromTimeT(time_t tt);
  time_t ToTimeT() const;





  static Time FromDoubleT(double dt);
  double ToDoubleT() const;

#if defined(OS_POSIX) || defined(OS_FUCHSIA)




  static Time FromTimeSpec(const timespec& ts);
#endif








  static Time FromJsTime(double ms_since_epoch);
  double ToJsTime() const;
  double ToJsTimeIgnoringNull() const;



  static Time FromJavaTime(int64_t ms_since_epoch);
  int64_t ToJavaTime() const;

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
  static Time FromTimeVal(struct timeval t);
  struct timeval ToTimeVal() const;
#endif

#if defined(OS_FUCHSIA)
  static Time FromZxTime(zx_time_t time);
  zx_time_t ToZxTime() const;
#endif

#if defined(OS_MACOSX)
  static Time FromCFAbsoluteTime(CFAbsoluteTime t);
  CFAbsoluteTime ToCFAbsoluteTime() const;
#endif

#if defined(OS_WIN)
  static Time FromFileTime(FILETIME ft);
  FILETIME ToFileTime() const;



  static const int kMinLowResolutionThresholdMs = 16;

  static void EnableHighResolutionTimer(bool enable);



  static void ReadMinTimerIntervalLowResMs();







  static bool ActivateHighResolutionTimer(bool activate);



  static bool IsHighResolutionTimerInUse();









  static void ResetHighResolutionTimerUsage();
  static double GetHighResolutionTimerUsage();
#endif  // defined(OS_WIN)



  static bool FromUTCExploded(const Exploded& exploded,
                              Time* time) WARN_UNUSED_RESULT {
    return FromExploded(false, exploded, time);
  }
  static bool FromLocalExploded(const Exploded& exploded,
                                Time* time) WARN_UNUSED_RESULT {
    return FromExploded(true, exploded, time);
  }














  static bool FromString(const char* time_string,
                         Time* parsed_time) WARN_UNUSED_RESULT {
    return FromStringInternal(time_string, true, parsed_time);
  }
  static bool FromUTCString(const char* time_string,
                            Time* parsed_time) WARN_UNUSED_RESULT {
    return FromStringInternal(time_string, false, parsed_time);
  }


  void UTCExplode(Exploded* exploded) const {
    return Explode(false, exploded);
  }
  void LocalExplode(Exploded* exploded) const {
    return Explode(true, exploded);
  }


  Time UTCMidnight() const { return Midnight(false); }
  Time LocalMidnight() const { return Midnight(true); }







  static constexpr Time FromInternalValue(int64_t us) { return Time(us); }

 private:
  friend class time_internal::TimeBase<Time>;

  constexpr explicit Time(int64_t microseconds_since_win_epoch)
      : TimeBase(microseconds_since_win_epoch) {}


  void Explode(bool is_local, Exploded* exploded) const;




  static bool FromExploded(bool is_local,
                           const Exploded& exploded,
                           Time* time) WARN_UNUSED_RESULT;


  Time Midnight(bool is_local) const;







  static bool FromStringInternal(const char* time_string,
                                 bool is_local,
                                 Time* parsed_time) WARN_UNUSED_RESULT;

  static bool ExplodedMostlyEquals(const Exploded& lhs,
                                   const Exploded& rhs) WARN_UNUSED_RESULT;


  static bool FromMillisecondsSinceUnixEpoch(int64_t unix_milliseconds,
                                             Time* time) WARN_UNUSED_RESULT;


  int64_t ToRoundedDownMillisecondsSinceUnixEpoch() const;
};

constexpr TimeDelta TimeDelta::FromDays(int days) {
  return days == std::numeric_limits<int>::max()
             ? Max()
             : TimeDelta(days * Time::kMicrosecondsPerDay);
}

constexpr TimeDelta TimeDelta::FromHours(int hours) {
  return hours == std::numeric_limits<int>::max()
             ? Max()
             : TimeDelta(hours * Time::kMicrosecondsPerHour);
}

constexpr TimeDelta TimeDelta::FromMinutes(int minutes) {
  return minutes == std::numeric_limits<int>::max()
             ? Max()
             : TimeDelta(minutes * Time::kMicrosecondsPerMinute);
}

constexpr TimeDelta TimeDelta::FromSeconds(int64_t secs) {
  return FromProduct(secs, Time::kMicrosecondsPerSecond);
}

constexpr TimeDelta TimeDelta::FromMilliseconds(int64_t ms) {
  return FromProduct(ms, Time::kMicrosecondsPerMillisecond);
}

constexpr TimeDelta TimeDelta::FromMicroseconds(int64_t us) {
  return TimeDelta(us);
}

constexpr TimeDelta TimeDelta::FromNanoseconds(int64_t ns) {
  return TimeDelta(ns / Time::kNanosecondsPerMicrosecond);
}

constexpr TimeDelta TimeDelta::FromSecondsD(double secs) {
  return FromDouble(secs * Time::kMicrosecondsPerSecond);
}

constexpr TimeDelta TimeDelta::FromMillisecondsD(double ms) {
  return FromDouble(ms * Time::kMicrosecondsPerMillisecond);
}

constexpr TimeDelta TimeDelta::FromMicrosecondsD(double us) {
  return FromDouble(us);
}

constexpr TimeDelta TimeDelta::FromNanosecondsD(double ns) {
  return FromDouble(ns / Time::kNanosecondsPerMicrosecond);
}

constexpr TimeDelta TimeDelta::Max() {
  return TimeDelta(std::numeric_limits<int64_t>::max());
}

constexpr TimeDelta TimeDelta::Min() {
  return TimeDelta(std::numeric_limits<int64_t>::min());
}

constexpr TimeDelta TimeDelta::FromDouble(double value) {
  return TimeDelta(saturated_cast<int64_t>(value));
}

constexpr TimeDelta TimeDelta::FromProduct(int64_t value,
                                           int64_t positive_value) {
  DCHECK(positive_value > 0);  // NOLINT, DCHECK_GT isn't constexpr.
  return value > std::numeric_limits<int64_t>::max() / positive_value
             ? Max()
             : value < std::numeric_limits<int64_t>::min() / positive_value
                   ? Min()
                   : TimeDelta(value * positive_value);
}

BASE_EXPORT std::ostream& operator<<(std::ostream& os, Time time);


class BASE_EXPORT TimeTicks : public time_internal::TimeBase<TimeTicks> {
 public:

  enum class Clock {
    FUCHSIA_ZX_CLOCK_MONOTONIC,
    LINUX_CLOCK_MONOTONIC,
    IOS_CF_ABSOLUTE_TIME_MINUS_KERN_BOOTTIME,
    MAC_MACH_ABSOLUTE_TIME,
    WIN_QPC,
    WIN_ROLLOVER_PROTECTED_TIME_GET_TIME
  };

  constexpr TimeTicks() : TimeBase(0) {}




  static TimeTicks Now();




  static bool IsHighResolution() WARN_UNUSED_RESULT;





  static bool IsConsistentAcrossProcesses() WARN_UNUSED_RESULT;

#if defined(OS_FUCHSIA)

  static TimeTicks FromZxTime(zx_time_t nanos_since_boot);
  zx_time_t ToZxTime() const;
#endif

#if defined(OS_WIN)



  static TimeTicks FromQPCValue(LONGLONG qpc_value);
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
  static TimeTicks FromMachAbsoluteTime(uint64_t mach_absolute_time);
#endif  // defined(OS_MACOSX) && !defined(OS_IOS)

#if defined(OS_ANDROID) || defined(OS_CHROMEOS)




  static TimeTicks FromUptimeMillis(int64_t uptime_millis_value);
#endif








  static TimeTicks UnixEpoch();



  TimeTicks SnappedToNextTick(TimeTicks tick_phase,
                              TimeDelta tick_interval) const;



  static Clock GetClock();







  static constexpr TimeTicks FromInternalValue(int64_t us) {
    return TimeTicks(us);
  }

 protected:
#if defined(OS_WIN)
  typedef DWORD (*TickFunctionType)(void);
  static TickFunctionType SetMockTickFunction(TickFunctionType ticker);
#endif

 private:
  friend class time_internal::TimeBase<TimeTicks>;


  constexpr explicit TimeTicks(int64_t us) : TimeBase(us) {}
};

BASE_EXPORT std::ostream& operator<<(std::ostream& os, TimeTicks time_ticks);


// thread is running.
class BASE_EXPORT ThreadTicks : public time_internal::TimeBase<ThreadTicks> {
 public:
  constexpr ThreadTicks() : TimeBase(0) {}

  static bool IsSupported() WARN_UNUSED_RESULT {
#if (defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME >= 0)) || \
    (defined(OS_MACOSX) && !defined(OS_IOS)) || defined(OS_ANDROID) ||  \
    defined(OS_FUCHSIA)
    return true;
#elif defined(OS_WIN)
    return IsSupportedWin();
#else
    return false;
#endif
  }


  static void WaitUntilInitialized() {
#if defined(OS_WIN)
    WaitUntilInitializedWin();
#endif
  }







  static ThreadTicks Now();

#if defined(OS_WIN)



  static ThreadTicks GetForThread(const PlatformThreadHandle& thread_handle);
#endif







  static constexpr ThreadTicks FromInternalValue(int64_t us) {
    return ThreadTicks(us);
  }

 private:
  friend class time_internal::TimeBase<ThreadTicks>;


  constexpr explicit ThreadTicks(int64_t us) : TimeBase(us) {}

#if defined(OS_WIN)
  FRIEND_TEST_ALL_PREFIXES(TimeTicks, TSCTicksPerSecond);

#if defined(ARCH_CPU_ARM64)



#else




  static double TSCTicksPerSecond();
#endif

  static bool IsSupportedWin() WARN_UNUSED_RESULT;
  static void WaitUntilInitializedWin();
#endif
};

BASE_EXPORT std::ostream& operator<<(std::ostream& os, ThreadTicks time_ticks);

}  // namespace base

#endif  // BASE_TIME_TIME_H_
