// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/message_loop/message_pump_win.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "base/bind.h"
#include "base/debug/alias.h"
#include "base/metrics/histogram_macros.h"
#include "base/numerics/ranges.h"
#include "base/numerics/safe_conversions.h"
#include "base/trace_event/trace_event.h"

namespace base {

namespace {

enum MessageLoopProblems {
  MESSAGE_POST_ERROR,
  COMPLETION_POST_ERROR,
  SET_TIMER_ERROR,
  RECEIVED_WM_QUIT_ERROR,
  MESSAGE_LOOP_PROBLEM_MAX,
};

// zero and the biggest DWORD value (or INFINITE if |next_task_time.is_max()|).
// Optionally, a recent value of Now() may be passed in to avoid resampling it.
DWORD GetSleepTimeoutMs(TimeTicks next_task_time,
                        TimeTicks recent_now = TimeTicks()) {


  DCHECK(!next_task_time.is_null());

  if (next_task_time.is_max())
    return INFINITE;

  auto now = recent_now.is_null() ? TimeTicks::Now() : recent_now;
  auto timeout_ms = (next_task_time - now).InMillisecondsRoundedUp();


  static_assert(!std::is_signed<DWORD>::value, "DWORD is unexpectedly signed");
  return saturated_cast<DWORD>(timeout_ms);
}

}  // namespace

// task (a series of such messages creates a continuous task pump).
static const int kMsgHaveWork = WM_USER + 1;

// MessagePumpWin public:

MessagePumpWin::MessagePumpWin() = default;
MessagePumpWin::~MessagePumpWin() = default;

void MessagePumpWin::Run(Delegate* delegate) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  RunState s;
  s.delegate = delegate;
  s.should_quit = false;
  s.run_depth = state_ ? state_->run_depth + 1 : 1;

  RunState* previous_state = state_;
  state_ = &s;

  DoRunLoop();

  state_ = previous_state;
}

void MessagePumpWin::Quit() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  DCHECK(state_);
  state_->should_quit = true;
}

// MessagePumpForUI public:

MessagePumpForUI::MessagePumpForUI() {
  bool succeeded = message_window_.Create(
      BindRepeating(&MessagePumpForUI::MessageCallback, Unretained(this)));
  DCHECK(succeeded);
}

MessagePumpForUI::~MessagePumpForUI() = default;

void MessagePumpForUI::ScheduleWork() {



  bool not_scheduled = false;
  if (!work_scheduled_.compare_exchange_strong(not_scheduled, true))
    return;  // Someone else continued the pumping.

  BOOL ret = PostMessage(message_window_.hwnd(), kMsgHaveWork, 0, 0);
  if (ret)
    return;  // There was room in the Window Message queue.









  work_scheduled_ = false;
  UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopProblem", MESSAGE_POST_ERROR,
                            MESSAGE_LOOP_PROBLEM_MAX);
}

void MessagePumpForUI::ScheduleDelayedWork(const TimeTicks& delayed_work_time) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);















  if (in_native_loop_ && !work_scheduled_) {


    ScheduleNativeTimer({delayed_work_time, TimeTicks::Now()});
  }
}

void MessagePumpForUI::EnableWmQuit() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);
  enable_wm_quit_ = true;
}

void MessagePumpForUI::AddObserver(Observer* observer) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);
  observers_.AddObserver(observer);
}

void MessagePumpForUI::RemoveObserver(Observer* observer) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);
  observers_.RemoveObserver(observer);
}

// MessagePumpForUI private:

bool MessagePumpForUI::MessageCallback(
    UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);
  switch (message) {
    case kMsgHaveWork:
      HandleWorkMessage();
      break;
    case WM_TIMER:
      if (wparam == reinterpret_cast<UINT_PTR>(this))
        HandleTimerMessage();
      break;
  }
  return false;
}

