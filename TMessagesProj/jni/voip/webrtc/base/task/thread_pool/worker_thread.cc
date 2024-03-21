// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/worker_thread.h"

#include <stddef.h>

#include <utility>

#include "base/compiler_specific.h"
#include "base/debug/alias.h"
#include "base/logging.h"
#include "base/task/thread_pool/environment_config.h"
#include "base/task/thread_pool/task_tracker.h"
#include "base/task/thread_pool/worker_thread_observer.h"
#include "base/threading/hang_watcher.h"
#include "base/time/time_override.h"
#include "base/trace_event/trace_event.h"

#if defined(OS_MACOSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#endif

namespace base {
namespace internal {

void WorkerThread::Delegate::WaitForWork(WaitableEvent* wake_up_event) {
  DCHECK(wake_up_event);
  const TimeDelta sleep_time = GetSleepTimeout();
  if (sleep_time.is_max()) {


    wake_up_event->Wait();
  } else {
    wake_up_event->TimedWait(sleep_time);
  }
}

WorkerThread::WorkerThread(ThreadPriority priority_hint,
                           std::unique_ptr<Delegate> delegate,
                           TrackedRef<TaskTracker> task_tracker,
                           const CheckedLock* predecessor_lock)
    : thread_lock_(predecessor_lock),
      delegate_(std::move(delegate)),
      task_tracker_(std::move(task_tracker)),
      priority_hint_(priority_hint),
      current_thread_priority_(GetDesiredThreadPriority()) {
  DCHECK(delegate_);
  DCHECK(task_tracker_);
  DCHECK(CanUseBackgroundPriorityForWorkerThread() ||
         priority_hint_ != ThreadPriority::BACKGROUND);
  wake_up_event_.declare_only_used_while_idle();
}

bool WorkerThread::Start(WorkerThreadObserver* worker_thread_observer) {
  CheckedLock::AssertNoLockHeldOnCurrentThread();
  CheckedAutoLock auto_lock(thread_lock_);
  DCHECK(thread_handle_.is_null());

  if (should_exit_.IsSet() || join_called_for_testing_.IsSet())
    return true;

  DCHECK(!worker_thread_observer_);
  worker_thread_observer_ = worker_thread_observer;

  self_ = this;

  constexpr size_t kDefaultStackSize = 0;
  PlatformThread::CreateWithPriority(kDefaultStackSize, this, &thread_handle_,
                                     current_thread_priority_);

  if (thread_handle_.is_null()) {
    self_ = nullptr;
    return false;
  }

  return true;
}

void WorkerThread::WakeUp() {



  CheckedLock::AssertNoLockHeldOnCurrentThread();


  DCHECK(!join_called_for_testing_.IsSet());
  DCHECK(!should_exit_.IsSet());
  wake_up_event_.Signal();
}

void WorkerThread::JoinForTesting() {
  DCHECK(!join_called_for_testing_.IsSet());
  join_called_for_testing_.Set();
  wake_up_event_.Signal();

  PlatformThreadHandle thread_handle;

  {
    CheckedAutoLock auto_lock(thread_lock_);

    if (thread_handle_.is_null())
      return;

    thread_handle = thread_handle_;

    thread_handle_ = PlatformThreadHandle();
  }

  PlatformThread::Join(thread_handle);
}

bool WorkerThread::ThreadAliveForTesting() const {
  CheckedAutoLock auto_lock(thread_lock_);
  return !thread_handle_.is_null();
}

WorkerThread::~WorkerThread() {
  CheckedAutoLock auto_lock(thread_lock_);

  if (!thread_handle_.is_null()) {
    DCHECK(!join_called_for_testing_.IsSet());
    PlatformThread::Detach(thread_handle_);
  }
}

void WorkerThread::Cleanup() {
  DCHECK(!should_exit_.IsSet());
  should_exit_.Set();
  wake_up_event_.Signal();
}

void WorkerThread::BeginUnusedPeriod() {
  CheckedAutoLock auto_lock(thread_lock_);
  DCHECK(last_used_time_.is_null());
  last_used_time_ = subtle::TimeTicksNowIgnoringOverride();
}

void WorkerThread::EndUnusedPeriod() {
  CheckedAutoLock auto_lock(thread_lock_);
  DCHECK(!last_used_time_.is_null());
  last_used_time_ = TimeTicks();
}

TimeTicks WorkerThread::GetLastUsedTime() const {
  CheckedAutoLock auto_lock(thread_lock_);
  return last_used_time_;
}

bool WorkerThread::ShouldExit() const {




  return should_exit_.IsSet() || join_called_for_testing_.IsSet() ||
         task_tracker_->IsShutdownComplete();
}

ThreadPriority WorkerThread::GetDesiredThreadPriority() const {

  if (task_tracker_->HasShutdownStarted())
    return ThreadPriority::NORMAL;

  return priority_hint_;
}

void WorkerThread::UpdateThreadPriority(
    ThreadPriority desired_thread_priority) {
  if (desired_thread_priority == current_thread_priority_)
    return;

  PlatformThread::SetCurrentThreadPriority(desired_thread_priority);
  current_thread_priority_ = desired_thread_priority;
}

void WorkerThread::ThreadMain() {
  if (priority_hint_ == ThreadPriority::BACKGROUND) {
    switch (delegate_->GetThreadLabel()) {
      case ThreadLabel::POOLED:
        RunBackgroundPooledWorker();
        return;
      case ThreadLabel::SHARED:
        RunBackgroundSharedWorker();
        return;
      case ThreadLabel::DEDICATED:
        RunBackgroundDedicatedWorker();
        return;
#if defined(OS_WIN)
      case ThreadLabel::SHARED_COM:
        RunBackgroundSharedCOMWorker();
        return;
      case ThreadLabel::DEDICATED_COM:
        RunBackgroundDedicatedCOMWorker();
        return;
#endif  // defined(OS_WIN)
    }
  }

  switch (delegate_->GetThreadLabel()) {
    case ThreadLabel::POOLED:
      RunPooledWorker();
      return;
    case ThreadLabel::SHARED:
      RunSharedWorker();
      return;
    case ThreadLabel::DEDICATED:
      RunDedicatedWorker();
      return;
#if defined(OS_WIN)
    case ThreadLabel::SHARED_COM:
      RunSharedCOMWorker();
      return;
    case ThreadLabel::DEDICATED_COM:
      RunDedicatedCOMWorker();
      return;
#endif  // defined(OS_WIN)
  }
}

NOINLINE void WorkerThread::RunPooledWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunBackgroundPooledWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunSharedWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunBackgroundSharedWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunDedicatedWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunBackgroundDedicatedWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

#if defined(OS_WIN)
NOINLINE void WorkerThread::RunSharedCOMWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunBackgroundSharedCOMWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunDedicatedCOMWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}

NOINLINE void WorkerThread::RunBackgroundDedicatedCOMWorker() {
  const int line_number = __LINE__;
  RunWorker();
  base::debug::Alias(&line_number);
}
#endif  // defined(OS_WIN)

void WorkerThread::RunWorker() {
  DCHECK_EQ(self_, this);
  TRACE_EVENT_INSTANT0("thread_pool", "WorkerThreadThread born",
                       TRACE_EVENT_SCOPE_THREAD);
  TRACE_EVENT_BEGIN0("thread_pool", "WorkerThreadThread active");

  if (worker_thread_observer_)
    worker_thread_observer_->OnWorkerThreadMainEntry();

  delegate_->OnMainEntry(this);


  const bool watch_for_hangs =
      base::HangWatcher::GetInstance() != nullptr &&
      GetDesiredThreadPriority() != ThreadPriority::BACKGROUND;

  base::ScopedClosureRunner unregister_for_hang_watching;
  if (watch_for_hangs) {
    unregister_for_hang_watching =
        base::HangWatcher::GetInstance()->RegisterThread();
  }

  {
    TRACE_EVENT_END0("thread_pool", "WorkerThreadThread active");
    delegate_->WaitForWork(&wake_up_event_);
    TRACE_EVENT_BEGIN0("thread_pool", "WorkerThreadThread active");
  }

  while (!ShouldExit()) {
#if defined(OS_MACOSX)
    mac::ScopedNSAutoreleasePool autorelease_pool;
#endif
    base::Optional<HangWatchScope> hang_watch_scope;
    if (watch_for_hangs)
      hang_watch_scope.emplace(base::HangWatchScope::kDefaultHangWatchTime);

    UpdateThreadPriority(GetDesiredThreadPriority());

    RegisteredTaskSource task_source = delegate_->GetWork(this);
    if (!task_source) {

      if (ShouldExit())
        break;

      TRACE_EVENT_END0("thread_pool", "WorkerThreadThread active");
      hang_watch_scope.reset();
      delegate_->WaitForWork(&wake_up_event_);
      TRACE_EVENT_BEGIN0("thread_pool", "WorkerThreadThread active");
      continue;
    }

    task_source = task_tracker_->RunAndPopNextTask(std::move(task_source));

    delegate_->DidProcessTask(std::move(task_source));





    wake_up_event_.Reset();
  }



  delegate_->OnMainExit(this);

  if (worker_thread_observer_)
    worker_thread_observer_->OnWorkerThreadMainExit();


  self_ = nullptr;

  TRACE_EVENT_END0("thread_pool", "WorkerThreadThread active");
  TRACE_EVENT_INSTANT0("thread_pool", "WorkerThreadThread dead",
                       TRACE_EVENT_SCOPE_THREAD);
}

}  // namespace internal
}  // namespace base
