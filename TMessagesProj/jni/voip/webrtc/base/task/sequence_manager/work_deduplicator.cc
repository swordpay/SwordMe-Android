// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/sequence_manager/work_deduplicator.h"

#include <utility>
#include "base/logging.h"

namespace base {
namespace sequence_manager {
namespace internal {

WorkDeduplicator::WorkDeduplicator(
    scoped_refptr<AssociatedThreadId> associated_thread)
    : associated_thread_(std::move(associated_thread)) {}

WorkDeduplicator::~WorkDeduplicator() = default;

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::BindToCurrentThread() {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  int previous_flags = state_.fetch_or(kBoundFlag);
  DCHECK_EQ(previous_flags & kBoundFlag, 0) << "Can't bind twice!";
  return previous_flags & kPendingDoWorkFlag
             ? ShouldScheduleWork::kScheduleImmediate
             : ShouldScheduleWork::kNotNeeded;
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::OnWorkRequested() {

  return state_.fetch_or(kPendingDoWorkFlag) == State::kIdle
             ? ShouldScheduleWork::kScheduleImmediate
             : ShouldScheduleWork::kNotNeeded;
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::OnDelayedWorkRequested()
    const {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);

  return state_.load() == State::kIdle ? ShouldScheduleWork::kScheduleImmediate
                                       : ShouldScheduleWork::kNotNeeded;
}

void WorkDeduplicator::OnWorkStarted() {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  DCHECK_EQ(state_.load() & kBoundFlag, kBoundFlag);

  state_.store(State::kInDoWork);
}

void WorkDeduplicator::WillCheckForMoreWork() {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  DCHECK_EQ(state_.load() & kBoundFlag, kBoundFlag);

  state_.store(State::kInDoWork);
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::DidCheckForMoreWork(
    NextTask next_task) {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  DCHECK_EQ(state_.load() & kBoundFlag, kBoundFlag);
  last_work_check_result_ = ShouldScheduleWork::kScheduleImmediate;
  if (next_task == NextTask::kIsImmediate) {
    state_.store(State::kDoWorkPending);
  } else {




    if (!(state_.fetch_and(~kInDoWorkFlag) & kPendingDoWorkFlag))
      last_work_check_result_ = ShouldScheduleWork::kNotNeeded;
  }
  return last_work_check_result_;
}

void WorkDeduplicator::OnDelayedWorkStarted() {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  OnWorkStarted();
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::OnDelayedWorkEnded(
    NextTask next_task) {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  ShouldScheduleWork prev_last_work_check_result = last_work_check_result_;
  WorkDeduplicator::ShouldScheduleWork should_schedule_work =
      DidCheckForMoreWork(next_task);
  if (prev_last_work_check_result == ShouldScheduleWork::kScheduleImmediate) {
    prev_last_work_check_result = ShouldScheduleWork::kNotNeeded;
    should_schedule_work = ShouldScheduleWork::kNotNeeded;
  }
  return should_schedule_work;
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base
