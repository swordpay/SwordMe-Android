// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_KQUEUE_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_KQUEUE_H_

#include <mach/mach.h>
#include <stdint.h>
#include <sys/event.h>

#include <vector>

#include "base/containers/id_map.h"
#include "base/files/scoped_file.h"
#include "base/location.h"
#include "base/mac/scoped_mach_port.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/watchable_io_message_pump_posix.h"

namespace base {

// capable of watching both POSIX file descriptors and Mach ports.
class BASE_EXPORT MessagePumpKqueue : public MessagePump,
                                      public WatchableIOMessagePumpPosix {
 public:
  class FdWatchController : public FdWatchControllerInterface {
   public:
    explicit FdWatchController(const Location& from_here);
    ~FdWatchController() override;

    bool StopWatchingFileDescriptor() override;

   protected:
    friend class MessagePumpKqueue;

    void Init(WeakPtr<MessagePumpKqueue> pump,
              int fd,
              int mode,
              FdWatcher* watcher);
    void Reset();

    int fd() { return fd_; }
    int mode() { return mode_; }
    FdWatcher* watcher() { return watcher_; }

   private:
    int fd_ = -1;
    int mode_ = 0;
    FdWatcher* watcher_ = nullptr;
    WeakPtr<MessagePumpKqueue> pump_;

    DISALLOW_COPY_AND_ASSIGN(FdWatchController);
  };


  class MachPortWatcher {
   public:
    virtual ~MachPortWatcher() {}
    virtual void OnMachMessageReceived(mach_port_t port) = 0;
  };


  class MachPortWatchController {
   public:
    explicit MachPortWatchController(const Location& from_here);
    ~MachPortWatchController();

    bool StopWatchingMachPort();

   protected:
    friend class MessagePumpKqueue;

    void Init(WeakPtr<MessagePumpKqueue> pump,
              mach_port_t port,
              MachPortWatcher* watcher);
    void Reset();

    mach_port_t port() { return port_; }
    MachPortWatcher* watcher() { return watcher_; }

   private:
    mach_port_t port_ = MACH_PORT_NULL;
    MachPortWatcher* watcher_ = nullptr;
    WeakPtr<MessagePumpKqueue> pump_;
    const Location from_here_;

    DISALLOW_COPY_AND_ASSIGN(MachPortWatchController);
  };

  MessagePumpKqueue();
  ~MessagePumpKqueue() override;

  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;




  bool WatchMachReceivePort(mach_port_t port,
                            MachPortWatchController* controller,
                            MachPortWatcher* delegate);

  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           int mode,
                           FdWatchController* controller,
                           FdWatcher* delegate);

 private:


  bool StopWatchingMachPort(MachPortWatchController* controller);
  bool StopWatchingFileDescriptor(FdWatchController* controller);





  bool DoInternalWork(Delegate::NextWorkInfo* next_work_info);



  bool ProcessEvents(int count);


  mac::ScopedMachReceiveRight wakeup_;

  mach_msg_empty_rcv_t wakeup_buffer_;


  mac::ScopedMachPortSet port_set_;


  IDMap<FdWatchController*> fd_controllers_;

  IDMap<MachPortWatchController*> port_controllers_;

  ScopedFD kqueue_;

  bool keep_running_ = true;


  size_t event_count_ = 1;


  std::vector<kevent64_s> events_{event_count_};

  WeakPtrFactory<MessagePumpKqueue> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpKqueue);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_KQUEUE_H_
