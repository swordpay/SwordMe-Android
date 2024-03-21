/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef API_TASK_QUEUE_TASK_QUEUE_BASE_H_
#define API_TASK_QUEUE_TASK_QUEUE_BASE_H_

#include <memory>
#include <utility>

#include "absl/functional/any_invocable.h"
#include "api/units/time_delta.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {

// in FIFO order and that tasks never overlap. Tasks may always execute on the
// same worker thread and they may not. To DCHECK that tasks are executing on a
// known task queue, use IsCurrent().
class RTC_LOCKABLE RTC_EXPORT TaskQueueBase {
 public:
  enum class DelayPrecision {


    kLow,



    kHigh,
  };











  virtual void Delete() = 0;







  virtual void PostTask(absl::AnyInvocable<void() &&> task) = 0;























  virtual void PostDelayedTask(absl::AnyInvocable<void() &&> task,
                               TimeDelta delay) = 0;
















  virtual void PostDelayedHighPrecisionTask(absl::AnyInvocable<void() &&> task,
                                            TimeDelta delay) = 0;


  void PostDelayedTaskWithPrecision(DelayPrecision precision,
                                    absl::AnyInvocable<void() &&> task,
                                    TimeDelta delay) {
    switch (precision) {
      case DelayPrecision::kLow:
        PostDelayedTask(std::move(task), delay);
        break;
      case DelayPrecision::kHigh:
        PostDelayedHighPrecisionTask(std::move(task), delay);
        break;
    }
  }



  static TaskQueueBase* Current();
  bool IsCurrent() const { return Current() == this; }

 protected:
  class RTC_EXPORT CurrentTaskQueueSetter {
   public:
    explicit CurrentTaskQueueSetter(TaskQueueBase* task_queue);
    CurrentTaskQueueSetter(const CurrentTaskQueueSetter&) = delete;
    CurrentTaskQueueSetter& operator=(const CurrentTaskQueueSetter&) = delete;
    ~CurrentTaskQueueSetter();

   private:
    TaskQueueBase* const previous_;
  };


  virtual ~TaskQueueBase() = default;
};

struct TaskQueueDeleter {
  void operator()(TaskQueueBase* task_queue) const { task_queue->Delete(); }
};

}  // namespace webrtc

#endif  // API_TASK_QUEUE_TASK_QUEUE_BASE_H_
