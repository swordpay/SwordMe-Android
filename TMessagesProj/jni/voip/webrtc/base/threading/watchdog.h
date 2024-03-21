// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// duration of time passes without proper attention.  The duration of time is
// specified at construction time.  The Watchdog may be used many times by
// simply calling Arm() (to start timing) and Disarm() (to reset the timer).
// The Watchdog is typically used under a debugger, where the stack traces on
// other threads can be examined if/when the Watchdog alarms.

// facilitate such code, an "enabled" argument for the constuctor can be used
// to permanently disable the watchdog.  Disabled watchdogs don't even spawn
// a second thread, and their methods call (Arm() and Disarm()) return very
// quickly.

#ifndef BASE_THREADING_WATCHDOG_H_
#define BASE_THREADING_WATCHDOG_H_

#include <string>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

namespace base {

class BASE_EXPORT Watchdog {
 public:

  Watchdog(const TimeDelta& duration,
           const std::string& thread_watched_name,
           bool enabled);
  virtual ~Watchdog();

  void Cleanup();


  bool IsJoinable();

  void Arm();  // Arm  starting now.
  void ArmSomeTimeDeltaAgo(const TimeDelta& time_delta);
  void ArmAtStartTime(const TimeTicks start_time);

  void Disarm();


  virtual void Alarm();


  static void ResetStaticData();

 private:
  class ThreadDelegate : public PlatformThread::Delegate {
   public:
    explicit ThreadDelegate(Watchdog* watchdog) : watchdog_(watchdog) {
    }
    void ThreadMain() override;

   private:
    void SetThreadName() const;

    Watchdog* watchdog_;
  };

  enum State {ARMED, DISARMED, SHUTDOWN, JOINABLE };

  bool enabled_;

  Lock lock_;  // Mutex for state_.
  ConditionVariable condition_variable_;
  State state_;
  const TimeDelta duration_;  // How long after start_time_ do we alarm?
  const std::string thread_watched_name_;
  PlatformThreadHandle handle_;
  ThreadDelegate delegate_;  // Store it, because it must outlive the thread.

  TimeTicks start_time_;  // Start of epoch, and alarm after duration_.

  DISALLOW_COPY_AND_ASSIGN(Watchdog);
};

}  // namespace base

#endif  // BASE_THREADING_WATCHDOG_H_
