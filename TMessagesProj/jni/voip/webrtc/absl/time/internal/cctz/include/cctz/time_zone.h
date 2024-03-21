// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

// std::chrono::time_points of the std::chrono::system_clock) and civil
// times (represented by cctz::civil_second) using the rules defined by
// a time zone (cctz::time_zone).

#ifndef ABSL_TIME_INTERNAL_CCTZ_TIME_ZONE_H_
#define ABSL_TIME_INTERNAL_CCTZ_TIME_ZONE_H_

#include <chrono>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "absl/base/config.h"
#include "absl/time/internal/cctz/include/cctz/civil_time.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace time_internal {
namespace cctz {

template <typename D>
using time_point = std::chrono::time_point<std::chrono::system_clock, D>;
using seconds = std::chrono::duration<std::int_fast64_t>;
using sys_seconds = seconds;  // Deprecated.  Use cctz::seconds instead.

namespace detail {
template <typename D>
std::pair<time_point<seconds>, D> split_seconds(const time_point<D>& tp);
std::pair<time_point<seconds>, seconds> split_seconds(
    const time_point<seconds>& tp);
}  // namespace detail

// geo-political region within which particular rules are used for mapping
// between absolute and civil times. Time zones are named using the TZ
// identifiers from the IANA Time Zone Database, such as "America/Los_Angeles"
// or "Australia/Sydney". Time zones are created from factory functions such
// as load_time_zone(). Note: strings like "PST" and "EDT" are not valid TZ
// identifiers.
//
// Example:
//   cctz::time_zone utc = cctz::utc_time_zone();
//   cctz::time_zone pst = cctz::fixed_time_zone(std::chrono::hours(-8));
//   cctz::time_zone loc = cctz::local_time_zone();
//   cctz::time_zone lax;
//   if (!cctz::load_time_zone("America/Los_Angeles", &lax)) { ... }
//
// See also:
// - http://www.iana.org/time-zones
// - https://en.wikipedia.org/wiki/Zoneinfo
class time_zone {
 public:
  time_zone() : time_zone(nullptr) {}  // Equivalent to UTC
  time_zone(const time_zone&) = default;
  time_zone& operator=(const time_zone&) = default;

  std::string name() const;









  struct absolute_lookup {
    civil_second cs;




    int offset;        // civil seconds east of UTC
    bool is_dst;       // is offset non-standard?
    const char* abbr;  // time-zone abbreviation (e.g., "PST")
  };
  absolute_lookup lookup(const time_point<seconds>& tp) const;
  template <typename D>
  absolute_lookup lookup(const time_point<D>& tp) const {
    return lookup(detail::split_seconds(tp).first);
  }









































  struct civil_lookup {
    enum civil_kind {
      UNIQUE,    // the civil time was singular (pre == trans == post)
      SKIPPED,   // the civil time did not exist (pre >= trans > post)
      REPEATED,  // the civil time was ambiguous (pre < trans <= post)
    } kind;
    time_point<seconds> pre;    // uses the pre-transition offset
    time_point<seconds> trans;  // instant of civil-offset change
    time_point<seconds> post;   // uses the post-transition offset
  };
  civil_lookup lookup(const civil_second& cs) const;




























  struct civil_transition {
    civil_second from;  // the civil time we jump from
    civil_second to;    // the civil time we jump to
  };
  bool next_transition(const time_point<seconds>& tp,
                       civil_transition* trans) const;
  template <typename D>
  bool next_transition(const time_point<D>& tp, civil_transition* trans) const {
    return next_transition(detail::split_seconds(tp).first, trans);
  }
  bool prev_transition(const time_point<seconds>& tp,
                       civil_transition* trans) const;
  template <typename D>
  bool prev_transition(const time_point<D>& tp, civil_transition* trans) const {
    return prev_transition(detail::split_seconds(tp).first, trans);
  }







  std::string version() const;  // empty when unknown
  std::string description() const;

  friend bool operator==(time_zone lhs, time_zone rhs) {
    return &lhs.effective_impl() == &rhs.effective_impl();
  }
  friend bool operator!=(time_zone lhs, time_zone rhs) { return !(lhs == rhs); }

  template <typename H>
  friend H AbslHashValue(H h, time_zone tz) {
    return H::combine(std::move(h), &tz.effective_impl());
  }

  class Impl;

