// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TIMER_ELAPSED_TIMER_H_
#define BASE_TIMER_ELAPSED_TIMER_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/time/time.h"

namespace base {

class BASE_EXPORT ElapsedTimer {
 public:
  ElapsedTimer();
  ElapsedTimer(ElapsedTimer&& other);

  void operator=(ElapsedTimer&& other);

  TimeDelta Elapsed() const;

  TimeTicks Begin() const { return begin_; }

 private:
  TimeTicks begin_;

  DISALLOW_COPY_AND_ASSIGN(ElapsedTimer);
};

class BASE_EXPORT ElapsedThreadTimer {
 public:
  ElapsedThreadTimer();


  TimeDelta Elapsed() const;

  bool is_supported() const { return is_supported_; }

 private:
  const bool is_supported_;
  const ThreadTicks begin_;

  DISALLOW_COPY_AND_ASSIGN(ElapsedThreadTimer);
};

// Elapsed(Thread)Timers will always return kMockElapsedTime from Elapsed().
// This is useful, for example, in unit tests that verify that their impl
// records timing histograms. It enables such tests to observe reliable timings.
class BASE_EXPORT ScopedMockElapsedTimersForTest {
 public:
  static constexpr TimeDelta kMockElapsedTime =
      TimeDelta::FromMilliseconds(1337);



  ScopedMockElapsedTimersForTest();
  ~ScopedMockElapsedTimersForTest();

 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedMockElapsedTimersForTest);
};

}  // namespace base

#endif  // BASE_TIMER_ELAPSED_TIMER_H_
