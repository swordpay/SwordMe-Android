/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// The implementation uses Grand Central Dispatch queues (GCD) to
// do the actual task queuing.

#include "rtc_base/task_queue_gcd.h"

#include <dispatch/dispatch.h>
#include <string.h>

#include <memory>

#include "absl/functional/any_invocable.h"
#include "absl/strings/string_view.h"
#include "api/task_queue/task_queue_base.h"
#include "api/units/time_delta.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/system/gcd_helpers.h"

namespace webrtc {
namespace {

int TaskQueuePriorityToGCD(TaskQueueFactory::Priority priority) {
  switch (priority) {
    case TaskQueueFactory::Priority::NORMAL:
      return DISPATCH_QUEUE_PRIORITY_DEFAULT;
    case TaskQueueFactory::Priority::HIGH:
      return DISPATCH_QUEUE_PRIORITY_HIGH;
    case TaskQueueFactory::Priority::LOW:
      return DISPATCH_QUEUE_PRIORITY_LOW;
  }
}

class TaskQueueGcd final : public TaskQueueBase {
 public:
  TaskQueueGcd(absl::string_view queue_name, int gcd_priority);

  void Delete() override;
  void PostTask(absl::AnyInvocable<void() &&> task) override;
  void PostDelayedTask(absl::AnyInvocable<void() &&> task,
                       TimeDelta delay) override;
  void PostDelayedHighPrecisionTask(absl::AnyInvocable<void() &&> task,
                                    TimeDelta delay) override;

 private:
  struct TaskContext {
    TaskContext(TaskQueueGcd* queue, absl::AnyInvocable<void() &&> task)
        : queue(queue), task(std::move(task)) {}

    TaskQueueGcd* const queue;
    absl::AnyInvocable<void() &&> task;
  };

  ~TaskQueueGcd() override;
  static void RunTask(void* task_context);
  static void SetNotActive(void* task_queue);
  static void DeleteQueue(void* task_queue);

  dispatch_queue_t queue_;
  bool is_active_;
};

TaskQueueGcd::TaskQueueGcd(absl::string_view queue_name, int gcd_priority)
    : queue_(RTCDispatchQueueCreateWithTarget(
          std::string(queue_name).c_str(),
          DISPATCH_QUEUE_SERIAL,
          dispatch_get_global_queue(gcd_priority, 0))),
      is_active_(true) {
  RTC_CHECK(queue_);
  dispatch_set_context(queue_, this);


  dispatch_set_finalizer_f(queue_, &DeleteQueue);
}

TaskQueueGcd::~TaskQueueGcd() = default;

void TaskQueueGcd::Delete() {
  RTC_DCHECK(!IsCurrent());








  dispatch_sync_f(queue_, this, &SetNotActive);
  dispatch_release(queue_);
}

void TaskQueueGcd::PostTask(absl::AnyInvocable<void() &&> task) {
  auto* context = new TaskContext(this, std::move(task));
  dispatch_async_f(queue_, context, &RunTask);
}

void TaskQueueGcd::PostDelayedTask(absl::AnyInvocable<void() &&> task,
                                   TimeDelta delay) {
  auto* context = new TaskContext(this, std::move(task));
  dispatch_after_f(dispatch_time(DISPATCH_TIME_NOW, delay.us() * NSEC_PER_USEC),
                   queue_, context, &RunTask);
}

void TaskQueueGcd::PostDelayedHighPrecisionTask(
    absl::AnyInvocable<void() &&> task,
    TimeDelta delay) {
  PostDelayedTask(std::move(task), delay);
}

void TaskQueueGcd::RunTask(void* task_context) {
  std::unique_ptr<TaskContext> tc(static_cast<TaskContext*>(task_context));
  if (!tc->queue->is_active_)
    return;

  CurrentTaskQueueSetter set_current(tc->queue);
  std::move(tc->task)();


  tc = nullptr;
}

void TaskQueueGcd::SetNotActive(void* task_queue) {
  static_cast<TaskQueueGcd*>(task_queue)->is_active_ = false;
}

void TaskQueueGcd::DeleteQueue(void* task_queue) {
  delete static_cast<TaskQueueGcd*>(task_queue);
}

class TaskQueueGcdFactory final : public TaskQueueFactory {
 public:
  std::unique_ptr<TaskQueueBase, TaskQueueDeleter> CreateTaskQueue(
      absl::string_view name,
      Priority priority) const override {
    return std::unique_ptr<TaskQueueBase, TaskQueueDeleter>(
        new TaskQueueGcd(name, TaskQueuePriorityToGCD(priority)));
  }
};

}  // namespace

std::unique_ptr<TaskQueueFactory> CreateTaskQueueGcdFactory() {
  return std::make_unique<TaskQueueGcdFactory>();
}

}  // namespace webrtc
