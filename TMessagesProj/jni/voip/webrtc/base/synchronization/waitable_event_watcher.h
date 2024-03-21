// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYNCHRONIZATION_WAITABLE_EVENT_WATCHER_H_
#define BASE_SYNCHRONIZATION_WAITABLE_EVENT_WATCHER_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/sequenced_task_runner.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/object_watcher.h"
#include "base/win/scoped_handle.h"
#elif defined(OS_MACOSX)
#include <dispatch/dispatch.h>

#include "base/mac/scoped_dispatch_object.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/waitable_event.h"
#else
#include "base/sequence_checker.h"
#include "base/synchronization/waitable_event.h"
#endif

#if !defined(OS_WIN)
#include "base/callback.h"
#endif

namespace base {

class Flag;
class AsyncWaiter;
class WaitableEvent;

//
// Each instance of this object can be waiting on a single WaitableEvent. When
// the waitable event is signaled, a callback is invoked on the sequence that
// called StartWatching(). This callback can be deleted by deleting the waiter.
//
// Typical usage:
//
//   class MyClass {
//    public:
//     void DoStuffWhenSignaled(WaitableEvent *waitable_event) {
//       watcher_.StartWatching(waitable_event,
//           base::BindOnce(&MyClass::OnWaitableEventSignaled, this);
//     }
//    private:
//     void OnWaitableEventSignaled(WaitableEvent* waitable_event) {
//       // OK, time to do stuff!
//     }
//     base::WaitableEventWatcher watcher_;
//   };
//
// In the above example, MyClass wants to "do stuff" when waitable_event
// becomes signaled. WaitableEventWatcher makes this task easy. When MyClass
// goes out of scope, the watcher_ will be destroyed, and there is no need to
// worry about OnWaitableEventSignaled being called on a deleted MyClass
// pointer.
//
// BEWARE: With automatically reset WaitableEvents, a signal may be lost if it
// occurs just before a WaitableEventWatcher is deleted. There is currently no
// safe way to stop watching an automatic reset WaitableEvent without possibly
// missing a signal.
//
// NOTE: you /are/ allowed to delete the WaitableEvent while still waiting on
// it with a Watcher. But pay attention: if the event was signaled and deleted
// right after, the callback may be called with deleted WaitableEvent pointer.

class BASE_EXPORT WaitableEventWatcher
#if defined(OS_WIN)
    : public win::ObjectWatcher::Delegate
#endif
{
 public:
  using EventCallback = OnceCallback<void(WaitableEvent*)>;

  WaitableEventWatcher();

#if defined(OS_WIN)
  ~WaitableEventWatcher() override;
#else
  ~WaitableEventWatcher();
#endif



  bool StartWatching(WaitableEvent* event,
                     EventCallback callback,
                     scoped_refptr<SequencedTaskRunner> task_runner);







  void StopWatching();

 private:
#if defined(OS_WIN)
  void OnObjectSignaled(HANDLE h) override;

  win::ScopedHandle duplicated_event_handle_;


  win::ObjectWatcher watcher_;

  EventCallback callback_;
  WaitableEvent* event_ = nullptr;
#elif defined(OS_MACOSX)


  void InvokeCallback();


  OnceClosure callback_;


  scoped_refptr<WaitableEvent::ReceiveRight> receive_right_;



  ScopedDispatchObject<dispatch_source_t> source_;


  WeakPtrFactory<WaitableEventWatcher> weak_ptr_factory_;
#else


  scoped_refptr<Flag> cancel_flag_;

  AsyncWaiter* waiter_ = nullptr;

  scoped_refptr<WaitableEvent::WaitableEventKernel> kernel_;


  SequenceChecker sequence_checker_;
#endif

  DISALLOW_COPY_AND_ASSIGN(WaitableEventWatcher);
};

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_WAITABLE_EVENT_WATCHER_H_
