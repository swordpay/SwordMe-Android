// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/timer/timer.h"

#include <stddef.h>

#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/threading/platform_thread.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/time/tick_clock.h"

namespace base {
namespace internal {

// on the current sequence. It also handles the following edge cases:
// - deleted by the task runner.
// - abandoned (orphaned) by Timer.
class BaseTimerTaskInternal {
 public:
  explicit BaseTimerTaskInternal(TimerBase* timer) : timer_(timer) {}

  ~BaseTimerTaskInternal() {



    if (timer_)
      timer_->AbandonAndStop();
  }

  void Run() {

    if (!timer_)
      return;

    timer_->scheduled_task_ = nullptr;


    TimerBase* timer = timer_;
    timer_ = nullptr;
    timer->RunScheduledTask();
  }

  void Abandon() { timer_ = nullptr; }

 private:
  TimerBase* timer_;

  DISALLOW_COPY_AND_ASSIGN(BaseTimerTaskInternal);
};

TimerBase::TimerBase() : TimerBase(nullptr) {}

TimerBase::TimerBase(const TickClock* tick_clock)
    : scheduled_task_(nullptr), tick_clock_(tick_clock), is_running_(false) {




  origin_sequence_checker_.DetachFromSequence();
}

TimerBase::TimerBase(const Location& posted_from, TimeDelta delay)
    : TimerBase(posted_from, delay, nullptr) {}

TimerBase::TimerBase(const Location& posted_from,
                     TimeDelta delay,
                     const TickClock* tick_clock)
    : scheduled_task_(nullptr),
      posted_from_(posted_from),
      delay_(delay),
      tick_clock_(tick_clock),
      is_running_(false) {

  origin_sequence_checker_.DetachFromSequence();
}

TimerBase::~TimerBase() {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());
  AbandonScheduledTask();
}

bool TimerBase::IsRunning() const {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());
  return is_running_;
}

TimeDelta TimerBase::GetCurrentDelay() const {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());
  return delay_;
}

void TimerBase::SetTaskRunner(scoped_refptr<SequencedTaskRunner> task_runner) {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());
  DCHECK(task_runner->RunsTasksInCurrentSequence());
  DCHECK(!IsRunning());
  task_runner_.swap(task_runner);
}

void TimerBase::StartInternal(const Location& posted_from, TimeDelta delay) {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());

  posted_from_ = posted_from;
  delay_ = delay;

  Reset();
}

void TimerBase::Stop() {




  is_running_ = false;

  origin_sequence_checker_.DetachFromSequence();

  OnStop();

}

void TimerBase::Reset() {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());

  if (!scheduled_task_) {
    PostNewScheduledTask(delay_);
    return;
  }

  if (delay_ > TimeDelta::FromMicroseconds(0))
    desired_run_time_ = Now() + delay_;
  else
    desired_run_time_ = TimeTicks();


  if (desired_run_time_ >= scheduled_run_time_) {
    is_running_ = true;
    return;
  }

  AbandonScheduledTask();
  PostNewScheduledTask(delay_);
}

TimeTicks TimerBase::Now() const {



  return tick_clock_ ? tick_clock_->NowTicks() : TimeTicks::Now();
}

void TimerBase::PostNewScheduledTask(TimeDelta delay) {



  DCHECK(!scheduled_task_);
  is_running_ = true;
  scheduled_task_ = new BaseTimerTaskInternal(this);
  if (delay > TimeDelta::FromMicroseconds(0)) {


    GetTaskRunner()->PostDelayedTask(
        posted_from_,
        BindOnce(&BaseTimerTaskInternal::Run, Owned(scheduled_task_)), delay);
    scheduled_run_time_ = desired_run_time_ = Now() + delay;
  } else {
    GetTaskRunner()->PostTask(
        posted_from_,
        BindOnce(&BaseTimerTaskInternal::Run, Owned(scheduled_task_)));
    scheduled_run_time_ = desired_run_time_ = TimeTicks();
  }
}

