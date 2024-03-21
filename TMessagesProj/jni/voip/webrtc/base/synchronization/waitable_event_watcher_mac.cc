// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/waitable_event_watcher.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

WaitableEventWatcher::WaitableEventWatcher() : weak_ptr_factory_(this) {}

WaitableEventWatcher::~WaitableEventWatcher() {
  StopWatching();
}

bool WaitableEventWatcher::StartWatching(
    WaitableEvent* event,
    EventCallback callback,
    scoped_refptr<SequencedTaskRunner> task_runner) {
  DCHECK(task_runner->RunsTasksInCurrentSequence());
  DCHECK(!source_ || dispatch_source_testcancel(source_));


  receive_right_ = event->receive_right_;

  callback_ = BindOnce(std::move(callback), event);



  WeakPtr<WaitableEventWatcher> weak_this = weak_ptr_factory_.GetWeakPtr();
  const bool auto_reset =
      event->policy_ == WaitableEvent::ResetPolicy::AUTOMATIC;



  if (!WaitableEvent::UseSlowWatchList(event->policy_)) {


    source_.reset(dispatch_source_create(
        DISPATCH_SOURCE_TYPE_MACH_RECV, receive_right_->Name(), 0,
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)));

    dispatch_source_t source = source_.get();
    mach_port_t name = receive_right_->Name();

    dispatch_source_set_event_handler(source_, ^{



      if (auto_reset && !WaitableEvent::PeekPort(name, true)) {
        return;
      }


      dispatch_source_cancel(source);

      task_runner->PostTask(
          FROM_HERE,
          BindOnce(&WaitableEventWatcher::InvokeCallback, weak_this));
    });
    dispatch_resume(source_);
  } else {


    OnceClosure watcher =
        BindOnce(IgnoreResult(&TaskRunner::PostTask), task_runner, FROM_HERE,
                 BindOnce(&WaitableEventWatcher::InvokeCallback, weak_this));





    scoped_refptr<WaitableEvent::ReceiveRight> receive_right(receive_right_);
    AutoLock lock(receive_right->SlowWatchList()->lock);
    if (event->IsSignaled()) {
      std::move(watcher).Run();
      return true;
    }
    receive_right_->SlowWatchList()->list.push_back(std::move(watcher));
  }

  return true;
}

void WaitableEventWatcher::StopWatching() {
  callback_.Reset();
  receive_right_ = nullptr;
  if (source_) {
    dispatch_source_cancel(source_);
    source_.reset();
  }
}

void WaitableEventWatcher::InvokeCallback() {


  if (callback_.is_null())
    return;
  source_.reset();
  receive_right_ = nullptr;
  std::move(callback_).Run();
}

}  // namespace base
