// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_H_
#define BASE_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_H_

#include "base/message_loop/message_pump.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/task/sequence_manager/lazy_now.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

class MessageLoopBase;
class TickClock;
struct PendingTask;

namespace sequence_manager {
namespace internal {

class AssociatedThreadId;
class SequencedTaskSource;

// actual work to be run. Hopefully we can stop using MessageLoop and this
// interface will become more concise.
class ThreadController {
 public:
  virtual ~ThreadController() = default;



  virtual void SetWorkBatchSize(int work_batch_size = 1) = 0;



  virtual void WillQueueTask(PendingTask* pending_task,
                             const char* task_queue_name) = 0;









  virtual void ScheduleWork() = 0;






  virtual void SetNextDelayedDoWork(LazyNow* lazy_now, TimeTicks run_time) = 0;



  virtual void SetSequencedTaskSource(SequencedTaskSource*) = 0;


  virtual void SetTimerSlack(TimerSlack timer_slack) = 0;



  virtual void BindToCurrentThread(
      std::unique_ptr<MessagePump> message_pump) = 0;


  virtual void SetTaskExecutionAllowed(bool allowed) = 0;

  virtual bool IsTaskExecutionAllowed() const = 0;

  virtual MessagePump* GetBoundMessagePump() const = 0;

  virtual bool ShouldQuitRunLoopWhenIdle() = 0;

#if defined(OS_IOS) || defined(OS_ANDROID)



  virtual void AttachToMessagePump() = 0;
#endif

#if defined(OS_IOS)


  virtual void DetachFromMessagePump() = 0;
#endif




  virtual bool RunsTasksInCurrentSequence() = 0;
  virtual const TickClock* GetClock() = 0;
  virtual void SetDefaultTaskRunner(scoped_refptr<SingleThreadTaskRunner>) = 0;
  virtual scoped_refptr<SingleThreadTaskRunner> GetDefaultTaskRunner() = 0;
  virtual void RestoreDefaultTaskRunner() = 0;
  virtual void AddNestingObserver(RunLoop::NestingObserver* observer) = 0;
  virtual void RemoveNestingObserver(RunLoop::NestingObserver* observer) = 0;
  virtual const scoped_refptr<AssociatedThreadId>& GetAssociatedThread()
      const = 0;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_H_
