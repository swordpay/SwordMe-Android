// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/RTCStatsCollectorCallback

#ifndef org_webrtc_RTCStatsCollectorCallback_JNI
#define org_webrtc_RTCStatsCollectorCallback_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RTCStatsCollectorCallback[];
const char kClassPath_org_webrtc_RTCStatsCollectorCallback[] =
    "org/webrtc/RTCStatsCollectorCallback";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RTCStatsCollectorCallback_clazz(nullptr);
#ifndef org_webrtc_RTCStatsCollectorCallback_clazz_defined
#define org_webrtc_RTCStatsCollectorCallback_clazz_defined
inline jclass org_webrtc_RTCStatsCollectorCallback_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RTCStatsCollectorCallback,
      &g_org_webrtc_RTCStatsCollectorCallback_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_RTCStatsCollectorCallback_onStatsDelivered(nullptr);
static void Java_RTCStatsCollectorCallback_onStatsDelivered(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& report) {
  jclass clazz = org_webrtc_RTCStatsCollectorCallback_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RTCStatsCollectorCallback_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onStatsDelivered",
          "(Lorg/webrtc/RTCStatsReport;)V",
          &g_org_webrtc_RTCStatsCollectorCallback_onStatsDelivered);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, report.obj());
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_RTCStatsCollectorCallback_JNI
