// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/WebRtcClassLoader

#ifndef org_webrtc_WebRtcClassLoader_JNI
#define org_webrtc_WebRtcClassLoader_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_WebRtcClassLoader[];
const char kClassPath_org_webrtc_WebRtcClassLoader[] = "org/webrtc/WebRtcClassLoader";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_WebRtcClassLoader_clazz(nullptr);
#ifndef org_webrtc_WebRtcClassLoader_clazz_defined
#define org_webrtc_WebRtcClassLoader_clazz_defined
inline jclass org_webrtc_WebRtcClassLoader_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_WebRtcClassLoader,
      &g_org_webrtc_WebRtcClassLoader_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_WebRtcClassLoader_getClassLoader(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_WebRtcClassLoader_getClassLoader(JNIEnv* env)
    {
  jclass clazz = org_webrtc_WebRtcClassLoader_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_WebRtcClassLoader_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getClassLoader",
          "()Ljava/lang/Object;",
          &g_org_webrtc_WebRtcClassLoader_getClassLoader);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_WebRtcClassLoader_JNI