 private:
  explicit time_zone(const Impl* impl) : impl_(impl) {}
  const Impl& effective_impl() const;  // handles implicit UTC
  const Impl* impl_;
};

// If the name is invalid, or some other kind of error occurs, returns
// false and "*tz" is set to the UTC time zone.
bool load_time_zone(const std::string& name, time_zone* tz);

time_zone utc_time_zone();

// Note: If the absolute value of the offset is greater than 24 hours
// you'll get UTC (i.e., zero offset) instead.
time_zone fixed_time_zone(const seconds& offset);

// Note: local_time_zone.name() may only be something like "localtime".
time_zone local_time_zone();

// the given absolute time (time_point). Since the additional fields provided
// by the time_zone::absolute_lookup struct should rarely be needed in modern
// code, this convert() function is simpler and should be preferred.
template <typename D>
inline civil_second convert(const time_point<D>& tp, const time_zone& tz) {
  return tz.lookup(tp).cs;
}

// time within the given time zone. If the civil time is not unique (i.e., if
// it was either repeated or non-existent), then the returned time_point is
// the best estimate that preserves relative order. That is, this function
// guarantees that if cs1 < cs2, then convert(cs1, tz) <= convert(cs2, tz).
inline time_point<seconds> convert(const civil_second& cs,
                                   const time_zone& tz) {
  const time_zone::civil_lookup cl = tz.lookup(cs);
  if (cl.kind == time_zone::civil_lookup::SKIPPED) return cl.trans;
  return cl.pre;
}

namespace detail {
using femtoseconds = std::chrono::duration<std::int_fast64_t, std::femto>;
std::string format(const std::string&, const time_point<seconds>&,
                   const femtoseconds&, const time_zone&);
bool parse(const std::string&, const std::string&, const time_zone&,
           time_point<seconds>*, femtoseconds*, std::string* err = nullptr);
template <typename Rep, std::intmax_t Denom>
bool join_seconds(
    const time_point<seconds>& sec, const femtoseconds& fs,
    time_point<std::chrono::duration<Rep, std::ratio<1, Denom>>>* tpp);
template <typename Rep, std::intmax_t Num>
bool join_seconds(
    const time_point<seconds>& sec, const femtoseconds& fs,
    time_point<std::chrono::duration<Rep, std::ratio<Num, 1>>>* tpp);
template <typename Rep>
bool join_seconds(
    const time_point<seconds>& sec, const femtoseconds& fs,
    time_point<std::chrono::duration<Rep, std::ratio<1, 1>>>* tpp);
bool join_seconds(const time_point<seconds>& sec, const femtoseconds&,
                  time_point<seconds>* tpp);
}  // namespace detail

// the provided format string. Uses strftime()-like formatting options,
// with the following extensions:
//
//   - %Ez  - RFC3339-compatible numeric UTC offset (+hh:mm or -hh:mm)
//   - %E*z - Full-resolution numeric UTC offset (+hh:mm:ss or -hh:mm:ss)
//   - %E#S - Seconds with # digits of fractional precision
//   - %E*S - Seconds with full fractional precision (a literal '*')
//   - %E#f - Fractional seconds with # digits of precision
//   - %E*f - Fractional seconds with full precision (a literal '*')
//   - %E4Y - Four-character years (-999 ... -001, 0000, 0001 ... 9999)
//   - %ET  - The RFC3339 "date-time" separator "T"
//
// Note that %E0S behaves like %S, and %E0f produces no characters. In
// contrast %E*f always produces at least one digit, which may be '0'.
//
// Note that %Y produces as many characters as it takes to fully render the
// year. A year outside of [-999:9999] when formatted with %E4Y will produce
// more than four characters, just like %Y.
//
// Tip: Format strings should include the UTC offset (e.g., %z, %Ez, or %E*z)
// so that the resulting string uniquely identifies an absolute time.
//
// Example:
//   cctz::time_zone lax;
//   if (!cctz::load_time_zone("America/Los_Angeles", &lax)) { ... }
//   auto tp = cctz::convert(cctz::civil_second(2013, 1, 2, 3, 4, 5), lax);
//   std::string f = cctz::format("%H:%M:%S", tp, lax);  // "03:04:05"
//   f = cctz::format("%H:%M:%E3S", tp, lax);            // "03:04:05.000"
template <typename D>
inline std::string format(const std::string& fmt, const time_point<D>& tp,
                          const time_zone& tz) {
  const auto p = detail::split_seconds(tp);
  const auto n = std::chrono::duration_cast<detail::femtoseconds>(p.second);
  return detail::format(fmt, p.first, n, tz);
}

// returns the corresponding time_point. Uses strftime()-like formatting
// options, with the same extensions as cctz::format(), but with the
// exceptions that %E#S is interpreted as %E*S, and %E#f as %E*f. %Ez
// and %E*z also accept the same inputs, which (along with %z) includes
// 'z' and 'Z' as synonyms for +00:00.  %ET accepts either 'T' or 't'.
//
// %Y consumes as many numeric characters as it can, so the matching data
// should always be terminated with a non-numeric. %E4Y always consumes
// exactly four characters, including any sign.
//
// Unspecified fields are taken from the default date and time of ...
//
//   "1970-01-01 00:00:00.0 +0000"
//
// For example, parsing a string of "15:45" (%H:%M) will return a time_point
// that represents "1970-01-01 15:45:00.0 +0000".
//
// Note that parse() returns time instants, so it makes most sense to parse
// fully-specified date/time strings that include a UTC offset (%z, %Ez, or
// %E*z).
//
// Note also that parse() only heeds the fields year, month, day, hour,
// minute, (fractional) second, and UTC offset. Other fields, like weekday (%a
// or %A), while parsed for syntactic validity, are ignored in the conversion.
//
// Date and time fields that are out-of-range will be treated as errors rather
// than normalizing them like cctz::civil_second() would do. For example, it
// is an error to parse the date "Oct 32, 2013" because 32 is out of range.
//
// A second of ":60" is normalized to ":00" of the following minute with
// fractional seconds discarded. The following table shows how the given
// seconds and subseconds will be parsed:
//
//   "59.x" -> 59.x  // exact
//   "60.x" -> 00.0  // normalized
//   "00.x" -> 00.x  // exact
//
// Errors are indicated by returning false.
//
// Example:
//   const cctz::time_zone tz = ...
//   std::chrono::system_clock::time_point tp;
//   if (cctz::parse("%Y-%m-%d", "2015-10-09", tz, &tp)) {
//     ...
//   }
template <typename D>
inline bool parse(const std::string& fmt, const std::string& input,
                  const time_zone& tz, time_point<D>* tpp) {
  time_point<seconds> sec;
  detail::femtoseconds fs;
  return detail::parse(fmt, input, tz, &sec, &fs) &&
         detail::join_seconds(sec, fs, tpp);
}

namespace detail {

// Undefined behavior if time_point<seconds> is not of sufficient range.
// Note that this means it is UB to call cctz::time_zone::lookup(tp) or
// cctz::format(fmt, tp, tz) with a time_point that is outside the range
// of a 64-bit std::time_t.
template <typename D>
std::pair<time_point<seconds>, D> split_seconds(const time_point<D>& tp) {
  auto sec = std::chrono::time_point_cast<seconds>(tp);
  auto sub = tp - sec;
  if (sub.count() < 0) {
    sec -= seconds(1);
    sub += seconds(1);
  }
  return {sec, std::chrono::duration_cast<D>(sub)};
}

inline std::pair<time_point<seconds>, seconds> split_seconds(
    const time_point<seconds>& tp) {
  return {tp, seconds::zero()};
}

// Floors to the resolution of time_point<D>. Returns false if time_point<D>
// is not of sufficient range.
template <typename Rep, std::intmax_t Denom>
bool join_seconds(
    const time_point<seconds>& sec, const femtoseconds& fs,
    time_point<std::chrono::duration<Rep, std::ratio<1, Denom>>>* tpp) {
  using D = std::chrono::duration<Rep, std::ratio<1, Denom>>;

  *tpp = std::chrono::time_point_cast<D>(sec);
  *tpp += std::chrono::duration_cast<D>(fs);
  return true;
}

template <typename Rep, std::intmax_t Num>
bool join_seconds(
    const time_point<seconds>& sec, const femtoseconds&,
    time_point<std::chrono::duration<Rep, std::ratio<Num, 1>>>* tpp) {
  using D = std::chrono::duration<Rep, std::ratio<Num, 1>>;
  auto count = sec.time_since_epoch().count();
  if (count >= 0 || count % Num == 0) {
    count /= Num;
  } else {
    count /= Num;
    count -= 1;
  }
  if (count > (std::numeric_limits<Rep>::max)()) return false;
  if (count < (std::numeric_limits<Rep>::min)()) return false;
  *tpp = time_point<D>() + D{static_cast<Rep>(count)};
  return true;
}

template <typename Rep>
bool join_seconds(
    const time_point<seconds>& sec, const femtoseconds&,
    time_point<std::chrono::duration<Rep, std::ratio<1, 1>>>* tpp) {
  using D = std::chrono::duration<Rep, std::ratio<1, 1>>;
  auto count = sec.time_since_epoch().count();
  if (count > (std::numeric_limits<Rep>::max)()) return false;
  if (count < (std::numeric_limits<Rep>::min)()) return false;
  *tpp = time_point<D>() + D{static_cast<Rep>(count)};
  return true;
}

inline bool join_seconds(const time_point<seconds>& sec, const femtoseconds&,
                         time_point<seconds>* tpp) {
  *tpp = sec;
  return true;
}

}  // namespace detail
}  // namespace cctz
}  // namespace time_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_TIME_INTERNAL_CCTZ_TIME_ZONE_H_
