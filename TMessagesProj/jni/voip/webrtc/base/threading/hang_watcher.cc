// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/hang_watcher.h"

#include <algorithm>
#include <atomic>
#include <utility>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/debug/dump_without_crashing.h"
#include "base/feature_list.h"
#include "base/no_destructor.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_checker.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"

namespace base {

const base::Feature HangWatcher::kEnableHangWatcher{
    "EnableHangWatcher", base::FEATURE_DISABLED_BY_DEFAULT};

const base::TimeDelta HangWatchScope::kDefaultHangWatchTime =
    base::TimeDelta::FromSeconds(10);

namespace {
HangWatcher* g_instance = nullptr;
}

constexpr const char* kThreadName = "HangWatcher";

// Monitor(). Increasing or decreasing this does not modify the type of hangs
// that can be detected. It instead increases the probability that a call to
// Monitor() will happen at the right time to catch a hang. This has to be
// balanced with power/cpu use concerns as busy looping would catch amost all
// hangs but present unacceptable overhead.
const base::TimeDelta kMonitoringPeriod = base::TimeDelta::FromSeconds(10);

HangWatchScope::HangWatchScope(TimeDelta timeout) {
  internal::HangWatchState* current_hang_watch_state =
      internal::HangWatchState::GetHangWatchStateForCurrentThread()->Get();


  if (!current_hang_watch_state) {
    return;
  }

  DCHECK(current_hang_watch_state)
      << "A scope can only be used on a thread that "
         "registered for hang watching with HangWatcher::RegisterThread.";

#if DCHECK_IS_ON()
  previous_scope_ = current_hang_watch_state->GetCurrentHangWatchScope();
  current_hang_watch_state->SetCurrentHangWatchScope(this);
#endif




  previous_deadline_ = current_hang_watch_state->GetDeadline();
  TimeTicks deadline = TimeTicks::Now() + timeout;
  current_hang_watch_state->SetDeadline(deadline);
}

HangWatchScope::~HangWatchScope() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  internal::HangWatchState* current_hang_watch_state =
      internal::HangWatchState::GetHangWatchStateForCurrentThread()->Get();


  if (!current_hang_watch_state) {
    return;
  }


  base::HangWatcher::GetInstance()->BlockIfCaptureInProgress();

#if DCHECK_IS_ON()

  DCHECK_EQ(this, current_hang_watch_state->GetCurrentHangWatchScope());
  current_hang_watch_state->SetCurrentHangWatchScope(previous_scope_);
#endif

  current_hang_watch_state->SetDeadline(previous_deadline_);


}

HangWatcher::HangWatcher(RepeatingClosure on_hang_closure)
    : monitor_period_(kMonitoringPeriod),
      should_monitor_(WaitableEvent::ResetPolicy::AUTOMATIC),
      on_hang_closure_(std::move(on_hang_closure)),
      thread_(this, kThreadName) {

  DETACH_FROM_THREAD(thread_checker_);

  should_monitor_.declare_only_used_while_idle();

  DCHECK(!g_instance);
  g_instance = this;
  Start();
}

HangWatcher::~HangWatcher() {
  DCHECK_EQ(g_instance, this);
  DCHECK(watch_states_.empty());
  g_instance = nullptr;
  Stop();
}

void HangWatcher::Start() {
  thread_.Start();
}

void HangWatcher::Stop() {
  keep_monitoring_.store(false, std::memory_order_relaxed);
  should_monitor_.Signal();
  thread_.Join();
}

bool HangWatcher::IsWatchListEmpty() {
  AutoLock auto_lock(watch_state_lock_);
  return watch_states_.empty();
}

void HangWatcher::Run() {


  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  while (keep_monitoring_.load(std::memory_order_relaxed)) {

    if (IsWatchListEmpty()) {
      should_monitor_.Wait();
    } else {
      Monitor();
    }

    if (keep_monitoring_.load(std::memory_order_relaxed)) {

      should_monitor_.TimedWait(monitor_period_);
    }
  }
}

HangWatcher* HangWatcher::GetInstance() {
  return g_instance;
}

void HangWatcher::RecordHang() {
  base::debug::DumpWithoutCrashing();



  volatile int inhibit_tail_call_optimization = __LINE__;
  ALLOW_UNUSED_LOCAL(inhibit_tail_call_optimization);
}

