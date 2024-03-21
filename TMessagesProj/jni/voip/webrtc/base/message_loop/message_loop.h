// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_LOOP_H_
#define BASE_MESSAGE_LOOP_MESSAGE_LOOP_H_

#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/callback_forward.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/message_loop/message_pump_type.h"
#include "base/message_loop/timer_slack.h"
#include "base/pending_task.h"
#include "base/run_loop.h"
#include "base/threading/thread_checker.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

class MessagePump;
class TaskObserver;

namespace sequence_manager {
class TaskQueue;
namespace internal {
class SequenceManagerImpl;
}  // namespace internal
}  // namespace sequence_manager

// at most one MessageLoop instance per thread.
//
// Events include at a minimum Task instances submitted to the MessageLoop's
// TaskRunner. Depending on the Type of message pump used by the MessageLoop
// other events such as UI messages may be processed.  On Windows APC calls (as
// time permits) and signals sent to a registered set of HANDLEs may also be
// processed.
//
// The MessageLoop's API should only be used directly by its owner (and users
// which the owner opts to share a MessageLoop* with). Other ways to access
// subsets of the MessageLoop API:
//   - base::RunLoop : Drive the MessageLoop from the thread it's bound to.
//   - base::Thread/SequencedTaskRunnerHandle : Post back to the MessageLoop
//     from a task running on it.
//   - SequenceLocalStorageSlot : Bind external state to this MessageLoop.
//   - base::MessageLoopCurrent : Access statically exposed APIs of this
//     MessageLoop.
//   - Embedders may provide their own static accessors to post tasks on
//     specific loops (e.g. content::BrowserThreads).
//
// NOTE: Unless otherwise specified, a MessageLoop's methods may only be called
// on the thread where the MessageLoop's Run method executes.
//
// NOTE: MessageLoop has task reentrancy protection.  This means that if a
// task is being processed, a second task cannot start until the first task is
// finished.  Reentrancy can happen when processing a task, and an inner
// message pump is created.  That inner pump then processes native messages
// which could implicitly start an inner task.  Inner message pumps are created
// with dialogs (DialogBox), common dialogs (GetOpenFileName), OLE functions
// (DoDragDrop), printer functions (StartDoc) and *many* others.
//
// Sample workaround when inner task processing is needed:
//   HRESULT hr;
//   {
//     MessageLoopCurrent::ScopedNestableTaskAllower allow;
//     hr = DoDragDrop(...); // Implicitly runs a modal message loop.
//   }
//   // Process |hr| (the result returned by DoDragDrop()).
//
// Please be SURE your task is reentrant (nestable) and all global variables
// are stable and accessible before calling SetNestableTasksAllowed(true).
//
// DEPRECATED: Use a SingleThreadTaskExecutor instead or TaskEnvironment
// for tests. TODO(https://crbug.com/891670/) remove this class.
class BASE_EXPORT MessageLoop {
 public:


  explicit MessageLoop(MessagePumpType type = MessagePumpType::DEFAULT);


  explicit MessageLoop(std::unique_ptr<MessagePump> custom_pump);

  virtual ~MessageLoop();

  void SetTimerSlack(TimerSlack timer_slack);


  virtual bool IsType(MessagePumpType type) const;

  MessagePumpType type() const { return type_; }


  void SetTaskRunner(scoped_refptr<SingleThreadTaskRunner> task_runner);

  scoped_refptr<SingleThreadTaskRunner> task_runner() const;



  void AddTaskObserver(TaskObserver* task_observer);
  void RemoveTaskObserver(TaskObserver* task_observer);






  bool IsIdleForTesting();

 protected:

  bool IsBoundToCurrentThread() const;

  using MessagePumpFactoryCallback =
      OnceCallback<std::unique_ptr<MessagePump>()>;






  MessageLoop(MessagePumpType type, std::unique_ptr<MessagePump> pump);

  void BindToCurrentThread();


  MessagePump* pump_ = nullptr;


  const std::unique_ptr<sequence_manager::internal::SequenceManagerImpl>
      sequence_manager_;


  const scoped_refptr<sequence_manager::TaskQueue> default_task_queue_;

 private:
  friend class MessageLoopTypedTest;
  friend class ScheduleWorkTest;
  friend class Thread;
  friend class sequence_manager::internal::SequenceManagerImpl;
  FRIEND_TEST_ALL_PREFIXES(MessageLoopTest, DeleteUnboundLoop);








  static std::unique_ptr<MessageLoop> CreateUnbound(MessagePumpType type);
  static std::unique_ptr<MessageLoop> CreateUnbound(
      std::unique_ptr<MessagePump> pump);

  scoped_refptr<sequence_manager::TaskQueue> CreateDefaultTaskQueue();

  std::unique_ptr<MessagePump> CreateMessagePump();

  sequence_manager::internal::SequenceManagerImpl* GetSequenceManagerImpl()
      const {
    return sequence_manager_.get();
  }

  const MessagePumpType type_;


  std::unique_ptr<MessagePump> custom_pump_;


  PlatformThreadId thread_id_ = kInvalidThreadId;


  THREAD_CHECKER(bound_thread_checker_);

  DISALLOW_COPY_AND_ASSIGN(MessageLoop);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_LOOP_H_
