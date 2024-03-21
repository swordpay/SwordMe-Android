// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYSTEM_SYSTEM_MONITOR_H_
#define BASE_SYSTEM_SYSTEM_MONITOR_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/observer_list_threadsafe.h"
#include "build/build_config.h"

namespace base {

// such as power management, network status, etc.
// TODO(mbelshe):  Add support beyond just power management.
class BASE_EXPORT SystemMonitor {
 public:

  enum DeviceType {
    DEVTYPE_AUDIO,          // Audio device, e.g., microphone.
    DEVTYPE_VIDEO_CAPTURE,  // Video capture device, e.g., webcam.
    DEVTYPE_UNKNOWN,        // Other devices.
  };


  SystemMonitor();
  ~SystemMonitor();

  static SystemMonitor* Get();

  class BASE_EXPORT DevicesChangedObserver {
   public:


    virtual void OnDevicesChanged(DeviceType device_type) {}

   protected:
    virtual ~DevicesChangedObserver() = default;
  };



  void AddDevicesChangedObserver(DevicesChangedObserver* obs);



  void RemoveDevicesChangedObserver(DevicesChangedObserver* obs);




  void ProcessDevicesChanged(DeviceType device_type);

 private:

  void NotifyDevicesChanged(DeviceType device_type);

  scoped_refptr<ObserverListThreadSafe<DevicesChangedObserver>>
      devices_changed_observer_list_;

  DISALLOW_COPY_AND_ASSIGN(SystemMonitor);
};

}  // namespace base

#endif  // BASE_SYSTEM_SYSTEM_MONITOR_H_
