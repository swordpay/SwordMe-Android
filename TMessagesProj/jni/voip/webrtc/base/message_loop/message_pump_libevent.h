// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_LIBEVENT_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_LIBEVENT_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/watchable_io_message_pump_posix.h"
#include "base/threading/thread_checker.h"

struct event_base;
struct event;

namespace base {

// TODO(dkegel): add support for background file IO somehow
class BASE_EXPORT MessagePumpLibevent : public MessagePump,
                                        public WatchableIOMessagePumpPosix {
 public:
  class FdWatchController : public FdWatchControllerInterface {
   public:
    explicit FdWatchController(const Location& from_here);

    ~FdWatchController() override;

    bool StopWatchingFileDescriptor() override;

   private:
    friend class MessagePumpLibevent;
    friend class MessagePumpLibeventTest;

    void Init(std::unique_ptr<event> e);

    std::unique_ptr<event> ReleaseEvent();

    void set_pump(MessagePumpLibevent* pump) { pump_ = pump; }
    MessagePumpLibevent* pump() const { return pump_; }

    void set_watcher(FdWatcher* watcher) { watcher_ = watcher; }

    void OnFileCanReadWithoutBlocking(int fd, MessagePumpLibevent* pump);
    void OnFileCanWriteWithoutBlocking(int fd, MessagePumpLibevent* pump);

    std::unique_ptr<event> event_;
    MessagePumpLibevent* pump_ = nullptr;
    FdWatcher* watcher_ = nullptr;


    bool* was_destroyed_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(FdWatchController);
  };

  MessagePumpLibevent();
  ~MessagePumpLibevent() override;

  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           int mode,
                           FdWatchController* controller,
                           FdWatcher* delegate);

  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

 private:
  friend class MessagePumpLibeventTest;

  bool Init();

  static void OnLibeventNotification(int fd, short flags, void* context);


  static void OnWakeup(int socket, short flags, void* context);

  bool keep_running_;

  bool in_run_;

  bool processed_io_events_;


  event_base* event_base_;

  int wakeup_pipe_in_;

  int wakeup_pipe_out_;

  event* wakeup_event_;

  ThreadChecker watch_file_descriptor_caller_checker_;
  DISALLOW_COPY_AND_ASSIGN(MessagePumpLibevent);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_LIBEVENT_H_
