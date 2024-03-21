// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_THREAD_GROUP_NATIVE_H_
#define BASE_TASK_THREAD_POOL_THREAD_GROUP_NATIVE_H_

#include "base/base_export.h"
#include "base/synchronization/atomic_flag.h"
#include "base/task/thread_pool/thread_group.h"

namespace base {
namespace internal {

class BASE_EXPORT ThreadGroupNative : public ThreadGroup {
 public:



  ~ThreadGroupNative() override;

  void Start(WorkerEnvironment worker_environment = WorkerEnvironment::NONE);

  void JoinForTesting() override;
  size_t GetMaxConcurrentNonBlockedTasksDeprecated() const override;
  void ReportHeartbeatMetrics() const override;
  void DidUpdateCanRunPolicy() override;

 protected:
  ThreadGroupNative(TrackedRef<TaskTracker> task_tracker,
                    TrackedRef<Delegate> delegate,
                    ThreadGroup* predecessor_thread_group);


  void RunNextTaskSourceImpl();

  virtual void JoinImpl() = 0;
  virtual void StartImpl() = 0;
  virtual void SubmitWork() = 0;

  WorkerEnvironment worker_environment_ = WorkerEnvironment::NONE;

 private:
  class ScopedCommandsExecutor;

  void UpdateSortKey(TaskSource::Transaction transaction) override;
  void PushTaskSourceAndWakeUpWorkers(
      TransactionWithRegisteredTaskSource transaction_with_task_source)
      override;
  void EnsureEnoughWorkersLockRequired(BaseScopedCommandsExecutor* executor)
      override EXCLUSIVE_LOCKS_REQUIRED(lock_);


  void UpdateMinAllowedPriorityLockRequired() EXCLUSIVE_LOCKS_REQUIRED(lock_);


  RegisteredTaskSource GetWork();

  bool started_ GUARDED_BY(lock_) = false;


  size_t num_pending_threadpool_work_ GUARDED_BY(lock_) = 0;

#if DCHECK_IS_ON()

  bool join_for_testing_returned_ = false;
#endif

  DISALLOW_COPY_AND_ASSIGN(ThreadGroupNative);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_THREAD_GROUP_NATIVE_H_
