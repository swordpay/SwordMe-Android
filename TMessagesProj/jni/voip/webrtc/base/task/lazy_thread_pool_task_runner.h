// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_LAZY_THREAD_POOL_TASK_RUNNER_H_
#define BASE_TASK_LAZY_THREAD_POOL_TASK_RUNNER_H_

#include <vector>

#include "base/atomicops.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task/common/checked_lock.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task/task_traits.h"
#include "base/thread_annotations.h"
#include "build/build_config.h"

//
// Lazy(Sequenced|SingleThread|COMSTA)TaskRunner is meant to be instantiated in
// an anonymous namespace (no static initializer is generated) and used to post
// tasks to the same thread-pool-bound sequence/thread from pieces of code that
// don't have a better way of sharing a TaskRunner. It is important to use this
// class instead of a self-managed global variable or LazyInstance so that the
// TaskRunners do not outlive the scope of the TaskEnvironment in unit tests
// (otherwise the next test in the same process will die in use-after-frees).
//
// IMPORTANT: Only use this API as a last resort. Prefer storing a
// (Sequenced|SingleThread)TaskRunner returned by
// base::ThreadPool::Create(Sequenced|SingleThread|COMSTA)TaskRunner() as a
// member on an object accessible by all PostTask() call sites.
//
// Example usage 1:
//
// namespace {
// base::LazyThreadPoolSequencedTaskRunner g_sequenced_task_runner =
//     LAZY_THREAD_POOL_SEQUENCED_TASK_RUNNER_INITIALIZER(
//         base::TaskTraits(base::MayBlock(),
//                          base::TaskPriority::USER_VISIBLE));
// }  // namespace
//
// void SequencedFunction() {
//   // Different invocations of this function post to the same
//   // MayBlock() SequencedTaskRunner.
//   g_sequenced_task_runner.Get()->PostTask(FROM_HERE, base::BindOnce(...));
// }
//
// Example usage 2:
//
// namespace {
// base::LazyThreadPoolSequencedTaskRunner g_sequenced_task_task_runner =
//     LAZY_THREAD_POOL_SEQUENCED_TASK_RUNNER_INITIALIZER(
//         base::TaskTraits(base::MayBlock()));
// }  // namespace
//
// // Code from different files can access the SequencedTaskRunner via this
// // function.
// scoped_refptr<base::SequencedTaskRunner> GetTaskRunner() {
//   return g_sequenced_task_runner.Get();
// }

namespace base {

namespace internal {
template <typename TaskRunnerType, bool com_sta>
class BASE_EXPORT LazyThreadPoolTaskRunner;
}  // namespace internal

using LazyThreadPoolSequencedTaskRunner =
    internal::LazyThreadPoolTaskRunner<SequencedTaskRunner, false>;

using LazyThreadPoolSingleThreadTaskRunner =
    internal::LazyThreadPoolTaskRunner<SingleThreadTaskRunner, false>;

#if defined(OS_WIN)
// Lazy COM-STA enabled SingleThreadTaskRunner.
using LazyThreadPoolCOMSTATaskRunner =
    internal::LazyThreadPoolTaskRunner<SingleThreadTaskRunner, true>;
#endif

#define LAZY_TASK_RUNNER_CONCATENATE_INTERNAL2(a, b) a##b
#define LAZY_TASK_RUNNER_CONCATENATE_INTERNAL(a, b) \
  LAZY_TASK_RUNNER_CONCATENATE_INTERNAL2(a, b)

// verify that their arguments are constexpr, which is important to prevent the
// generation of a static initializer.

#define LAZY_THREAD_POOL_SEQUENCED_TASK_RUNNER_INITIALIZER(traits)     \
  base::LazyThreadPoolSequencedTaskRunner::CreateInternal(traits);     \
  /* ThreadPool() as a trait is deprecated and implicit here */        \
  static_assert(!traits.use_thread_pool(), "");                        \
  ALLOW_UNUSED_TYPE constexpr base::TaskTraits                         \
      LAZY_TASK_RUNNER_CONCATENATE_INTERNAL(kVerifyTraitsAreConstexpr, \
                                            __LINE__) = traits

// |thread_mode| specifies whether the SingleThreadTaskRunner can share its
// thread with other SingleThreadTaskRunners.
#define LAZY_THREAD_POOL_SINGLE_THREAD_TASK_RUNNER_INITIALIZER(traits,      \
                                                               thread_mode) \
  base::LazyThreadPoolSingleThreadTaskRunner::CreateInternal(traits,        \
                                                             thread_mode);  \
  /* ThreadPool() as a trait is deprecated and implicit here */             \
  static_assert(!traits.use_thread_pool(), "");                             \
  ALLOW_UNUSED_TYPE constexpr base::TaskTraits                              \
      LAZY_TASK_RUNNER_CONCATENATE_INTERNAL(kVerifyTraitsAreConstexpr,      \
                                            __LINE__) = traits;             \
  ALLOW_UNUSED_TYPE constexpr base::SingleThreadTaskRunnerThreadMode        \
      LAZY_TASK_RUNNER_CONCATENATE_INTERNAL(kVerifyThreadModeIsConstexpr,   \
                                            __LINE__) = thread_mode

