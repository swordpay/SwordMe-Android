// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_ANDROID_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_ANDROID_H_

#include <jni.h>
#include <memory>

#include "base/android/scoped_java_ref.h"
#include "base/base_export.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/message_loop/message_pump.h"
#include "base/optional.h"
#include "base/time/time.h"

struct ALooper;

namespace base {

class RunLoop;

// OS_ANDROID platform.
class BASE_EXPORT MessagePumpForUI : public MessagePump {
 public:
  MessagePumpForUI();
  ~MessagePumpForUI() override;

  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;


  virtual void Attach(Delegate* delegate);




  void Abort() { should_abort_ = true; }
  bool IsAborted() { return should_abort_; }
  bool ShouldQuit() const { return should_abort_ || quit_; }


  void QuitWhenIdle(base::OnceClosure callback);


  void OnDelayedLooperCallback();
  void OnNonDelayedLooperCallback();

 protected:
  void SetDelegate(Delegate* delegate) { delegate_ = delegate; }
  virtual bool IsTestImplementation() const;

 private:
  void DoIdleWork();





  std::unique_ptr<RunLoop> run_loop_;

  bool should_abort_ = false;

  bool quit_ = false;

  Delegate* delegate_ = nullptr;




  Optional<TimeTicks> delayed_scheduled_time_;

  base::OnceClosure on_quit_callback_;

  int non_delayed_fd_;

  int delayed_fd_;

  ALooper* looper_ = nullptr;

  JNIEnv* env_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpForUI);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_ANDROID_H_
