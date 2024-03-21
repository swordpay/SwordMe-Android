// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_WORK_DEDUPLICATOR_H_
#define BASE_TASK_SEQUENCE_MANAGER_WORK_DEDUPLICATOR_H_

#include <atomic>

#include "base/base_export.h"
#include "base/task/sequence_manager/associated_thread_id.h"

namespace base {
namespace sequence_manager {
namespace internal {

// expensive. The idea is a DoWork will (maybe) run a task before computing the
// delay till the next task. If the task run posts another task, we don't want
// it to schedule work because the DoWork will post a continuation as needed
// with the latest state taken into consideration (fences, enable / disable
// queue, task cancellation, etc...) Other threads can also post DoWork at any
// time, including while we're computing the delay till the next task. To
// account for that, we have split a DoWork up into two sections:
// [OnWorkStarted .. WillCheckForMoreWork] and
// [WillCheckForMoreWork .. DidCheckForMoreWork] where DidCheckForMoreWork
// detects if another thread called OnWorkRequested.
//
// Nesting is assumed to be dealt with by the ThreadController.
//
// Most methods are thread-affine except for On(Delayed)WorkRequested which are
// is thread-safe.
class BASE_EXPORT WorkDeduplicator {
 public:


  explicit WorkDeduplicator(
      scoped_refptr<AssociatedThreadId> associated_thread);

  ~WorkDeduplicator();

  enum ShouldScheduleWork {
    kScheduleImmediate,
    kNotNeeded,
  };


  ShouldScheduleWork BindToCurrentThread();






























  ShouldScheduleWork OnWorkRequested();












  ShouldScheduleWork OnDelayedWorkRequested() const;


  void OnWorkStarted();







  void WillCheckForMoreWork();

  enum NextTask {
    kIsImmediate,
    kIsDelayed,
  };




  ShouldScheduleWork DidCheckForMoreWork(NextTask next_task);







  void OnDelayedWorkStarted();
  ShouldScheduleWork OnDelayedWorkEnded(NextTask next_task);

 private:
  enum Flags {
    kInDoWorkFlag = 1 << 0,
    kPendingDoWorkFlag = 1 << 1,
    kBoundFlag = 1 << 2,
  };

  enum State {
    kUnbound = 0,
    kIdle = Flags::kBoundFlag,
    kDoWorkPending = Flags::kPendingDoWorkFlag | Flags::kBoundFlag,
    kInDoWork = Flags::kInDoWorkFlag | Flags::kBoundFlag,
  };

  std::atomic<int> state_{State::kUnbound};

  scoped_refptr<AssociatedThreadId> associated_thread_;

  ShouldScheduleWork last_work_check_result_ = ShouldScheduleWork::kNotNeeded;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_WORK_DEDUPLICATOR_H_
