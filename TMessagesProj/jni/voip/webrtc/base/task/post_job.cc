// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/post_job.h"

#include "base/task/scoped_set_task_priority_for_current_thread.h"
#include "base/task/thread_pool/job_task_source.h"
#include "base/task/thread_pool/pooled_task_runner_delegate.h"
#include "base/task/thread_pool/thread_pool_impl.h"
#include "base/task/thread_pool/thread_pool_instance.h"

namespace base {

JobDelegate::JobDelegate(
    internal::JobTaskSource* task_source,
    internal::PooledTaskRunnerDelegate* pooled_task_runner_delegate)
    : task_source_(task_source),
      pooled_task_runner_delegate_(pooled_task_runner_delegate) {
  DCHECK(task_source_);
#if DCHECK_IS_ON()
  recorded_increase_version_ = task_source_->GetConcurrencyIncreaseVersion();

  recorded_max_concurrency_ = task_source_->GetMaxConcurrency();
#endif  // DCHECK_IS_ON()
}

JobDelegate::~JobDelegate() {
#if DCHECK_IS_ON()


  size_t expected_max_concurrency = recorded_max_concurrency_;
  if (!last_should_yield_ && expected_max_concurrency > 0)
    --expected_max_concurrency;
  AssertExpectedConcurrency(expected_max_concurrency);
#endif  // DCHECK_IS_ON()
}

bool JobDelegate::ShouldYield() {
#if DCHECK_IS_ON()

  DCHECK(!last_should_yield_);
#endif  // DCHECK_IS_ON()
  const bool should_yield =
      task_source_->ShouldYield() ||
      (pooled_task_runner_delegate_ &&
       pooled_task_runner_delegate_->ShouldYield(task_source_));

#if DCHECK_IS_ON()
  last_should_yield_ = should_yield;
#endif  // DCHECK_IS_ON()
  return should_yield;
}

void JobDelegate::YieldIfNeeded() {

}

void JobDelegate::NotifyConcurrencyIncrease() {
  task_source_->NotifyConcurrencyIncrease();
}

void JobDelegate::AssertExpectedConcurrency(size_t expected_max_concurrency) {
















#if DCHECK_IS_ON()

  if (task_source_->GetMaxConcurrency() <= expected_max_concurrency)
    return;

  const size_t actual_version = task_source_->GetConcurrencyIncreaseVersion();
  DCHECK_LE(recorded_increase_version_, actual_version);
  if (recorded_increase_version_ < actual_version)
    return;

  const bool updated = task_source_->WaitForConcurrencyIncreaseUpdate(
      recorded_increase_version_);
  DCHECK(updated ||
         task_source_->GetMaxConcurrency() <= expected_max_concurrency)
      << "Value returned by |max_concurrency_callback| is expected to "
         "decrease, unless NotifyConcurrencyIncrease() is called.";

  recorded_increase_version_ = task_source_->GetConcurrencyIncreaseVersion();
  recorded_max_concurrency_ = task_source_->GetMaxConcurrency();
#endif  // DCHECK_IS_ON()
}

JobHandle::JobHandle() = default;

JobHandle::JobHandle(scoped_refptr<internal::JobTaskSource> task_source)
    : task_source_(std::move(task_source)) {}

JobHandle::~JobHandle() {
  DCHECK(!task_source_)
      << "The Job must be cancelled, detached or joined before its "
         "JobHandle is destroyed.";
}

JobHandle::JobHandle(JobHandle&&) = default;

JobHandle& JobHandle::operator=(JobHandle&& other) {
  DCHECK(!task_source_)
      << "The Job must be cancelled, detached or joined before its "
         "JobHandle is re-assigned.";
  task_source_ = std::move(other.task_source_);
  return *this;
}

void JobHandle::UpdatePriority(TaskPriority new_priority) {
  task_source_->delegate()->UpdatePriority(task_source_, new_priority);
}

void JobHandle::NotifyConcurrencyIncrease() {
  task_source_->NotifyConcurrencyIncrease();
}

void JobHandle::Join() {
  DCHECK_GE(internal::GetTaskPriorityForCurrentThread(),
            task_source_->priority_racy())
      << "Join may not be called on Job with higher priority than the current "
         "thread.";
  UpdatePriority(internal::GetTaskPriorityForCurrentThread());
  bool must_run = task_source_->WillJoin();
  while (must_run)
    must_run = task_source_->RunJoinTask();


  task_source_->delegate()->RemoveJobTaskSource(task_source_);
  task_source_ = nullptr;
}

void JobHandle::Cancel() {
  task_source_->Cancel();
  Join();
}

void JobHandle::CancelAndDetach() {
  task_source_->Cancel();
  Detach();
}

void JobHandle::Detach() {
  DCHECK(task_source_);
  task_source_ = nullptr;
}

JobHandle PostJob(const Location& from_here,
                  const TaskTraits& traits,
                  RepeatingCallback<void(JobDelegate*)> worker_task,
                  RepeatingCallback<size_t()> max_concurrency_callback) {
  DCHECK(ThreadPoolInstance::Get())
      << "Ref. Prerequisite section of post_task.h.\n\n"
         "Hint: if this is in a unit test, you're likely merely missing a "
         "base::test::TaskEnvironment member in your fixture.\n";


  DCHECK_EQ(traits.extension_id(),
            TaskTraitsExtensionStorage::kInvalidExtensionId);

  auto task_source = base::MakeRefCounted<internal::JobTaskSource>(
      from_here, traits, std::move(worker_task),
      std::move(max_concurrency_callback),
      static_cast<internal::ThreadPoolImpl*>(ThreadPoolInstance::Get()));
  const bool queued =
      static_cast<internal::ThreadPoolImpl*>(ThreadPoolInstance::Get())
          ->EnqueueJobTaskSource(task_source);
  if (queued)
    return internal::JobTaskSource::CreateJobHandle(std::move(task_source));
  return JobHandle();
}

}  // namespace base
