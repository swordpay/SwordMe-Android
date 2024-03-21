// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TIMER_HI_RES_TIMER_MANAGER_H_
#define BASE_TIMER_HI_RES_TIMER_MANAGER_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/power_monitor/power_observer.h"
#include "base/timer/timer.h"
#include "build/build_config.h"

namespace base {

// when not running on battery power.
class BASE_EXPORT HighResolutionTimerManager : public base::PowerObserver {
 public:
  HighResolutionTimerManager();
  ~HighResolutionTimerManager() override;

  void OnPowerStateChange(bool on_battery_power) override;
  void OnSuspend() override;
  void OnResume() override;

  bool hi_res_clock_available() const { return hi_res_clock_available_; }

 private:

  void UseHiResClock(bool use);

  bool hi_res_clock_available_;

#if defined(OS_WIN)

  base::RepeatingTimer timer_;
#endif

  DISALLOW_COPY_AND_ASSIGN(HighResolutionTimerManager);
};

}  // namespace base

#endif  // BASE_TIMER_HI_RES_TIMER_MANAGER_H_
