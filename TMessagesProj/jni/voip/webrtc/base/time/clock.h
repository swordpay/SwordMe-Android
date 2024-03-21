// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TIME_CLOCK_H_
#define BASE_TIME_CLOCK_H_

#include "base/base_export.h"
#include "base/time/time.h"

namespace base {

// intended to be able to test the behavior of classes with respect to
// time.
//
// See DefaultClock (base/time/default_clock.h) for the default
// implementation that simply uses Time::Now().
//
// (An implementation that uses Time::SystemTime() should be added as
// needed.)
//
// See SimpleTestClock (base/test/simple_test_clock.h) for a simple
// test implementation.
//
// See TickClock (base/time/tick_clock.h) for the equivalent interface for
// TimeTicks.
class BASE_EXPORT Clock {
 public:
  virtual ~Clock();



  virtual Time Now() const = 0;
};

}  // namespace base

#endif  // BASE_TIME_CLOCK_H_
