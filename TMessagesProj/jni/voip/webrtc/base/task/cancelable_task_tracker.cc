// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/cancelable_task_tracker.h"

#include <stddef.h>

#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback_helpers.h"
#include "base/feature_list.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/metrics/histogram_macros.h"
#include "base/sequenced_task_runner.h"
#include "base/task/scoped_set_task_priority_for_current_thread.h"
#include "base/task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

namespace {

void RunOrPostToTaskRunner(scoped_refptr<SequencedTaskRunner> task_runner,
                           OnceClosure closure) {
  if (task_runner->RunsTasksInCurrentSequence())
    std::move(closure).Run();
  else
    task_runner->PostTask(FROM_HERE, std::move(closure));
}

// whether off-sequence cancelation is worthwhile.

// numeric values should never be reused.
enum class TaskStatus {
  kSameSequenceLive = 0,
  kOffSequenceLive = 1,
  kSameSequenceCanceled = 2,
  kOffSequenceCanceled = 3,
  kMaxValue = kOffSequenceCanceled,
};

void UmaRecordTaskDuration(bool same_sequence,
                           bool background,
                           bool canceled,
                           TimeDelta duration) {
#define DECLARE_HISTOGRAM(suffix)                                     \
  Histogram::FactoryTimeGet(                                          \
      "Scheduler.CancelableTaskTracker.TaskDuration2_" suffix,        \
      TimeDelta::FromMilliseconds(1), TimeDelta::FromSeconds(10), 50, \
      Histogram::kUmaTargetedHistogramFlag)

  static HistogramBase* histograms[] = {
      DECLARE_HISTOGRAM("LiveForegroundOffSequence"),
      DECLARE_HISTOGRAM("LiveForegroundSameSequence"),
      DECLARE_HISTOGRAM("LiveBackgroundOffSequence"),
      DECLARE_HISTOGRAM("LiveBackgroundSameSequence"),
      DECLARE_HISTOGRAM("CanceledForegroundOffSequence"),
      DECLARE_HISTOGRAM("CanceledForegroundSameSequence"),
      DECLARE_HISTOGRAM("CanceledBackgroundOffSequence"),
      DECLARE_HISTOGRAM("CanceledBackgroundSameSequence")};

  int i = (same_sequence ? 1 : 0) + (background ? 2 : 0) + (canceled ? 4 : 0);
  histograms[i]->AddTimeMillisecondsGranularity(duration);
}

const base::Feature kAllowOffSequenceTaskCancelation{
    "AllowOffSequenceTaskCancelation", base::FEATURE_ENABLED_BY_DEFAULT};

bool AllowOffSequenceTaskCancelation() {
  if (!base::FeatureList::GetInstance())
    return true;
  return base::FeatureList::IsEnabled(kAllowOffSequenceTaskCancelation);
}

}  // namespace

const CancelableTaskTracker::TaskId CancelableTaskTracker::kBadTaskId = 0;

CancelableTaskTracker::CancelableTaskTracker() {
  weak_this_ = weak_factory_.GetWeakPtr();
}

CancelableTaskTracker::~CancelableTaskTracker() {
  DCHECK(sequence_checker_.CalledOnValidSequence());

  TryCancelAll();
}

CancelableTaskTracker::TaskId CancelableTaskTracker::PostTask(
    TaskRunner* task_runner,
    const Location& from_here,
    OnceClosure task) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  CHECK(weak_this_);

  return PostTaskAndReply(task_runner, from_here, std::move(task), DoNothing());
}

CancelableTaskTracker::TaskId CancelableTaskTracker::PostTaskAndReply(
    TaskRunner* task_runner,
    const Location& from_here,
    OnceClosure task,
    OnceClosure reply) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  CHECK(weak_this_);

  DCHECK(SequencedTaskRunnerHandle::IsSet());

  auto flag = MakeRefCounted<TaskCancellationFlag>();

  TaskId id = next_id_;
  next_id_++;  // int64_t is big enough that we ignore the potential overflow.


  OnceClosure untrack_closure =
      BindOnce(&CancelableTaskTracker::Untrack, Unretained(this), id);
  bool success = task_runner->PostTaskAndReply(
      from_here,
      BindOnce(&RunIfNotCanceled, SequencedTaskRunnerHandle::Get(), flag,
               std::move(task)),
      BindOnce(&RunThenUntrackIfNotCanceled, SequencedTaskRunnerHandle::Get(),
               flag, std::move(reply), std::move(untrack_closure)));

  if (!success)
    return kBadTaskId;

  Track(id, std::move(flag));
  return id;
}

