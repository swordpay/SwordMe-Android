// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POWER_MONITOR_POWER_OBSERVER_H_
#define BASE_POWER_MONITOR_POWER_OBSERVER_H_

#include "base/base_export.h"
#include "base/compiler_specific.h"

namespace base {

class BASE_EXPORT PowerObserver {
 public:


  virtual void OnPowerStateChange(bool on_battery_power) {}

  virtual void OnSuspend() {}

  virtual void OnResume() {}

 protected:
  virtual ~PowerObserver() = default;
};

}  // namespace base

#endif  // BASE_POWER_MONITOR_POWER_OBSERVER_H_
