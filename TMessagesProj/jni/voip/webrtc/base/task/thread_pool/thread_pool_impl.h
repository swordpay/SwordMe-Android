// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_THREAD_POOL_IMPL_H_
#define BASE_TASK_THREAD_POOL_THREAD_POOL_IMPL_H_

#include <memory>
#include <vector>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "base/sequence_checker.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/atomic_flag.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task/task_executor.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/delayed_task_manager.h"
#include "base/task/thread_pool/environment_config.h"
#include "base/task/thread_pool/pooled_single_thread_task_runner_manager.h"
#include "base/task/thread_pool/pooled_task_runner_delegate.h"
#include "base/task/thread_pool/task_source.h"
#include "base/task/thread_pool/task_tracker.h"
#include "base/task/thread_pool/thread_group.h"
#include "base/task/thread_pool/thread_group_impl.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/updateable_sequenced_task_runner.h"
#include "build/build_config.h"

#if defined(OS_POSIX) && !defined(OS_NACL_SFI)
#include "base/task/thread_pool/task_tracker_posix.h"
#endif

#if defined(OS_WIN)
#include "base/win/com_init_check_hook.h"
#endif

namespace base {

class Thread;

namespace internal {

class BASE_EXPORT ThreadPoolImpl : public ThreadPoolInstance,
                                   public TaskExecutor,
                                   public ThreadGroup::Delegate,
                                   public PooledTaskRunnerDelegate {
 public:
  using TaskTrackerImpl =
#if defined(OS_POSIX) && !defined(OS_NACL_SFI)
      TaskTrackerPosix;
#else
      TaskTracker;
#endif


  explicit ThreadPoolImpl(StringPiece histogram_label);

  ThreadPoolImpl(StringPiece histogram_label,
                 std::unique_ptr<TaskTrackerImpl> task_tracker);

  ~ThreadPoolImpl() override;

  void Start(const ThreadPoolInstance::InitParams& init_params,
             WorkerThreadObserver* worker_thread_observer) override;
  int GetMaxConcurrentNonBlockedTasksWithTraitsDeprecated(
      const TaskTraits& traits) const override;
  void Shutdown() override;
  void FlushForTesting() override;
  void FlushAsyncForTesting(OnceClosure flush_callback) override;
  void JoinForTesting() override;
  void BeginFence() override;
  void EndFence() override;
  void BeginBestEffortFence() override;
  void EndBestEffortFence() override;

  bool PostDelayedTask(const Location& from_here,
                       const TaskTraits& traits,
                       OnceClosure task,
                       TimeDelta delay) override;
  scoped_refptr<TaskRunner> CreateTaskRunner(const TaskTraits& traits) override;
  scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits) override;
  scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;
#if defined(OS_WIN)
  scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;
#endif  // defined(OS_WIN)
  scoped_refptr<UpdateableSequencedTaskRunner>
  CreateUpdateableSequencedTaskRunner(const TaskTraits& traits);

  bool EnqueueJobTaskSource(scoped_refptr<JobTaskSource> task_source) override;
  void RemoveJobTaskSource(scoped_refptr<JobTaskSource> task_source) override;
  void UpdatePriority(scoped_refptr<TaskSource> task_source,
                      TaskPriority priority) override;




  Optional<TimeTicks> NextScheduledRunTimeForTesting() const;


  void ProcessRipeDelayedTasksForTesting();

 private:


  void UpdateCanRunPolicy();



  TaskTraits VerifyAndAjustIncomingTraits(TaskTraits traits) const;

  void ReportHeartbeatMetrics() const;

  const ThreadGroup* GetThreadGroupForTraits(const TaskTraits& traits) const;

  ThreadGroup* GetThreadGroupForTraits(const TaskTraits& traits) override;



  bool PostTaskWithSequenceNow(Task task, scoped_refptr<Sequence> sequence);

  bool PostTaskWithSequence(Task task,
                            scoped_refptr<Sequence> sequence) override;
  bool ShouldYield(const TaskSource* task_source) const override;

  const std::unique_ptr<TaskTrackerImpl> task_tracker_;
  std::unique_ptr<Thread> service_thread_;
  DelayedTaskManager delayed_task_manager_;
  PooledSingleThreadTaskRunnerManager single_thread_task_runner_manager_;






  AtomicFlag all_tasks_user_blocking_;

  std::unique_ptr<ThreadGroup> foreground_thread_group_;
  std::unique_ptr<ThreadGroupImpl> background_thread_group_;


  bool started_ = false;


  const bool has_disable_best_effort_switch_;


  int num_fences_ = 0;
  int num_best_effort_fences_ = 0;

#if DCHECK_IS_ON()

  AtomicFlag join_for_testing_returned_;
#endif

#if defined(OS_WIN) && defined(COM_INIT_CHECK_HOOK_ENABLED)

  base::win::ComInitCheckHook com_init_check_hook_;
#endif

  SEQUENCE_CHECKER(sequence_checker_);

  TrackedRefFactory<ThreadGroup::Delegate> tracked_ref_factory_;

  DISALLOW_COPY_AND_ASSIGN(ThreadPoolImpl);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_THREAD_POOL_IMPL_H_
