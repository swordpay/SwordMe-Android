// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_TIME_DOMAIN_H_
#define BASE_TASK_SEQUENCE_MANAGER_TIME_DOMAIN_H_

#include <map>

#include "base/callback.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/sequence_manager/lazy_now.h"
#include "base/task/sequence_manager/task_queue_impl.h"
#include "base/time/time.h"

namespace base {
namespace sequence_manager {

class SequenceManager;

namespace internal {
class AssociatedThreadId;
class SequenceManagerImpl;
class TaskQueueImpl;
}  // namespace internal

// This class allows overrides to enable clock overriding on some TaskQueues
// (e.g. auto-advancing virtual time, throttled clock, etc).
//
// TaskQueue maintains its own next wake-up time and communicates it
// to the TimeDomain, which aggregates wake-ups across registered TaskQueues
// into a global wake-up, which ultimately gets passed to the ThreadController.
class BASE_EXPORT TimeDomain {
 public:
  virtual ~TimeDomain();



  virtual LazyNow CreateLazyNow() const = 0;



  virtual TimeTicks Now() const = 0;







  virtual Optional<TimeDelta> DelayTillNextTask(LazyNow* lazy_now) = 0;

  void AsValueInto(trace_event::TracedValue* state) const;
  bool HasPendingHighResolutionTasks() const;

  bool Empty() const;



  virtual bool MaybeFastForwardToNextTask(bool quit_when_idle_requested) = 0;

 protected:
  TimeDomain();

  SequenceManager* sequence_manager() const;

  Optional<TimeTicks> NextScheduledRunTime() const;

  size_t NumberOfScheduledWakeUps() const {
    return delayed_wake_up_queue_.size();
  }



  virtual void SetNextDelayedDoWork(LazyNow* lazy_now, TimeTicks run_time);


  virtual void RequestDoWork();

  virtual void AsValueIntoInternal(trace_event::TracedValue* state) const;

  virtual const char* GetName() const = 0;



  virtual void OnRegisterWithSequenceManager(
      internal::SequenceManagerImpl* sequence_manager);

 private:
  friend class internal::TaskQueueImpl;
  friend class internal::SequenceManagerImpl;
  friend class TestTimeDomain;




  void SetNextWakeUpForQueue(internal::TaskQueueImpl* queue,
                             Optional<internal::DelayedWakeUp> wake_up,
                             internal::WakeUpResolution resolution,
                             LazyNow* lazy_now);

  void UnregisterQueue(internal::TaskQueueImpl* queue);


  void MoveReadyDelayedTasksToWorkQueues(LazyNow* lazy_now);

  struct ScheduledDelayedWakeUp {
    internal::DelayedWakeUp wake_up;
    internal::WakeUpResolution resolution;
    internal::TaskQueueImpl* queue;

    bool operator<=(const ScheduledDelayedWakeUp& other) const {
      if (wake_up == other.wake_up) {
        return static_cast<int>(resolution) <=
               static_cast<int>(other.resolution);
      }
      return wake_up <= other.wake_up;
    }

    void SetHeapHandle(base::internal::HeapHandle handle) {
      DCHECK(handle.IsValid());
      queue->set_heap_handle(handle);
    }

    void ClearHeapHandle() {
      DCHECK(queue->heap_handle().IsValid());
      queue->set_heap_handle(base::internal::HeapHandle());
    }

    HeapHandle GetHeapHandle() const { return queue->heap_handle(); }
  };

  internal::SequenceManagerImpl* sequence_manager_;  // Not owned.
  base::internal::IntrusiveHeap<ScheduledDelayedWakeUp> delayed_wake_up_queue_;
  int pending_high_res_wake_up_count_ = 0;

  scoped_refptr<internal::AssociatedThreadId> associated_thread_;
  DISALLOW_COPY_AND_ASSIGN(TimeDomain);
};

}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_TIME_DOMAIN_H_
