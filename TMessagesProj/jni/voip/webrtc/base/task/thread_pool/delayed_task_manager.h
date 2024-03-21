// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_DELAYED_TASK_MANAGER_H_
#define BASE_TASK_THREAD_POOL_DELAYED_TASK_MANAGER_H_

#include <memory>
#include <utility>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "base/synchronization/atomic_flag.h"
#include "base/task/common/checked_lock.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/thread_pool/task.h"
#include "base/thread_annotations.h"
#include "base/time/default_tick_clock.h"
#include "base/time/tick_clock.h"

namespace base {

class SequencedTaskRunner;

namespace internal {

// ripe for execution. Tasks are not forwarded before Start() is called. This
// class is thread-safe.
class BASE_EXPORT DelayedTaskManager {
 public:

  using PostTaskNowCallback = OnceCallback<void(Task task)>;

  DelayedTaskManager(
      const TickClock* tick_clock = DefaultTickClock::GetInstance());
  ~DelayedTaskManager();




  void Start(scoped_refptr<SequencedTaskRunner> service_thread_task_runner);



  void AddDelayedTask(Task task,
                      PostTaskNowCallback post_task_now_callback,
                      scoped_refptr<TaskRunner> task_runner);

  void ProcessRipeTasks();

  Optional<TimeTicks> NextScheduledRunTime() const;

 private:
  struct DelayedTask {
    DelayedTask();
    DelayedTask(Task task,
                PostTaskNowCallback callback,
                scoped_refptr<TaskRunner> task_runner);
    DelayedTask(DelayedTask&& other);
    ~DelayedTask();

    DelayedTask& operator=(DelayedTask&& other);

    bool operator<=(const DelayedTask& other) const;

    Task task;
    PostTaskNowCallback callback;
    scoped_refptr<TaskRunner> task_runner;

    bool IsScheduled() const;


    void SetScheduled();

    void SetHeapHandle(const HeapHandle& handle) {}

    void ClearHeapHandle() {}

    HeapHandle GetHeapHandle() const { return HeapHandle::Invalid(); }

   private:
    bool scheduled_ = false;
    DISALLOW_COPY_AND_ASSIGN(DelayedTask);
  };



  TimeTicks GetTimeToScheduleProcessRipeTasksLockRequired()
      EXCLUSIVE_LOCKS_REQUIRED(queue_lock_);



  void ScheduleProcessRipeTasksOnServiceThread(
      TimeTicks process_ripe_tasks_time);

  const RepeatingClosure process_ripe_tasks_closure_;

  const TickClock* const tick_clock_;





  mutable CheckedLock queue_lock_;

  scoped_refptr<SequencedTaskRunner> service_thread_task_runner_;

  IntrusiveHeap<DelayedTask> delayed_task_queue_ GUARDED_BY(queue_lock_);

  DISALLOW_COPY_AND_ASSIGN(DelayedTaskManager);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_DELAYED_TASK_MANAGER_H_
