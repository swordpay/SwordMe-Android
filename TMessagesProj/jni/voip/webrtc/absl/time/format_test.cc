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

#include <cstdint>
#include <limits>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/time/internal/test_util.h"
#include "absl/time/time.h"

using testing::HasSubstr;

namespace {

// and trailing characters.  For example: TestFormatSpecifier(t, "%a", "Thu").
void TestFormatSpecifier(absl::Time t, absl::TimeZone tz,
                         const std::string& fmt, const std::string& ans) {
  EXPECT_EQ(ans, absl::FormatTime(fmt, t, tz));
  EXPECT_EQ("xxx " + ans, absl::FormatTime("xxx " + fmt, t, tz));
  EXPECT_EQ(ans + " yyy", absl::FormatTime(fmt + " yyy", t, tz));
  EXPECT_EQ("xxx " + ans + " yyy",
            absl::FormatTime("xxx " + fmt + " yyy", t, tz));
}

// Testing FormatTime()
//

TEST(FormatTime, Basics) {
  absl::TimeZone tz = absl::UTCTimeZone();
  absl::Time t = absl::FromTimeT(0);

  EXPECT_EQ("", absl::FormatTime("", t, tz));
  EXPECT_EQ(" ", absl::FormatTime(" ", t, tz));
  EXPECT_EQ("  ", absl::FormatTime("  ", t, tz));
  EXPECT_EQ("xxx", absl::FormatTime("xxx", t, tz));
  std::string big(128, 'x');
  EXPECT_EQ(big, absl::FormatTime(big, t, tz));

  std::string bigger(100000, 'x');
  EXPECT_EQ(bigger, absl::FormatTime(bigger, t, tz));

  t += absl::Hours(13) + absl::Minutes(4) + absl::Seconds(5);
  t += absl::Milliseconds(6) + absl::Microseconds(7) + absl::Nanoseconds(8);
  EXPECT_EQ("1970-01-01", absl::FormatTime("%Y-%m-%d", t, tz));
  EXPECT_EQ("13:04:05", absl::FormatTime("%H:%M:%S", t, tz));
  EXPECT_EQ("13:04:05.006", absl::FormatTime("%H:%M:%E3S", t, tz));
  EXPECT_EQ("13:04:05.006007", absl::FormatTime("%H:%M:%E6S", t, tz));
  EXPECT_EQ("13:04:05.006007008", absl::FormatTime("%H:%M:%E9S", t, tz));
}

TEST(FormatTime, LocaleSpecific) {
  const absl::TimeZone tz = absl::UTCTimeZone();
  absl::Time t = absl::FromTimeT(0);

  TestFormatSpecifier(t, tz, "%a", "Thu");
  TestFormatSpecifier(t, tz, "%A", "Thursday");
  TestFormatSpecifier(t, tz, "%b", "Jan");
  TestFormatSpecifier(t, tz, "%B", "January");

  const std::string s =
      absl::FormatTime("%c", absl::FromTimeT(0), absl::UTCTimeZone());
  EXPECT_THAT(s, HasSubstr("1970"));
  EXPECT_THAT(s, HasSubstr("00:00:00"));

  TestFormatSpecifier(t, tz, "%p", "AM");
  TestFormatSpecifier(t, tz, "%x", "01/01/70");
  TestFormatSpecifier(t, tz, "%X", "00:00:00");
}

TEST(FormatTime, ExtendedSeconds) {
  const absl::TimeZone tz = absl::UTCTimeZone();

  absl::Time t = absl::FromTimeT(0) + absl::Seconds(5);
  EXPECT_EQ("05", absl::FormatTime("%E*S", t, tz));
  EXPECT_EQ("05.000000000000000", absl::FormatTime("%E15S", t, tz));

  t += absl::Milliseconds(6) + absl::Microseconds(7) + absl::Nanoseconds(8);
  EXPECT_EQ("05.006007008", absl::FormatTime("%E*S", t, tz));
  EXPECT_EQ("05", absl::FormatTime("%E0S", t, tz));
  EXPECT_EQ("05.006007008000000", absl::FormatTime("%E15S", t, tz));

  t = absl::FromUnixMicros(-1);
  EXPECT_EQ("1969-12-31 23:59:59.999999",
            absl::FormatTime("%Y-%m-%d %H:%M:%E*S", t, tz));



  t = absl::FromUnixMicros(1395024427333304);
  EXPECT_EQ("2014-03-17 02:47:07.333304",
            absl::FormatTime("%Y-%m-%d %H:%M:%E*S", t, tz));
  t += absl::Microseconds(1);
  EXPECT_EQ("2014-03-17 02:47:07.333305",
            absl::FormatTime("%Y-%m-%d %H:%M:%E*S", t, tz));
}

TEST(FormatTime, RFC1123FormatPadsYear) {  // locale specific
  absl::TimeZone tz = absl::UTCTimeZone();

  absl::Time t = absl::FromCivil(absl::CivilSecond(77, 6, 28, 9, 8, 7), tz);
  EXPECT_EQ("Mon, 28 Jun 0077 09:08:07 +0000",
            absl::FormatTime(absl::RFC1123_full, t, tz));
  EXPECT_EQ("28 Jun 0077 09:08:07 +0000",
            absl::FormatTime(absl::RFC1123_no_wday, t, tz));
}

TEST(FormatTime, InfiniteTime) {
  absl::TimeZone tz = absl::time_internal::LoadTimeZone("America/Los_Angeles");

  EXPECT_EQ("infinite-future",
            absl::FormatTime("%H:%M blah", absl::InfiniteFuture(), tz));
  EXPECT_EQ("infinite-past",
            absl::FormatTime("%H:%M blah", absl::InfinitePast(), tz));
}

// Testing ParseTime()
//

TEST(ParseTime, Basics) {
  absl::Time t = absl::FromTimeT(1234567890);
  std::string err;

  EXPECT_TRUE(absl::ParseTime("", "", &t, &err)) << err;
  EXPECT_EQ(absl::UnixEpoch(), t);  // everything defaulted
  EXPECT_TRUE(absl::ParseTime(" ", " ", &t, &err)) << err;
  EXPECT_TRUE(absl::ParseTime("  ", "  ", &t, &err)) << err;
  EXPECT_TRUE(absl::ParseTime("x", "x", &t, &err)) << err;
  EXPECT_TRUE(absl::ParseTime("xxx", "xxx", &t, &err)) << err;

  EXPECT_TRUE(absl::ParseTime("%Y-%m-%d %H:%M:%S %z",
                              "2013-06-28 19:08:09 -0800", &t, &err))
      << err;
  const auto ci = absl::FixedTimeZone(-8 * 60 * 60).At(t);
  EXPECT_EQ(absl::CivilSecond(2013, 6, 28, 19, 8, 9), ci.cs);
  EXPECT_EQ(absl::ZeroDuration(), ci.subsecond);
}

TEST(ParseTime, NullErrorString) {
  absl::Time t;
  EXPECT_FALSE(absl::ParseTime("%Q", "invalid format", &t, nullptr));
  EXPECT_FALSE(absl::ParseTime("%H", "12 trailing data", &t, nullptr));
  EXPECT_FALSE(
      absl::ParseTime("%H out of range", "42 out of range", &t, nullptr));
}

TEST(ParseTime, WithTimeZone) {
  const absl::TimeZone tz =
      absl::time_internal::LoadTimeZone("America/Los_Angeles");
  absl::Time t;
  std::string e;

  EXPECT_TRUE(
      absl::ParseTime("%Y-%m-%d %H:%M:%S", "2013-06-28 19:08:09", tz, &t, &e))
      << e;
  auto ci = tz.At(t);
  EXPECT_EQ(absl::CivilSecond(2013, 6, 28, 19, 8, 9), ci.cs);
  EXPECT_EQ(absl::ZeroDuration(), ci.subsecond);

  EXPECT_TRUE(absl::ParseTime("%Y-%m-%d %H:%M:%S %z",
                              "2013-06-28 19:08:09 +0800", tz, &t, &e))
      << e;
  ci = absl::FixedTimeZone(8 * 60 * 60).At(t);
  EXPECT_EQ(absl::CivilSecond(2013, 6, 28, 19, 8, 9), ci.cs);
  EXPECT_EQ(absl::ZeroDuration(), ci.subsecond);
}

TEST(ParseTime, ErrorCases) {
  absl::Time t = absl::FromTimeT(0);
  std::string err;

  EXPECT_FALSE(absl::ParseTime("%S", "123", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));

  err.clear();
  EXPECT_FALSE(absl::ParseTime("%Q", "x", &t, &err)) << err;


  EXPECT_FALSE(err.empty());

  EXPECT_FALSE(absl::ParseTime("%m-%d", "2-3 blah", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));

  EXPECT_FALSE(absl::ParseTime("%m-%d", "2-31", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Out-of-range"));

  EXPECT_TRUE(absl::ParseTime("%z", "-0203", &t, &err)) << err;
  EXPECT_FALSE(absl::ParseTime("%z", "- 2 3", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_TRUE(absl::ParseTime("%Ez", "-02:03", &t, &err)) << err;
  EXPECT_FALSE(absl::ParseTime("%Ez", "- 2: 3", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));

  EXPECT_FALSE(absl::ParseTime("%Ez", "+-08:00", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%Ez", "-+08:00", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));

  EXPECT_FALSE(absl::ParseTime("%Y", "-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%E4Y", "-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%H", "-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%M", "-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%S", "-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%z", "+-000", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%Ez", "+-0:00", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
  EXPECT_FALSE(absl::ParseTime("%z", "-00-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));
  EXPECT_FALSE(absl::ParseTime("%Ez", "-00:-0", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));
}

TEST(ParseTime, ExtendedSeconds) {
  std::string err;
  absl::Time t;



  t = absl::UnixEpoch();
  EXPECT_TRUE(absl::ParseTime("%E*S", "0.2147483647", &t, &err)) << err;
  EXPECT_EQ(absl::UnixEpoch() + absl::Nanoseconds(214748364) +
                absl::Nanoseconds(1) / 2,
            t);
  t = absl::UnixEpoch();
  EXPECT_TRUE(absl::ParseTime("%E*S", "0.2147483648", &t, &err)) << err;
  EXPECT_EQ(absl::UnixEpoch() + absl::Nanoseconds(214748364) +
                absl::Nanoseconds(3) / 4,
            t);


  t = absl::UnixEpoch();
  EXPECT_TRUE(absl::ParseTime(
      "%E*S", "0.214748364801234567890123456789012345678901234567890123456789",
      &t, &err))
      << err;
  EXPECT_EQ(absl::UnixEpoch() + absl::Nanoseconds(214748364) +
                absl::Nanoseconds(3) / 4,
            t);
}

TEST(ParseTime, ExtendedOffsetErrors) {
  std::string err;
  absl::Time t;

  EXPECT_FALSE(absl::ParseTime("%z", "-123", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));

  EXPECT_FALSE(absl::ParseTime("%z", "-1", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));

  EXPECT_FALSE(absl::ParseTime("%Ez", "-12:3", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));

  EXPECT_FALSE(absl::ParseTime("%Ez", "-123", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Illegal trailing data"));

  EXPECT_FALSE(absl::ParseTime("%Ez", "-1", &t, &err)) << err;
  EXPECT_THAT(err, HasSubstr("Failed to parse"));
}

TEST(ParseTime, InfiniteTime) {
  absl::Time t;
  std::string err;
  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "infinite-future", &t, &err));
  EXPECT_EQ(absl::InfiniteFuture(), t);

  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "  infinite-future", &t, &err));
  EXPECT_EQ(absl::InfiniteFuture(), t);
  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "infinite-future  ", &t, &err));
  EXPECT_EQ(absl::InfiniteFuture(), t);
  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "  infinite-future  ", &t, &err));
  EXPECT_EQ(absl::InfiniteFuture(), t);

  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "infinite-past", &t, &err));
  EXPECT_EQ(absl::InfinitePast(), t);

  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "  infinite-past", &t, &err));
  EXPECT_EQ(absl::InfinitePast(), t);
  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "infinite-past  ", &t, &err));
  EXPECT_EQ(absl::InfinitePast(), t);
  EXPECT_TRUE(absl::ParseTime("%H:%M blah", "  infinite-past  ", &t, &err));
  EXPECT_EQ(absl::InfinitePast(), t);

  absl::TimeZone tz = absl::UTCTimeZone();
  EXPECT_TRUE(absl::ParseTime("infinite-future %H:%M", "infinite-future 03:04",
                              &t, &err));
  EXPECT_NE(absl::InfiniteFuture(), t);
  EXPECT_EQ(3, tz.At(t).cs.hour());
  EXPECT_EQ(4, tz.At(t).cs.minute());

  EXPECT_TRUE(
      absl::ParseTime("infinite-past %H:%M", "infinite-past 03:04", &t, &err));
  EXPECT_NE(absl::InfinitePast(), t);
  EXPECT_EQ(3, tz.At(t).cs.hour());
  EXPECT_EQ(4, tz.At(t).cs.minute());

  EXPECT_FALSE(absl::ParseTime("infinite-future %H:%M", "03:04", &t, &err));
  EXPECT_FALSE(absl::ParseTime("infinite-past %H:%M", "03:04", &t, &err));
}

