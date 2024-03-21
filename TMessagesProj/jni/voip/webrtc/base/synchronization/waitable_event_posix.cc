// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include <algorithm>
#include <limits>
#include <vector>

#include "base/debug/activity_tracker.h"
#include "base/logging.h"
#include "base/optional.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"
#include "base/time/time_override.h"

// A WaitableEvent on POSIX is implemented as a wait-list. Currently we don't
// support cross-process events (where one process can signal an event which
// others are waiting on). Because of this, we can avoid having one thread per
// listener in several cases.
//
// The WaitableEvent maintains a list of waiters, protected by a lock. Each
// waiter is either an async wait, in which case we have a Task and the
// MessageLoop to run it on, or a blocking wait, in which case we have the
// condition variable to signal.
//
// Waiting involves grabbing the lock and adding oneself to the wait list. Async
// waits can be canceled, which means grabbing the lock and removing oneself
// from the list.
//
// Waiting on multiple events is handled by adding a single, synchronous wait to
// the wait-list of many events. An event passes a pointer to itself when
// firing a waiter and so we can store that pointer to find out which event
// triggered.
// -----------------------------------------------------------------------------

namespace base {

// This is just an abstract base class for waking the two types of waiters
// -----------------------------------------------------------------------------
WaitableEvent::WaitableEvent(ResetPolicy reset_policy,
                             InitialState initial_state)
    : kernel_(new WaitableEventKernel(reset_policy, initial_state)) {}

WaitableEvent::~WaitableEvent() = default;

void WaitableEvent::Reset() {
  base::AutoLock locked(kernel_->lock_);
  kernel_->signaled_ = false;
}

void WaitableEvent::Signal() {
  base::AutoLock locked(kernel_->lock_);

  if (kernel_->signaled_)
    return;

  if (kernel_->manual_reset_) {
    SignalAll();
    kernel_->signaled_ = true;
  } else {


    if (!SignalOne())
      kernel_->signaled_ = true;
  }
}

bool WaitableEvent::IsSignaled() {
  base::AutoLock locked(kernel_->lock_);

  const bool result = kernel_->signaled_;
  if (result && !kernel_->manual_reset_)
    kernel_->signaled_ = false;
  return result;
}

// Synchronous waits

// This is a synchronous waiter. The thread is waiting on the given condition
// variable and the fired flag in this object.
// -----------------------------------------------------------------------------
class SyncWaiter : public WaitableEvent::Waiter {
 public:
  SyncWaiter()
      : fired_(false), signaling_event_(nullptr), lock_(), cv_(&lock_) {}

  bool Fire(WaitableEvent* signaling_event) override {
    base::AutoLock locked(lock_);

    if (fired_)
      return false;

    fired_ = true;
    signaling_event_ = signaling_event;

    cv_.Broadcast();




    return true;
  }

  WaitableEvent* signaling_event() const {
    return signaling_event_;
  }




  bool Compare(void* tag) override { return this == tag; }



  bool fired() const {
    return fired_;
  }





  void Disable() {
    fired_ = true;
  }

  base::Lock* lock() {
    return &lock_;
  }

  base::ConditionVariable* cv() {
    return &cv_;
  }

