// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_THREAD_GROUP_IMPL_H_
#define BASE_TASK_THREAD_POOL_THREAD_GROUP_IMPL_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/containers/stack.h"
#include "base/gtest_prod_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/waitable_event.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"
#include "base/task/thread_pool/thread_group.h"
#include "base/task/thread_pool/tracked_ref.h"
#include "base/task/thread_pool/worker_thread.h"
#include "base/task/thread_pool/worker_thread_stack.h"
#include "base/time/time.h"

namespace base {

class HistogramBase;
class WorkerThreadObserver;

namespace internal {

class TaskTracker;

//
// The thread group doesn't create threads until Start() is called. Tasks can be
// posted at any time but will not run until after Start() is called.
//
// This class is thread-safe.
class BASE_EXPORT ThreadGroupImpl : public ThreadGroup {
 public:








  ThreadGroupImpl(StringPiece histogram_label,
                  StringPiece thread_group_label,
                  ThreadPriority priority_hint,
                  TrackedRef<TaskTracker> task_tracker,
                  TrackedRef<Delegate> delegate);












  void Start(int max_tasks,
             int max_best_effort_tasks,
             TimeDelta suggested_reclaim_time,
             scoped_refptr<SequencedTaskRunner> service_thread_task_runner,
             WorkerThreadObserver* worker_thread_observer,
             WorkerEnvironment worker_environment,
             Optional<TimeDelta> may_block_threshold = Optional<TimeDelta>());



  ~ThreadGroupImpl() override;

  void JoinForTesting() override;
  size_t GetMaxConcurrentNonBlockedTasksDeprecated() const override;
  void ReportHeartbeatMetrics() const override;
  void DidUpdateCanRunPolicy() override;

  const HistogramBase* num_tasks_before_detach_histogram() const {
    return num_tasks_before_detach_histogram_;
  }






  void WaitForWorkersIdleForTesting(size_t n);

  void WaitForWorkersIdleLockRequiredForTesting(size_t n)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);

  void WaitForAllWorkersIdleForTesting();



  void WaitForWorkersCleanedUpForTesting(size_t n);

  size_t NumberOfWorkersForTesting() const;

  size_t GetMaxTasksForTesting() const;

  size_t NumberOfIdleWorkersForTesting() const;

 private:
  class ScopedCommandsExecutor;
  class WorkerThreadDelegateImpl;


  friend class ThreadGroupImplBlockingTest;
  friend class ThreadGroupImplMayBlockTest;
  FRIEND_TEST_ALL_PREFIXES(ThreadGroupImplBlockingTest,
                           ThreadBlockUnblockPremature);

  void UpdateSortKey(TaskSource::Transaction transaction) override;
  void PushTaskSourceAndWakeUpWorkers(
      TransactionWithRegisteredTaskSource transaction_with_task_source)
      override;
  void EnsureEnoughWorkersLockRequired(BaseScopedCommandsExecutor* executor)
      override EXCLUSIVE_LOCKS_REQUIRED(lock_);


  void MaintainAtLeastOneIdleWorkerLockRequired(
      ScopedCommandsExecutor* executor) EXCLUSIVE_LOCKS_REQUIRED(lock_);

  bool CanWorkerCleanupForTestingLockRequired() EXCLUSIVE_LOCKS_REQUIRED(lock_);


  scoped_refptr<WorkerThread> CreateAndRegisterWorkerLockRequired(
      ScopedCommandsExecutor* executor) EXCLUSIVE_LOCKS_REQUIRED(lock_);

  size_t GetNumAwakeWorkersLockRequired() const EXCLUSIVE_LOCKS_REQUIRED(lock_);


  size_t GetDesiredNumAwakeWorkersLockRequired() const
      EXCLUSIVE_LOCKS_REQUIRED(lock_);



  void AdjustMaxTasks();


  TimeDelta may_block_threshold_for_testing() const {
    return after_start().may_block_threshold;
  }



