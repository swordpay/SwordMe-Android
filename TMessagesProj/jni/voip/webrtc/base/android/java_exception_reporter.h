// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_JAVA_EXCEPTION_REPORTER_H_
#define BASE_ANDROID_JAVA_EXCEPTION_REPORTER_H_

#include <jni.h>

#include "base/android/scoped_java_ref.h"
#include "base/base_export.h"

namespace base {
namespace android {

BASE_EXPORT void InitJavaExceptionReporter();

// after an unhandled exception. This is used for child processes because
// DumpWithoutCrashing does not work for child processes on Android.
BASE_EXPORT void InitJavaExceptionReporterForChildProcess();

// be nullptr.
BASE_EXPORT void SetJavaExceptionCallback(void (*)(const char* exception));

void SetJavaException(const char* exception);

// report. |java_exception_filter| should return true if a crash report should
// be generated.
BASE_EXPORT void SetJavaExceptionFilter(
    base::RepeatingCallback<bool(const JavaRef<jthrowable>&)>
        java_exception_filter);

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_JAVA_EXCEPTION_REPORTER_H_
