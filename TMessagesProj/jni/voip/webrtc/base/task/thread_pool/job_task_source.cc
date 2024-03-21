// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/job_task_source.h"

#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/task/task_features.h"
#include "base/task/thread_pool/pooled_task_runner_delegate.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"
#include "base/time/time_override.h"

namespace base {
namespace internal {

//
// The write operation on |state_| in WillRunTask() uses
// std::memory_order_release, matched by std::memory_order_acquire on read
// operations (in DidProcessTask()) to establish a
// Release-Acquire ordering. When a call to WillRunTask() is caused by an
// increase of max concurrency followed by an associated
// NotifyConcurrencyIncrease(), the priority queue lock guarantees an
// happens-after relation with NotifyConcurrencyIncrease(). This ensures that an
// increase of max concurrency that happened-before NotifyConcurrencyIncrease()
// is visible to a read operation that happens-after WillRunTask().
//
// In DidProcessTask(), this is necessary to
// ensure that the task source is always re-enqueued when it needs to. When the
// task source needs to be queued, either because the current task yielded or
// because of NotifyConcurrencyIncrease(), one of the following is true:
//   A) DidProcessTask() happens-after WillRunTask():
//    T1: Current task returns (because it is done) or yields.
//    T2: Increases the value returned by GetMaxConcurrency()
//        NotifyConcurrencyIncrease() enqueues the task source
//    T3: WillRunTask(), in response to the concurrency increase - Release
//        Does not keep the TaskSource in PriorityQueue because it is at max
//        concurrency
//    T1: DidProcessTask() - Acquire - Because of memory barrier, sees the same
//        (or newer) max concurrency as T2
//        Re-enqueues the TaskSource because no longer at max concurrency
//      Without the memory barrier, T1 may see an outdated max concurrency that
//      is lower than the actual max concurrency and won't re-enqueue the
//      task source, because it thinks it's already saturated.
//      The task source often needs to be re-enqueued if its task
//      completed because it yielded and |max_concurrency| wasn't decreased.
//   B) DidProcessTask() happens-before WillRunTask():
//    T1: Current task returns (because it is done) or yields
//    T2: Increases the value returned by GetMaxConcurrency()
//        NotifyConcurrencyIncrease() enqueues the task source
//    T1: DidProcessTask() - Acquire (ineffective)
//      Since the task source is already in the queue, it doesn't matter
//      whether T1 re-enqueues the task source or not.
// Note that stale values the other way around can cause incorrectly
// re-enqueuing this task_source, which is not an issue because the queues
// support empty task sources.

JobTaskSource::State::State() = default;
JobTaskSource::State::~State() = default;

JobTaskSource::State::Value JobTaskSource::State::Cancel() {
  return {value_.fetch_or(kCanceledMask, std::memory_order_relaxed)};
}

JobTaskSource::State::Value
JobTaskSource::State::TryIncrementWorkerCountFromWorkerRelease(
    size_t max_concurrency) {
  uint32_t value_before_add = value_.load(std::memory_order_relaxed);



  while (!(value_before_add & kCanceledMask) &&
         (value_before_add >> kWorkerCountBitOffset) < max_concurrency &&
         !value_.compare_exchange_weak(
             value_before_add, value_before_add + kWorkerCountIncrement,
             std::memory_order_release, std::memory_order_relaxed)) {
  }
  return {value_before_add};
}

JobTaskSource::State::Value
JobTaskSource::State::DecrementWorkerCountFromWorkerAcquire() {
  const size_t value_before_sub =
      value_.fetch_sub(kWorkerCountIncrement, std::memory_order_acquire);
  DCHECK((value_before_sub >> kWorkerCountBitOffset) > 0);
  return {value_before_sub};
}

JobTaskSource::State::Value
JobTaskSource::State::IncrementWorkerCountFromJoiningThread() {
  size_t value_before_add =
      value_.fetch_add(kWorkerCountIncrement, std::memory_order_relaxed);
  return {value_before_add};
}

JobTaskSource::State::Value
JobTaskSource::State::DecrementWorkerCountFromJoiningThread() {
  const size_t value_before_sub =
      value_.fetch_sub(kWorkerCountIncrement, std::memory_order_relaxed);
  DCHECK((value_before_sub >> kWorkerCountBitOffset) > 0);
  return {value_before_sub};
}

JobTaskSource::State::Value JobTaskSource::State::Load() const {
  return {value_.load(std::memory_order_relaxed)};
}

JobTaskSource::JoinFlag::JoinFlag() = default;
JobTaskSource::JoinFlag::~JoinFlag() = default;

void JobTaskSource::JoinFlag::SetWaiting() {
  const auto previous_value =
      value_.exchange(kWaitingForWorkerToYield, std::memory_order_relaxed);
  DCHECK(previous_value == kNotWaiting);
}

bool JobTaskSource::JoinFlag::ShouldWorkerYield() {


  return value_.fetch_and(kWaitingForWorkerToSignal,
                          std::memory_order_relaxed) ==
         kWaitingForWorkerToYield;
}

bool JobTaskSource::JoinFlag::ShouldWorkerSignal() {
  return value_.exchange(kNotWaiting, std::memory_order_relaxed) != kNotWaiting;
}

JobTaskSource::JobTaskSource(
    const Location& from_here,
    const TaskTraits& traits,
    RepeatingCallback<void(JobDelegate*)> worker_task,
    RepeatingCallback<size_t()> max_concurrency_callback,
    PooledTaskRunnerDelegate* delegate)
    : TaskSource(traits, nullptr, TaskSourceExecutionMode::kJob),
      from_here_(from_here),
      max_concurrency_callback_(std::move(max_concurrency_callback)),
      worker_task_(std::move(worker_task)),
      primary_task_(base::BindRepeating(
          [](JobTaskSource* self) {

            JobDelegate job_delegate{self, self->delegate_};
            self->worker_task_.Run(&job_delegate);
          },
          base::Unretained(this))),
      queue_time_(TimeTicks::Now()),
      delegate_(delegate) {
  DCHECK(delegate_);
}

JobTaskSource::~JobTaskSource() {

  DCHECK_EQ(state_.Load().worker_count(), 0U);
}

ExecutionEnvironment JobTaskSource::GetExecutionEnvironment() {
  return {SequenceToken::Create(), nullptr};
}

bool JobTaskSource::WillJoin() {
  {
    CheckedAutoLock auto_lock(lock_);
    DCHECK(!worker_released_condition_);  // This may only be called once.
    worker_released_condition_ = lock_.CreateConditionVariable();
  }



  const auto state_before_add = state_.IncrementWorkerCountFromJoiningThread();

  if (!state_before_add.is_canceled() &&
      state_before_add.worker_count() < GetMaxConcurrency()) {
    return true;
  }
  return WaitForParticipationOpportunity();
}

bool JobTaskSource::RunJoinTask() {
  JobDelegate job_delegate{this, nullptr};
  worker_task_.Run(&job_delegate);



  const auto state = state_.Load();
  if (!state.is_canceled() && state.worker_count() <= GetMaxConcurrency())
    return true;

  return WaitForParticipationOpportunity();
}

void JobTaskSource::Cancel(TaskSource::Transaction* transaction) {



  state_.Cancel();

#if DCHECK_IS_ON()
  {
    AutoLock auto_lock(version_lock_);
    ++increase_version_;
    version_condition_.Broadcast();
  }
#endif  // DCHECK_IS_ON()
}

bool JobTaskSource::WaitForParticipationOpportunity() {
  CheckedAutoLock auto_lock(lock_);


  auto state = state_.Load();
  size_t max_concurrency = GetMaxConcurrency();




  while (!((state.worker_count() <= max_concurrency && !state.is_canceled()) ||
           state.worker_count() == 1)) {


    join_flag_.SetWaiting();




    worker_released_condition_->Wait();
    state = state_.Load();
    max_concurrency = GetMaxConcurrency();
  }

  if (state.worker_count() <= max_concurrency && !state.is_canceled())
    return true;


  DCHECK_EQ(state.worker_count(), 1U);
  DCHECK(state.is_canceled() || max_concurrency == 0U);
  state_.DecrementWorkerCountFromJoiningThread();
  return false;
}

TaskSource::RunStatus JobTaskSource::WillRunTask() {
  const size_t max_concurrency = GetMaxConcurrency();


  const auto state_before_add =
      state_.TryIncrementWorkerCountFromWorkerRelease(max_concurrency);





  if (state_before_add.is_canceled())
    return RunStatus::kDisallowed;
  const size_t worker_count_before_add = state_before_add.worker_count();

  if (worker_count_before_add >= max_concurrency)
    return RunStatus::kDisallowed;

  DCHECK_LT(worker_count_before_add, max_concurrency);
  return max_concurrency == worker_count_before_add + 1
             ? RunStatus::kAllowedSaturated
             : RunStatus::kAllowedNotSaturated;
}

size_t JobTaskSource::GetRemainingConcurrency() const {


  const auto state = state_.Load();
  const size_t max_concurrency = GetMaxConcurrency();

  if (state.is_canceled() || state.worker_count() > max_concurrency)
    return 0;
  return max_concurrency - state.worker_count();
}

void JobTaskSource::NotifyConcurrencyIncrease() {
#if DCHECK_IS_ON()
  {
    AutoLock auto_lock(version_lock_);
    ++increase_version_;
    version_condition_.Broadcast();
  }
#endif  // DCHECK_IS_ON()


  if (GetRemainingConcurrency() == 0)
    return;

  {


    CheckedAutoLock auto_lock(lock_);
    if (join_flag_.ShouldWorkerSignal())
      worker_released_condition_->Signal();
  }






  delegate_->EnqueueJobTaskSource(this);
}

size_t JobTaskSource::GetMaxConcurrency() const {
  return max_concurrency_callback_.Run();
}

bool JobTaskSource::ShouldYield() {



  return TS_UNCHECKED_READ(join_flag_).ShouldWorkerYield() ||
         state_.Load().is_canceled();
}

#if DCHECK_IS_ON()

size_t JobTaskSource::GetConcurrencyIncreaseVersion() const {
  AutoLock auto_lock(version_lock_);
  return increase_version_;
}

bool JobTaskSource::WaitForConcurrencyIncreaseUpdate(size_t recorded_version) {
  AutoLock auto_lock(version_lock_);
  constexpr TimeDelta timeout = TimeDelta::FromSeconds(1);
  const base::TimeTicks start_time = subtle::TimeTicksNowIgnoringOverride();
  do {
    DCHECK_LE(recorded_version, increase_version_);
    const auto state = state_.Load();
    if (recorded_version != increase_version_ || state.is_canceled())
      return true;

    ScopedAllowBaseSyncPrimitivesOutsideBlockingScope
        allow_base_sync_primitives;
    version_condition_.TimedWait(timeout);
  } while (subtle::TimeTicksNowIgnoringOverride() - start_time < timeout);
  return false;
}

#endif  // DCHECK_IS_ON()

Task JobTaskSource::TakeTask(TaskSource::Transaction* transaction) {


  DCHECK_GT(state_.Load().worker_count(), 0U);
  DCHECK(primary_task_);
  return Task(from_here_, primary_task_, TimeDelta());
}

bool JobTaskSource::DidProcessTask(TaskSource::Transaction* transaction) {



  CheckedAutoLockMaybe auto_lock(transaction ? nullptr : &lock_);
  AnnotateAcquiredLockAlias annotate(lock_, lock_);


  const auto state_before_sub = state_.DecrementWorkerCountFromWorkerAcquire();

  if (join_flag_.ShouldWorkerSignal())
    worker_released_condition_->Signal();

  if (state_before_sub.is_canceled())
    return false;

  DCHECK_GT(state_before_sub.worker_count(), 0U);


  return state_before_sub.worker_count() <= GetMaxConcurrency();
}

SequenceSortKey JobTaskSource::GetSortKey() const {
  return SequenceSortKey(traits_.priority(), queue_time_);
}

Task JobTaskSource::Clear(TaskSource::Transaction* transaction) {
  Cancel();



  return Task(from_here_, DoNothing(), TimeDelta());
}

}  // namespace internal
}  // namespace base
