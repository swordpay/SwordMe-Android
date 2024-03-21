// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_THREAD_H_
#define BASE_THREADING_THREAD_H_

#include <stddef.h>

#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/message_loop/message_pump_type.h"
#include "base/message_loop/timer_slack.h"
#include "base/sequence_checker.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/atomic_flag.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"

namespace base {

class MessagePump;
class RunLoop;
namespace sequence_manager {
class TimeDomain;
}

// base::Create(Sequenced|SingleThread)TaskRunner().
//
// A simple thread abstraction that establishes a MessageLoop on a new thread.
// The consumer uses the MessageLoop of the thread to cause code to execute on
// the thread.  When this object is destroyed the thread is terminated.  All
// pending tasks queued on the thread's message loop will run to completion
// before the thread is terminated.
//
// WARNING! SUBCLASSES MUST CALL Stop() IN THEIR DESTRUCTORS!  See ~Thread().
//
// After the thread is stopped, the destruction sequence is:
//
//  (1) Thread::CleanUp()
//  (2) MessageLoop::~MessageLoop
//  (3.b) MessageLoopCurrent::DestructionObserver::WillDestroyCurrentMessageLoop
//
// This API is not thread-safe: unless indicated otherwise its methods are only
// valid from the owning sequence (which is the one from which Start() is
// invoked -- should it differ from the one on which it was constructed).
//
// Sometimes it's useful to kick things off on the initial sequence (e.g.
// construction, Start(), task_runner()), but to then hand the Thread over to a
// pool of users for the last one of them to destroy it when done. For that use
// case, Thread::DetachFromSequence() allows the owning sequence to give up
// ownership. The caller is then responsible to ensure a happens-after
// relationship between the DetachFromSequence() call and the next use of that
// Thread object (including ~Thread()).
class BASE_EXPORT Thread : PlatformThread::Delegate {
 public:
  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() {}

    virtual scoped_refptr<SingleThreadTaskRunner> GetDefaultTaskRunner() = 0;



    virtual void BindToCurrentThread(TimerSlack timer_slack) = 0;
  };

  struct BASE_EXPORT Options {
    using MessagePumpFactory =
        RepeatingCallback<std::unique_ptr<MessagePump>()>;

    Options();
    Options(MessagePumpType type, size_t size);
    Options(Options&& other);
    ~Options();


    MessagePumpType message_pump_type = MessagePumpType::DEFAULT;



    Delegate* delegate = nullptr;

    TimerSlack timer_slack = TIMER_SLACK_NONE;


    sequence_manager::TimeDomain* task_queue_time_domain = nullptr;





    MessagePumpFactory message_pump_factory;



    size_t stack_size = 0;

    ThreadPriority priority = ThreadPriority::NORMAL;






    bool joinable = true;
  };


  explicit Thread(const std::string& name);








  ~Thread() override;

#if defined(OS_WIN)





  void init_com_with_mta(bool use_mta) {
    DCHECK(!delegate_);
    com_status_ = use_mta ? MTA : STA;
  }
#endif







  bool Start();






  bool StartWithOptions(const Options& options);





  bool StartAndWaitForTesting();



  bool WaitUntilThreadStarted() const;

  void FlushForTesting();














  void Stop();










  void StopSoon();





  void DetachFromSequence();








  scoped_refptr<SingleThreadTaskRunner> task_runner() const {










    DCHECK(owning_sequence_checker_.CalledOnValidSequence() ||
           (id_event_.IsSignaled() && id_ == PlatformThread::CurrentId()) ||
           delegate_);
    return delegate_ ? delegate_->GetDefaultTaskRunner() : nullptr;
  }

  const std::string& thread_name() const { return name_; }







  PlatformThreadId GetThreadId() const;

  bool IsRunning() const;

 protected:

  virtual void Init() {}

  virtual void Run(RunLoop* run_loop);

  virtual void CleanUp() {}

  static void SetThreadWasQuitProperly(bool flag);
  static bool GetThreadWasQuitProperly();

 private:

  friend class MessageLoopTaskRunnerTest;
  friend class ScheduleWorkTest;

#if defined(OS_WIN)
  enum ComStatus {
    NONE,
    STA,
    MTA,
  };
#endif

  void ThreadMain() override;

  void ThreadQuitHelper();

#if defined(OS_WIN)

  ComStatus com_status_ = NONE;
#endif


  bool joinable_ = true;




  bool stopping_ = false;

  bool running_ = false;
  mutable base::Lock running_lock_;  // Protects |running_|.

  PlatformThreadHandle thread_;
  mutable base::Lock thread_lock_;  // Protects |thread_|.

  PlatformThreadId id_ = kInvalidThreadId;

  mutable WaitableEvent id_event_;


  std::unique_ptr<Delegate> delegate_;
  RunLoop* run_loop_ = nullptr;


  TimerSlack timer_slack_ = TIMER_SLACK_NONE;

  const std::string name_;

  mutable WaitableEvent start_event_;


  SequenceChecker owning_sequence_checker_;

  DISALLOW_COPY_AND_ASSIGN(Thread);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_H_
