// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_TASK_QUEUE_H_
#define BASE_TASK_SEQUENCE_MANAGER_TASK_QUEUE_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/single_thread_task_runner.h"
#include "base/task/common/checked_lock.h"
#include "base/task/sequence_manager/lazy_now.h"
#include "base/task/sequence_manager/tasks.h"
#include "base/task/task_observer.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

namespace base {

class TaskObserver;

namespace trace_event {
class BlameContext;
}

namespace sequence_manager {

namespace internal {
class AssociatedThreadId;
class SequenceManagerImpl;
class TaskQueueImpl;
}  // namespace internal

class TimeDomain;

// and stop using ref-counting because we're no longer tied to task runner
// lifecycle and there's no other need for ref-counting either.
// NOTE: When TaskQueue gets automatically deleted on zero ref-count,
// TaskQueueImpl gets gracefully shutdown. It means that it doesn't get
// unregistered immediately and might accept some last minute tasks until
// SequenceManager will unregister it at some point. It's done to ensure that
// task queue always gets unregistered on the main thread.
class BASE_EXPORT TaskQueue : public RefCountedThreadSafe<TaskQueue> {
 public:
  class Observer {
   public:
    virtual ~Observer() = default;









    virtual void OnQueueNextWakeUpChanged(TimeTicks next_wake_up) = 0;
  };

  virtual void ShutdownTaskQueue();

  void ShutdownTaskQueueGracefully();


  enum QueuePriority : uint8_t {



    kControlPriority = 0,





    kHighestPriority = 1,

    kVeryHighPriority = 2,

    kHighPriority = 3,

    kNormalPriority = 4,
    kLowPriority = 5,


    kBestEffortPriority = 6,

    kQueuePriorityCount = 7,
    kFirstQueuePriority = kControlPriority,
  };

  static const char* PriorityToString(QueuePriority priority);

  struct Spec {
    explicit Spec(const char* name) : name(name) {}

    Spec SetShouldMonitorQuiescence(bool should_monitor) {
      should_monitor_quiescence = should_monitor;
      return *this;
    }

    Spec SetShouldNotifyObservers(bool run_observers) {
      should_notify_observers = run_observers;
      return *this;
    }


    Spec SetDelayedFencesAllowed(bool allow_delayed_fences) {
      delayed_fence_allowed = allow_delayed_fences;
      return *this;
    }

    Spec SetTimeDomain(TimeDomain* domain) {
      time_domain = domain;
      return *this;
    }

    const char* name;
    bool should_monitor_quiescence = false;
    TimeDomain* time_domain = nullptr;
    bool should_notify_observers = true;
    bool delayed_fence_allowed = false;
  };

  TaskQueue(std::unique_ptr<internal::TaskQueueImpl> impl,
            const TaskQueue::Spec& spec);









  class BASE_EXPORT TaskTiming {
   public:
    enum class State { NotStarted, Running, Finished };
    enum class TimeRecordingPolicy { DoRecord, DoNotRecord };

    TaskTiming(bool has_wall_time, bool has_thread_time);

    bool has_wall_time() const { return has_wall_time_; }
    bool has_thread_time() const { return has_thread_time_; }

    base::TimeTicks start_time() const {
      DCHECK(has_wall_time());
      return start_time_;
    }
    base::TimeTicks end_time() const {
      DCHECK(has_wall_time());
      return end_time_;
    }
    base::TimeDelta wall_duration() const {
      DCHECK(has_wall_time());
      return end_time_ - start_time_;
    }
    base::ThreadTicks start_thread_time() const {
      DCHECK(has_thread_time());
      return start_thread_time_;
    }
    base::ThreadTicks end_thread_time() const {
      DCHECK(has_thread_time());
      return end_thread_time_;
    }
    base::TimeDelta thread_duration() const {
      DCHECK(has_thread_time());
      return end_thread_time_ - start_thread_time_;
    }

    State state() const { return state_; }

