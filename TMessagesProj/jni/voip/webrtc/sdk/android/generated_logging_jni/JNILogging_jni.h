// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/JNILogging

#ifndef org_webrtc_JNILogging_JNI
#define org_webrtc_JNILogging_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_JNILogging[];
const char kClassPath_org_webrtc_JNILogging[] = "org/webrtc/JNILogging";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_JNILogging_clazz(nullptr);
#ifndef org_webrtc_JNILogging_clazz_defined
#define org_webrtc_JNILogging_clazz_defined
inline jclass org_webrtc_JNILogging_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_JNILogging,
      &g_org_webrtc_JNILogging_clazz);
}
#endif



static std::atomic<jmethodID> g_org_webrtc_JNILogging_logToInjectable(nullptr);
static void Java_JNILogging_logToInjectable(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jstring>& message,
    const base::android::JavaRef<jobject>& severity,
    const base::android::JavaRef<jstring>& tag) {
  jclass clazz = org_webrtc_JNILogging_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_JNILogging_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "logToInjectable",
          "(Ljava/lang/String;Ljava/lang/Integer;Ljava/lang/String;)V",
          &g_org_webrtc_JNILogging_logToInjectable);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, message.obj(), severity.obj(), tag.obj());
}

#endif  // org_webrtc_JNILogging_JNI
