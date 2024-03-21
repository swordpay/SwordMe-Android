// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time/time.h"

namespace base {

TimeTicks TimeTicks::FromUptimeMillis(int64_t uptime_millis_value) {












  return TimeTicks(uptime_millis_value * Time::kMicrosecondsPerMillisecond);
}

}  // namespace base
