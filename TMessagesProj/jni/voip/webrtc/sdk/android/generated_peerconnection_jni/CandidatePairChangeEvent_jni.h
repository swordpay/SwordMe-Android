// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/CandidatePairChangeEvent

#ifndef org_webrtc_CandidatePairChangeEvent_JNI
#define org_webrtc_CandidatePairChangeEvent_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_CandidatePairChangeEvent[];
const char kClassPath_org_webrtc_CandidatePairChangeEvent[] = "org/webrtc/CandidatePairChangeEvent";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_CandidatePairChangeEvent_clazz(nullptr);
#ifndef org_webrtc_CandidatePairChangeEvent_clazz_defined
#define org_webrtc_CandidatePairChangeEvent_clazz_defined
inline jclass org_webrtc_CandidatePairChangeEvent_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_CandidatePairChangeEvent,
      &g_org_webrtc_CandidatePairChangeEvent_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_CandidatePairChangeEvent_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_CandidatePairChangeEvent_Constructor(JNIEnv*
    env, const base::android::JavaRef<jobject>& local,
    const base::android::JavaRef<jobject>& remote,
    JniIntWrapper lastDataReceivedMs,
    const base::android::JavaRef<jstring>& reason,
    JniIntWrapper estimatedDisconnectedTimeMs) {
  jclass clazz = org_webrtc_CandidatePairChangeEvent_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_CandidatePairChangeEvent_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Lorg/webrtc/IceCandidate;Lorg/webrtc/IceCandidate;ILjava/lang/String;I)V",
          &g_org_webrtc_CandidatePairChangeEvent_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, local.obj(), remote.obj(), as_jint(lastDataReceivedMs),
              reason.obj(), as_jint(estimatedDisconnectedTimeMs));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_CandidatePairChangeEvent_JNI
