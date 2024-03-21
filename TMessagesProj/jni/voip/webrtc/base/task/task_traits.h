// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_TASK_TRAITS_H_
#define BASE_TASK_TASK_TRAITS_H_

#include <stdint.h>

#include <iosfwd>
#include <tuple>
#include <type_traits>
#include <utility>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/task/task_traits_extension.h"
#include "base/traits_bag.h"
#include "build/build_config.h"

// but it this is necessary to have it in this direction during the migration
// from old code that used base::ThreadPool as a trait.
#include "base/task/thread_pool.h"

namespace base {

class PostTaskAndroid;

//
// Note: internal algorithms depend on priorities being expressed as a
// continuous zero-based list from lowest to highest priority. Users of this API
// shouldn't otherwise care about nor use the underlying values.
//
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.base.task
enum class TaskPriority : uint8_t {

  LOWEST = 0,












  BEST_EFFORT = LOWEST,








  USER_VISIBLE,











  USER_BLOCKING,

  HIGHEST = USER_BLOCKING
};

enum class TaskShutdownBehavior : uint8_t {















  CONTINUE_ON_SHUTDOWN,









  SKIP_ON_SHUTDOWN,







  BLOCK_SHUTDOWN,
};

//
// ThreadPolicy and priority updates
// ---------------------------------
//
//   If the TaskPriority of an UpdateableSequencedTaskRunner is increased while
//   one of its tasks is running at background thread priority, the task's
//   execution will have to complete at background thread priority (may take a
//   long time) before the next task can be scheduled with the new TaskPriority.
//   If it is important that priority increases take effect quickly,
//   MUST_USE_FOREGROUND should be used to prevent the tasks from running at
//   background thread priority. If it is important to minimize impact on the
//   rest on the system when the TaskPriority is BEST_EFFORT, PREFER_BACKGROUND
//   should be used.
//
// ThreadPolicy and priority inversions
// ------------------------------------
//
//   A priority inversion occurs when a task running at background thread
//   priority is descheduled while holding a resource needed by a thread of
//   higher priority. MUST_USE_FOREGROUND can be combined with BEST_EFFORT to
//   indicate that a task has a low priority, but shouldn't run at background
//   thread priority in order to avoid priority inversions. Please consult with
//   //base/task/OWNERS if you suspect a priority inversion.
enum class ThreadPolicy : uint8_t {






  PREFER_BACKGROUND,

  MUST_USE_FOREGROUND
};

// that wait on synchronous file I/O operations: read or write a file from disk,
// interact with a pipe or a socket, rename or delete a file, enumerate files in
// a directory, etc. This trait isn't required for the mere use of locks. For
// tasks that block on base/ synchronization primitives, see the
// WithBaseSyncPrimitives trait.
struct MayBlock {};

//
// Tasks with this trait will pass base::AssertBaseSyncPrimitivesAllowed(), i.e.
// will be allowed on the following methods :
// - base::WaitableEvent::Wait
// - base::ConditionVariable::Wait
// - base::PlatformThread::Join
// - base::PlatformThread::Sleep
// - base::Process::WaitForExit
// - base::Process::WaitForExitWithTimeout
//
// Tasks should generally not use these methods.
//
// Instead of waiting on a WaitableEvent or a ConditionVariable, put the work
// that should happen after the wait in a callback and post that callback from
// where the WaitableEvent or ConditionVariable would have been signaled. If
// something needs to be scheduled after many tasks have executed, use
// base::BarrierClosure.
//
// On Windows, join processes asynchronously using base::win::ObjectWatcher.
//
// MayBlock() must be specified in conjunction with this trait if and only if
// removing usage of methods listed above in the labeled tasks would still
// result in tasks that may block (per MayBlock()'s definition).
//
// In doubt, consult with //base/task/OWNERS.
struct WithBaseSyncPrimitives {};

class BASE_EXPORT TaskTraits {
 public:

  struct ValidTrait {
    ValidTrait(TaskPriority);
    ValidTrait(TaskShutdownBehavior);
    ValidTrait(ThreadPolicy);
    ValidTrait(MayBlock);
    ValidTrait(WithBaseSyncPrimitives);
    ValidTrait(ThreadPool);
  };



