void MessagePumpForUI::DoRunLoop() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);












  for (;;) {









    in_native_loop_ = false;
    state_->delegate->BeforeDoInternalWork();
    DCHECK(!in_native_loop_);

    bool more_work_is_plausible = ProcessNextWindowsMessage();
    in_native_loop_ = false;
    if (state_->should_quit)
      break;

    Delegate::NextWorkInfo next_work_info = state_->delegate->DoSomeWork();
    in_native_loop_ = false;
    more_work_is_plausible |= next_work_info.is_immediate();
    if (state_->should_quit)
      break;

    if (installed_native_timer_) {




      KillNativeTimer();
    }

    if (more_work_is_plausible)
      continue;

    more_work_is_plausible = state_->delegate->DoIdleWork();


    DCHECK(!in_native_loop_);
    DCHECK(!installed_native_timer_);
    if (state_->should_quit)
      break;

    if (more_work_is_plausible)
      continue;

    state_->delegate->BeforeWait();
    WaitForWork(next_work_info);
  }
}

void MessagePumpForUI::WaitForWork(Delegate::NextWorkInfo next_work_info) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);


  DWORD wait_flags = MWMO_INPUTAVAILABLE;
  for (DWORD delay = GetSleepTimeoutMs(next_work_info.delayed_run_time,
                                       next_work_info.recent_now);
       delay != 0; delay = GetSleepTimeoutMs(next_work_info.delayed_run_time)) {

    base::debug::Alias(&delay);
    base::debug::Alias(&wait_flags);
    DWORD result = MsgWaitForMultipleObjectsEx(0, nullptr, delay, QS_ALLINPUT,
                                               wait_flags);

    if (WAIT_OBJECT_0 == result) {













      MSG msg = {0};
      bool has_pending_sent_message =
          (HIWORD(::GetQueueStatus(QS_SENDMESSAGE)) & QS_SENDMESSAGE) != 0;
      if (has_pending_sent_message ||
          ::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
        return;
      }



      wait_flags = 0;
    }

    DCHECK_NE(WAIT_FAILED, result) << GetLastError();
  }
}

void MessagePumpForUI::HandleWorkMessage() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);


  in_native_loop_ = true;



  if (!state_) {

    work_scheduled_ = false;
    return;
  }



  ProcessPumpReplacementMessage();

  Delegate::NextWorkInfo next_work_info = state_->delegate->DoSomeWork();
  if (next_work_info.is_immediate()) {
    ScheduleWork();
  } else {
    ScheduleNativeTimer(next_work_info);
  }
}

void MessagePumpForUI::HandleTimerMessage() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);







  if (!installed_native_timer_)
    return;


  KillNativeTimer();



  if (!state_)
    return;

  Delegate::NextWorkInfo next_work_info = state_->delegate->DoSomeWork();
  if (next_work_info.is_immediate()) {
    ScheduleWork();
  } else {
    ScheduleNativeTimer(next_work_info);
  }
}

void MessagePumpForUI::ScheduleNativeTimer(
    Delegate::NextWorkInfo next_work_info) {
  DCHECK(!next_work_info.is_immediate());
  DCHECK(in_native_loop_);






  if (installed_native_timer_ &&
      *installed_native_timer_ == next_work_info.delayed_run_time) {
    return;
  }

  if (next_work_info.delayed_run_time.is_max())
    return;



















  UINT delay_msec = strict_cast<UINT>(GetSleepTimeoutMs(
      next_work_info.delayed_run_time, next_work_info.recent_now));
  if (delay_msec == 0) {
    ScheduleWork();
  } else {


    delay_msec = ClampToRange(delay_msec, UINT(USER_TIMER_MINIMUM),
                              UINT(USER_TIMER_MAXIMUM));

    base::debug::Alias(&delay_msec);
    UINT_PTR ret =
        ::SetTimer(message_window_.hwnd(), reinterpret_cast<UINT_PTR>(this),
                   delay_msec, nullptr);
    installed_native_timer_ = next_work_info.delayed_run_time;

    if (ret)
      return;



    UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopProblem", SET_TIMER_ERROR,
                              MESSAGE_LOOP_PROBLEM_MAX);
  }
}

