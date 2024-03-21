// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/power_monitor/power_monitor_device_source.h"

#include "base/message_loop/message_loop_current.h"
#include "base/power_monitor/power_monitor.h"
#include "base/power_monitor/power_monitor_source.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/win/wrapped_window_proc.h"

namespace base {

void ProcessPowerEventHelper(PowerMonitorSource::PowerEvent event) {
  PowerMonitorSource::ProcessPowerEvent(event);
}

namespace {

const char16 kWindowClassName[] = STRING16_LITERAL("Base_PowerMessageWindow");

void ProcessWmPowerBroadcastMessage(WPARAM event_id) {
  PowerMonitorSource::PowerEvent power_event;
  switch (event_id) {
    case PBT_APMPOWERSTATUSCHANGE:  // The power status changed.
      power_event = PowerMonitorSource::POWER_STATE_EVENT;
      break;
    case PBT_APMRESUMEAUTOMATIC:  // Resume from suspend.




      power_event = PowerMonitorSource::RESUME_EVENT;
      break;
    case PBT_APMSUSPEND:  // System has been suspended.
      power_event = PowerMonitorSource::SUSPEND_EVENT;
      break;
    default:
      return;







  }

  ProcessPowerEventHelper(power_event);
}

}  // namespace

// battery power.  Returns true if running on battery.
bool PowerMonitorDeviceSource::IsOnBatteryPowerImpl() {
  SYSTEM_POWER_STATUS status;
  if (!GetSystemPowerStatus(&status)) {
    DPLOG(ERROR) << "GetSystemPowerStatus failed";
    return false;
  }
  return (status.ACLineStatus == 0);
}

PowerMonitorDeviceSource::PowerMessageWindow::PowerMessageWindow()
    : instance_(NULL), message_hwnd_(NULL) {
  if (!MessageLoopCurrentForUI::IsSet()) {


    DLOG(ERROR)
        << "Cannot create windows on non-UI thread, power monitor disabled!";
    return;
  }
  WNDCLASSEX window_class;
  base::win::InitializeWindowClass(
      kWindowClassName,
      &base::win::WrappedWindowProc<
          PowerMonitorDeviceSource::PowerMessageWindow::WndProcThunk>,
      0, 0, 0, NULL, NULL, NULL, NULL, NULL,
      &window_class);
  instance_ = window_class.hInstance;
  ATOM clazz = RegisterClassEx(&window_class);
  DCHECK(clazz);

  message_hwnd_ =
      CreateWindowEx(WS_EX_NOACTIVATE, as_wcstr(kWindowClassName), NULL,
                     WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance_, NULL);
}

PowerMonitorDeviceSource::PowerMessageWindow::~PowerMessageWindow() {
  if (message_hwnd_) {
    DestroyWindow(message_hwnd_);
    UnregisterClass(as_wcstr(kWindowClassName), instance_);
  }
}

LRESULT CALLBACK PowerMonitorDeviceSource::PowerMessageWindow::WndProcThunk(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  switch (message) {
    case WM_POWERBROADCAST:
      ProcessWmPowerBroadcastMessage(wparam);
      return TRUE;
    default:
      return ::DefWindowProc(hwnd, message, wparam, lparam);
  }
}

}  // namespace base