scoped_refptr<SequencedTaskRunner> TimerBase::GetTaskRunner() {
  return task_runner_.get() ? task_runner_ : SequencedTaskRunnerHandle::Get();
}

void TimerBase::AbandonScheduledTask() {



  if (scheduled_task_) {
    scheduled_task_->Abandon();
    scheduled_task_ = nullptr;
  }
}

void TimerBase::RunScheduledTask() {




  if (!is_running_)
    return;

  if (desired_run_time_ > scheduled_run_time_) {


    TimeTicks now = Now();


    if (desired_run_time_ > now) {

      PostNewScheduledTask(desired_run_time_ - now);
      return;
    }
  }

  RunUserTask();

}

}  // namespace internal

OneShotTimer::OneShotTimer() = default;
OneShotTimer::OneShotTimer(const TickClock* tick_clock)
    : internal::TimerBase(tick_clock) {}
OneShotTimer::~OneShotTimer() = default;

void OneShotTimer::Start(const Location& posted_from,
                         TimeDelta delay,
                         OnceClosure user_task) {
  user_task_ = std::move(user_task);
  StartInternal(posted_from, delay);
}

void OneShotTimer::FireNow() {
  DCHECK(origin_sequence_checker_.CalledOnValidSequence());
  DCHECK(!task_runner_) << "FireNow() is incompatible with SetTaskRunner()";
  DCHECK(IsRunning());

  RunUserTask();
}

void OneShotTimer::OnStop() {
  user_task_.Reset();


}

void OneShotTimer::RunUserTask() {


  OnceClosure task = std::move(user_task_);
  Stop();
  DCHECK(task);
  std::move(task).Run();

}

RepeatingTimer::RepeatingTimer() = default;
RepeatingTimer::RepeatingTimer(const TickClock* tick_clock)
    : internal::TimerBase(tick_clock) {}
RepeatingTimer::~RepeatingTimer() = default;

RepeatingTimer::RepeatingTimer(const Location& posted_from,
                               TimeDelta delay,
                               RepeatingClosure user_task)
    : internal::TimerBase(posted_from, delay),
      user_task_(std::move(user_task)) {}
RepeatingTimer::RepeatingTimer(const Location& posted_from,
                               TimeDelta delay,
                               RepeatingClosure user_task,
                               const TickClock* tick_clock)
    : internal::TimerBase(posted_from, delay, tick_clock),
      user_task_(std::move(user_task)) {}

void RepeatingTimer::Start(const Location& posted_from,
                           TimeDelta delay,
                           RepeatingClosure user_task) {
  user_task_ = std::move(user_task);
  StartInternal(posted_from, delay);
}

void RepeatingTimer::OnStop() {}
void RepeatingTimer::RunUserTask() {


  RepeatingClosure task = user_task_;
  PostNewScheduledTask(GetCurrentDelay());
  task.Run();

}

RetainingOneShotTimer::RetainingOneShotTimer() = default;
RetainingOneShotTimer::RetainingOneShotTimer(const TickClock* tick_clock)
    : internal::TimerBase(tick_clock) {}
RetainingOneShotTimer::~RetainingOneShotTimer() = default;

RetainingOneShotTimer::RetainingOneShotTimer(const Location& posted_from,
                                             TimeDelta delay,
                                             RepeatingClosure user_task)
    : internal::TimerBase(posted_from, delay),
      user_task_(std::move(user_task)) {}
RetainingOneShotTimer::RetainingOneShotTimer(const Location& posted_from,
                                             TimeDelta delay,
                                             RepeatingClosure user_task,
                                             const TickClock* tick_clock)
    : internal::TimerBase(posted_from, delay, tick_clock),
      user_task_(std::move(user_task)) {}

void RetainingOneShotTimer::Start(const Location& posted_from,
                                  TimeDelta delay,
                                  RepeatingClosure user_task) {
  user_task_ = std::move(user_task);
  StartInternal(posted_from, delay);
}

void RetainingOneShotTimer::OnStop() {}
void RetainingOneShotTimer::RunUserTask() {


  RepeatingClosure task = user_task_;
  Stop();
  task.Run();

}

}  // namespace base
