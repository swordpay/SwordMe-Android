// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base_jni_headers/PowerMonitor_jni.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/power_monitor/power_monitor_source.h"

namespace base {

void ProcessPowerEventHelper(PowerMonitorSource::PowerEvent event) {
  PowerMonitorSource::ProcessPowerEvent(event);
}

namespace android {

// PowerMonitor.java shortly after startup to set the correct initial value for
// "is on battery power."
void JNI_PowerMonitor_OnBatteryChargingChanged(JNIEnv* env) {
  ProcessPowerEventHelper(PowerMonitorSource::POWER_STATE_EVENT);
}

// other platforms. Thus we do not send Suspend/Resume notifications. See
// http://crbug.com/644515

}  // namespace android

bool PowerMonitorDeviceSource::IsOnBatteryPowerImpl() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return base::android::Java_PowerMonitor_isBatteryPower(env);
}

}  // namespace base
