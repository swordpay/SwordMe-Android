/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_TASK_UTILS_REPEATING_TASK_H_
#define RTC_BASE_TASK_UTILS_REPEATING_TASK_H_

#include <memory>
#include <type_traits>
#include <utility>

#include "absl/functional/any_invocable.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/task_queue/task_queue_base.h"
#include "api/units/time_delta.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {

namespace webrtc_repeating_task_impl {

void RepeatingTaskHandleDTraceProbeStart();
void RepeatingTaskHandleDTraceProbeDelayedStart();
void RepeatingTaskImplDTraceProbeRun();

}  // namespace webrtc_repeating_task_impl

// until they are stopped or the TaskQueue is destroyed. It allows starting and
// stopping multiple times, but you must stop one task before starting another
// and it can only be stopped when in the running state. The public interface is
// not thread safe.
class RepeatingTaskHandle {
 public:
  RepeatingTaskHandle() = default;
  ~RepeatingTaskHandle() = default;
  RepeatingTaskHandle(RepeatingTaskHandle&& other) = default;
  RepeatingTaskHandle& operator=(RepeatingTaskHandle&& other) = default;
  RepeatingTaskHandle(const RepeatingTaskHandle&) = delete;
  RepeatingTaskHandle& operator=(const RepeatingTaskHandle&) = delete;






  static RepeatingTaskHandle Start(TaskQueueBase* task_queue,
                                   absl::AnyInvocable<TimeDelta()> closure,
                                   TaskQueueBase::DelayPrecision precision =
                                       TaskQueueBase::DelayPrecision::kLow,
                                   Clock* clock = Clock::GetRealTimeClock());


  static RepeatingTaskHandle DelayedStart(
      TaskQueueBase* task_queue,
      TimeDelta first_delay,
      absl::AnyInvocable<TimeDelta()> closure,
      TaskQueueBase::DelayPrecision precision =
          TaskQueueBase::DelayPrecision::kLow,
      Clock* clock = Clock::GetRealTimeClock());




  void Stop();


  bool Running() const;

 private:
  explicit RepeatingTaskHandle(
      rtc::scoped_refptr<PendingTaskSafetyFlag> alive_flag)
      : repeating_task_(std::move(alive_flag)) {}
  rtc::scoped_refptr<PendingTaskSafetyFlag> repeating_task_;
};

}  // namespace webrtc
#endif  // RTC_BASE_TASK_UTILS_REPEATING_TASK_H_
