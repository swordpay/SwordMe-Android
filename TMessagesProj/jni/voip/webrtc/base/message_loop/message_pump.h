// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_H_

#include "base/base_export.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/message_loop/timer_slack.h"
#include "base/sequence_checker.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

class TimeTicks;

class BASE_EXPORT MessagePump {
 public:
  using MessagePumpFactory = std::unique_ptr<MessagePump>();


  static void OverrideMessagePumpForUIFactory(MessagePumpFactory* factory);

  static bool IsMessagePumpForUIFactoryOveridden();

  static std::unique_ptr<MessagePump> Create(MessagePumpType type);


  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() = default;





    virtual void BeforeDoInternalWork() = 0;



    virtual void BeforeWait() = 0;

    struct NextWorkInfo {


      TimeDelta remaining_delay() const {
        DCHECK(!delayed_run_time.is_null() && !delayed_run_time.is_max());
        DCHECK_GE(TimeTicks::Now(), recent_now);
        return delayed_run_time - recent_now;
      }

      bool is_immediate() const { return delayed_run_time.is_null(); }



      TimeTicks delayed_run_time;



      TimeTicks recent_now;
    };








    virtual NextWorkInfo DoSomeWork() = 0;



    virtual bool DoIdleWork() = 0;
  };

  MessagePump();
  virtual ~MessagePump();





























































  virtual void Run(Delegate* delegate) = 0;


  virtual void Quit() = 0;









  virtual void ScheduleWork() = 0;









  virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time) = 0;

  virtual void SetTimerSlack(TimerSlack timer_slack);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_H_
