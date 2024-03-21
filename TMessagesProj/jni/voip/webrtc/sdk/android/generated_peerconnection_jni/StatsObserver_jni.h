// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/StatsObserver

#ifndef org_webrtc_StatsObserver_JNI
#define org_webrtc_StatsObserver_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_StatsObserver[];
const char kClassPath_org_webrtc_StatsObserver[] = "org/webrtc/StatsObserver";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_StatsObserver_clazz(nullptr);
#ifndef org_webrtc_StatsObserver_clazz_defined
#define org_webrtc_StatsObserver_clazz_defined
inline jclass org_webrtc_StatsObserver_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_StatsObserver,
      &g_org_webrtc_StatsObserver_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_StatsObserver_onComplete(nullptr);
static void Java_StatsObserver_onComplete(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobjectArray>& reports) {
  jclass clazz = org_webrtc_StatsObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_StatsObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onComplete",
          "([Lorg/webrtc/StatsReport;)V",
          &g_org_webrtc_StatsObserver_onComplete);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, reports.obj());
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_StatsObserver_JNI