  TimeDelta blocked_workers_poll_period_for_testing() const {
    return after_start().blocked_workers_poll_period;
  }


  void ScheduleAdjustMaxTasks();

  void MaybeScheduleAdjustMaxTasksLockRequired(ScopedCommandsExecutor* executor)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);


  bool ShouldPeriodicallyAdjustMaxTasksLockRequired()
      EXCLUSIVE_LOCKS_REQUIRED(lock_);



  void UpdateMinAllowedPriorityLockRequired() EXCLUSIVE_LOCKS_REQUIRED(lock_);


  void DecrementTasksRunningLockRequired(TaskPriority priority)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);
  void IncrementTasksRunningLockRequired(TaskPriority priority)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);



  void DecrementMaxTasksLockRequired(TaskPriority priority)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);
  void IncrementMaxTasksLockRequired(TaskPriority priority)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);

  struct InitializedInStart {
    InitializedInStart();
    ~InitializedInStart();

#if DCHECK_IS_ON()

    bool initialized = false;
#endif

    size_t initial_max_tasks = 0;

    TimeDelta suggested_reclaim_time;

    WorkerEnvironment worker_environment = WorkerEnvironment::NONE;

    scoped_refptr<SequencedTaskRunner> service_thread_task_runner;

    WorkerThreadObserver* worker_thread_observer = nullptr;

    bool may_block_without_delay;
    bool fixed_max_best_effort_tasks;


    TimeDelta may_block_threshold;


    TimeDelta blocked_workers_poll_period;
  } initialized_in_start_;

  InitializedInStart& in_start() {
#if DCHECK_IS_ON()
    DCHECK(!initialized_in_start_.initialized);
#endif
    return initialized_in_start_;
  }
  const InitializedInStart& after_start() const {
#if DCHECK_IS_ON()
    DCHECK(initialized_in_start_.initialized);
#endif
    return initialized_in_start_;
  }

  const std::string thread_group_label_;
  const ThreadPriority priority_hint_;

  std::vector<scoped_refptr<WorkerThread>> workers_ GUARDED_BY(lock_);


  size_t max_tasks_ GUARDED_BY(lock_) = 0;
  size_t max_best_effort_tasks_ GUARDED_BY(lock_) = 0;


  size_t num_running_tasks_ GUARDED_BY(lock_) = 0;
  size_t num_running_best_effort_tasks_ GUARDED_BY(lock_) = 0;



  int num_unresolved_may_block_ GUARDED_BY(lock_) = 0;
  int num_unresolved_best_effort_may_block_ GUARDED_BY(lock_) = 0;





  WorkerThreadStack idle_workers_stack_ GUARDED_BY(lock_);

  std::unique_ptr<ConditionVariable> idle_workers_stack_cv_for_testing_
      GUARDED_BY(lock_);


  base::stack<TimeTicks, std::vector<TimeTicks>> cleanup_timestamps_
      GUARDED_BY(lock_);

  bool adjust_max_tasks_posted_ GUARDED_BY(lock_) = false;

  bool worker_cleanup_disallowed_for_testing_ GUARDED_BY(lock_) = false;







  size_t num_workers_cleaned_up_for_testing_ GUARDED_BY(lock_) = 0;
#if DCHECK_IS_ON()
  bool some_workers_cleaned_up_for_testing_ GUARDED_BY(lock_) = false;
#endif


  std::unique_ptr<ConditionVariable> num_workers_cleaned_up_for_testing_cv_
      GUARDED_BY(lock_);

  bool join_for_testing_started_ GUARDED_BY(lock_) = false;







  HistogramBase* const detach_duration_histogram_;


  HistogramBase* const num_tasks_before_detach_histogram_;


  HistogramBase* const num_workers_histogram_;


  HistogramBase* const num_active_workers_histogram_;







  TrackedRefFactory<ThreadGroupImpl> tracked_ref_factory_;

  DISALLOW_COPY_AND_ASSIGN(ThreadGroupImpl);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_THREAD_GROUP_IMPL_H_
