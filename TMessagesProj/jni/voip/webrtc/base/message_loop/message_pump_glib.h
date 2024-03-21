// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_GLIB_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_GLIB_H_

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/watchable_io_message_pump_posix.h"
#include "base/observer_list.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"

typedef struct _GMainContext GMainContext;
typedef struct _GPollFD GPollFD;
typedef struct _GSource GSource;

namespace base {

// platforms using GLib.
class BASE_EXPORT MessagePumpGlib : public MessagePump,
                                    public WatchableIOMessagePumpPosix {
 public:
  class FdWatchController : public FdWatchControllerInterface {
   public:
    explicit FdWatchController(const Location& from_here);
    ~FdWatchController() override;

    bool StopWatchingFileDescriptor() override;

   private:
    friend class MessagePumpGlib;
    friend class MessagePumpGLibFdWatchTest;







    bool InitOrUpdate(int fd, int mode, FdWatcher* watcher);

    bool IsInitialized() const;



    bool Attach(MessagePumpGlib* pump);



    void NotifyCanRead();
    void NotifyCanWrite();

    FdWatcher* watcher_ = nullptr;
    GSource* source_ = nullptr;
    std::unique_ptr<GPollFD> poll_fd_;


    bool* was_destroyed_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(FdWatchController);
  };

  MessagePumpGlib();
  ~MessagePumpGlib() override;


  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           int mode,
                           FdWatchController* controller,
                           FdWatcher* delegate);






  int HandlePrepare();
  bool HandleCheck();
  void HandleDispatch();

  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;



  bool HandleFdWatchCheck(FdWatchController* controller);
  void HandleFdWatchDispatch(FdWatchController* controller);

 private:
  bool ShouldQuit() const;


  struct RunState;

  RunState* state_;



  GMainContext* context_;


  GSource* work_source_;




  int wakeup_pipe_read_;
  int wakeup_pipe_write_;

  std::unique_ptr<GPollFD> wakeup_gpollfd_;

  THREAD_CHECKER(watch_fd_caller_checker_);

  DISALLOW_COPY_AND_ASSIGN(MessagePumpGlib);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_GLIB_H_
