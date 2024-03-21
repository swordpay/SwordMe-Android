// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_RUNNER_H_
#define BASE_TASK_RUNNER_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/post_task_and_reply_with_result_internal.h"
#include "base/time/time.h"

namespace base {

struct TaskRunnerTraits;

// OnceClosure objects).  The TaskRunner interface provides a way of
// decoupling task posting from the mechanics of how each task will be
// run.  TaskRunner provides very weak guarantees as to how posted
// tasks are run (or if they're run at all).  In particular, it only
// guarantees:
//
//   - Posting a task will not run it synchronously.  That is, no
//     Post*Task method will call task.Run() directly.
//
//   - Increasing the delay can only delay when the task gets run.
//     That is, increasing the delay may not affect when the task gets
//     run, or it could make it run later than it normally would, but
//     it won't make it run earlier than it normally would.
//
// TaskRunner does not guarantee the order in which posted tasks are
// run, whether tasks overlap, or whether they're run on a particular
// thread.  Also it does not guarantee a memory model for shared data
// between tasks.  (In other words, you should use your own
// synchronization/locking primitives if you need to share data
// between tasks.)
//
// Implementations of TaskRunner should be thread-safe in that all
// methods must be safe to call on any thread.  Ownership semantics
// for TaskRunners are in general not clear, which is why the
// interface itself is RefCountedThreadSafe.
//
// Some theoretical implementations of TaskRunner:
//
//   - A TaskRunner that uses a thread pool to run posted tasks.
//
//   - A TaskRunner that, for each task, spawns a non-joinable thread
//     to run that task and immediately quit.
//
//   - A TaskRunner that stores the list of posted tasks and has a
//     method Run() that runs each runnable task in random order.
class BASE_EXPORT TaskRunner
    : public RefCountedThreadSafe<TaskRunner, TaskRunnerTraits> {
 public:





  bool PostTask(const Location& from_here, OnceClosure task);



  virtual bool PostDelayedTask(const Location& from_here,
                               OnceClosure task,
                               base::TimeDelta delay) = 0;









































  bool PostTaskAndReply(const Location& from_here,
                        OnceClosure task,
                        OnceClosure reply);














  template <typename TaskReturnType, typename ReplyArgType>
  bool PostTaskAndReplyWithResult(const Location& from_here,
                                  OnceCallback<TaskReturnType()> task,
                                  OnceCallback<void(ReplyArgType)> reply) {
    DCHECK(task);
    DCHECK(reply);

    auto* result = new std::unique_ptr<TaskReturnType>();
    return PostTaskAndReply(
        from_here,
        BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>,
                 std::move(task), result),
        BindOnce(&internal::ReplyAdapter<TaskReturnType, ReplyArgType>,
                 std::move(reply), Owned(result)));
  }

 protected:
  friend struct TaskRunnerTraits;

  TaskRunner();
  virtual ~TaskRunner();



  virtual void OnDestruct() const;
};

struct BASE_EXPORT TaskRunnerTraits {
  static void Destruct(const TaskRunner* task_runner);
};

}  // namespace base

#endif  // BASE_TASK_RUNNER_H_
