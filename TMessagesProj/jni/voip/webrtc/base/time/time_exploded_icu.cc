// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time/time.h"

#include <memory>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "third_party/icu/source/i18n/unicode/calendar.h"
#include "third_party/icu/source/i18n/unicode/timezone.h"

namespace base {

static_assert(
    sizeof(Time::Exploded::year) == sizeof(int32_t),
    "The sizes of Time::Exploded members and ICU date fields do not match.");

namespace {

// and for GMT otherwise. Returns null on error.
std::unique_ptr<icu::Calendar> CreateCalendar(bool is_local) {
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<icu::Calendar> calendar =
      base::WrapUnique(is_local ? icu::Calendar::createInstance(status)
                                : icu::Calendar::createInstance(
                                      *icu::TimeZone::getGMT(), status));
  CHECK(U_SUCCESS(status));

  return calendar;
}

}  // namespace

void Time::Explode(bool is_local, Exploded* exploded) const {
  std::unique_ptr<icu::Calendar> calendar = CreateCalendar(is_local);

  UErrorCode status = U_ZERO_ERROR;
  calendar->setTime(ToRoundedDownMillisecondsSinceUnixEpoch(), status);
  DCHECK(U_SUCCESS(status));

  exploded->year = calendar->get(UCAL_YEAR, status);
  DCHECK(U_SUCCESS(status));

  exploded->month = calendar->get(UCAL_MONTH, status) + 1;
  DCHECK(U_SUCCESS(status));

  exploded->day_of_week = calendar->get(UCAL_DAY_OF_WEEK, status) - 1;
  DCHECK(U_SUCCESS(status));
  exploded->day_of_month = calendar->get(UCAL_DAY_OF_MONTH, status);
  DCHECK(U_SUCCESS(status));
  exploded->hour = calendar->get(UCAL_HOUR_OF_DAY, status);
  DCHECK(U_SUCCESS(status));
  exploded->minute = calendar->get(UCAL_MINUTE, status);
  DCHECK(U_SUCCESS(status));
  exploded->second = calendar->get(UCAL_SECOND, status);
  DCHECK(U_SUCCESS(status));
  exploded->millisecond = calendar->get(UCAL_MILLISECOND, status);
  DCHECK(U_SUCCESS(status));
}

bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {

  CheckedNumeric<int> month = exploded.month;
  month--;
  if (!month.IsValid()) {
    *time = Time(0);
    return false;
  }

  std::unique_ptr<icu::Calendar> calendar = CreateCalendar(is_local);


  calendar->setLenient(false);

  calendar->set(exploded.year, month.ValueOrDie(), exploded.day_of_month,
                exploded.hour, exploded.minute, exploded.second);
  calendar->set(UCAL_MILLISECOND, exploded.millisecond);


  UErrorCode status = U_ZERO_ERROR;
  UDate date = calendar->getTime(status);
  if (U_FAILURE(status)) {
    *time = Time(0);
    return false;
  }

  return FromMillisecondsSinceUnixEpoch(date, time);
}

}  // namespace base