CancelableTaskTracker::TaskId CancelableTaskTracker::NewTrackedTaskId(
    IsCanceledCallback* is_canceled_cb) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  DCHECK(SequencedTaskRunnerHandle::IsSet());

  TaskId id = next_id_;
  next_id_++;  // int64_t is big enough that we ignore the potential overflow.

  auto flag = MakeRefCounted<TaskCancellationFlag>();


  OnceClosure untrack_closure =
      BindOnce(&CancelableTaskTracker::Untrack, Unretained(this), id);

  ScopedClosureRunner untrack_runner(
      BindOnce(&RunOrPostToTaskRunner, SequencedTaskRunnerHandle::Get(),
               BindOnce(&RunIfNotCanceled, SequencedTaskRunnerHandle::Get(),
                        flag, std::move(untrack_closure))));

  *is_canceled_cb = BindRepeating(&IsCanceled, SequencedTaskRunnerHandle::Get(),
                                  flag, std::move(untrack_runner));

  Track(id, std::move(flag));
  return id;
}

void CancelableTaskTracker::TryCancel(TaskId id) {
  DCHECK(sequence_checker_.CalledOnValidSequence());

  const auto it = task_flags_.find(id);
  if (it == task_flags_.end()) {






    return;
  }
  it->second->data.Set();




  task_flags_.erase(it);
}

void CancelableTaskTracker::TryCancelAll() {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  for (const auto& it : task_flags_)
    it.second->data.Set();
  task_flags_.clear();
}

bool CancelableTaskTracker::HasTrackedTasks() const {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  return !task_flags_.empty();
}

void CancelableTaskTracker::RunIfNotCanceled(
    const scoped_refptr<SequencedTaskRunner>& origin_task_runner,
    const scoped_refptr<TaskCancellationFlag>& flag,
    OnceClosure task) {






  const bool was_canceled = flag->data.IsSet();
  const bool same_sequence = origin_task_runner->RunsTasksInCurrentSequence();
  const TaskStatus task_status =
      was_canceled ? (same_sequence ? TaskStatus::kSameSequenceCanceled
                                    : TaskStatus::kOffSequenceCanceled)
                   : (same_sequence ? TaskStatus::kSameSequenceLive
                                    : TaskStatus::kOffSequenceLive);
  UMA_HISTOGRAM_ENUMERATION("Scheduler.CancelableTaskTracker.TaskStatus",
                            task_status);


  const bool skip_task =
      was_canceled && (AllowOffSequenceTaskCancelation() || same_sequence);
  if (skip_task)
    return;

  const TimeTicks before_task_ticks = TimeTicks::Now();
  std::move(task).Run();
  const TimeDelta duration = TimeTicks::Now() - before_task_ticks;
  const bool is_background =
      internal::GetTaskPriorityForCurrentThread() < TaskPriority::USER_VISIBLE;
  UmaRecordTaskDuration(same_sequence, is_background, was_canceled, duration);
}

void CancelableTaskTracker::RunThenUntrackIfNotCanceled(
    const scoped_refptr<SequencedTaskRunner>& origin_task_runner,
    const scoped_refptr<TaskCancellationFlag>& flag,
    OnceClosure task,
    OnceClosure untrack) {
  RunIfNotCanceled(origin_task_runner, flag, std::move(task));
  RunIfNotCanceled(origin_task_runner, flag, std::move(untrack));
}

bool CancelableTaskTracker::IsCanceled(
    const scoped_refptr<SequencedTaskRunner>& origin_task_runner,
    const scoped_refptr<TaskCancellationFlag>& flag,
    const ScopedClosureRunner& cleanup_runner) {
  return flag->data.IsSet() &&
         (AllowOffSequenceTaskCancelation() ||
          origin_task_runner->RunsTasksInCurrentSequence());
}

void CancelableTaskTracker::Track(TaskId id,
                                  scoped_refptr<TaskCancellationFlag> flag) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  CHECK(weak_this_);
  bool success = task_flags_.insert(std::make_pair(id, std::move(flag))).second;
  DCHECK(success);
}

void CancelableTaskTracker::Untrack(TaskId id) {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  CHECK(weak_this_);
  size_t num = task_flags_.erase(id);
  DCHECK_EQ(1u, num);
}

}  // namespace base
