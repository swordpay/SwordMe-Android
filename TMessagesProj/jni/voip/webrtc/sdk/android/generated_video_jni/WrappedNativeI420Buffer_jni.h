// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/WrappedNativeI420Buffer

#ifndef org_webrtc_WrappedNativeI420Buffer_JNI
#define org_webrtc_WrappedNativeI420Buffer_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_WrappedNativeI420Buffer[];
const char kClassPath_org_webrtc_WrappedNativeI420Buffer[] = "org/webrtc/WrappedNativeI420Buffer";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_WrappedNativeI420Buffer_clazz(nullptr);
#ifndef org_webrtc_WrappedNativeI420Buffer_clazz_defined
#define org_webrtc_WrappedNativeI420Buffer_clazz_defined
inline jclass org_webrtc_WrappedNativeI420Buffer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_WrappedNativeI420Buffer,
      &g_org_webrtc_WrappedNativeI420Buffer_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_WrappedNativeI420Buffer_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_WrappedNativeI420Buffer_Constructor(JNIEnv*
    env, JniIntWrapper width,
    JniIntWrapper height,
    const base::android::JavaRef<jobject>& dataY,
    JniIntWrapper strideY,
    const base::android::JavaRef<jobject>& dataU,
    JniIntWrapper strideU,
    const base::android::JavaRef<jobject>& dataV,
    JniIntWrapper strideV,
    jlong nativeBuffer) {
  jclass clazz = org_webrtc_WrappedNativeI420Buffer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_WrappedNativeI420Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(IILjava/nio/ByteBuffer;ILjava/nio/ByteBuffer;ILjava/nio/ByteBuffer;IJ)V",
          &g_org_webrtc_WrappedNativeI420Buffer_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(width), as_jint(height), dataY.obj(),
              as_jint(strideY), dataU.obj(), as_jint(strideU), dataV.obj(), as_jint(strideV),
              nativeBuffer);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_WrappedNativeI420Buffer_JNI
