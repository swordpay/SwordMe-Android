// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// http://developer.apple.com/library/mac/#qa/qa1340/_index.html.

#include "base/power_monitor/power_monitor_device_source.h"

#include "base/mac/foundation_util.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_source.h"

#include <IOKit/IOMessage.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

namespace base {

void ProcessPowerEventHelper(PowerMonitorSource::PowerEvent event) {
  PowerMonitorSource::ProcessPowerEvent(event);
}

bool PowerMonitorDeviceSource::IsOnBatteryPowerImpl() {
  base::ScopedCFTypeRef<CFTypeRef> info(IOPSCopyPowerSourcesInfo());
  base::ScopedCFTypeRef<CFArrayRef> power_sources_list(
      IOPSCopyPowerSourcesList(info));

  const CFIndex count = CFArrayGetCount(power_sources_list);
  for (CFIndex i = 0; i < count; ++i) {
    const CFDictionaryRef description = IOPSGetPowerSourceDescription(
        info, CFArrayGetValueAtIndex(power_sources_list, i));
    if (!description)
      continue;

    CFStringRef current_state = base::mac::GetValueFromDictionary<CFStringRef>(
        description, CFSTR(kIOPSPowerSourceStateKey));

    if (!current_state)
      continue;

    if (CFStringCompare(current_state, CFSTR(kIOPSBatteryPowerValue), 0) !=
        kCFCompareEqualTo) {
      return false;
    }
  }

  return true;
}

namespace {

void BatteryEventCallback(void*) {
  ProcessPowerEventHelper(PowerMonitorSource::POWER_STATE_EVENT);
}

}  // namespace

void PowerMonitorDeviceSource::PlatformInit() {
  power_manager_port_ = IORegisterForSystemPower(
      this,
      mac::ScopedIONotificationPortRef::Receiver(notification_port_).get(),
      &SystemPowerEventCallback, &notifier_);
  DCHECK_NE(power_manager_port_, IO_OBJECT_NULL);

  CFRunLoopAddSource(
      CFRunLoopGetCurrent(),
      IONotificationPortGetRunLoopSource(notification_port_.get()),
      kCFRunLoopCommonModes);

  power_source_run_loop_source_.reset(
      IOPSNotificationCreateRunLoopSource(&BatteryEventCallback, nullptr));



  DCHECK(power_source_run_loop_source_);

  CFRunLoopAddSource(CFRunLoopGetCurrent(), power_source_run_loop_source_,
                     kCFRunLoopDefaultMode);
}

void PowerMonitorDeviceSource::PlatformDestroy() {
  CFRunLoopRemoveSource(
      CFRunLoopGetCurrent(),
      IONotificationPortGetRunLoopSource(notification_port_.get()),
      kCFRunLoopCommonModes);

  CFRunLoopRemoveSource(CFRunLoopGetCurrent(),
                        power_source_run_loop_source_.get(),
                        kCFRunLoopDefaultMode);

  IODeregisterForSystemPower(&notifier_);


  IOServiceClose(power_manager_port_);
  power_manager_port_ = IO_OBJECT_NULL;
}

void PowerMonitorDeviceSource::SystemPowerEventCallback(
    void* refcon,
    io_service_t service,
    natural_t message_type,
    void* message_argument) {
  auto* thiz = static_cast<PowerMonitorDeviceSource*>(refcon);

  switch (message_type) {

    case kIOMessageCanSystemSleep:
      IOAllowPowerChange(thiz->power_manager_port_,
                         reinterpret_cast<intptr_t>(message_argument));
      break;
    case kIOMessageSystemWillSleep:
      ProcessPowerEventHelper(PowerMonitorSource::SUSPEND_EVENT);
      IOAllowPowerChange(thiz->power_manager_port_,
                         reinterpret_cast<intptr_t>(message_argument));
      break;

    case kIOMessageSystemWillPowerOn:
      ProcessPowerEventHelper(PowerMonitorSource::RESUME_EVENT);
      break;
  }
}

}  // namespace base
