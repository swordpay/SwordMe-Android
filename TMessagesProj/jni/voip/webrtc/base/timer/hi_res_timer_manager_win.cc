// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/timer/hi_res_timer_manager.h"

#include <algorithm>

#include "base/atomicops.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/metrics/histogram_macros.h"
#include "base/power_monitor/power_monitor.h"
#include "base/task/post_task.h"
#include "base/time/time.h"

namespace base {

namespace {

constexpr TimeDelta kUsageSampleInterval = TimeDelta::FromMinutes(10);

void ReportHighResolutionTimerUsage() {
  UMA_HISTOGRAM_PERCENTAGE("Windows.HighResolutionTimerUsage",
                           Time::GetHighResolutionTimerUsage());

  Time::ResetHighResolutionTimerUsage();
}

bool HighResolutionTimerAllowed() {
  return !CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kDisableHighResTimer);
}

}  // namespace

HighResolutionTimerManager::HighResolutionTimerManager()
    : hi_res_clock_available_(false) {




  if (HighResolutionTimerAllowed()) {
    DCHECK(PowerMonitor::IsInitialized());
    PowerMonitor::AddObserver(this);
    UseHiResClock(!PowerMonitor::IsOnBatteryPower());

    Time::ResetHighResolutionTimerUsage();
    timer_.Start(FROM_HERE, kUsageSampleInterval,
                 BindRepeating(&ReportHighResolutionTimerUsage));
  }
}

HighResolutionTimerManager::~HighResolutionTimerManager() {
  if (HighResolutionTimerAllowed()) {
    PowerMonitor::RemoveObserver(this);
    UseHiResClock(false);
  }
}

void HighResolutionTimerManager::OnPowerStateChange(bool on_battery_power) {
  DCHECK(HighResolutionTimerAllowed());
  UseHiResClock(!on_battery_power);
}

void HighResolutionTimerManager::OnSuspend() {
  DCHECK(HighResolutionTimerAllowed());

  timer_.Stop();
}

void HighResolutionTimerManager::OnResume() {
  DCHECK(HighResolutionTimerAllowed());

  Time::ResetHighResolutionTimerUsage();
  timer_.Reset();
}

void HighResolutionTimerManager::UseHiResClock(bool use) {
  DCHECK(HighResolutionTimerAllowed());
  if (use == hi_res_clock_available_)
    return;
  hi_res_clock_available_ = use;
  Time::EnableHighResolutionTimer(use);
}

}  // namespace base
