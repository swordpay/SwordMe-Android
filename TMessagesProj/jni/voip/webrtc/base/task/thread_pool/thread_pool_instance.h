// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_THREAD_POOL_INSTANCE_H_
#define BASE_TASK_THREAD_POOL_THREAD_POOL_INSTANCE_H_

#include <memory>
#include <vector>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_piece.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task/task_traits.h"
#include "base/task_runner.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace gin {
class V8Platform;
}

namespace content {
// Can't use the FRIEND_TEST_ALL_PREFIXES macro because the test is in a
// different namespace.
class BrowserMainLoopTest_CreateThreadsInSingleProcess_Test;
}  // namespace content

namespace base {

class WorkerThreadObserver;
class ThreadPoolTestHelpers;

// by the post_task.h API.
//
// The thread pool doesn't create threads until Start() is called. Tasks can
// be posted at any time but will not run until after Start() is called.
//
// The instance methods of this class are thread-safe.
//
// Note: All thread pool users should go through base/task/post_task.h instead
// of this interface except for the one callsite per process which manages the
// process's instance.
class BASE_EXPORT ThreadPoolInstance {
 public:
  struct BASE_EXPORT InitParams {
    enum class CommonThreadPoolEnvironment {

      DEFAULT,
#if defined(OS_WIN)

      COM_MTA,





      DEPRECATED_COM_STA_IN_FOREGROUND_GROUP,
#endif  // defined(OS_WIN)
    };

    InitParams(int max_num_foreground_threads_in);
    ~InitParams();


    int max_num_foreground_threads;

    CommonThreadPoolEnvironment common_thread_pool_environment =
        CommonThreadPoolEnvironment::DEFAULT;















    TimeDelta suggested_reclaim_time =
#if defined(OS_ANDROID)
        TimeDelta::FromMinutes(5);
#else
        TimeDelta::FromSeconds(30);
#endif
  };








  class BASE_EXPORT ScopedExecutionFence {
   public:
    ScopedExecutionFence();
    ~ScopedExecutionFence();

   private:
    DISALLOW_COPY_AND_ASSIGN(ScopedExecutionFence);
  };

  class BASE_EXPORT ScopedBestEffortExecutionFence {
   public:
    ScopedBestEffortExecutionFence();
    ~ScopedBestEffortExecutionFence();

   private:
    DISALLOW_COPY_AND_ASSIGN(ScopedBestEffortExecutionFence);
  };



  virtual ~ThreadPoolInstance() = default;








  virtual void Start(
      const InitParams& init_params,
      WorkerThreadObserver* worker_thread_observer = nullptr) = 0;









  virtual void Shutdown() = 0;





  virtual void FlushForTesting() = 0;





  virtual void FlushAsyncForTesting(OnceClosure flush_callback) = 0;




  virtual void JoinForTesting() = 0;
















#if !defined(OS_NACL)





  static void CreateAndStartWithDefaultParams(StringPiece name);


  void StartWithDefaultParams();
#endif  // !defined(OS_NACL)






  static void Create(StringPiece name);



  static void Set(std::unique_ptr<ThreadPoolInstance> thread_pool);










  static ThreadPoolInstance* Get();

 private:
  friend class ThreadPoolTestHelpers;
  friend class gin::V8Platform;
  friend class content::BrowserMainLoopTest_CreateThreadsInSingleProcess_Test;










  virtual int GetMaxConcurrentNonBlockedTasksWithTraitsDeprecated(
      const TaskTraits& traits) const = 0;


  virtual void BeginFence() = 0;
  virtual void EndFence() = 0;
  virtual void BeginBestEffortFence() = 0;
  virtual void EndBestEffortFence() = 0;
};

}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_THREAD_POOL_INSTANCE_H_
