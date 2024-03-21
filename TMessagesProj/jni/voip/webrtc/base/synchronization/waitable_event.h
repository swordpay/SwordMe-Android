// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYNCHRONIZATION_WAITABLE_EVENT_H_
#define BASE_SYNCHRONIZATION_WAITABLE_EVENT_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/macros.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/scoped_handle.h"
#elif defined(OS_MACOSX)
#include <mach/mach.h>

#include <list>
#include <memory>

#include "base/callback_forward.h"
#include "base/mac/scoped_mach_port.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <list>
#include <utility>

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#endif

namespace base {

class TimeDelta;

// allow one thread to wait for another thread to finish some work. For
// non-Windows systems, this can only be used from within a single address
// space.
//
// Use a WaitableEvent when you would otherwise use a Lock+ConditionVariable to
// protect a simple boolean value.  However, if you find yourself using a
// WaitableEvent in conjunction with a Lock to wait for a more complex state
// change (e.g., for an item to be added to a queue), then you should probably
// be using a ConditionVariable instead of a WaitableEvent.
//
// NOTE: On Windows, this class provides a subset of the functionality afforded
// by a Windows event object.  This is intentional.  If you are writing Windows
// specific code and you need other features of a Windows event, then you might
// be better off just using an Windows event directly.
class BASE_EXPORT WaitableEvent {
 public:



  enum class ResetPolicy { MANUAL, AUTOMATIC };


  enum class InitialState { SIGNALED, NOT_SIGNALED };


  WaitableEvent(ResetPolicy reset_policy = ResetPolicy::MANUAL,
                InitialState initial_state = InitialState::NOT_SIGNALED);

#if defined(OS_WIN)



  explicit WaitableEvent(win::ScopedHandle event_handle);
#endif

  ~WaitableEvent();

  void Reset();


  void Signal();


  bool IsSignaled();








  void Wait();






  bool TimedWait(const TimeDelta& wait_delta);

#if defined(OS_WIN)
  HANDLE handle() const { return handle_.Get(); }
#endif






  void declare_only_used_while_idle() { waiting_is_blocking_ = false; }












  static size_t WaitMany(WaitableEvent** waitables, size_t count);




  class Waiter {
   public:













    virtual bool Fire(WaitableEvent* signaling_event) = 0;




    virtual bool Compare(void* tag) = 0;

   protected:
    virtual ~Waiter() = default;
  };

 private:
  friend class WaitableEventWatcher;

#if defined(OS_WIN)
  win::ScopedHandle handle_;
#elif defined(OS_MACOSX)








  static bool UseSlowWatchList(ResetPolicy policy);




  static bool PeekPort(mach_port_t port, bool dequeue);






  class ReceiveRight : public RefCountedThreadSafe<ReceiveRight> {
   public:
    ReceiveRight(mach_port_t name, bool create_slow_watch_list);

    mach_port_t Name() const { return right_.get(); }


    struct WatchList {
      WatchList();
      ~WatchList();



      Lock lock;
      std::list<OnceClosure> list;
    };

    WatchList* SlowWatchList() const { return slow_watch_list_.get(); }

   private:
    friend class RefCountedThreadSafe<ReceiveRight>;
    ~ReceiveRight();

    mac::ScopedMachReceiveRight right_;


    std::unique_ptr<WatchList> slow_watch_list_;

    DISALLOW_COPY_AND_ASSIGN(ReceiveRight);
  };

  const ResetPolicy policy_;

  scoped_refptr<ReceiveRight> receive_right_;



  mac::ScopedMachSendRight send_right_;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)










  struct WaitableEventKernel :
      public RefCountedThreadSafe<WaitableEventKernel> {
   public:
    WaitableEventKernel(ResetPolicy reset_policy, InitialState initial_state);

    bool Dequeue(Waiter* waiter, void* tag);

    base::Lock lock_;
    const bool manual_reset_;
    bool signaled_;
    std::list<Waiter*> waiters_;

   private:
    friend class RefCountedThreadSafe<WaitableEventKernel>;
    ~WaitableEventKernel();
  };

  typedef std::pair<WaitableEvent*, size_t> WaiterAndIndex;





  static size_t EnqueueMany(WaiterAndIndex* waitables,
                            size_t count, Waiter* waiter);

  bool SignalAll();
  bool SignalOne();
  void Enqueue(Waiter* waiter);

  scoped_refptr<WaitableEventKernel> kernel_;
#endif


  bool waiting_is_blocking_ = true;

  DISALLOW_COPY_AND_ASSIGN(WaitableEvent);
};

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_WAITABLE_EVENT_H_
