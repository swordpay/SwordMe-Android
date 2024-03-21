// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TIME_DEFAULT_TICK_CLOCK_H_
#define BASE_TIME_DEFAULT_TICK_CLOCK_H_

#include "base/base_export.h"
#include "base/time/tick_clock.h"

namespace base {

// This is typically used by components that expose a SetTickClockForTesting().
// Note: Overriding Time/TimeTicks altogether via
// TaskEnvironment::TimeSource::MOCK_TIME is now the preferred of
// overriding time in unit tests. As such, there shouldn't be many new use cases
// for TickClock/DefaultTickClock anymore.
class BASE_EXPORT DefaultTickClock : public TickClock {
 public:
  ~DefaultTickClock() override;

  TimeTicks NowTicks() const override;

  static const DefaultTickClock* GetInstance();
};

}  // namespace base

#endif  // BASE_TIME_DEFAULT_TICK_CLOCK_H_