  template <class... ArgTypes,
            class CheckArgumentsAreValid = std::enable_if_t<
                trait_helpers::AreValidTraits<ValidTrait, ArgTypes...>::value ||
                trait_helpers::AreValidTraitsForExtension<ArgTypes...>::value>>
  constexpr TaskTraits(ArgTypes... args)
      : extension_(trait_helpers::GetTaskTraitsExtension(
            trait_helpers::AreValidTraits<ValidTrait, ArgTypes...>{},
            args...)),
        priority_(
            trait_helpers::GetEnum<TaskPriority, TaskPriority::USER_BLOCKING>(
                args...)),
        shutdown_behavior_(
            static_cast<uint8_t>(
                trait_helpers::GetEnum<TaskShutdownBehavior,
                                       TaskShutdownBehavior::SKIP_ON_SHUTDOWN>(
                    args...)) |
            (trait_helpers::HasTrait<TaskShutdownBehavior, ArgTypes...>()
                 ? kIsExplicitFlag
                 : 0)),
        thread_policy_(
            static_cast<uint8_t>(
                trait_helpers::GetEnum<ThreadPolicy,
                                       ThreadPolicy::PREFER_BACKGROUND>(
                    args...)) |
            (trait_helpers::HasTrait<ThreadPolicy, ArgTypes...>()
                 ? kIsExplicitFlag
                 : 0)),
        may_block_(trait_helpers::HasTrait<MayBlock, ArgTypes...>()),
        with_base_sync_primitives_(
            trait_helpers::HasTrait<WithBaseSyncPrimitives, ArgTypes...>()),
        use_thread_pool_(trait_helpers::HasTrait<ThreadPool, ArgTypes...>()) {}

  constexpr TaskTraits(const TaskTraits& other) = default;
  TaskTraits& operator=(const TaskTraits& other) = default;

  bool operator==(const TaskTraits& other) const {
    static_assert(sizeof(TaskTraits) == 15,
                  "Update comparison operator when TaskTraits change");
    return extension_ == other.extension_ && priority_ == other.priority_ &&
           shutdown_behavior_ == other.shutdown_behavior_ &&
           thread_policy_ == other.thread_policy_ &&
           may_block_ == other.may_block_ &&
           with_base_sync_primitives_ == other.with_base_sync_primitives_ &&
           use_thread_pool_ == other.use_thread_pool_;
  }

  void UpdatePriority(TaskPriority priority) { priority_ = priority; }

  constexpr TaskPriority priority() const { return priority_; }

  constexpr bool shutdown_behavior_set_explicitly() const {
    return shutdown_behavior_ & kIsExplicitFlag;
  }

  constexpr TaskShutdownBehavior shutdown_behavior() const {
    return static_cast<TaskShutdownBehavior>(shutdown_behavior_ &
                                             ~kIsExplicitFlag);
  }

  constexpr bool thread_policy_set_explicitly() const {
    return thread_policy_ & kIsExplicitFlag;
  }

  constexpr ThreadPolicy thread_policy() const {
    return static_cast<ThreadPolicy>(thread_policy_ & ~kIsExplicitFlag);
  }

  constexpr bool may_block() const { return may_block_; }

  constexpr bool with_base_sync_primitives() const {
    return with_base_sync_primitives_;
  }

  constexpr bool use_thread_pool() const { return use_thread_pool_; }

  uint8_t extension_id() const { return extension_.extension_id; }


  template <class TaskTraitsExtension>
  const TaskTraitsExtension GetExtension() const {
    DCHECK_EQ(TaskTraitsExtension::kExtensionId, extension_.extension_id);
    return TaskTraitsExtension::Parse(extension_);
  }

 private:
  friend PostTaskAndroid;

  TaskTraits(TaskPriority priority,
             bool may_block,
             bool use_thread_pool,
             TaskTraitsExtensionStorage extension)
      : extension_(extension),
        priority_(priority),
        shutdown_behavior_(
            static_cast<uint8_t>(TaskShutdownBehavior::SKIP_ON_SHUTDOWN)),
        thread_policy_(static_cast<uint8_t>(ThreadPolicy::PREFER_BACKGROUND)),
        may_block_(may_block),
        with_base_sync_primitives_(false),
        use_thread_pool_(use_thread_pool) {
    static_assert(sizeof(TaskTraits) == 15, "Keep this constructor up to date");


    const bool has_extension =
        (extension_.extension_id !=
         TaskTraitsExtensionStorage::kInvalidExtensionId);
    DCHECK(use_thread_pool_ ^ has_extension)
        << "Traits must explicitly specify a destination (e.g. ThreadPool or a "
           "named thread like BrowserThread)";
  }


  static constexpr uint8_t kIsExplicitFlag = 0x80;

  TaskTraitsExtensionStorage extension_;
  TaskPriority priority_;
  uint8_t shutdown_behavior_;
  uint8_t thread_policy_;
  bool may_block_;
  bool with_base_sync_primitives_;
  bool use_thread_pool_;
};

// should only be used for tracing and debugging.
BASE_EXPORT const char* TaskPriorityToString(TaskPriority task_priority);
BASE_EXPORT const char* TaskShutdownBehaviorToString(
    TaskShutdownBehavior task_priority);

// DCHECK and EXPECT statements.
BASE_EXPORT std::ostream& operator<<(std::ostream& os,
                                     const TaskPriority& shutdown_behavior);
BASE_EXPORT std::ostream& operator<<(
    std::ostream& os,
    const TaskShutdownBehavior& shutdown_behavior);

}  // namespace base

#endif  // BASE_TASK_TASK_TRAITS_H_
