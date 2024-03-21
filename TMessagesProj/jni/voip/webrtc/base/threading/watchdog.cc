// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/watchdog.h"

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/threading/platform_thread.h"

namespace base {

namespace {

// armed will expire (also alarm).  To diminish this effect, we track any
// delay due to debugger breaks, and we *try* to adjust the effective start
// time of other alarms to step past the debugging break.
// Without this safety net, any alarm will typically trigger a host of follow
// on alarms from callers that specify old times.

struct StaticData {

  Lock lock;

  TimeTicks last_debugged_alarm_time;

  TimeDelta last_debugged_alarm_delay;
};

StaticData* GetStaticData() {
  static base::NoDestructor<StaticData> static_data;
  return static_data.get();
}

}  // namespace

Watchdog::Watchdog(const TimeDelta& duration,
                   const std::string& thread_watched_name,
                   bool enabled)
  : enabled_(enabled),
    lock_(),
    condition_variable_(&lock_),
    state_(DISARMED),
    duration_(duration),
    thread_watched_name_(thread_watched_name),
    delegate_(this) {
  if (!enabled_)
    return;  // Don't start thread, or doing anything really.
  enabled_ = PlatformThread::Create(0,  // Default stack size.
                                    &delegate_,
                                    &handle_);
  DCHECK(enabled_);
}

Watchdog::~Watchdog() {
  if (!enabled_)
    return;
  if (!IsJoinable())
    Cleanup();
  PlatformThread::Join(handle_);
}

void Watchdog::Cleanup() {
  if (!enabled_)
    return;
  AutoLock lock(lock_);
  state_ = SHUTDOWN;
  condition_variable_.Signal();
}

bool Watchdog::IsJoinable() {
  if (!enabled_)
    return true;
  AutoLock lock(lock_);
  return (state_ == JOINABLE);
}

void Watchdog::Arm() {
  ArmAtStartTime(TimeTicks::Now());
}

void Watchdog::ArmSomeTimeDeltaAgo(const TimeDelta& time_delta) {
  ArmAtStartTime(TimeTicks::Now() - time_delta);
}

void Watchdog::ArmAtStartTime(const TimeTicks start_time) {
  AutoLock lock(lock_);
  start_time_ = start_time;
  state_ = ARMED;


  condition_variable_.Signal();
}

void Watchdog::Disarm() {
  AutoLock lock(lock_);
  state_ = DISARMED;


}

void Watchdog::Alarm() {
  DVLOG(1) << "Watchdog alarmed for " << thread_watched_name_;
}

// Internal private methods that the watchdog thread uses.

void Watchdog::ThreadDelegate::ThreadMain() {
  SetThreadName();
  TimeDelta remaining_duration;
  StaticData* static_data = GetStaticData();
  while (1) {
    AutoLock lock(watchdog_->lock_);
    while (DISARMED == watchdog_->state_)
      watchdog_->condition_variable_.Wait();
    if (SHUTDOWN == watchdog_->state_) {
      watchdog_->state_ = JOINABLE;
      return;
    }
    DCHECK(ARMED == watchdog_->state_);
    remaining_duration = watchdog_->duration_ -
        (TimeTicks::Now() - watchdog_->start_time_);
    if (remaining_duration.InMilliseconds() > 0) {

      watchdog_->condition_variable_.TimedWait(remaining_duration);
      continue;
    }


    {
      AutoLock static_lock(static_data->lock);
      if (static_data->last_debugged_alarm_time > watchdog_->start_time_) {


        watchdog_->start_time_ += static_data->last_debugged_alarm_delay;
        if (static_data->last_debugged_alarm_time > watchdog_->start_time_)

          watchdog_->state_ = DISARMED;
        continue;
      }
    }
    watchdog_->state_ = DISARMED;  // Only alarm at most once.
    TimeTicks last_alarm_time = TimeTicks::Now();
    {
      AutoUnlock unlock(watchdog_->lock_);
      watchdog_->Alarm();  // Set a break point here to debug on alarms.
    }
    TimeDelta last_alarm_delay = TimeTicks::Now() - last_alarm_time;
    if (last_alarm_delay <= TimeDelta::FromMilliseconds(2))
      continue;

    AutoLock static_lock(static_data->lock);

    static_data->last_debugged_alarm_time = last_alarm_time;
    static_data->last_debugged_alarm_delay = last_alarm_delay;
  }
}

void Watchdog::ThreadDelegate::SetThreadName() const {
  std::string name = watchdog_->thread_watched_name_ + " Watchdog";
  PlatformThread::SetName(name);
  DVLOG(1) << "Watchdog active: " << name;
}

void Watchdog::ResetStaticData() {
  StaticData* static_data = GetStaticData();
  AutoLock lock(static_data->lock);

  static_data->last_debugged_alarm_time = TimeTicks::Min();
  static_data->last_debugged_alarm_delay = TimeDelta();
}

}  // namespace base
