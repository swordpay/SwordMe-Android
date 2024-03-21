// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_HANG_WATCHER_H_
#define BASE_THREADING_HANG_WATCHER_H_

#include <atomic>
#include <memory>
#include <vector>

#include "base/atomicops.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/feature_list.h"
#include "base/synchronization/lock.h"
#include "base/threading/simple_thread.h"
#include "base/threading/thread_checker.h"
#include "base/threading/thread_local.h"
#include "base/time/time.h"

namespace base {
class HangWatchScope;
namespace internal {
class HangWatchState;
}  // namespace internal
}  // namespace base

namespace base {

// watched for hangs of more than |timeout| by the HangWatcher.
//
// Example usage:
//
//  void FooBar(){
//    HangWatchScope scope(base::TimeDelta::FromSeconds(5));
//    DoSomeWork();
//  }
//
// If DoSomeWork() takes more than 5s to run and the HangWatcher
// inspects the thread state before Foobar returns a hang will be
// reported.
//
// HangWatchScopes are typically meant to live on the stack. In some cases it's
// necessary to keep a HangWatchScope instance as a class member but special
// care is required when doing so as a HangWatchScope that stays alive longer
// than intended will generate non-actionable hang reports.
class BASE_EXPORT HangWatchScope {
 public:





  static const base::TimeDelta kDefaultHangWatchTime;

  explicit HangWatchScope(TimeDelta timeout);
  ~HangWatchScope();

  HangWatchScope(const HangWatchScope&) = delete;
  HangWatchScope& operator=(const HangWatchScope&) = delete;

 private:

  THREAD_CHECKER(thread_checker_);


  TimeTicks previous_deadline_;

#if DCHECK_IS_ON()

  HangWatchScope* previous_scope_;
#endif
};

// HangWatchStates for deadline overruns. This happens at a regular interval on
// a separate thread. Only one instance of HangWatcher can exist at a time
// within a single process. This instance must outlive all monitored threads.
class BASE_EXPORT HangWatcher : public DelegateSimpleThread::Delegate {
 public:
  static const base::Feature kEnableHangWatcher;



  explicit HangWatcher(RepeatingClosure on_hang_closure);

  ~HangWatcher() override;

  HangWatcher(const HangWatcher&) = delete;
  HangWatcher& operator=(const HangWatcher&) = delete;

  static HangWatcher* GetInstance();


  NOINLINE static void RecordHang();



  ScopedClosureRunner RegisterThread()
      LOCKS_EXCLUDED(watch_state_lock_) WARN_UNUSED_RESULT;


  void SetAfterMonitorClosureForTesting(base::RepeatingClosure closure);


  void SetMonitoringPeriodForTesting(base::TimeDelta period);



  void SignalMonitorEventForTesting();



  void BlockIfCaptureInProgress();

 private:
  THREAD_CHECKER(thread_checker_);


  void Monitor();

  void Start();

  void Stop();


  void Run() override;

  base::TimeDelta monitor_period_;

  std::atomic<bool> keep_monitoring_{true};


  WaitableEvent should_monitor_;

  bool IsWatchListEmpty() LOCKS_EXCLUDED(watch_state_lock_);


  void UnregisterThread() LOCKS_EXCLUDED(watch_state_lock_);

  const RepeatingClosure on_hang_closure_;
  Lock watch_state_lock_;

  std::vector<std::unique_ptr<internal::HangWatchState>> watch_states_
      GUARDED_BY(watch_state_lock_);

  base::DelegateSimpleThread thread_;

  base::RepeatingClosure after_monitor_closure_for_testing_;

  base::Lock capture_lock_;
  std::atomic<bool> capture_in_progress{false};

  FRIEND_TEST_ALL_PREFIXES(HangWatcherTest, NestedScopes);
};

// intended to be used outside of base.
namespace internal {

// thread. Instances of this class are accessed concurrently by the associated
// thread and the HangWatcher. The HangWatcher owns instances of this
// class and outside of it they are accessed through
// GetHangWatchStateForCurrentThread().
class BASE_EXPORT HangWatchState {
 public:
  HangWatchState();
  ~HangWatchState();

  HangWatchState(const HangWatchState&) = delete;
  HangWatchState& operator=(const HangWatchState&) = delete;


  static std::unique_ptr<HangWatchState> CreateHangWatchStateForCurrentThread();



  static ThreadLocalPointer<HangWatchState>*
  GetHangWatchStateForCurrentThread();


  TimeTicks GetDeadline() const;

  void SetDeadline(TimeTicks deadline);

  bool IsOverDeadline() const;

#if DCHECK_IS_ON()

  void SetCurrentHangWatchScope(HangWatchScope* scope);

  HangWatchScope* GetCurrentHangWatchScope();
#endif

 private:


  THREAD_CHECKER(thread_checker_);


  std::atomic<TimeTicks> deadline_{base::TimeTicks::Max()};

#if DCHECK_IS_ON()










  HangWatchScope* current_hang_watch_scope_{nullptr};
#endif
};

}  // namespace internal
}  // namespace base

#endif  // BASE_THREADING_HANG_WATCHER_H_
