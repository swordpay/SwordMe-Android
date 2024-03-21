// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_LIBRARY_LOADER_LIBRARY_LOADER_HOOKS_H_
#define BASE_ANDROID_LIBRARY_LOADER_LIBRARY_LOADER_HOOKS_H_

#include <jni.h>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/metrics/field_trial.h"

namespace base {

namespace android {

// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.base.library_loader
enum LibraryProcessType {

  PROCESS_UNINITIALIZED = 0,

  PROCESS_BROWSER = 1,

  PROCESS_CHILD = 2,

  PROCESS_WEBVIEW = 3,

  PROCESS_WEBVIEW_CHILD = 4,

  PROCESS_WEBLAYER = 5,

  PROCESS_WEBLAYER_CHILD = 6,
};

// Returns true on low-end devices, where this speeds up startup, and false
// elsewhere, where it slows it down. See
// https://bugs.chromium.org/p/chromium/issues/detail?id=758566#c71 for details.
BASE_EXPORT bool IsUsingOrderfileOptimization();

typedef bool NativeInitializationHook(LibraryProcessType library_process_type);

BASE_EXPORT void SetNativeInitializationHook(
    NativeInitializationHook native_initialization_hook);

typedef void NonMainDexJniRegistrationHook();

BASE_EXPORT void SetNonMainDexJniRegistrationHook(
    NonMainDexJniRegistrationHook jni_registration_hook);

// are set by
// JNI_LibraryLoader_RegisterChromiumAndroidLinkerRendererHistogram().
BASE_EXPORT void RecordLibraryLoaderRendererHistograms();

// libraries are loaded. The hook function should register the JNI bindings
// required to start the application. It should return true for success and
// false for failure.
// Note: this can't use base::Callback because there is no way of initializing
// the default callback without using static objects, which we forbid.
typedef bool LibraryLoadedHook(JNIEnv* env,
                               jclass clazz,
                               LibraryProcessType library_process_type);

// SetLibraryLoadedHook may only be called from JNI_OnLoad. The hook function
// should register the JNI bindings required to start the application.

BASE_EXPORT void SetLibraryLoadedHook(LibraryLoadedHook* func);

// version matches the version expected by Java before completing JNI
// registration.
// Note: argument must remain valid at least until library loading is complete.
BASE_EXPORT void SetVersionNumber(const char* version_number);

// created.
BASE_EXPORT void LibraryLoaderExitHook();

// shared library.
void InitAtExitManager();

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_LIBRARY_LOADER_LIBRARY_LOADER_HOOKS_H_
