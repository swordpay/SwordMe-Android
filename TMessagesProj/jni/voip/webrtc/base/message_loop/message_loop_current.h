// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_
#define BASE_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_

#include <ostream>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/message_loop/message_pump_for_io.h"
#include "base/message_loop/message_pump_for_ui.h"
#include "base/pending_task.h"
#include "base/single_thread_task_runner.h"
#include "base/task/task_observer.h"
#include "build/build_config.h"

namespace web {
class WebTaskEnvironment;
}

namespace base {

namespace sequence_manager {
namespace internal {
class SequenceManagerImpl;
}
}  // namespace sequence_manager

// bound to the thread it's obtained on.
//
// MessageLoopCurrent(ForUI|ForIO) is available statically through
// MessageLoopCurrent(ForUI|ForIO)::Get() on threads that have a matching
// MessageLoop instance. APIs intended for all consumers on the thread should be
// on MessageLoopCurrent(ForUI|ForIO), while APIs intended for the owner of the
// instance should be on MessageLoop(ForUI|ForIO).
//
// Why: Historically MessageLoop::current() gave access to the full MessageLoop
// API, preventing both addition of powerful owner-only APIs as well as making
// it harder to remove callers of deprecated APIs (that need to stick around for
// a few owner-only use cases and re-accrue callers after cleanup per remaining
// publicly available).
//
// As such, many methods below are flagged as deprecated and should be removed
// (or moved back to MessageLoop) once all static callers have been migrated.
class BASE_EXPORT MessageLoopCurrent {
 public:


  MessageLoopCurrent(const MessageLoopCurrent& other) = default;
  MessageLoopCurrent(MessageLoopCurrent&& other) = default;
  MessageLoopCurrent& operator=(const MessageLoopCurrent& other) = default;

  bool operator==(const MessageLoopCurrent& other) const;


  static MessageLoopCurrent Get();


  static MessageLoopCurrent GetNull();



  static bool IsSet();



  MessageLoopCurrent* operator->() { return this; }
  explicit operator bool() const { return !!current_; }











  class BASE_EXPORT DestructionObserver {
   public:
    virtual void WillDestroyCurrentMessageLoop() = 0;

   protected:
    virtual ~DestructionObserver() = default;
  };


  void AddDestructionObserver(DestructionObserver* destruction_observer);


  void RemoveDestructionObserver(DestructionObserver* destruction_observer);



  void SetTaskRunner(scoped_refptr<SingleThreadTaskRunner> task_runner);



  void AddTaskObserver(TaskObserver* task_observer);
  void RemoveTaskObserver(TaskObserver* task_observer);


  void SetAddQueueTimeToTasks(bool enable);























  void SetNestableTasksAllowed(bool allowed);
  bool NestableTasksAllowed() const;







  class BASE_EXPORT ScopedNestableTaskAllower {
   public:
    ScopedNestableTaskAllower();
    ~ScopedNestableTaskAllower();

   private:
    sequence_manager::internal::SequenceManagerImpl* const sequence_manager_;
    const bool old_state_;
  };

  bool IsBoundToCurrentThread() const;





  bool IsIdleForTesting();

 protected:
  explicit MessageLoopCurrent(
      sequence_manager::internal::SequenceManagerImpl* sequence_manager)
      : current_(sequence_manager) {}

  static sequence_manager::internal::SequenceManagerImpl*
  GetCurrentSequenceManagerImpl();

  friend class MessagePumpLibeventTest;
  friend class ScheduleWorkTest;
  friend class Thread;
  friend class sequence_manager::internal::SequenceManagerImpl;
  friend class MessageLoopTaskRunnerTest;
  friend class web::WebTaskEnvironment;

  sequence_manager::internal::SequenceManagerImpl* current_;
};

#if !defined(OS_NACL)

class BASE_EXPORT MessageLoopCurrentForUI : public MessageLoopCurrent {
 public:


  static MessageLoopCurrentForUI Get();

  static bool IsSet();

  MessageLoopCurrentForUI* operator->() { return this; }

#if defined(USE_OZONE) && !defined(OS_FUCHSIA) && !defined(OS_WIN)
  static_assert(
      std::is_base_of<WatchableIOMessagePumpPosix, MessagePumpForUI>::value,
      "MessageLoopCurrentForUI::WatchFileDescriptor is supported only"
      "by MessagePumpLibevent and MessagePumpGlib implementations.");
  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           MessagePumpForUI::Mode mode,
                           MessagePumpForUI::FdWatchController* controller,
                           MessagePumpForUI::FdWatcher* delegate);
#endif

#if defined(OS_IOS)




  void Attach();
#endif

#if defined(OS_ANDROID)




  void Abort();
#endif

#if defined(OS_WIN)
  void AddMessagePumpObserver(MessagePumpForUI::Observer* observer);
  void RemoveMessagePumpObserver(MessagePumpForUI::Observer* observer);
#endif

 private:
  explicit MessageLoopCurrentForUI(
      sequence_manager::internal::SequenceManagerImpl* current)
      : MessageLoopCurrent(current) {}

  MessagePumpForUI* GetMessagePumpForUI() const;
};

#endif  // !defined(OS_NACL)

class BASE_EXPORT MessageLoopCurrentForIO : public MessageLoopCurrent {
 public:


  static MessageLoopCurrentForIO Get();

  static bool IsSet();

  MessageLoopCurrentForIO* operator->() { return this; }

#if !defined(OS_NACL_SFI)

#if defined(OS_WIN)

  HRESULT RegisterIOHandler(HANDLE file, MessagePumpForIO::IOHandler* handler);
  bool RegisterJobObject(HANDLE job, MessagePumpForIO::IOHandler* handler);
  bool WaitForIOCompletion(DWORD timeout, MessagePumpForIO::IOHandler* filter);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)


  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           MessagePumpForIO::Mode mode,
                           MessagePumpForIO::FdWatchController* controller,
                           MessagePumpForIO::FdWatcher* delegate);
#endif  // defined(OS_WIN)

#if defined(OS_MACOSX) && !defined(OS_IOS)
  bool WatchMachReceivePort(
      mach_port_t port,
      MessagePumpForIO::MachPortWatchController* controller,
      MessagePumpForIO::MachPortWatcher* delegate);
#endif

#if defined(OS_FUCHSIA)

  bool WatchZxHandle(zx_handle_t handle,
                     bool persistent,
                     zx_signals_t signals,
                     MessagePumpForIO::ZxHandleWatchController* controller,
                     MessagePumpForIO::ZxHandleWatcher* delegate);
#endif  // defined(OS_FUCHSIA)

#endif  // !defined(OS_NACL_SFI)

 private:
  explicit MessageLoopCurrentForIO(
      sequence_manager::internal::SequenceManagerImpl* current)
      : MessageLoopCurrent(current) {}

  MessagePumpForIO* GetMessagePumpForIO() const;
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_
