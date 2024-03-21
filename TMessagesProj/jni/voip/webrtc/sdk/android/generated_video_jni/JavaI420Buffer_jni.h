// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/JavaI420Buffer

#ifndef org_webrtc_JavaI420Buffer_JNI
#define org_webrtc_JavaI420Buffer_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_JavaI420Buffer[];
const char kClassPath_org_webrtc_JavaI420Buffer[] = "org/webrtc/JavaI420Buffer";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_JavaI420Buffer_clazz(nullptr);
#ifndef org_webrtc_JavaI420Buffer_clazz_defined
#define org_webrtc_JavaI420Buffer_clazz_defined
inline jclass org_webrtc_JavaI420Buffer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_JavaI420Buffer,
      &g_org_webrtc_JavaI420Buffer_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static void JNI_JavaI420Buffer_CropAndScaleI420(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& srcY,
    jint srcStrideY,
    const base::android::JavaParamRef<jobject>& srcU,
    jint srcStrideU,
    const base::android::JavaParamRef<jobject>& srcV,
    jint srcStrideV,
    jint cropX,
    jint cropY,
    jint cropWidth,
    jint cropHeight,
    const base::android::JavaParamRef<jobject>& dstY,
    jint dstStrideY,
    const base::android::JavaParamRef<jobject>& dstU,
    jint dstStrideU,
    const base::android::JavaParamRef<jobject>& dstV,
    jint dstStrideV,
    jint scaleWidth,
    jint scaleHeight);

JNI_GENERATOR_EXPORT void Java_org_webrtc_JavaI420Buffer_nativeCropAndScaleI420(
    JNIEnv* env,
    jclass jcaller,
    jobject srcY,
    jint srcStrideY,
    jobject srcU,
    jint srcStrideU,
    jobject srcV,
    jint srcStrideV,
    jint cropX,
    jint cropY,
    jint cropWidth,
    jint cropHeight,
    jobject dstY,
    jint dstStrideY,
    jobject dstU,
    jint dstStrideU,
    jobject dstV,
    jint dstStrideV,
    jint scaleWidth,
    jint scaleHeight) {
  return JNI_JavaI420Buffer_CropAndScaleI420(env, base::android::JavaParamRef<jobject>(env, srcY),
      srcStrideY, base::android::JavaParamRef<jobject>(env, srcU), srcStrideU,
      base::android::JavaParamRef<jobject>(env, srcV), srcStrideV, cropX, cropY, cropWidth,
      cropHeight, base::android::JavaParamRef<jobject>(env, dstY), dstStrideY,
      base::android::JavaParamRef<jobject>(env, dstU), dstStrideU,
      base::android::JavaParamRef<jobject>(env, dstV), dstStrideV, scaleWidth, scaleHeight);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_JavaI420Buffer_JNI
