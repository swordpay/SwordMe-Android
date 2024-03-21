// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/waitable_event_watcher.h"

#include <utility>

#include "base/bind.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

// WaitableEventWatcher (async waits).
//
// The basic design is that we add an AsyncWaiter to the wait-list of the event.
// That AsyncWaiter has a pointer to SequencedTaskRunner, and a Task to be
// posted to it. The task ends up calling the callback when it runs on the
// sequence.
//
// Since the wait can be canceled, we have a thread-safe Flag object which is
// set when the wait has been canceled. At each stage in the above, we check the
// flag before going onto the next stage. Since the wait may only be canceled in
// the sequence which runs the Task, we are assured that the callback cannot be
// called after canceling...

// A thread-safe, reference-counted, write-once flag.
// -----------------------------------------------------------------------------
class Flag : public RefCountedThreadSafe<Flag> {
 public:
  Flag() { flag_ = false; }

  void Set() {
    AutoLock locked(lock_);
    flag_ = true;
  }

  bool value() const {
    AutoLock locked(lock_);
    return flag_;
  }

 private:
  friend class RefCountedThreadSafe<Flag>;
  ~Flag() = default;

  mutable Lock lock_;
  bool flag_;

  DISALLOW_COPY_AND_ASSIGN(Flag);
};

// This is an asynchronous waiter which posts a task to a SequencedTaskRunner
// when fired. An AsyncWaiter may only be in a single wait-list.
// -----------------------------------------------------------------------------
class AsyncWaiter : public WaitableEvent::Waiter {
 public:
  AsyncWaiter(scoped_refptr<SequencedTaskRunner> task_runner,
              base::OnceClosure callback,
              Flag* flag)
      : task_runner_(std::move(task_runner)),
        callback_(std::move(callback)),
        flag_(flag) {}

  bool Fire(WaitableEvent* event) override {

    if (!flag_->value())
      task_runner_->PostTask(FROM_HERE, std::move(callback_));


    delete this;


    return true;
  }

  bool Compare(void* tag) override { return tag == flag_.get(); }

 private:
  const scoped_refptr<SequencedTaskRunner> task_runner_;
  base::OnceClosure callback_;
  const scoped_refptr<Flag> flag_;
};

// For async waits we need to run a callback on a sequence. We do this by
// posting an AsyncCallbackHelper task, which calls the callback and keeps track
// of when the event is canceled.
// -----------------------------------------------------------------------------
void AsyncCallbackHelper(Flag* flag,
                         WaitableEventWatcher::EventCallback callback,
                         WaitableEvent* event) {

  if (!flag->value()) {

    flag->Set();
    std::move(callback).Run(event);
  }
}

WaitableEventWatcher::WaitableEventWatcher() {
  sequence_checker_.DetachFromSequence();
}

WaitableEventWatcher::~WaitableEventWatcher() {



  if (cancel_flag_ && !cancel_flag_->value())
    StopWatching();
}

// The Handle is how the user cancels a wait. After deleting the Handle we
// insure that the delegate cannot be called.
// -----------------------------------------------------------------------------
bool WaitableEventWatcher::StartWatching(
    WaitableEvent* event,
    EventCallback callback,
    scoped_refptr<SequencedTaskRunner> task_runner) {
  DCHECK(sequence_checker_.CalledOnValidSequence());



  if (cancel_flag_.get() && cancel_flag_->value())
    cancel_flag_ = nullptr;

  DCHECK(!cancel_flag_) << "StartWatching called while still watching";

  cancel_flag_ = new Flag;
  OnceClosure internal_callback =
      base::BindOnce(&AsyncCallbackHelper, base::RetainedRef(cancel_flag_),
                     std::move(callback), event);
  WaitableEvent::WaitableEventKernel* kernel = event->kernel_.get();

  AutoLock locked(kernel->lock_);

  if (kernel->signaled_) {
    if (!kernel->manual_reset_)
      kernel->signaled_ = false;


    task_runner->PostTask(FROM_HERE, std::move(internal_callback));
    return true;
  }

  kernel_ = kernel;
  waiter_ = new AsyncWaiter(std::move(task_runner),
                            std::move(internal_callback), cancel_flag_.get());
  event->Enqueue(waiter_);

  return true;
}

void WaitableEventWatcher::StopWatching() {
  DCHECK(sequence_checker_.CalledOnValidSequence());

  if (!cancel_flag_.get())  // if not currently watching...
    return;

  if (cancel_flag_->value()) {


    cancel_flag_ = nullptr;
    return;
  }

  if (!kernel_.get()) {








    cancel_flag_->Set();
    cancel_flag_ = nullptr;
    return;
  }

  AutoLock locked(kernel_->lock_);














  if (kernel_->Dequeue(waiter_, cancel_flag_.get())) {




    delete waiter_;
    cancel_flag_ = nullptr;
    return;
  }




  cancel_flag_->Set();
  cancel_flag_ = nullptr;







}

}  // namespace base
