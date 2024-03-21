// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/thread.h"

#include <type_traits>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/scoped_refptr.h"
#include "base/message_loop/message_loop_current.h"
#include "base/message_loop/message_pump.h"
#include "base/run_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/task/sequence_manager/sequence_manager_impl.h"
#include "base/task/sequence_manager/task_queue.h"
#include "base/task/simple_task_executor.h"
#include "base/third_party/dynamic_annotations/dynamic_annotations.h"
#include "base/threading/thread_id_name_manager.h"
#include "base/threading/thread_local.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"

#if defined(OS_POSIX) && !defined(OS_NACL)
#include "base/files/file_descriptor_watcher_posix.h"
#endif

#if defined(OS_WIN)
#include "base/win/scoped_com_initializer.h"
#endif

namespace base {

namespace {

// because its Stop method was called.  This allows us to catch cases where
// MessageLoop::QuitWhenIdle() is called directly, which is unexpected when
// using a Thread to setup and run a MessageLoop.
base::LazyInstance<base::ThreadLocalBoolean>::Leaky lazy_tls_bool =
    LAZY_INSTANCE_INITIALIZER;

class SequenceManagerThreadDelegate : public Thread::Delegate {
 public:
  explicit SequenceManagerThreadDelegate(
      MessagePumpType message_pump_type,
      OnceCallback<std::unique_ptr<MessagePump>()> message_pump_factory,
      sequence_manager::TimeDomain* time_domain)
      : sequence_manager_(
            sequence_manager::internal::SequenceManagerImpl::CreateUnbound(
                sequence_manager::SequenceManager::Settings::Builder()
                    .SetMessagePumpType(message_pump_type)
                    .Build())),
        default_task_queue_(sequence_manager_->CreateTaskQueue(
            sequence_manager::TaskQueue::Spec("default_tq")
                .SetTimeDomain(time_domain))),
        message_pump_factory_(std::move(message_pump_factory)) {
    sequence_manager_->SetDefaultTaskRunner(default_task_queue_->task_runner());
  }

  ~SequenceManagerThreadDelegate() override = default;

  scoped_refptr<SingleThreadTaskRunner> GetDefaultTaskRunner() override {















    return sequence_manager_->GetTaskRunner();
  }

  void BindToCurrentThread(TimerSlack timer_slack) override {
    sequence_manager_->BindToMessagePump(
        std::move(message_pump_factory_).Run());
    sequence_manager_->SetTimerSlack(timer_slack);
    simple_task_executor_.emplace(GetDefaultTaskRunner());
  }

 private:
  std::unique_ptr<sequence_manager::internal::SequenceManagerImpl>
      sequence_manager_;
  scoped_refptr<sequence_manager::TaskQueue> default_task_queue_;
  OnceCallback<std::unique_ptr<MessagePump>()> message_pump_factory_;
  base::Optional<SimpleTaskExecutor> simple_task_executor_;
};

}  // namespace

Thread::Options::Options() = default;

Thread::Options::Options(MessagePumpType type, size_t size)
    : message_pump_type(type), stack_size(size) {}

Thread::Options::Options(Options&& other) = default;

Thread::Options::~Options() = default;

Thread::Thread(const std::string& name)
    : id_event_(WaitableEvent::ResetPolicy::MANUAL,
                WaitableEvent::InitialState::NOT_SIGNALED),
      name_(name),
      start_event_(WaitableEvent::ResetPolicy::MANUAL,
                   WaitableEvent::InitialState::NOT_SIGNALED) {




  owning_sequence_checker_.DetachFromSequence();
}

Thread::~Thread() {
  Stop();
}

bool Thread::Start() {
  DCHECK(owning_sequence_checker_.CalledOnValidSequence());

  Options options;
#if defined(OS_WIN)
  if (com_status_ == STA)
    options.message_pump_type = MessagePumpType::UI;
#endif
  return StartWithOptions(options);
}

bool Thread::StartWithOptions(const Options& options) {
  DCHECK(owning_sequence_checker_.CalledOnValidSequence());
  DCHECK(!delegate_);
  DCHECK(!IsRunning());
  DCHECK(!stopping_) << "Starting a non-joinable thread a second time? That's "
                     << "not allowed!";
#if defined(OS_WIN)
  DCHECK((com_status_ != STA) ||
         (options.message_pump_type == MessagePumpType::UI));
#endif

  id_event_.Reset();
  id_ = kInvalidThreadId;

  SetThreadWasQuitProperly(false);

  timer_slack_ = options.timer_slack;

  if (options.delegate) {
    DCHECK(!options.message_pump_factory);
    DCHECK(!options.task_queue_time_domain);
    delegate_ = WrapUnique(options.delegate);
  } else if (options.message_pump_factory) {
    delegate_ = std::make_unique<SequenceManagerThreadDelegate>(
        MessagePumpType::CUSTOM, options.message_pump_factory,
        options.task_queue_time_domain);
  } else {
    delegate_ = std::make_unique<SequenceManagerThreadDelegate>(
        options.message_pump_type,
        BindOnce([](MessagePumpType type) { return MessagePump::Create(type); },
                 options.message_pump_type),
        options.task_queue_time_domain);
  }

  start_event_.Reset();



  {
    AutoLock lock(thread_lock_);
    bool success =
        options.joinable
            ? PlatformThread::CreateWithPriority(options.stack_size, this,
                                                 &thread_, options.priority)
            : PlatformThread::CreateNonJoinableWithPriority(
                  options.stack_size, this, options.priority);
    if (!success) {
      DLOG(ERROR) << "failed to create thread";
      return false;
    }
  }

  joinable_ = options.joinable;

  return true;
}

bool Thread::StartAndWaitForTesting() {
  DCHECK(owning_sequence_checker_.CalledOnValidSequence());
  bool result = Start();
  if (!result)
    return false;
  WaitUntilThreadStarted();
  return true;
}

bool Thread::WaitUntilThreadStarted() const {
  DCHECK(owning_sequence_checker_.CalledOnValidSequence());
  if (!delegate_)
    return false;

  base::ScopedAllowBaseSyncPrimitivesOutsideBlockingScope allow_wait;
  start_event_.Wait();
  return true;
}

void Thread::FlushForTesting() {
  DCHECK(owning_sequence_checker_.CalledOnValidSequence());
  if (!delegate_)
    return;

  WaitableEvent done(WaitableEvent::ResetPolicy::AUTOMATIC,
                     WaitableEvent::InitialState::NOT_SIGNALED);
  task_runner()->PostTask(FROM_HERE,
                          BindOnce(&WaitableEvent::Signal, Unretained(&done)));
  done.Wait();
}

void Thread::Stop() {
  DCHECK(joinable_);




  AutoLock lock(thread_lock_);

  StopSoon();

  if (thread_.is_null())
    return;




  PlatformThread::Join(thread_);
  thread_ = base::PlatformThreadHandle();


  DCHECK(!delegate_);

  stopping_ = false;
}

void Thread::StopSoon() {




  if (stopping_ || !delegate_)
    return;

  stopping_ = true;

  task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&Thread::ThreadQuitHelper, Unretained(this)));
}