void MessagePumpForUI::KillNativeTimer() {
  DCHECK(installed_native_timer_);
  const bool success =
      ::KillTimer(message_window_.hwnd(), reinterpret_cast<UINT_PTR>(this));
  DPCHECK(success);
  installed_native_timer_.reset();
}

bool MessagePumpForUI::ProcessNextWindowsMessage() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);




  bool sent_messages_in_queue = false;
  DWORD queue_status = ::GetQueueStatus(QS_SENDMESSAGE);
  if (HIWORD(queue_status) & QS_SENDMESSAGE)
    sent_messages_in_queue = true;

  MSG msg;
  if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != FALSE)
    return ProcessMessageHelper(msg);

  return sent_messages_in_queue;
}

bool MessagePumpForUI::ProcessMessageHelper(const MSG& msg) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  TRACE_EVENT1("base,toplevel", "MessagePumpForUI::ProcessMessageHelper",
               "message", msg.message);
  if (WM_QUIT == msg.message) {




    if (enable_wm_quit_) {
      state_->should_quit = true;
      return false;
    }
    UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopProblem",
                              RECEIVED_WM_QUIT_ERROR, MESSAGE_LOOP_PROBLEM_MAX);
    return true;
  }

  if (msg.message == kMsgHaveWork && msg.hwnd == message_window_.hwnd())
    return ProcessPumpReplacementMessage();

  for (Observer& observer : observers_)
    observer.WillDispatchMSG(msg);
  ::TranslateMessage(&msg);
  ::DispatchMessage(&msg);
  for (Observer& observer : observers_)
    observer.DidDispatchMSG(msg);

  return true;
}

bool MessagePumpForUI::ProcessPumpReplacementMessage() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);









  MSG msg;
  const bool have_message =
      ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != FALSE;

  DCHECK(!have_message || kMsgHaveWork != msg.message ||
         msg.hwnd != message_window_.hwnd());

  DCHECK(work_scheduled_);
  work_scheduled_ = false;

  if (!have_message)
    return false;

  if (WM_QUIT == msg.message) {




    ::PostQuitMessage(static_cast<int>(msg.wParam));











    return true;
  }




  ScheduleWork();
  return ProcessMessageHelper(msg);
}

// MessagePumpForIO public:

MessagePumpForIO::IOContext::IOContext() {
  memset(&overlapped, 0, sizeof(overlapped));
}

MessagePumpForIO::MessagePumpForIO() {
  port_.Set(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr,
                                     reinterpret_cast<ULONG_PTR>(nullptr), 1));
  DCHECK(port_.IsValid());
}

MessagePumpForIO::~MessagePumpForIO() = default;

void MessagePumpForIO::ScheduleWork() {



  bool not_scheduled = false;
  if (!work_scheduled_.compare_exchange_strong(not_scheduled, true))
    return;  // Someone else continued the pumping.

  BOOL ret = ::PostQueuedCompletionStatus(port_.Get(), 0,
                                          reinterpret_cast<ULONG_PTR>(this),
                                          reinterpret_cast<OVERLAPPED*>(this));
  if (ret)
    return;  // Post worked perfectly.


  work_scheduled_ = false;  // Clarify that we didn't succeed.
  UMA_HISTOGRAM_ENUMERATION("Chrome.MessageLoopProblem", COMPLETION_POST_ERROR,
                            MESSAGE_LOOP_PROBLEM_MAX);
}

void MessagePumpForIO::ScheduleDelayedWork(const TimeTicks& delayed_work_time) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);



}

