// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time/time.h"

#include <zircon/syscalls.h>

#include "base/fuchsia/fuchsia_logging.h"
#include "base/time/time_override.h"

namespace base {


namespace subtle {
Time TimeNowIgnoringOverride() {
  zx_time_t nanos_since_unix_epoch;
  zx_status_t status = zx_clock_get(ZX_CLOCK_UTC, &nanos_since_unix_epoch);
  ZX_CHECK(status == ZX_OK, status);

  return Time::FromZxTime(nanos_since_unix_epoch);
}

Time TimeNowFromSystemTimeIgnoringOverride() {

  return TimeNowIgnoringOverride();
}
}  // namespace subtle


namespace subtle {
TimeTicks TimeTicksNowIgnoringOverride() {
  const zx_time_t nanos_since_boot = zx_clock_get_monotonic();
  CHECK(nanos_since_boot != 0);
  return TimeTicks::FromZxTime(nanos_since_boot);
}
}  // namespace subtle

TimeDelta TimeDelta::FromZxDuration(zx_duration_t nanos) {
  return TimeDelta::FromNanoseconds(nanos);
}

zx_duration_t TimeDelta::ToZxDuration() const {
  return InNanoseconds();
}

Time Time::FromZxTime(zx_time_t nanos_since_unix_epoch) {
  return Time::UnixEpoch() + TimeDelta::FromNanoseconds(nanos_since_unix_epoch);
}

zx_time_t Time::ToZxTime() const {
  return (*this - Time::UnixEpoch()).InNanoseconds();
}

TimeTicks::Clock TimeTicks::GetClock() {
  return Clock::FUCHSIA_ZX_CLOCK_MONOTONIC;
}

bool TimeTicks::IsHighResolution() {
  return true;
}

bool TimeTicks::IsConsistentAcrossProcesses() {
  return true;
}

TimeTicks TimeTicks::FromZxTime(zx_time_t nanos_since_boot) {
  return TimeTicks() + TimeDelta::FromNanoseconds(nanos_since_boot);
}

zx_time_t TimeTicks::ToZxTime() const {
  return (*this - TimeTicks()).InNanoseconds();
}


namespace subtle {
ThreadTicks ThreadTicksNowIgnoringOverride() {
  zx_time_t nanos_since_thread_started;
  zx_status_t status =
      zx_clock_get(ZX_CLOCK_THREAD, &nanos_since_thread_started);
  ZX_CHECK(status == ZX_OK, status);
  DCHECK(nanos_since_thread_started != 0);
  return ThreadTicks() + TimeDelta::FromNanoseconds(nanos_since_thread_started);
}
}  // namespace subtle

}  // namespace base
