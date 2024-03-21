// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_APPLICATION_STATUS_LISTENER_H_
#define BASE_ANDROID_APPLICATION_STATUS_LISTENER_H_

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"

namespace base {
namespace android {

// way that ensures they're always the same than their Java counterpart.
//
// Note that these states represent the most visible Activity state.
// If there are activities with states paused and stopped, only
// HAS_PAUSED_ACTIVITIES should be returned.
//
// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.base
enum ApplicationState {
  APPLICATION_STATE_UNKNOWN = 0,
  APPLICATION_STATE_HAS_RUNNING_ACTIVITIES = 1,
  APPLICATION_STATE_HAS_PAUSED_ACTIVITIES = 2,
  APPLICATION_STATE_HAS_STOPPED_ACTIVITIES = 3,
  APPLICATION_STATE_HAS_DESTROYED_ACTIVITIES = 4
};

// Application. This mirrors org.chromium.base.ApplicationStatus.
// any thread.
//
// To start listening, create a new instance, passing a callback to a
// function that takes an ApplicationState parameter. To stop listening,
// simply delete the listener object. The implementation guarantees
// that the callback will always be called on the thread that created
// the listener.
//
// Example:
//
//    void OnApplicationStateChange(ApplicationState state) {
//       ...
//    }
//
//    // Start listening.
//    auto my_listener = ApplicationStatusListener::New(
//        base::BindRepeating(&OnApplicationStateChange));
//
//    ...
//
//    // Stop listening.
//    my_listener.reset();
//
class BASE_EXPORT ApplicationStatusListener {
 public:
  using ApplicationStateChangeCallback =
      base::RepeatingCallback<void(ApplicationState)>;

  virtual ~ApplicationStatusListener();

  virtual void SetCallback(const ApplicationStateChangeCallback& callback) = 0;

  virtual void Notify(ApplicationState state) = 0;

  static std::unique_ptr<ApplicationStatusListener> New(
      const ApplicationStateChangeCallback& callback);

  static void NotifyApplicationStateChange(ApplicationState state);

  static ApplicationState GetState();

  static bool HasVisibleActivities();

 protected:
  ApplicationStatusListener();

 private:
  DISALLOW_COPY_AND_ASSIGN(ApplicationStatusListener);
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_APPLICATION_STATUS_LISTENER_H_
