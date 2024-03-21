// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/message_loop/message_pump_android.h"

#include <android/looper.h>
#include <errno.h>
#include <fcntl.h>
#include <jni.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/callback_helpers.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "build/build_config.h"

// use syscall to make use of timerfd. Once the min API level is 20, we can
// directly use timerfd.h.
#ifndef __NR_timerfd_create
#error "Unable to find syscall for __NR_timerfd_create"
#endif

#ifndef TFD_TIMER_ABSTIME
#define TFD_TIMER_ABSTIME (1 << 0)
#endif

using base::android::JavaParamRef;
using base::android::ScopedJavaLocalRef;

namespace base {

namespace {

int timerfd_create(int clockid, int flags) {
  return syscall(__NR_timerfd_create, clockid, flags);
}

int timerfd_settime(int ufc,
                    int flags,
                    const struct itimerspec* utmr,
                    struct itimerspec* otmr) {
  return syscall(__NR_timerfd_settime, ufc, flags, utmr, otmr);
}

// into our code due to the inconsistent ABI on older Android OS versions.
#if defined(ARCH_CPU_X86)
#define STACK_ALIGN __attribute__((force_align_arg_pointer))
#else
#define STACK_ALIGN
#endif

STACK_ALIGN int NonDelayedLooperCallback(int fd, int events, void* data) {
  if (events & ALOOPER_EVENT_HANGUP)
    return 0;

  DCHECK(events & ALOOPER_EVENT_INPUT);
  MessagePumpForUI* pump = reinterpret_cast<MessagePumpForUI*>(data);
  pump->OnNonDelayedLooperCallback();
  return 1;  // continue listening for events
}

STACK_ALIGN int DelayedLooperCallback(int fd, int events, void* data) {
  if (events & ALOOPER_EVENT_HANGUP)
    return 0;

  DCHECK(events & ALOOPER_EVENT_INPUT);
  MessagePumpForUI* pump = reinterpret_cast<MessagePumpForUI*>(data);
  pump->OnDelayedLooperCallback();
  return 1;  // continue listening for events
}

}  // namespace

MessagePumpForUI::MessagePumpForUI()
    : env_(base::android::AttachCurrentThread()) {




  non_delayed_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  CHECK_NE(non_delayed_fd_, -1);
  DCHECK_EQ(TimeTicks::GetClock(), TimeTicks::Clock::LINUX_CLOCK_MONOTONIC);




  delayed_fd_ = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK | O_CLOEXEC);
  CHECK_NE(delayed_fd_, -1);

  looper_ = ALooper_prepare(0);
  DCHECK(looper_);

  ALooper_acquire(looper_);
  ALooper_addFd(looper_, non_delayed_fd_, 0, ALOOPER_EVENT_INPUT,
                &NonDelayedLooperCallback, reinterpret_cast<void*>(this));
  ALooper_addFd(looper_, delayed_fd_, 0, ALOOPER_EVENT_INPUT,
                &DelayedLooperCallback, reinterpret_cast<void*>(this));
}

MessagePumpForUI::~MessagePumpForUI() {
  DCHECK_EQ(ALooper_forThread(), looper_);
  ALooper_removeFd(looper_, non_delayed_fd_);
  ALooper_removeFd(looper_, delayed_fd_);
  ALooper_release(looper_);
  looper_ = nullptr;

  close(non_delayed_fd_);
  close(delayed_fd_);
}

void MessagePumpForUI::OnDelayedLooperCallback() {





  if (base::android::HasException(env_))
    return;


  if (ShouldQuit())
    return;

  uint64_t value;
  int ret = read(delayed_fd_, &value, sizeof(value));









  DPCHECK(ret >= 0 || errno == EAGAIN);

  delayed_scheduled_time_.reset();

  Delegate::NextWorkInfo next_work_info = delegate_->DoSomeWork();

  if (ShouldQuit())
    return;

  if (next_work_info.is_immediate()) {
    ScheduleWork();
    return;
  }

  DoIdleWork();
  if (!next_work_info.delayed_run_time.is_max())
    ScheduleDelayedWork(next_work_info.delayed_run_time);
}