 private:
  bool fired_;
  WaitableEvent* signaling_event_;  // The WaitableEvent which woke us
  base::Lock lock_;
  base::ConditionVariable cv_;
};

void WaitableEvent::Wait() {
  bool result = TimedWait(TimeDelta::Max());
  DCHECK(result) << "TimedWait() should never fail with infinite timeout";
}

bool WaitableEvent::TimedWait(const TimeDelta& wait_delta) {
  if (wait_delta <= TimeDelta())
    return IsSignaled();



  Optional<debug::ScopedEventWaitActivity> event_activity;
  Optional<internal::ScopedBlockingCallWithBaseSyncPrimitives>
      scoped_blocking_call;
  if (waiting_is_blocking_) {
    event_activity.emplace(this);
    scoped_blocking_call.emplace(FROM_HERE, BlockingType::MAY_BLOCK);
  }

  kernel_->lock_.Acquire();
  if (kernel_->signaled_) {
    if (!kernel_->manual_reset_) {


      kernel_->signaled_ = false;
    }

    kernel_->lock_.Release();
    return true;
  }

  SyncWaiter sw;
  if (!waiting_is_blocking_)
    sw.cv()->declare_only_used_while_idle();
  sw.lock()->Acquire();

  Enqueue(&sw);
  kernel_->lock_.Release();







  const TimeTicks end_time =
      wait_delta.is_max() ? TimeTicks::Max()
                          : subtle::TimeTicksNowIgnoringOverride() + wait_delta;
  for (TimeDelta remaining = wait_delta; remaining > TimeDelta() && !sw.fired();
       remaining = end_time.is_max()
                       ? TimeDelta::Max()
                       : end_time - subtle::TimeTicksNowIgnoringOverride()) {
    if (end_time.is_max())
      sw.cv()->Wait();
    else
      sw.cv()->TimedWait(remaining);
  }

  const bool return_value = sw.fired();





  sw.Disable();
  sw.lock()->Release();





  kernel_->lock_.Acquire();
  kernel_->Dequeue(&sw, &sw);
  kernel_->lock_.Release();

  return return_value;
}

// Synchronous waiting on multiple objects.

static bool  // StrictWeakOrdering
cmp_fst_addr(const std::pair<WaitableEvent*, unsigned> &a,
             const std::pair<WaitableEvent*, unsigned> &b) {
  return a.first < b.first;
}

// NO_THREAD_SAFETY_ANALYSIS: Complex control flow.
size_t WaitableEvent::WaitMany(WaitableEvent** raw_waitables,
                               size_t count) NO_THREAD_SAFETY_ANALYSIS {
  DCHECK(count) << "Cannot wait on no events";
  internal::ScopedBlockingCallWithBaseSyncPrimitives scoped_blocking_call(
      FROM_HERE, BlockingType::MAY_BLOCK);

  debug::ScopedEventWaitActivity event_activity(raw_waitables[0]);



  std::vector<std::pair<WaitableEvent*, size_t> > waitables;
  waitables.reserve(count);
  for (size_t i = 0; i < count; ++i)
    waitables.push_back(std::make_pair(raw_waitables[i], i));

  DCHECK_EQ(count, waitables.size());

  sort(waitables.begin(), waitables.end(), cmp_fst_addr);



  for (size_t i = 0; i < waitables.size() - 1; ++i) {
    DCHECK(waitables[i].first != waitables[i+1].first);
  }

  SyncWaiter sw;

  const size_t r = EnqueueMany(&waitables[0], count, &sw);
  if (r < count) {


    return waitables[r].second;
  }


  sw.lock()->Acquire();

    for (size_t i = 0; i < count; ++i) {
      waitables[count - (1 + i)].first->kernel_->lock_.Release();
    }

    for (;;) {
      if (sw.fired())
        break;

      sw.cv()->Wait();
    }
  sw.lock()->Release();

  WaitableEvent *const signaled_event = sw.signaling_event();

  size_t signaled_index = 0;


  for (size_t i = 0; i < count; ++i) {
    if (raw_waitables[i] != signaled_event) {
      raw_waitables[i]->kernel_->lock_.Acquire();



        raw_waitables[i]->kernel_->Dequeue(&sw, &sw);
      raw_waitables[i]->kernel_->lock_.Release();
    } else {



      raw_waitables[i]->kernel_->lock_.Acquire();
      raw_waitables[i]->kernel_->lock_.Release();
      signaled_index = i;
    }
  }

  return signaled_index;
}

// If return value == count:
//   The locks of the WaitableEvents have been taken in order and the Waiter has
//   been enqueued in the wait-list of each. None of the WaitableEvents are
//   currently signaled
// else:
//   None of the WaitableEvent locks are held. The Waiter has not been enqueued
//   in any of them and the return value is the index of the WaitableEvent which
//   was signaled with the lowest input index from the original WaitMany call.
// -----------------------------------------------------------------------------
// static
// NO_THREAD_SAFETY_ANALYSIS: Complex control flow.
size_t WaitableEvent::EnqueueMany(std::pair<WaitableEvent*, size_t>* waitables,
                                  size_t count,
                                  Waiter* waiter) NO_THREAD_SAFETY_ANALYSIS {
  size_t winner = count;
  size_t winner_index = count;
  for (size_t i = 0; i < count; ++i) {
    auto& kernel = waitables[i].first->kernel_;
    kernel->lock_.Acquire();
    if (kernel->signaled_ && waitables[i].second < winner) {
      winner = waitables[i].second;
      winner_index = i;
    }
  }


  if (winner == count) {
    for (size_t i = 0; i < count; ++i)
      waitables[i].first->Enqueue(waiter);
    return count;
  }


  for (auto* w = waitables + count - 1; w >= waitables; --w) {
    auto& kernel = w->first->kernel_;
    if (w->second == winner) {
      if (!kernel->manual_reset_)
        kernel->signaled_ = false;
    }
    kernel->lock_.Release();
  }

  return winner_index;
}


// Private functions...

WaitableEvent::WaitableEventKernel::WaitableEventKernel(
    ResetPolicy reset_policy,
    InitialState initial_state)
    : manual_reset_(reset_policy == ResetPolicy::MANUAL),
      signaled_(initial_state == InitialState::SIGNALED) {}

WaitableEvent::WaitableEventKernel::~WaitableEventKernel() = default;

// Wake all waiting waiters. Called with lock held.
// -----------------------------------------------------------------------------
bool WaitableEvent::SignalAll() {
  bool signaled_at_least_one = false;

  for (auto* i : kernel_->waiters_) {
    if (i->Fire(this))
      signaled_at_least_one = true;
  }

  kernel_->waiters_.clear();
  return signaled_at_least_one;
}

// Try to wake a single waiter. Return true if one was woken. Called with lock
// held.
// ---------------------------------------------------------------------------
bool WaitableEvent::SignalOne() {
  for (;;) {
    if (kernel_->waiters_.empty())
      return false;

    const bool r = (*kernel_->waiters_.begin())->Fire(this);
    kernel_->waiters_.pop_front();
    if (r)
      return true;
  }
}

// Add a waiter to the list of those waiting. Called with lock held.
// -----------------------------------------------------------------------------
void WaitableEvent::Enqueue(Waiter* waiter) {
  kernel_->waiters_.push_back(waiter);
}

// Remove a waiter from the list of those waiting. Return true if the waiter was
// actually removed. Called with lock held.
// -----------------------------------------------------------------------------
bool WaitableEvent::WaitableEventKernel::Dequeue(Waiter* waiter, void* tag) {
  for (auto i = waiters_.begin(); i != waiters_.end(); ++i) {
    if (*i == waiter && (*i)->Compare(tag)) {
      waiters_.erase(i);
      return true;
    }
  }

  return false;
}


}  // namespace base