TEST(ParseTime, FailsOnUnrepresentableTime) {
  const absl::TimeZone utc = absl::UTCTimeZone();
  absl::Time t;
  EXPECT_FALSE(
      absl::ParseTime("%Y-%m-%d", "-292277022657-01-27", utc, &t, nullptr));
  EXPECT_TRUE(
      absl::ParseTime("%Y-%m-%d", "-292277022657-01-28", utc, &t, nullptr));
  EXPECT_TRUE(
      absl::ParseTime("%Y-%m-%d", "292277026596-12-04", utc, &t, nullptr));
  EXPECT_FALSE(
      absl::ParseTime("%Y-%m-%d", "292277026596-12-05", utc, &t, nullptr));
}

// Roundtrip test for FormatTime()/ParseTime().
//

TEST(FormatParse, RoundTrip) {
  const absl::TimeZone lax =
      absl::time_internal::LoadTimeZone("America/Los_Angeles");
  const absl::Time in =
      absl::FromCivil(absl::CivilSecond(1977, 6, 28, 9, 8, 7), lax);
  const absl::Duration subseconds = absl::Nanoseconds(654321);
  std::string err;

  {
    absl::Time out;
    const std::string s =
        absl::FormatTime(absl::RFC3339_full, in + subseconds, lax);
    EXPECT_TRUE(absl::ParseTime(absl::RFC3339_full, s, &out, &err))
        << s << ": " << err;
    EXPECT_EQ(in + subseconds, out);  // RFC3339_full includes %Ez
  }

  {
    absl::Time out;
    const std::string s = absl::FormatTime(absl::RFC1123_full, in, lax);
    EXPECT_TRUE(absl::ParseTime(absl::RFC1123_full, s, &out, &err))
        << s << ": " << err;
    EXPECT_EQ(in, out);  // RFC1123_full includes %z
  }









#if !defined(_MSC_VER) && !defined(__EMSCRIPTEN__)


  {
    absl::Time out;
    const std::string s = absl::FormatTime("%c", in, absl::UTCTimeZone());
    EXPECT_TRUE(absl::ParseTime("%c", s, &out, &err)) << s << ": " << err;
    EXPECT_EQ(in, out);
  }
#endif  // !_MSC_VER && !__EMSCRIPTEN__
}

TEST(FormatParse, RoundTripDistantFuture) {
  const absl::TimeZone tz = absl::UTCTimeZone();
  const absl::Time in =
      absl::FromUnixSeconds(std::numeric_limits<int64_t>::max());
  std::string err;

  absl::Time out;
  const std::string s = absl::FormatTime(absl::RFC3339_full, in, tz);
  EXPECT_TRUE(absl::ParseTime(absl::RFC3339_full, s, &out, &err))
      << s << ": " << err;
  EXPECT_EQ(in, out);
}

TEST(FormatParse, RoundTripDistantPast) {
  const absl::TimeZone tz = absl::UTCTimeZone();
  const absl::Time in =
      absl::FromUnixSeconds(std::numeric_limits<int64_t>::min());
  std::string err;

  absl::Time out;
  const std::string s = absl::FormatTime(absl::RFC3339_full, in, tz);
  EXPECT_TRUE(absl::ParseTime(absl::RFC3339_full, s, &out, &err))
      << s << ": " << err;
  EXPECT_EQ(in, out);
}

}  // namespace
