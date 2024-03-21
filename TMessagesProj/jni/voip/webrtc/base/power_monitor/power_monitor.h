// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POWER_MONITOR_POWER_MONITOR_H_
#define BASE_POWER_MONITOR_POWER_MONITOR_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/no_destructor.h"
#include "base/observer_list_threadsafe.h"
#include "base/power_monitor/power_observer.h"

namespace base {

class PowerMonitorSource;

// the change event. The threading model of this class is as follows:
// Once initialized, it is threadsafe. However, the client must ensure that
// initialization happens before any other methods are invoked, including
// IsInitialized(). IsInitialized() exists only as a convenience for detection
// of test contexts where the PowerMonitor global is never created.
class BASE_EXPORT PowerMonitor {
 public:




  static void Initialize(std::unique_ptr<PowerMonitorSource> source);



  static bool IsInitialized();










  static bool AddObserver(PowerObserver* observer);
  static void RemoveObserver(PowerObserver* observer);


  static bool IsOnBatteryPower();




  static void ShutdownForTesting();

 private:
  friend class PowerMonitorSource;
  friend class base::NoDestructor<PowerMonitor>;

  PowerMonitor();
  ~PowerMonitor();

  static PowerMonitorSource* Source();

  static void NotifyPowerStateChange(bool battery_in_use);
  static void NotifySuspend();
  static void NotifyResume();

  static PowerMonitor* GetInstance();

  scoped_refptr<ObserverListThreadSafe<PowerObserver>> observers_;
  std::unique_ptr<PowerMonitorSource> source_;

  DISALLOW_COPY_AND_ASSIGN(PowerMonitor);
};

}  // namespace base

#endif  // BASE_POWER_MONITOR_POWER_MONITOR_H_
