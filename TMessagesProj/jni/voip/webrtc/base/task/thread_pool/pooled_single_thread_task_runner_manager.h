// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_POOLED_SINGLE_THREAD_TASK_RUNNER_MANAGER_H_
#define BASE_TASK_THREAD_POOL_POOLED_SINGLE_THREAD_TASK_RUNNER_MANAGER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/task/common/checked_lock.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task/thread_pool/environment_config.h"
#include "base/task/thread_pool/tracked_ref.h"
#include "base/thread_annotations.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"

namespace base {

class TaskTraits;
class WorkerThreadObserver;
class SingleThreadTaskRunner;

namespace internal {

class DelayedTaskManager;
class WorkerThread;
class TaskTracker;

namespace {

class WorkerThreadDelegate;

}  // namespace

// SingleThreadTaskRunners.
//
// SingleThreadTaskRunners using SingleThreadTaskRunnerThreadMode::SHARED are
// backed by shared WorkerThreads for each COM+task environment combination.
// These workers are lazily instantiated and then only reclaimed during
// JoinForTesting()
//
// No threads are created (and hence no tasks can run) before Start() is called.
//
// This class is thread-safe.
class BASE_EXPORT PooledSingleThreadTaskRunnerManager final {
 public:
  PooledSingleThreadTaskRunnerManager(TrackedRef<TaskTracker> task_tracker,
                                      DelayedTaskManager* delayed_task_manager);
  ~PooledSingleThreadTaskRunnerManager();





  void Start(WorkerThreadObserver* worker_thread_observer = nullptr);


  void DidUpdateCanRunPolicy();




  scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode);

#if defined(OS_WIN)




  scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode);
#endif  // defined(OS_WIN)

  void JoinForTesting();

 private:
  class PooledSingleThreadTaskRunner;

  enum ContinueOnShutdown {
    IS_CONTINUE_ON_SHUTDOWN,
    IS_NOT_CONTINUE_ON_SHUTDOWN,
    CONTINUE_ON_SHUTDOWN_COUNT,
  };

  static ContinueOnShutdown TraitsToContinueOnShutdown(
      const TaskTraits& traits);

  template <typename DelegateType>
  scoped_refptr<PooledSingleThreadTaskRunner> CreateTaskRunnerImpl(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode);

  template <typename DelegateType>
  std::unique_ptr<WorkerThreadDelegate> CreateWorkerThreadDelegate(
      const std::string& name,
      int id,
      SingleThreadTaskRunnerThreadMode thread_mode);

  template <typename DelegateType>
  WorkerThread* CreateAndRegisterWorkerThread(
      const std::string& name,
      SingleThreadTaskRunnerThreadMode thread_mode,
      ThreadPriority priority_hint) EXCLUSIVE_LOCKS_REQUIRED(lock_);

  template <typename DelegateType>
  WorkerThread*& GetSharedWorkerThreadForTraits(const TaskTraits& traits);

  void UnregisterWorkerThread(WorkerThread* worker);

  void ReleaseSharedWorkerThreads();

  const TrackedRef<TaskTracker> task_tracker_;
  DelayedTaskManager* const delayed_task_manager_;


  WorkerThreadObserver* worker_thread_observer_ = nullptr;

  CheckedLock lock_;
  std::vector<scoped_refptr<WorkerThread>> workers_ GUARDED_BY(lock_);
  int next_worker_id_ GUARDED_BY(lock_) = 0;





  WorkerThread* shared_worker_threads_[ENVIRONMENT_COUNT]
                                      [CONTINUE_ON_SHUTDOWN_COUNT] GUARDED_BY(
                                          lock_) = {};
#if defined(OS_WIN)
  WorkerThread* shared_com_worker_threads_
      [ENVIRONMENT_COUNT][CONTINUE_ON_SHUTDOWN_COUNT] GUARDED_BY(lock_) = {};
#endif  // defined(OS_WIN)

  bool started_ GUARDED_BY(lock_) = false;

  DISALLOW_COPY_AND_ASSIGN(PooledSingleThreadTaskRunnerManager);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_POOLED_SINGLE_THREAD_TASK_RUNNER_MANAGER_H_