void Thread::DetachFromSequence() {
  DCHECK(owning_sequence_checker_.CalledOnValidSequence());
  owning_sequence_checker_.DetachFromSequence();
}

PlatformThreadId Thread::GetThreadId() const {

  base::ScopedAllowBaseSyncPrimitivesOutsideBlockingScope allow_wait;
  id_event_.Wait();
  return id_;
}

bool Thread::IsRunning() const {







  if (delegate_ && !stopping_)
    return true;


  AutoLock lock(running_lock_);
  return running_;
}

void Thread::Run(RunLoop* run_loop) {

  DCHECK(id_event_.IsSignaled());
  DCHECK_EQ(id_, PlatformThread::CurrentId());

  run_loop->Run();
}

void Thread::SetThreadWasQuitProperly(bool flag) {
  lazy_tls_bool.Pointer()->Set(flag);
}

bool Thread::GetThreadWasQuitProperly() {
  bool quit_properly = true;
#if DCHECK_IS_ON()
  quit_properly = lazy_tls_bool.Pointer()->Get();
#endif
  return quit_properly;
}

void Thread::ThreadMain() {


  DCHECK(!id_event_.IsSignaled());



  DCHECK_EQ(kInvalidThreadId, id_);
  id_ = PlatformThread::CurrentId();
  DCHECK_NE(kInvalidThreadId, id_);
  id_event_.Signal();

  PlatformThread::SetName(name_.c_str());
  ANNOTATE_THREAD_NAME(name_.c_str());  // Tell the name to race detector.

  DCHECK(delegate_);

  delegate_->BindToCurrentThread(timer_slack_);
  DCHECK(MessageLoopCurrent::Get());
  DCHECK(ThreadTaskRunnerHandle::IsSet());

#if defined(OS_POSIX) && !defined(OS_NACL)

  std::unique_ptr<FileDescriptorWatcher> file_descriptor_watcher;
  if (MessageLoopCurrentForIO::IsSet()) {
    file_descriptor_watcher.reset(
        new FileDescriptorWatcher(delegate_->GetDefaultTaskRunner()));
  }
#endif

#if defined(OS_WIN)
  std::unique_ptr<win::ScopedCOMInitializer> com_initializer;
  if (com_status_ != NONE) {
    com_initializer.reset(
        (com_status_ == STA)
            ? new win::ScopedCOMInitializer()
            : new win::ScopedCOMInitializer(win::ScopedCOMInitializer::kMTA));
  }
#endif

  Init();

  {
    AutoLock lock(running_lock_);
    running_ = true;
  }

  start_event_.Signal();

  RunLoop run_loop;
  run_loop_ = &run_loop;
  Run(run_loop_);

  {
    AutoLock lock(running_lock_);
    running_ = false;
  }

  CleanUp();

#if defined(OS_WIN)
  com_initializer.reset();
#endif

  DCHECK(GetThreadWasQuitProperly());


  delegate_.reset();
  run_loop_ = nullptr;
}

void Thread::ThreadQuitHelper() {
  DCHECK(run_loop_);
  run_loop_->QuitWhenIdle();
  SetThreadWasQuitProperly(true);
}

}  // namespace base
