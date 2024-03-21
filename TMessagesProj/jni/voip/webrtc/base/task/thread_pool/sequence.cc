// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/sequence.h"

#include <utility>

#include "base/bind.h"
#include "base/critical_closure.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/task/task_features.h"
#include "base/time/time.h"

namespace base {
namespace internal {

Sequence::Transaction::Transaction(Sequence* sequence)
    : TaskSource::Transaction(sequence) {}

Sequence::Transaction::Transaction(Sequence::Transaction&& other) = default;

Sequence::Transaction::~Transaction() = default;

bool Sequence::Transaction::WillPushTask() const {






  return sequence()->queue_.empty() && !sequence()->has_worker_;
}

void Sequence::Transaction::PushTask(Task task) {


  CHECK(task.task);
  DCHECK(task.queue_time.is_null());

  bool should_be_queued = WillPushTask();
  task.queue_time = TimeTicks::Now();

  task.task = sequence()->traits_.shutdown_behavior() ==
                      TaskShutdownBehavior::BLOCK_SHUTDOWN
                  ? MakeCriticalClosure(std::move(task.task))
                  : std::move(task.task);

  sequence()->queue_.push(std::move(task));


  if (should_be_queued && sequence()->task_runner())
    sequence()->task_runner()->AddRef();
}

TaskSource::RunStatus Sequence::WillRunTask() {


  DCHECK(!has_worker_);




  has_worker_ = true;
  return RunStatus::kAllowedSaturated;
}

size_t Sequence::GetRemainingConcurrency() const {
  return 1;
}

Task Sequence::TakeTask(TaskSource::Transaction* transaction) {
  CheckedAutoLockMaybe auto_lock(transaction ? nullptr : &lock_);

  DCHECK(has_worker_);
  DCHECK(!queue_.empty());
  DCHECK(queue_.front().task);

  auto next_task = std::move(queue_.front());
  queue_.pop();
  return next_task;
}

bool Sequence::DidProcessTask(TaskSource::Transaction* transaction) {
  CheckedAutoLockMaybe auto_lock(transaction ? nullptr : &lock_);


  DCHECK(has_worker_);
  has_worker_ = false;

  if (queue_.empty()) {
    ReleaseTaskRunner();
    return false;
  }



  return true;
}

SequenceSortKey Sequence::GetSortKey() const {
  DCHECK(!queue_.empty());
  return SequenceSortKey(traits_.priority(), queue_.front().queue_time);
}

Task Sequence::Clear(TaskSource::Transaction* transaction) {
  CheckedAutoLockMaybe auto_lock(transaction ? nullptr : &lock_);

  if (!queue_.empty() && !has_worker_)
    ReleaseTaskRunner();
  return Task(FROM_HERE,
              base::BindOnce(
                  [](base::queue<Task> queue) {
                    while (!queue.empty())
                      queue.pop();
                  },
                  std::move(queue_)),
              TimeDelta());
}

void Sequence::ReleaseTaskRunner() {
  if (!task_runner())
    return;
  if (execution_mode() == TaskSourceExecutionMode::kParallel) {
    static_cast<PooledParallelTaskRunner*>(task_runner())
        ->UnregisterSequence(this);
  }


  task_runner()->Release();
}

Sequence::Sequence(const TaskTraits& traits,
                   TaskRunner* task_runner,
                   TaskSourceExecutionMode execution_mode)
    : TaskSource(traits, task_runner, execution_mode) {}

Sequence::~Sequence() = default;

Sequence::Transaction Sequence::BeginTransaction() {
  return Transaction(this);
}

ExecutionEnvironment Sequence::GetExecutionEnvironment() {
  return {token_, &sequence_local_storage_};
}

}  // namespace internal
}  // namespace base
