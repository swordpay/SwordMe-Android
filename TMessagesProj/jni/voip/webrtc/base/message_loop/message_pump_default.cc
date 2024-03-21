// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/message_loop/message_pump_default.h"

#include "base/auto_reset.h"
#include "base/logging.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"

#if defined(OS_MACOSX)
#include <mach/thread_policy.h>

#include "base/mac/mach_logging.h"
#include "base/mac/scoped_mach_port.h"
#include "base/mac/scoped_nsautorelease_pool.h"
#endif

namespace base {

MessagePumpDefault::MessagePumpDefault()
    : keep_running_(true),
      event_(WaitableEvent::ResetPolicy::AUTOMATIC,
             WaitableEvent::InitialState::NOT_SIGNALED) {
  event_.declare_only_used_while_idle();
}

MessagePumpDefault::~MessagePumpDefault() = default;

void MessagePumpDefault::Run(Delegate* delegate) {
  AutoReset<bool> auto_reset_keep_running(&keep_running_, true);

  for (;;) {
#if defined(OS_MACOSX)
    mac::ScopedNSAutoreleasePool autorelease_pool;
#endif

    Delegate::NextWorkInfo next_work_info = delegate->DoSomeWork();
    bool has_more_immediate_work = next_work_info.is_immediate();
    if (!keep_running_)
      break;

    if (has_more_immediate_work)
      continue;

    has_more_immediate_work = delegate->DoIdleWork();
    if (!keep_running_)
      break;

    if (has_more_immediate_work)
      continue;

    if (next_work_info.delayed_run_time.is_max()) {
      event_.Wait();
    } else {
      event_.TimedWait(next_work_info.remaining_delay());
    }


  }
}

void MessagePumpDefault::Quit() {
  keep_running_ = false;
}

void MessagePumpDefault::ScheduleWork() {


  event_.Signal();
}

void MessagePumpDefault::ScheduleDelayedWork(
    const TimeTicks& delayed_work_time) {





}

#if defined(OS_MACOSX)
void MessagePumpDefault::SetTimerSlack(TimerSlack timer_slack) {
  thread_latency_qos_policy_data_t policy{};
  policy.thread_latency_qos_tier = timer_slack == TIMER_SLACK_MAXIMUM
                                       ? LATENCY_QOS_TIER_3
                                       : LATENCY_QOS_TIER_UNSPECIFIED;
  mac::ScopedMachSendRight thread_port(mach_thread_self());
  kern_return_t kr =
      thread_policy_set(thread_port.get(), THREAD_LATENCY_QOS_POLICY,
                        reinterpret_cast<thread_policy_t>(&policy),
                        THREAD_LATENCY_QOS_POLICY_COUNT);
  MACH_DVLOG_IF(1, kr != KERN_SUCCESS, kr) << "thread_policy_set";
}
#endif

}  // namespace base