HRESULT MessagePumpForIO::RegisterIOHandler(HANDLE file_handle,
                                            IOHandler* handler) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  HANDLE port = ::CreateIoCompletionPort(
      file_handle, port_.Get(), reinterpret_cast<ULONG_PTR>(handler), 1);
  return (port != nullptr) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

bool MessagePumpForIO::RegisterJobObject(HANDLE job_handle,
                                         IOHandler* handler) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  JOBOBJECT_ASSOCIATE_COMPLETION_PORT info;
  info.CompletionKey = handler;
  info.CompletionPort = port_.Get();
  return ::SetInformationJobObject(job_handle,
                                   JobObjectAssociateCompletionPortInformation,
                                   &info, sizeof(info)) != FALSE;
}

// MessagePumpForIO private:

void MessagePumpForIO::DoRunLoop() {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  for (;;) {









    Delegate::NextWorkInfo next_work_info = state_->delegate->DoSomeWork();
    bool more_work_is_plausible = next_work_info.is_immediate();
    if (state_->should_quit)
      break;

    state_->delegate->BeforeWait();
    more_work_is_plausible |= WaitForIOCompletion(0, nullptr);
    if (state_->should_quit)
      break;

    if (more_work_is_plausible)
      continue;

    more_work_is_plausible = state_->delegate->DoIdleWork();
    if (state_->should_quit)
      break;

    if (more_work_is_plausible)
      continue;

    state_->delegate->BeforeWait();
    WaitForWork(next_work_info);
  }
}

// the next set of timers.
void MessagePumpForIO::WaitForWork(Delegate::NextWorkInfo next_work_info) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);


  DCHECK_EQ(1, state_->run_depth) << "Cannot nest an IO message loop!";

  DWORD timeout = GetSleepTimeoutMs(next_work_info.delayed_run_time,
                                    next_work_info.recent_now);

  base::debug::Alias(&timeout);
  WaitForIOCompletion(timeout, nullptr);
}

bool MessagePumpForIO::WaitForIOCompletion(DWORD timeout, IOHandler* filter) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  IOItem item;
  if (completed_io_.empty() || !MatchCompletedIOItem(filter, &item)) {

    if (!GetIOItem(timeout, &item))
      return false;

    if (ProcessInternalIOItem(item))
      return true;
  }

  if (filter && item.handler != filter) {

    completed_io_.push_back(item);
  } else {
    item.handler->OnIOCompleted(item.context, item.bytes_transfered,
                                item.error);
  }
  return true;
}

bool MessagePumpForIO::GetIOItem(DWORD timeout, IOItem* item) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  memset(item, 0, sizeof(*item));
  ULONG_PTR key = reinterpret_cast<ULONG_PTR>(nullptr);
  OVERLAPPED* overlapped = nullptr;
  if (!::GetQueuedCompletionStatus(port_.Get(), &item->bytes_transfered, &key,
                                   &overlapped, timeout)) {
    if (!overlapped)
      return false;  // Nothing in the queue.
    item->error = GetLastError();
    item->bytes_transfered = 0;
  }

  item->handler = reinterpret_cast<IOHandler*>(key);
  item->context = reinterpret_cast<IOContext*>(overlapped);
  return true;
}

bool MessagePumpForIO::ProcessInternalIOItem(const IOItem& item) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(item.context) &&
      reinterpret_cast<void*>(this) == reinterpret_cast<void*>(item.handler)) {

    DCHECK(!item.bytes_transfered);
    work_scheduled_ = false;
    return true;
  }
  return false;
}

bool MessagePumpForIO::MatchCompletedIOItem(IOHandler* filter, IOItem* item) {
  DCHECK_CALLED_ON_VALID_THREAD(bound_thread_);

  DCHECK(!completed_io_.empty());
  for (std::list<IOItem>::iterator it = completed_io_.begin();
       it != completed_io_.end(); ++it) {
    if (!filter || it->handler == filter) {
      *item = *it;
      completed_io_.erase(it);
      return true;
    }
  }
  return false;
}

}  // namespace base