void MessagePumpForUI::OnNonDelayedLooperCallback() {





  if (base::android::HasException(env_))
    return;


  if (ShouldQuit())
    return;


  constexpr uint64_t kTryNativeTasksBeforeIdleBit = uint64_t(1) << 32;







  uint64_t pre_work_value = 0;
  int ret = read(non_delayed_fd_, &pre_work_value, sizeof(pre_work_value));
  DPCHECK(ret >= 0);
  DCHECK_GT(pre_work_value, 0U);





  Delegate::NextWorkInfo next_work_info;
  do {
    if (ShouldQuit())
      return;

    next_work_info = delegate_->DoSomeWork();
  } while (next_work_info.is_immediate());



  if (ShouldQuit())
    return;


  if (pre_work_value != kTryNativeTasksBeforeIdleBit) {





    ret = write(non_delayed_fd_, &kTryNativeTasksBeforeIdleBit,
                sizeof(kTryNativeTasksBeforeIdleBit));
    DPCHECK(ret >= 0);
    return;
  }









  DCHECK_EQ(pre_work_value, kTryNativeTasksBeforeIdleBit);

  if (ShouldQuit())
    return;







  DoIdleWork();
  if (!next_work_info.delayed_run_time.is_max())
    ScheduleDelayedWork(next_work_info.delayed_run_time);
}

void MessagePumpForUI::DoIdleWork() {
  if (delegate_->DoIdleWork()) {



    ScheduleWork();
  }
}

void MessagePumpForUI::Run(Delegate* delegate) {
  DCHECK(IsTestImplementation());


  quit_ = false;

  SetDelegate(delegate);


  ScheduleWork();
  while (true) {




    ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
    if (quit_)
      break;
  }
}

void MessagePumpForUI::Attach(Delegate* delegate) {
  DCHECK(!quit_);





  SetDelegate(delegate);
  run_loop_ = std::make_unique<RunLoop>();


  if (!run_loop_->BeforeRun())
    NOTREACHED();
}

void MessagePumpForUI::Quit() {
  if (quit_)
    return;

  quit_ = true;

  int64_t value;

  read(delayed_fd_, &value, sizeof(value));

  read(non_delayed_fd_, &value, sizeof(value));

  if (run_loop_) {
    run_loop_->AfterRun();
    run_loop_ = nullptr;
  }
  if (on_quit_callback_) {
    std::move(on_quit_callback_).Run();
  }
}

void MessagePumpForUI::ScheduleWork() {









  uint64_t value = 1;
  int ret = write(non_delayed_fd_, &value, sizeof(value));
  DPCHECK(ret >= 0);
}

void MessagePumpForUI::ScheduleDelayedWork(const TimeTicks& delayed_work_time) {
  if (ShouldQuit())
    return;

  if (delayed_scheduled_time_ && *delayed_scheduled_time_ == delayed_work_time)
    return;

  DCHECK(!delayed_work_time.is_null());
  delayed_scheduled_time_ = delayed_work_time;
  int64_t nanos = delayed_work_time.since_origin().InNanoseconds();
  struct itimerspec ts;
  ts.it_interval.tv_sec = 0;  // Don't repeat.
  ts.it_interval.tv_nsec = 0;
  ts.it_value.tv_sec = nanos / TimeTicks::kNanosecondsPerSecond;
  ts.it_value.tv_nsec = nanos % TimeTicks::kNanosecondsPerSecond;

  int ret = timerfd_settime(delayed_fd_, TFD_TIMER_ABSTIME, &ts, nullptr);
  DPCHECK(ret >= 0);
}

void MessagePumpForUI::QuitWhenIdle(base::OnceClosure callback) {
  DCHECK(!on_quit_callback_);
  DCHECK(run_loop_);
  on_quit_callback_ = std::move(callback);
  run_loop_->QuitWhenIdle();

  ScheduleWork();
}

bool MessagePumpForUI::IsTestImplementation() const {
  return false;
}

}  // namespace base
