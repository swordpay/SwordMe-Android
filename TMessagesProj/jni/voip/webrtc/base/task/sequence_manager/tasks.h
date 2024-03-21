// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_TASKS_H_
#define BASE_TASK_SEQUENCE_MANAGER_TASKS_H_

#include "base/pending_task.h"
#include "base/sequenced_task_runner.h"
#include "base/task/sequence_manager/enqueue_order.h"

namespace base {
namespace sequence_manager {

using TaskType = uint8_t;
constexpr TaskType kTaskTypeNone = 0;

namespace internal {

enum class WakeUpResolution { kLow, kHigh };

// Eventually it becomes a PendingTask once accepted by a TaskQueueImpl.
struct BASE_EXPORT PostedTask {
  explicit PostedTask(scoped_refptr<SequencedTaskRunner> task_runner,
                      OnceClosure callback = OnceClosure(),
                      Location location = Location(),
                      TimeDelta delay = TimeDelta(),
                      Nestable nestable = Nestable::kNestable,
                      TaskType task_type = kTaskTypeNone);
  PostedTask(PostedTask&& move_from) noexcept;
  ~PostedTask();

  OnceClosure callback;
  Location location;
  TimeDelta delay;
  Nestable nestable;
  TaskType task_type;


  scoped_refptr<SequencedTaskRunner> task_runner;

  TimeTicks queue_time;

  DISALLOW_COPY_AND_ASSIGN(PostedTask);
};

// same point in time will be ordered by their sequence numbers.
struct DelayedWakeUp {
  TimeTicks time;
  int sequence_num;

  bool operator!=(const DelayedWakeUp& other) const {
    return time != other.time || other.sequence_num != sequence_num;
  }

  bool operator==(const DelayedWakeUp& other) const {
    return !(*this != other);
  }

  bool operator<=(const DelayedWakeUp& other) const {
    if (time == other.time) {

      DCHECK(sequence_num != other.sequence_num || this == &other);


      return (sequence_num - other.sequence_num) <= 0;
    }
    return time < other.time;
  }
};

}  // namespace internal

struct BASE_EXPORT Task : public PendingTask {
  Task(internal::PostedTask posted_task,
       TimeTicks delayed_run_time,
       EnqueueOrder sequence_order,
       EnqueueOrder enqueue_order = EnqueueOrder(),
       internal::WakeUpResolution wake_up_resolution =
           internal::WakeUpResolution::kLow);
  Task(Task&& move_from);
  ~Task();
  Task& operator=(Task&& other);

  internal::DelayedWakeUp delayed_wake_up() const {
    return internal::DelayedWakeUp{delayed_run_time, sequence_num};
  }


  EnqueueOrder enqueue_order() const {
    DCHECK(enqueue_order_);
    return enqueue_order_;
  }

  void set_enqueue_order(EnqueueOrder enqueue_order) {
    DCHECK(!enqueue_order_);
    enqueue_order_ = enqueue_order;
  }

  bool enqueue_order_set() const { return enqueue_order_; }

  TaskType task_type;


  scoped_refptr<SequencedTaskRunner> task_runner;

#if DCHECK_IS_ON()
  bool cross_thread_;
#endif

 private:





  EnqueueOrder enqueue_order_;
};

}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_TASKS_H_
