// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_MEMORY_PRESSURE_MONITOR_H_
#define BASE_MEMORY_MEMORY_PRESSURE_MONITOR_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/memory_pressure_listener.h"
#include "base/time/time.h"

namespace base {

// than an abstract base class.

// OS specific implementations of this class. An instance of the memory
// pressure observer is created at the process level, tracks memory usage, and
// pushes memory state change notifications to the static function
// base::MemoryPressureListener::NotifyMemoryPressure. This is turn notifies
// all MemoryPressureListener instances via a callback.
class BASE_EXPORT MemoryPressureMonitor {
 public:
  using MemoryPressureLevel = base::MemoryPressureListener::MemoryPressureLevel;
  using DispatchCallback =
      base::RepeatingCallback<void(MemoryPressureLevel level)>;

  virtual ~MemoryPressureMonitor();

  static MemoryPressureMonitor* Get();

  static void RecordMemoryPressure(MemoryPressureLevel level, int ticks);

  static const base::TimeDelta kUMAMemoryPressureLevelPeriod;

  virtual MemoryPressureLevel GetCurrentPressureLevel() const = 0;


  virtual void SetDispatchCallback(const DispatchCallback& callback) = 0;

 protected:
  MemoryPressureMonitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(MemoryPressureMonitor);
};

}  // namespace base

#endif  // BASE_MEMORY_MEMORY_PRESSURE_MONITOR_H_
