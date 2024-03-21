// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_RUN_LOOP_H_
#define BASE_RUN_LOOP_H_

#include <utility>
#include <vector>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/containers/stack.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/sequence_checker.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

namespace test {
class ScopedRunLoopTimeout;
class ScopedDisableRunLoopTimeout;
}  // namespace test

#if defined(OS_ANDROID)
class MessagePumpForUI;
#endif

#if defined(OS_IOS)
class MessagePumpUIApplication;
#endif

class SingleThreadTaskRunner;

// A RunLoop::Delegate must have been bound to this thread (ref.
// RunLoop::RegisterDelegateForCurrentThread()) prior to using any of RunLoop's
// member and static methods unless explicitly indicated otherwise (e.g.
// IsRunning/IsNestedOnCurrentThread()). RunLoop::Run can only be called once
// per RunLoop lifetime. Create a RunLoop on the stack and call Run/Quit to run
// a nested RunLoop but please do not use nested loops in production code!
class BASE_EXPORT RunLoop {
 public:






















  enum class Type {
    kDefault,
    kNestableTasksAllowed,
  };

  RunLoop(Type type = Type::kDefault);
  ~RunLoop();



  void Run();










  void RunUntilIdle();

  bool running() const {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    return running_;
  }

















  void Quit();
  void QuitWhenIdle();























  RepeatingClosure QuitClosure();
  RepeatingClosure QuitWhenIdleClosure();


  static bool IsRunningOnCurrentThread();



  static bool IsNestedOnCurrentThread();

  class BASE_EXPORT NestingObserver {
   public:

    virtual void OnBeginNestedRunLoop() = 0;

    virtual void OnExitNestedRunLoop() {}

   protected:
    virtual ~NestingObserver() = default;
  };

  static void AddNestingObserverOnCurrentThread(NestingObserver* observer);
  static void RemoveNestingObserverOnCurrentThread(NestingObserver* observer);






  class BASE_EXPORT Delegate {
   public:
    Delegate();
    virtual ~Delegate();













    virtual void Run(bool application_tasks_allowed, TimeDelta timeout) = 0;
    virtual void Quit() = 0;







    virtual void EnsureWorkScheduled() = 0;

   protected:


    bool ShouldQuitWhenIdle();

   private:


    friend class RunLoop;



    using RunLoopStack = stack<RunLoop*, std::vector<RunLoop*>>;

    RunLoopStack active_run_loops_;
    ObserverList<RunLoop::NestingObserver>::Unchecked nesting_observers_;

#if DCHECK_IS_ON()
    bool allow_running_for_testing_ = true;
#endif


    bool bound_ = false;

    THREAD_CHECKER(bound_thread_checker_);

    DISALLOW_COPY_AND_ASSIGN(Delegate);
  };



  static void RegisterDelegateForCurrentThread(Delegate* delegate);






  static void QuitCurrentDeprecated();
  static void QuitCurrentWhenIdleDeprecated();
  static RepeatingClosure QuitCurrentWhenIdleClosureDeprecated();








  class BASE_EXPORT ScopedDisallowRunningForTesting {
   public:
    ScopedDisallowRunningForTesting();
    ~ScopedDisallowRunningForTesting();

   private:
#if DCHECK_IS_ON()
    Delegate* current_delegate_;
    const bool previous_run_allowance_;
#endif  // DCHECK_IS_ON()

    DISALLOW_COPY_AND_ASSIGN(ScopedDisallowRunningForTesting);
  };


  struct BASE_EXPORT RunLoopTimeout {
    RunLoopTimeout();
    ~RunLoopTimeout();
    TimeDelta timeout;
    RepeatingClosure on_timeout;
  };

 private:
  FRIEND_TEST_ALL_PREFIXES(MessageLoopTypedTest, RunLoopQuitOrderAfter);

#if defined(OS_ANDROID)


  friend class MessagePumpForUI;
#endif

#if defined(OS_IOS)


  friend class MessagePumpUIApplication;
#endif

  friend class test::ScopedRunLoopTimeout;
  friend class test::ScopedDisableRunLoopTimeout;

  static void SetTimeoutForCurrentThread(const RunLoopTimeout* timeout);
  static const RunLoopTimeout* GetTimeoutForCurrentThread();

  bool BeforeRun();
  void AfterRun();



  Delegate* const delegate_;

  const Type type_;

#if DCHECK_IS_ON()
  bool run_called_ = false;
#endif

  bool quit_called_ = false;
  bool running_ = false;




  bool quit_when_idle_received_ = false;



  bool allow_quit_current_deprecated_ = true;






  SEQUENCE_CHECKER(sequence_checker_);

  const scoped_refptr<SingleThreadTaskRunner> origin_task_runner_;

  WeakPtrFactory<RunLoop> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(RunLoop);
};

}  // namespace base

#endif  // BASE_RUN_LOOP_H_
