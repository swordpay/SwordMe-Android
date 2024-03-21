// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_H_
#define BASE_TASK_THREAD_POOL_H_

#include <memory>
#include <utility>

namespace base {
// TODO(gab): thread_pool.h should include task_traits.h but it can't during the
// migration because task_traits.h has to include thread_pool.h to get the old
// base::ThreadPool() trait constructor and that would create a circular
// dependency. Some of the includes below result in an extended version of this
// circular dependency. These forward-declarations are temporarily required for
// the duration of the migration.
enum class TaskPriority : uint8_t;
enum class TaskShutdownBehavior : uint8_t;
enum class ThreadPolicy : uint8_t;
struct MayBlock;
struct WithBaseSyncPrimitives;
class TaskTraits;
// UpdateableSequencedTaskRunner is part of this dance too because
// updateable_sequenced_task_runner.h includes task_traits.h
class UpdateableSequencedTaskRunner;
}  // namespace base

#include "base/base_export.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/location.h"
#include "base/memory/scoped_refptr.h"
#include "base/post_task_and_reply_with_result_internal.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task_runner.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

//
// To post a simple one-off task with default traits:
//     base::ThreadPool::PostTask(FROM_HERE, base::BindOnce(...));
//
// To post a high priority one-off task to respond to a user interaction:
//     base::ThreadPool::PostTask(
//         FROM_HERE,
//         {base::TaskPriority::USER_BLOCKING},
//         base::BindOnce(...));
//
// To post tasks that must run in sequence with default traits:
//     scoped_refptr<SequencedTaskRunner> task_runner =
//         base::ThreadPool::CreateSequencedTaskRunner();
//     task_runner->PostTask(FROM_HERE, base::BindOnce(...));
//     task_runner->PostTask(FROM_HERE, base::BindOnce(...));
//
// To post tasks that may block, must run in sequence and can be skipped on
// shutdown:
//     scoped_refptr<SequencedTaskRunner> task_runner =
//         base::ThreadPool::CreateSequencedTaskRunner(
//             {MayBlock(), TaskShutdownBehavior::SKIP_ON_SHUTDOWN});
//     task_runner->PostTask(FROM_HERE, base::BindOnce(...));
//     task_runner->PostTask(FROM_HERE, base::BindOnce(...));
//
// The default traits apply to tasks that:
//     (1) don't block (ref. MayBlock() and WithBaseSyncPrimitives()),
//     (2) prefer inheriting the current priority to specifying their own, and
//     (3) can either block shutdown or be skipped on shutdown
//         (implementation is free to choose a fitting default).
// Explicit traits must be specified for tasks for which these loose
// requirements are not sufficient.
//
// Prerequisite: A ThreadPoolInstance must have been registered for the current
// process via ThreadPoolInstance::Set() before the API below can be invoked.
// This is typically done during the initialization phase in each process. If
// your code is not running in that phase, you most likely don't have to worry
// about this. You will encounter DCHECKs or nullptr dereferences if this is
// violated. For tests, use base::test::TaskEnvironment.
class BASE_EXPORT ThreadPool {
 public:






  ThreadPool() = default;

  static bool PostTask(const Location& from_here, OnceClosure task);
  inline static bool PostTask(OnceClosure task,
                              const Location& from_here = Location::Current()) {
    return PostTask(from_here, std::move(task));
  }




  static bool PostDelayedTask(const Location& from_here,
                              OnceClosure task,
                              TimeDelta delay);

  static bool PostTaskAndReply(const Location& from_here,
                               OnceClosure task,
                               OnceClosure reply);







  template <template <typename> class CallbackType,
            typename TaskReturnType,
            typename ReplyArgType,
            typename = EnableIfIsBaseCallback<CallbackType>>
  static bool PostTaskAndReplyWithResult(
      const Location& from_here,
      CallbackType<TaskReturnType()> task,
      CallbackType<void(ReplyArgType)> reply) {
    return ThreadPool::PostTaskAndReplyWithResult(from_here, std::move(task),
                                                  std::move(reply));
  }


  static bool PostTask(const Location& from_here,
                       const TaskTraits& traits,
                       OnceClosure task);






  static bool PostDelayedTask(const Location& from_here,
                              const TaskTraits& traits,
                              OnceClosure task,
                              TimeDelta delay);





  static bool PostTaskAndReply(const Location& from_here,
                               const TaskTraits& traits,
                               OnceClosure task,
                               OnceClosure reply);











  template <template <typename> class CallbackType,
            typename TaskReturnType,
            typename ReplyArgType,
            typename = EnableIfIsBaseCallback<CallbackType>>
  static bool PostTaskAndReplyWithResult(
      const Location& from_here,
      const TaskTraits& traits,
      CallbackType<TaskReturnType()> task,
      CallbackType<void(ReplyArgType)> reply) {
    auto* result = new std::unique_ptr<TaskReturnType>();
    return PostTaskAndReply(
        from_here, traits,
        BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>,
                 std::move(task), result),
        BindOnce(&internal::ReplyAdapter<TaskReturnType, ReplyArgType>,
                 std::move(reply), Owned(result)));
  }


  static scoped_refptr<TaskRunner> CreateTaskRunner(const TaskTraits& traits);


  static scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits);









  static scoped_refptr<UpdateableSequencedTaskRunner>
  CreateUpdateableSequencedTaskRunner(const TaskTraits& traits);













  static scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode =
          SingleThreadTaskRunnerThreadMode::SHARED);

#if defined(OS_WIN)












  static scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode =
          SingleThreadTaskRunnerThreadMode::SHARED);
#endif  // defined(OS_WIN)
};

}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_H_
