// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POWER_MONITOR_POWER_MONITOR_DEVICE_SOURCE_H_
#define BASE_POWER_MONITOR_POWER_MONITOR_DEVICE_SOURCE_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/power_monitor/power_monitor_source.h"
#include "base/power_monitor/power_observer.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#endif  // !OS_WIN

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include <IOKit/IOTypes.h>

#include "base/mac/scoped_cftyperef.h"
#include "base/mac/scoped_ionotificationportref.h"
#endif

#if defined(OS_IOS)
#include <objc/runtime.h>
#endif  // OS_IOS

namespace base {

// the change event.
class BASE_EXPORT PowerMonitorDeviceSource : public PowerMonitorSource {
 public:
  PowerMonitorDeviceSource();
  ~PowerMonitorDeviceSource() override;

#if defined(OS_CHROMEOS)




  static void SetPowerSource(bool on_battery);
  static void HandleSystemSuspending();
  static void HandleSystemResumed();
#endif

 private:
#if defined(OS_WIN)


  class PowerMessageWindow {
   public:
    PowerMessageWindow();
    ~PowerMessageWindow();

   private:
    static LRESULT CALLBACK WndProcThunk(HWND hwnd,
                                         UINT message,
                                         WPARAM wparam,
                                         LPARAM lparam);

    HMODULE instance_;

    HWND message_hwnd_;
  };
#endif  // OS_WIN

#if defined(OS_MACOSX)
  void PlatformInit();
  void PlatformDestroy();

#if !defined(OS_IOS)

  static void SystemPowerEventCallback(void* refcon,
                                       io_service_t service,
                                       natural_t message_type,
                                       void* message_argument);
#endif  // !OS_IOS
#endif  // OS_MACOSX



  bool IsOnBatteryPowerImpl() override;

#if defined(OS_MACOSX) && !defined(OS_IOS)

  io_connect_t power_manager_port_ = IO_OBJECT_NULL;

  mac::ScopedIONotificationPortRef notification_port_;

  io_object_t notifier_ = IO_OBJECT_NULL;

  ScopedCFTypeRef<CFRunLoopSourceRef> power_source_run_loop_source_;
#endif

#if defined(OS_IOS)

  std::vector<id> notification_observers_;
#endif

#if defined(OS_WIN)
  PowerMessageWindow power_message_window_;
#endif

  DISALLOW_COPY_AND_ASSIGN(PowerMonitorDeviceSource);
};

}  // namespace base

#endif  // BASE_POWER_MONITOR_POWER_MONITOR_DEVICE_SOURCE_H_
