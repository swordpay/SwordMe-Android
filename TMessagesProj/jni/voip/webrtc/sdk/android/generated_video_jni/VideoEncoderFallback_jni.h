// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoEncoderFallback

#ifndef org_webrtc_VideoEncoderFallback_JNI
#define org_webrtc_VideoEncoderFallback_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoderFallback[];
const char kClassPath_org_webrtc_VideoEncoderFallback[] = "org/webrtc/VideoEncoderFallback";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoEncoderFallback_clazz(nullptr);
#ifndef org_webrtc_VideoEncoderFallback_clazz_defined
#define org_webrtc_VideoEncoderFallback_clazz_defined
inline jclass org_webrtc_VideoEncoderFallback_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoderFallback,
      &g_org_webrtc_VideoEncoderFallback_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jlong JNI_VideoEncoderFallback_CreateEncoder(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& fallback,
    const base::android::JavaParamRef<jobject>& primary);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_VideoEncoderFallback_nativeCreateEncoder(
    JNIEnv* env,
    jclass jcaller,
    jobject fallback,
    jobject primary) {
  return JNI_VideoEncoderFallback_CreateEncoder(env, base::android::JavaParamRef<jobject>(env,
      fallback), base::android::JavaParamRef<jobject>(env, primary));
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoEncoderFallback_JNI
