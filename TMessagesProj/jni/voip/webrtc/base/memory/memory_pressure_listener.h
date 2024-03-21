// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// platforms that have such signals, such as Android and ChromeOS.
// The app will try to discard buffers that aren't deemed essential (individual
// modules will implement their own policy).

#ifndef BASE_MEMORY_MEMORY_PRESSURE_LISTENER_H_
#define BASE_MEMORY_MEMORY_PRESSURE_LISTENER_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"

namespace base {

// function that takes a MemoryPressureLevel parameter. To stop listening,
// simply delete the listener object. The implementation guarantees
// that the callback will always be called on the thread that created
// the listener.
// Note that even on the same thread, the callback is not guaranteed to be
// called synchronously within the system memory pressure broadcast.
// Please see notes in MemoryPressureLevel enum below: some levels are
// absolutely critical, and if not enough memory is returned to the system,
// it'll potentially kill the app, and then later the app will have to be
// cold-started.
//
// Example:
//
//    void OnMemoryPressure(MemoryPressureLevel memory_pressure_level) {
//       ...
//    }
//
//    // Start listening.
//    auto listener = std::make_unique<MemoryPressureListener>(
//        base::BindRepeating(&OnMemoryPressure));
//
//    ...
//
//    // Stop listening.
//    listener.reset();
//
class BASE_EXPORT MemoryPressureListener {
 public:


  enum MemoryPressureLevel {



    MEMORY_PRESSURE_LEVEL_NONE,


    MEMORY_PRESSURE_LEVEL_MODERATE,



    MEMORY_PRESSURE_LEVEL_CRITICAL,
  };

  using MemoryPressureCallback = RepeatingCallback<void(MemoryPressureLevel)>;
  using SyncMemoryPressureCallback =
      RepeatingCallback<void(MemoryPressureLevel)>;

  explicit MemoryPressureListener(
      const MemoryPressureCallback& memory_pressure_callback);
  MemoryPressureListener(
      const MemoryPressureCallback& memory_pressure_callback,
      const SyncMemoryPressureCallback& sync_memory_pressure_callback);

  ~MemoryPressureListener();

  static void NotifyMemoryPressure(MemoryPressureLevel memory_pressure_level);



  static bool AreNotificationsSuppressed();
  static void SetNotificationsSuppressed(bool suppressed);
  static void SimulatePressureNotification(
      MemoryPressureLevel memory_pressure_level);

  void Notify(MemoryPressureLevel memory_pressure_level);
  void SyncNotify(MemoryPressureLevel memory_pressure_level);

 private:
  static void DoNotifyMemoryPressure(MemoryPressureLevel memory_pressure_level);

  MemoryPressureCallback callback_;
  SyncMemoryPressureCallback sync_memory_pressure_callback_;

  DISALLOW_COPY_AND_ASSIGN(MemoryPressureListener);
};

}  // namespace base

#endif  // BASE_MEMORY_MEMORY_PRESSURE_LISTENER_H_
