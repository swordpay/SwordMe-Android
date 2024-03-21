// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_WIN_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_WIN_H_

#include <windows.h>

#include <atomic>
#include <list>
#include <memory>

#include "base/base_export.h"
#include "base/message_loop/message_pump.h"
#include "base/observer_list.h"
#include "base/optional.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "base/win/message_window.h"
#include "base/win/scoped_handle.h"

namespace base {

// for Windows. It provides basic functionality like handling of observers and
// controlling the lifetime of the message pump.
class BASE_EXPORT MessagePumpWin : public MessagePump {
 public:
  MessagePumpWin();
  ~MessagePumpWin() override;

  void Run(Delegate* delegate) override;
  void Quit() override;

 protected:
  struct RunState {
    Delegate* delegate;

    bool should_quit;

    int run_depth;
  };

  virtual void DoRunLoop() = 0;





















  std::atomic_bool work_scheduled_{false};

  RunState* state_ = nullptr;

  THREAD_CHECKER(bound_thread_);
};

// MessagePumpForUI extends MessagePumpWin with methods that are particular to a
// MessageLoop instantiated with TYPE_UI.
//
// MessagePumpForUI implements a "traditional" Windows message pump. It contains
// a nearly infinite loop that peeks out messages, and then dispatches them.
// Intermixed with those peeks are callouts to DoSomeWork. When there are no
// events to be serviced, this pump goes into a wait state. In most cases, this
// message pump handles all processing.
//
// However, when a task, or windows event, invokes on the stack a native dialog
// box or such, that window typically provides a bare bones (native?) message
// pump.  That bare-bones message pump generally supports little more than a
// peek of the Windows message queue, followed by a dispatch of the peeked
// message.  MessageLoop extends that bare-bones message pump to also service
// Tasks, at the cost of some complexity.
//
// The basic structure of the extension (referred to as a sub-pump) is that a
// special message, kMsgHaveWork, is repeatedly injected into the Windows
// Message queue.  Each time the kMsgHaveWork message is peeked, checks are made
// for an extended set of events, including the availability of Tasks to run.
//
// After running a task, the special message kMsgHaveWork is again posted to the
// Windows Message queue, ensuring a future time slice for processing a future
// event.  To prevent flooding the Windows Message queue, care is taken to be
// sure that at most one kMsgHaveWork message is EVER pending in the Window's
// Message queue.
//
// There are a few additional complexities in this system where, when there are
// no Tasks to run, this otherwise infinite stream of messages which drives the
// sub-pump is halted.  The pump is automatically re-started when Tasks are
// queued.
//
// A second complexity is that the presence of this stream of posted tasks may
// prevent a bare-bones message pump from ever peeking a WM_PAINT or WM_TIMER.
// Such paint and timer events always give priority to a posted message, such as
// kMsgHaveWork messages.  As a result, care is taken to do some peeking in
// between the posting of each kMsgHaveWork message (i.e., after kMsgHaveWork is
// peeked, and before a replacement kMsgHaveWork is posted).
//
// NOTE: Although it may seem odd that messages are used to start and stop this
// flow (as opposed to signaling objects, etc.), it should be understood that
// the native message pump will *only* respond to messages.  As a result, it is
// an excellent choice.  It is also helpful that the starter messages that are
// placed in the queue when new task arrive also awakens DoRunLoop.
//
class BASE_EXPORT MessagePumpForUI : public MessagePumpWin {
 public:
  MessagePumpForUI();
  ~MessagePumpForUI() override;

  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

  void EnableWmQuit();


  class BASE_EXPORT Observer {
   public:
    virtual void WillDispatchMSG(const MSG& msg) = 0;
    virtual void DidDispatchMSG(const MSG& msg) = 0;
  };

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* obseerver);

 private:
  bool MessageCallback(UINT message,
                       WPARAM wparam,
                       LPARAM lparam,
                       LRESULT* result);
  void DoRunLoop() override;
  void WaitForWork(Delegate::NextWorkInfo next_work_info);
  void HandleWorkMessage();
  void HandleTimerMessage();
  void ScheduleNativeTimer(Delegate::NextWorkInfo next_work_info);
  void KillNativeTimer();
  bool ProcessNextWindowsMessage();
  bool ProcessMessageHelper(const MSG& msg);
  bool ProcessPumpReplacementMessage();

  base::win::MessageWindow message_window_;


  bool enable_wm_quit_ = false;



  Optional<TimeTicks> installed_native_timer_;



  bool in_native_loop_ = false;

  ObserverList<Observer>::Unchecked observers_;
};

// MessagePumpForIO extends MessagePumpWin with methods that are particular to a
// MessageLoop instantiated with TYPE_IO. This version of MessagePump does not
// deal with Windows mesagges, and instead has a Run loop based on Completion
// Ports so it is better suited for IO operations.
//
class BASE_EXPORT MessagePumpForIO : public MessagePumpWin {
 public:
  struct BASE_EXPORT IOContext {
    IOContext();
    OVERLAPPED overlapped;
  };




































  class IOHandler {
   public:
    virtual ~IOHandler() {}




    virtual void OnIOCompleted(IOContext* context,
                               DWORD bytes_transfered,
                               DWORD error) = 0;
  };

  MessagePumpForIO();
  ~MessagePumpForIO() override;

  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;



  HRESULT RegisterIOHandler(HANDLE file_handle, IOHandler* handler);




  bool RegisterJobObject(HANDLE job_handle, IOHandler* handler);









  bool WaitForIOCompletion(DWORD timeout, IOHandler* filter);

 private:
  struct IOItem {
    IOHandler* handler;
    IOContext* context;
    DWORD bytes_transfered;
    DWORD error;
  };

  void DoRunLoop() override;
  void WaitForWork(Delegate::NextWorkInfo next_work_info);
  bool MatchCompletedIOItem(IOHandler* filter, IOItem* item);
  bool GetIOItem(DWORD timeout, IOItem* item);
  bool ProcessInternalIOItem(const IOItem& item);

  win::ScopedHandle port_;


  std::list<IOItem> completed_io_;
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_WIN_H_