    void RecordTaskStart(LazyNow* now);
    void RecordTaskEnd(LazyNow* now);

   protected:
    State state_ = State::NotStarted;

    bool has_wall_time_;
    bool has_thread_time_;

    base::TimeTicks start_time_;
    base::TimeTicks end_time_;
    base::ThreadTicks start_thread_time_;
    base::ThreadTicks end_thread_time_;
  };


  class BASE_EXPORT QueueEnabledVoter {
   public:
    ~QueueEnabledVoter();

    QueueEnabledVoter(const QueueEnabledVoter&) = delete;
    const QueueEnabledVoter& operator=(const QueueEnabledVoter&) = delete;





    void SetVoteToEnable(bool enabled);

    bool IsVotingToEnable() const { return enabled_; }

   private:
    friend class TaskQueue;
    explicit QueueEnabledVoter(scoped_refptr<TaskQueue> task_queue);

    scoped_refptr<TaskQueue> const task_queue_;
    bool enabled_;
  };




  std::unique_ptr<QueueEnabledVoter> CreateQueueEnabledVoter();

  bool IsQueueEnabled() const;

  bool IsEmpty() const;

  size_t GetNumberOfPendingTasks() const;


  bool HasTaskToRunImmediately() const;




  Optional<TimeTicks> GetNextScheduledWakeUp();

  virtual const char* GetName() const;


  void SetQueuePriority(QueuePriority priority);

  QueuePriority GetQueuePriority() const;


  void AddTaskObserver(TaskObserver* task_observer);
  void RemoveTaskObserver(TaskObserver* task_observer);



  void SetBlameContext(trace_event::BlameContext* blame_context);


  void SetTimeDomain(TimeDomain* domain);

  TimeDomain* GetTimeDomain() const;

  enum class InsertFencePosition {
    kNow,  // Tasks posted on the queue up till this point further may run.

    kBeginningOfTime,  // No tasks posted on this queue may run.
  };
















  void InsertFence(InsertFencePosition position);



  void InsertFenceAt(TimeTicks time);


  void RemoveFence();



  bool HasActiveFence();

  bool BlockedByFence() const;




  EnqueueOrder GetEnqueueOrderAtWhichWeBecameUnblocked() const;

  void SetObserver(Observer* observer);




  void SetShouldReportPostedTasksWhenDisabled(bool should_report);






  scoped_refptr<SingleThreadTaskRunner> CreateTaskRunner(TaskType task_type);

  scoped_refptr<SingleThreadTaskRunner> task_runner() const {
    return default_task_runner_;
  }

 protected:
  virtual ~TaskQueue();

  internal::TaskQueueImpl* GetTaskQueueImpl() const { return impl_.get(); }

 private:
  friend class RefCountedThreadSafe<TaskQueue>;
  friend class internal::SequenceManagerImpl;
  friend class internal::TaskQueueImpl;

  void AddQueueEnabledVoter(bool voter_is_enabled);
  void RemoveQueueEnabledVoter(bool voter_is_enabled);
  bool AreAllQueueEnabledVotersEnabled() const;
  void OnQueueEnabledVoteChanged(bool enabled);

  bool IsOnMainThread() const;




  std::unique_ptr<internal::TaskQueueImpl> TakeTaskQueueImpl();





  mutable base::internal::CheckedLock impl_lock_{
      base::internal::UniversalPredecessor{}};
  std::unique_ptr<internal::TaskQueueImpl> impl_;

  const WeakPtr<internal::SequenceManagerImpl> sequence_manager_;

  scoped_refptr<internal::AssociatedThreadId> associated_thread_;
  scoped_refptr<SingleThreadTaskRunner> default_task_runner_;

  int enabled_voter_count_ = 0;
  int voter_count_ = 0;
  const char* name_;

  DISALLOW_COPY_AND_ASSIGN(TaskQueue);
};

}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_TASK_QUEUE_H_
