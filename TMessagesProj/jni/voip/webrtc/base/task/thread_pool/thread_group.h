// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_THREAD_GROUP_H_
#define BASE_TASK_THREAD_POOL_THREAD_GROUP_H_

#include "base/base_export.h"
#include "base/memory/ref_counted.h"
#include "base/task/common/checked_lock.h"
#include "base/task/thread_pool/priority_queue.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"
#include "base/task/thread_pool/tracked_ref.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/scoped_windows_thread_environment.h"
#endif

namespace base {
namespace internal {

class TaskTracker;

// subset of the threads in the thread pool (see GetThreadGroupForTraits() for
// thread group selection logic when posting tasks and creating task runners).
class BASE_EXPORT ThreadGroup {
 public:

  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() = default;



    virtual ThreadGroup* GetThreadGroupForTraits(const TaskTraits& traits) = 0;
  };

  enum class WorkerEnvironment {

    NONE,
#if defined(OS_WIN)

    COM_MTA,

    COM_STA,
#endif  // defined(OS_WIN)
  };

  virtual ~ThreadGroup();

  void BindToCurrentThread();

  void UnbindFromCurrentThread();

  bool IsBoundToCurrentThread() const;




  RegisteredTaskSource RemoveTaskSource(const TaskSource& task_source);





  virtual void UpdateSortKey(TaskSource::Transaction transaction) = 0;





  virtual void PushTaskSourceAndWakeUpWorkers(
      TransactionWithRegisteredTaskSource transaction_with_task_source) = 0;







  void InvalidateAndHandoffAllTaskSourcesToOtherThreadGroup(
      ThreadGroup* destination_thread_group);





  bool ShouldYield(TaskPriority priority) const;






  virtual void JoinForTesting() = 0;




  virtual size_t GetMaxConcurrentNonBlockedTasksDeprecated() const = 0;

  virtual void ReportHeartbeatMetrics() const = 0;


  virtual void DidUpdateCanRunPolicy() = 0;

 protected:



  class BaseScopedCommandsExecutor {
   public:
    void ScheduleReleaseTaskSource(RegisteredTaskSource task_source);

   protected:
    BaseScopedCommandsExecutor();
    ~BaseScopedCommandsExecutor();

   private:
    std::vector<RegisteredTaskSource> task_sources_to_release_;

    DISALLOW_COPY_AND_ASSIGN(BaseScopedCommandsExecutor);
  };


  class ScopedReenqueueExecutor {
   public:
    ScopedReenqueueExecutor();
    ~ScopedReenqueueExecutor();


    void SchedulePushTaskSourceAndWakeUpWorkers(
        TransactionWithRegisteredTaskSource transaction_with_task_source,
        ThreadGroup* destination_thread_group);

   private:


    Optional<TransactionWithRegisteredTaskSource> transaction_with_task_source_;
    ThreadGroup* destination_thread_group_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(ScopedReenqueueExecutor);
  };







  ThreadGroup(TrackedRef<TaskTracker> task_tracker,
              TrackedRef<Delegate> delegate,
              ThreadGroup* predecessor_thread_group = nullptr);

#if defined(OS_WIN)
  static std::unique_ptr<win::ScopedWindowsThreadEnvironment>
  GetScopedWindowsThreadEnvironment(WorkerEnvironment environment);
#endif

  const TrackedRef<TaskTracker> task_tracker_;
  const TrackedRef<Delegate> delegate_;


  size_t GetNumAdditionalWorkersForBestEffortTaskSourcesLockRequired() const
      EXCLUSIVE_LOCKS_REQUIRED(lock_);



  size_t GetNumAdditionalWorkersForForegroundTaskSourcesLockRequired() const
      EXCLUSIVE_LOCKS_REQUIRED(lock_);



  virtual void EnsureEnoughWorkersLockRequired(
      BaseScopedCommandsExecutor* executor) EXCLUSIVE_LOCKS_REQUIRED(lock_) = 0;


  void ReEnqueueTaskSourceLockRequired(
      BaseScopedCommandsExecutor* workers_executor,
      ScopedReenqueueExecutor* reenqueue_executor,
      TransactionWithRegisteredTaskSource transaction_with_task_source)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);




  RegisteredTaskSource TakeRegisteredTaskSource(
      BaseScopedCommandsExecutor* executor) EXCLUSIVE_LOCKS_REQUIRED(lock_);

  void UpdateSortKeyImpl(BaseScopedCommandsExecutor* executor,
                         TaskSource::Transaction transaction);
  void PushTaskSourceAndWakeUpWorkersImpl(
      BaseScopedCommandsExecutor* executor,
      TransactionWithRegisteredTaskSource transaction_with_task_source);




  mutable CheckedLock lock_;

  PriorityQueue priority_queue_ GUARDED_BY(lock_);





  std::atomic<TaskPriority> min_allowed_priority_ GUARDED_BY(lock_){
      TaskPriority::BEST_EFFORT};



  ThreadGroup* replacement_thread_group_ = nullptr;

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadGroup);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_THREAD_GROUP_H_