ScopedClosureRunner HangWatcher::RegisterThread() {
  AutoLock auto_lock(watch_state_lock_);

  watch_states_.push_back(
      internal::HangWatchState::CreateHangWatchStateForCurrentThread());

  if (watch_states_.size() == 1) {
    should_monitor_.Signal();
  }

  return ScopedClosureRunner(BindOnce(&HangWatcher::UnregisterThread,
                                      Unretained(HangWatcher::GetInstance())));
}

void HangWatcher::Monitor() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  bool must_invoke_hang_closure = false;
  {
    AutoLock auto_lock(watch_state_lock_);
    for (const auto& watch_state : watch_states_) {
      if (watch_state->IsOverDeadline()) {
        must_invoke_hang_closure = true;
        break;
      }
    }
  }

  if (must_invoke_hang_closure) {
    capture_in_progress.store(true, std::memory_order_relaxed);
    base::AutoLock scope_lock(capture_lock_);


    on_hang_closure_.Run();

    capture_in_progress.store(false, std::memory_order_relaxed);
  }

  if (after_monitor_closure_for_testing_) {
    after_monitor_closure_for_testing_.Run();
  }
}

void HangWatcher::SetAfterMonitorClosureForTesting(
    base::RepeatingClosure closure) {
  after_monitor_closure_for_testing_ = std::move(closure);
}

void HangWatcher::SetMonitoringPeriodForTesting(base::TimeDelta period) {
  monitor_period_ = period;
}

void HangWatcher::SignalMonitorEventForTesting() {
  should_monitor_.Signal();
}

void HangWatcher::BlockIfCaptureInProgress() {




  if (capture_in_progress.load(std::memory_order_relaxed)) {
    base::AutoLock hang_lock(capture_lock_);
  }
}

void HangWatcher::UnregisterThread() {
  AutoLock auto_lock(watch_state_lock_);

  internal::HangWatchState* current_hang_watch_state =
      internal::HangWatchState::GetHangWatchStateForCurrentThread()->Get();

  auto it =
      std::find_if(watch_states_.cbegin(), watch_states_.cend(),
                   [current_hang_watch_state](
                       const std::unique_ptr<internal::HangWatchState>& state) {
                     return state.get() == current_hang_watch_state;
                   });

  DCHECK(it != watch_states_.end());

  watch_states_.erase(it);
}

namespace internal {

// when setting the first legitimate value.
HangWatchState::HangWatchState() {

  DCHECK(!GetHangWatchStateForCurrentThread()->Get());

  GetHangWatchStateForCurrentThread()->Set(this);
}

HangWatchState::~HangWatchState() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  DCHECK_EQ(GetHangWatchStateForCurrentThread()->Get(), this);
  GetHangWatchStateForCurrentThread()->Set(nullptr);

#if DCHECK_IS_ON()


  DCHECK(!current_hang_watch_scope_);
#endif
}

std::unique_ptr<HangWatchState>
HangWatchState::CreateHangWatchStateForCurrentThread() {

  std::unique_ptr<HangWatchState> hang_state =
      std::make_unique<HangWatchState>();

  DCHECK_EQ(GetHangWatchStateForCurrentThread()->Get(), hang_state.get());

  return hang_state;
}

TimeTicks HangWatchState::GetDeadline() const {
  return deadline_.load(std::memory_order_relaxed);
}

void HangWatchState::SetDeadline(TimeTicks deadline) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  deadline_.store(deadline, std::memory_order_relaxed);
}

bool HangWatchState::IsOverDeadline() const {
  return TimeTicks::Now() > deadline_.load(std::memory_order_relaxed);
}

#if DCHECK_IS_ON()
void HangWatchState::SetCurrentHangWatchScope(HangWatchScope* scope) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  current_hang_watch_scope_ = scope;
}

HangWatchScope* HangWatchState::GetCurrentHangWatchScope() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  return current_hang_watch_scope_;
}
#endif

ThreadLocalPointer<HangWatchState>*
HangWatchState::GetHangWatchStateForCurrentThread() {
  static NoDestructor<ThreadLocalPointer<HangWatchState>> hang_watch_state;
  return hang_watch_state.get();
}

}  // namespace internal

}  // namespace base
