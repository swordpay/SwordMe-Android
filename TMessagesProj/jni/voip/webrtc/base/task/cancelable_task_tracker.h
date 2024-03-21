// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TaskRunner, and is able to cancel the task later if it's not needed
// anymore.  On destruction, CancelableTaskTracker will cancel all
// tracked tasks.
//
// Each cancelable task can be associated with a reply (also a OnceClosure).
// After the task is run on the TaskRunner, |reply| will be posted back to
// originating TaskRunner.
//
// NOTE:
//
// CancelableCallback (base/cancelable_callback.h) and WeakPtr binding are
// preferred solutions for canceling a task. However, they don't support
// cancelation from another sequence. This is sometimes a performance critical
// requirement. E.g. We need to cancel database lookup task on DB thread when
// user changes inputed text. If it is performance critical to do a best effort
// cancelation of a task, then CancelableTaskTracker is appropriate, otherwise
// use one of the other mechanisms.
//
// THREAD-SAFETY:
//
// 1. A CancelableTaskTracker object must be created, used, and destroyed on a
//    single sequence.
//
// 2. It's safe to destroy a CancelableTaskTracker while there are outstanding
//    tasks. This is commonly used to cancel all outstanding tasks.
//
// 3. The task is deleted on the target sequence, and the reply are deleted on
//    the originating sequence.
//
// 4. IsCanceledCallback can be run or deleted on any sequence.
#ifndef BASE_TASK_CANCELABLE_TASK_TRACKER_H_
#define BASE_TASK_CANCELABLE_TASK_TRACKER_H_

#include <stdint.h>

#include <memory>
#include <utility>

#include "base/base_export.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/containers/small_map.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/post_task_and_reply_with_result_internal.h"
#include "base/sequence_checker.h"
#include "base/synchronization/atomic_flag.h"

namespace base {

class Location;
class ScopedClosureRunner;
class TaskRunner;

class BASE_EXPORT CancelableTaskTracker {
 public:

  typedef int64_t TaskId;
  static const TaskId kBadTaskId;

  using IsCanceledCallback = RepeatingCallback<bool()>;

  CancelableTaskTracker();

  ~CancelableTaskTracker();

  TaskId PostTask(TaskRunner* task_runner,
                  const Location& from_here,
                  OnceClosure task);

  TaskId PostTaskAndReply(TaskRunner* task_runner,
                          const Location& from_here,
                          OnceClosure task,
                          OnceClosure reply);

  template <typename TaskReturnType, typename ReplyArgType>
  TaskId PostTaskAndReplyWithResult(TaskRunner* task_runner,
                                    const Location& from_here,
                                    OnceCallback<TaskReturnType()> task,
                                    OnceCallback<void(ReplyArgType)> reply) {
    auto* result = new std::unique_ptr<TaskReturnType>();
    return PostTaskAndReply(
        task_runner, from_here,
        BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>,
                 std::move(task), Unretained(result)),
        BindOnce(&internal::ReplyAdapter<TaskReturnType, ReplyArgType>,
                 std::move(reply), Owned(result)));
  }









  TaskId NewTrackedTaskId(IsCanceledCallback* is_canceled_cb);







  void TryCancel(TaskId id);


  void TryCancelAll();


  bool HasTrackedTasks() const;

 private:




  using TaskCancellationFlag = RefCountedData<AtomicFlag>;

  static void RunIfNotCanceled(
      const scoped_refptr<SequencedTaskRunner>& origin_task_runner,
      const scoped_refptr<TaskCancellationFlag>& flag,
      OnceClosure task);
  static void RunThenUntrackIfNotCanceled(
      const scoped_refptr<SequencedTaskRunner>& origin_task_runner,
      const scoped_refptr<TaskCancellationFlag>& flag,
      OnceClosure task,
      OnceClosure untrack);
  static bool IsCanceled(
      const scoped_refptr<SequencedTaskRunner>& origin_task_runner,
      const scoped_refptr<TaskCancellationFlag>& flag,
      const ScopedClosureRunner& cleanup_runner);

  void Track(TaskId id, scoped_refptr<TaskCancellationFlag> flag);
  void Untrack(TaskId id);




  small_map<std::map<TaskId, scoped_refptr<TaskCancellationFlag>>, 4>
      task_flags_;

  TaskId next_id_ = 1;
  SequenceChecker sequence_checker_;

  base::WeakPtr<CancelableTaskTracker> weak_this_;
  base::WeakPtrFactory<CancelableTaskTracker> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(CancelableTaskTracker);
};

}  // namespace base

#endif  // BASE_TASK_CANCELABLE_TASK_TRACKER_H_
