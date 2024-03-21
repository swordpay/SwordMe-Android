// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_JOB_TASK_SOURCE_H_
#define BASE_TASK_THREAD_POOL_JOB_TASK_SOURCE_H_

#include <stddef.h>

#include <atomic>
#include <limits>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/task/post_job.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/sequence_sort_key.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"

namespace base {
namespace internal {

class PooledTaskRunnerDelegate;

//
// Derived classes control the intended concurrency with GetMaxConcurrency().
class BASE_EXPORT JobTaskSource : public TaskSource {
 public:
  JobTaskSource(const Location& from_here,
                const TaskTraits& traits,
                RepeatingCallback<void(JobDelegate*)> worker_task,
                RepeatingCallback<size_t()> max_concurrency_callback,
                PooledTaskRunnerDelegate* delegate);

  static JobHandle CreateJobHandle(
      scoped_refptr<internal::JobTaskSource> task_source) {
    return JobHandle(std::move(task_source));
  }


  void NotifyConcurrencyIncrease();





  bool WillJoin();





  bool RunJoinTask();


  void Cancel(TaskSource::Transaction* transaction = nullptr);

  ExecutionEnvironment GetExecutionEnvironment() override;
  size_t GetRemainingConcurrency() const override;


  size_t GetMaxConcurrency() const;


  bool ShouldYield();

  PooledTaskRunnerDelegate* delegate() const { return delegate_; }

#if DCHECK_IS_ON()
  size_t GetConcurrencyIncreaseVersion() const;


  bool WaitForConcurrencyIncreaseUpdate(size_t recorded_version);
#endif  // DCHECK_IS_ON()

 private:


  class State {
   public:
    static constexpr size_t kCanceledMask = 1;
    static constexpr size_t kWorkerCountBitOffset = 1;
    static constexpr size_t kWorkerCountIncrement = 1 << kWorkerCountBitOffset;

    struct Value {
      size_t worker_count() const { return value >> kWorkerCountBitOffset; }

      bool is_canceled() const { return value & kCanceledMask; }

      uint32_t value;
    };

    State();
    ~State();


    Value Cancel();



    Value TryIncrementWorkerCountFromWorkerRelease(size_t max_concurrency);


    Value DecrementWorkerCountFromWorkerAcquire();


    Value IncrementWorkerCountFromJoiningThread();


    Value DecrementWorkerCountFromJoiningThread();

    Value Load() const;

   private:
    std::atomic<uint32_t> value_{0};
  };


  class JoinFlag {
   public:
    static constexpr uint32_t kNotWaiting = 0;
    static constexpr uint32_t kWaitingForWorkerToSignal = 1;
    static constexpr uint32_t kWaitingForWorkerToYield = 3;


    static_assert((kWaitingForWorkerToYield & kWaitingForWorkerToSignal) ==
                      kWaitingForWorkerToSignal,
                  "");

    JoinFlag();
    ~JoinFlag();


    void SetWaiting();




    bool ShouldWorkerYield();




    bool ShouldWorkerSignal();

   private:
    std::atomic<uint32_t> value_{kNotWaiting};
  };

  ~JobTaskSource() override;





  bool WaitForParticipationOpportunity();

  RunStatus WillRunTask() override;
  Task TakeTask(TaskSource::Transaction* transaction) override;
  Task Clear(TaskSource::Transaction* transaction) override;
  bool DidProcessTask(TaskSource::Transaction* transaction) override;
  SequenceSortKey GetSortKey() const override;

  State state_;


  JoinFlag join_flag_ GUARDED_BY(lock_);

  std::unique_ptr<ConditionVariable> worker_released_condition_
      GUARDED_BY(lock_);

  const Location from_here_;
  RepeatingCallback<size_t()> max_concurrency_callback_;

  RepeatingCallback<void(JobDelegate*)> worker_task_;

  RepeatingClosure primary_task_;

  const TimeTicks queue_time_;
  PooledTaskRunnerDelegate* delegate_;

#if DCHECK_IS_ON()

  mutable Lock version_lock_;

  ConditionVariable version_condition_{&version_lock_};

  size_t increase_version_ GUARDED_BY(version_lock_) = 0;
#endif  // DCHECK_IS_ON()

  DISALLOW_COPY_AND_ASSIGN(JobTaskSource);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_JOB_TASK_SOURCE_H_
