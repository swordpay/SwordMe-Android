// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// formatting for displaying the time.

#ifndef BASE_I18N_TIME_FORMATTING_H_
#define BASE_I18N_TIME_FORMATTING_H_

#include "base/compiler_specific.h"
#include "base/i18n/base_i18n_export.h"
#include "base/strings/string16.h"

namespace base {

class Time;
class TimeDelta;

enum HourClockType {
  k12HourClock,  // Uses 1-12. e.g., "3:07 PM"
  k24HourClock,  // Uses 0-23. e.g., "15:07"
};

enum AmPmClockType {
  kDropAmPm,  // Drops AM/PM sign. e.g., "3:07"
  kKeepAmPm,  // Keeps AM/PM sign. e.g., "3:07 PM"
};

// requiring third_party/icu dependencies with this file.
enum DurationFormatWidth {
  DURATION_WIDTH_WIDE,    // "3 hours, 7 minutes"
  DURATION_WIDTH_SHORT,   // "3 hr, 7 min"
  DURATION_WIDTH_NARROW,  // "3h 7m"
  DURATION_WIDTH_NUMERIC  // "3:07"
};

// necessary.
enum DateFormat {

  DATE_FORMAT_YEAR_MONTH,

  DATE_FORMAT_MONTH_WEEKDAY_DAY,
};

BASE_I18N_EXPORT string16 TimeFormatTimeOfDay(const Time& time);

// e.g., "15:07:30.568"
BASE_I18N_EXPORT string16 TimeFormatTimeOfDayWithMilliseconds(const Time& time);

// "3:07 PM" (type == k12HourClock, ampm == kKeepAmPm).
// "3:07"    (type == k12HourClock, ampm == kDropAmPm).
// "15:07"   (type == k24HourClock).
BASE_I18N_EXPORT string16 TimeFormatTimeOfDayWithHourClockType(
    const Time& time,
    HourClockType type,
    AmPmClockType ampm);

BASE_I18N_EXPORT string16 TimeFormatShortDate(const Time& time);

BASE_I18N_EXPORT string16 TimeFormatShortDateNumeric(const Time& time);

BASE_I18N_EXPORT string16 TimeFormatShortDateAndTime(const Time& time);

BASE_I18N_EXPORT string16 TimeFormatMonthAndYear(const Time& time);

// "12/13/52 2:44:30 PM PST".
BASE_I18N_EXPORT string16
TimeFormatShortDateAndTimeWithTimeZone(const Time& time);

// "Monday, March 6, 2008 2:44:30 PM".
BASE_I18N_EXPORT string16 TimeFormatFriendlyDateAndTime(const Time& time);

// "Monday, March 6, 2008".
BASE_I18N_EXPORT string16 TimeFormatFriendlyDate(const Time& time);

// when an unusual time format is needed, e.g. "Feb. 2, 18:00".
//
// See http://userguide.icu-project.org/formatparse/datetime for details.
BASE_I18N_EXPORT string16 TimeFormatWithPattern(const Time& time,
                                                const char* pattern);

// "3:07" or "3 hours, 7 minutes", and returns true on success. See
// DurationFormatWidth for details.
//
// Please don't use width = DURATION_WIDTH_NUMERIC when the time duration
// can possibly be larger than 24h, as the hour value will be cut below 24
// after formatting.
// TODO(crbug.com/675791): fix function output when width =
// DURATION_WIDTH_NUMERIC.
BASE_I18N_EXPORT bool TimeDurationFormat(const TimeDelta time,
                                         const DurationFormatWidth width,
                                         string16* out) WARN_UNUSED_RESULT;

// e.g., "3:07:30" or "3 hours, 7 minutes, 30 seconds", and returns true on
// success. See DurationFormatWidth for details.
//
// Please don't use width = DURATION_WIDTH_NUMERIC when the time duration
// can possibly be larger than 24h, as the hour value will be cut below 24
// after formatting.
// TODO(crbug.com/675791): fix function output when width =
// DURATION_WIDTH_NUMERIC.
BASE_I18N_EXPORT bool TimeDurationFormatWithSeconds(
    const TimeDelta time,
    const DurationFormatWidth width,
    string16* out) WARN_UNUSED_RESULT;

// or "March 2016 - December 2016". See DateFormat for details.
BASE_I18N_EXPORT string16 DateIntervalFormat(const Time& begin_time,
                                             const Time& end_time,
                                             DateFormat format);

// k12HourClock (en-US).
// k24HourClock (en-GB).
BASE_I18N_EXPORT HourClockType GetHourClockType();

}  // namespace base

#endif  // BASE_I18N_TIME_FORMATTING_H_
