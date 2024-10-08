// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/IceCandidateErrorEvent

#ifndef org_webrtc_IceCandidateErrorEvent_JNI
#define org_webrtc_IceCandidateErrorEvent_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_IceCandidateErrorEvent[];
const char kClassPath_org_webrtc_IceCandidateErrorEvent[] = "org/webrtc/IceCandidateErrorEvent";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_IceCandidateErrorEvent_clazz(nullptr);
#ifndef org_webrtc_IceCandidateErrorEvent_clazz_defined
#define org_webrtc_IceCandidateErrorEvent_clazz_defined
inline jclass org_webrtc_IceCandidateErrorEvent_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_IceCandidateErrorEvent,
      &g_org_webrtc_IceCandidateErrorEvent_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_IceCandidateErrorEvent_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceCandidateErrorEvent_Constructor(JNIEnv*
    env, const base::android::JavaRef<jstring>& address,
    JniIntWrapper port,
    const base::android::JavaRef<jstring>& url,
    JniIntWrapper errorCode,
    const base::android::JavaRef<jstring>& errorText) {
  jclass clazz = org_webrtc_IceCandidateErrorEvent_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_IceCandidateErrorEvent_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;)V",
          &g_org_webrtc_IceCandidateErrorEvent_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, address.obj(), as_jint(port), url.obj(), as_jint(errorCode),
              errorText.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_IceCandidateErrorEvent_JNI
