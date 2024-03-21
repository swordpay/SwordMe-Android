// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_FUCHSIA_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_FUCHSIA_H_

#include <lib/async/wait.h>

#include "base/base_export.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/watchable_io_message_pump_posix.h"

typedef struct fdio fdio_t;

namespace async {
class Loop;
}  // namespace async

namespace base {

class BASE_EXPORT MessagePumpFuchsia : public MessagePump,
                                       public WatchableIOMessagePumpPosix {
 public:

  class ZxHandleWatcher {
   public:
    virtual void OnZxHandleSignalled(zx_handle_t handle,
                                     zx_signals_t signals) = 0;

   protected:
    virtual ~ZxHandleWatcher() {}
  };

  class ZxHandleWatchController : public async_wait_t {
   public:
    explicit ZxHandleWatchController(const Location& from_here);

    virtual ~ZxHandleWatchController();


    bool StopWatchingZxHandle();

    const Location& created_from_location() { return created_from_location_; }

   protected:
    friend class MessagePumpFuchsia;

    virtual bool WaitBegin();

    static void HandleSignal(async_dispatcher_t* async,
                             async_wait_t* wait,
                             zx_status_t status,
                             const zx_packet_signal_t* signal);

    const Location created_from_location_;






    bool* was_stopped_ = nullptr;

    ZxHandleWatcher* watcher_ = nullptr;

    WeakPtr<MessagePumpFuchsia> weak_pump_;


    bool persistent_ = false;

    DISALLOW_COPY_AND_ASSIGN(ZxHandleWatchController);
  };

  class FdWatchController : public FdWatchControllerInterface,
                            public ZxHandleWatchController,
                            public ZxHandleWatcher {
   public:
    explicit FdWatchController(const Location& from_here);
    ~FdWatchController() override;

    bool StopWatchingFileDescriptor() override;

   private:
    friend class MessagePumpFuchsia;

    bool WaitBegin() override;

    void OnZxHandleSignalled(zx_handle_t handle, zx_signals_t signals) override;

    FdWatcher* watcher_ = nullptr;
    int fd_ = -1;
    uint32_t desired_events_ = 0;

    fdio_t* io_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(FdWatchController);
  };

  enum Mode {
    WATCH_READ = 1 << 0,
    WATCH_WRITE = 1 << 1,
    WATCH_READ_WRITE = WATCH_READ | WATCH_WRITE
  };

  MessagePumpFuchsia();
  ~MessagePumpFuchsia() override;

  bool WatchZxHandle(zx_handle_t handle,
                     bool persistent,
                     zx_signals_t signals,
                     ZxHandleWatchController* controller,
                     ZxHandleWatcher* delegate);
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


  bool HandleIoEventsUntil(zx_time_t deadline);

  bool keep_running_ = true;

  std::unique_ptr<async::Loop> async_loop_;

  base::WeakPtrFactory<MessagePumpFuchsia> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpFuchsia);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_FUCHSIA_H_