// SingleThreadTaskRunner. |thread_mode| specifies whether the COM STA
// SingleThreadTaskRunner can share its thread with other
// SingleThreadTaskRunners.
#define LAZY_COM_STA_TASK_RUNNER_INITIALIZER(traits, thread_mode)            \
  base::LazyThreadPoolCOMSTATaskRunner::CreateInternal(traits, thread_mode); \
  /* ThreadPool() as a trait is deprecated and implicit here */              \
  static_assert(!traits.use_thread_pool(), "");                              \
  ALLOW_UNUSED_TYPE constexpr base::TaskTraits                               \
      LAZY_TASK_RUNNER_CONCATENATE_INTERNAL(kVerifyTraitsAreConstexpr,       \
                                            __LINE__) = traits;              \
  ALLOW_UNUSED_TYPE constexpr base::SingleThreadTaskRunnerThreadMode         \
      LAZY_TASK_RUNNER_CONCATENATE_INTERNAL(kVerifyThreadModeIsConstexpr,    \
                                            __LINE__) = thread_mode

namespace internal {

template <typename TaskRunnerType, bool com_sta>
class BASE_EXPORT LazyThreadPoolTaskRunner {
 public:






  static constexpr LazyThreadPoolTaskRunner CreateInternal(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode =
          SingleThreadTaskRunnerThreadMode::SHARED) {
    return LazyThreadPoolTaskRunner(traits, thread_mode);
  }


  scoped_refptr<TaskRunnerType> Get();

 private:
  constexpr LazyThreadPoolTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode =
          SingleThreadTaskRunnerThreadMode::SHARED)
      : traits_(traits), thread_mode_(thread_mode) {}

  void Reset();

  scoped_refptr<TaskRunnerType> Create();




  static TaskRunnerType* CreateRaw(void* void_self);

  const TaskTraits traits_;

  const SingleThreadTaskRunnerThreadMode thread_mode_;





  subtle::AtomicWord state_ = 0;



};

// callback to the current ScopedLazyTaskRunnerListForTesting, if any.
// Callbacks run when the ScopedLazyTaskRunnerListForTesting is
// destroyed. In a test process, a ScopedLazyTaskRunnerListForTesting
// must be instantiated before any LazyThreadPoolTaskRunner becomes active.
class BASE_EXPORT ScopedLazyTaskRunnerListForTesting {
 public:
  ScopedLazyTaskRunnerListForTesting();
  ~ScopedLazyTaskRunnerListForTesting();

 private:
  friend class LazyThreadPoolTaskRunner<SequencedTaskRunner, false>;
  friend class LazyThreadPoolTaskRunner<SingleThreadTaskRunner, false>;

#if defined(OS_WIN)
  friend class LazyThreadPoolTaskRunner<SingleThreadTaskRunner, true>;
#endif

  void AddCallback(OnceClosure callback);

  CheckedLock lock_;

  std::vector<OnceClosure> callbacks_ GUARDED_BY(lock_);

  DISALLOW_COPY_AND_ASSIGN(ScopedLazyTaskRunnerListForTesting);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_LAZY_THREAD_POOL_TASK_RUNNER_H_
