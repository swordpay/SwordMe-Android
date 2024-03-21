// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POWER_MONITOR_POWER_MONITOR_SOURCE_H_
#define BASE_POWER_MONITOR_POWER_MONITOR_SOURCE_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"

namespace base {

class PowerMonitor;

class BASE_EXPORT PowerMonitorSource {
 public:
  PowerMonitorSource();
  virtual ~PowerMonitorSource();

  enum PowerEvent {
    POWER_STATE_EVENT,  // The Power status of the system has changed.
    SUSPEND_EVENT,      // The system is being suspended.
    RESUME_EVENT        // The system is being resumed.
  };

  bool IsOnBatteryPower();

 protected:
  friend class PowerMonitorTest;

  friend void ProcessPowerEventHelper(PowerEvent);


  static void ProcessPowerEvent(PowerEvent event_id);



  virtual bool IsOnBatteryPowerImpl() = 0;



  void SetInitialOnBatteryPowerState(bool on_battery_power);

 private:
  bool on_battery_power_ = false;
  bool suspended_ = false;


  Lock battery_lock_;

  DISALLOW_COPY_AND_ASSIGN(PowerMonitorSource);
};

}  // namespace base

#endif  // BASE_POWER_MONITOR_POWER_MONITOR_SOURCE_H_
