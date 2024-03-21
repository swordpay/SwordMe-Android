// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/NV12Buffer

#ifndef org_webrtc_NV12Buffer_JNI
#define org_webrtc_NV12Buffer_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_NV12Buffer[];
const char kClassPath_org_webrtc_NV12Buffer[] = "org/webrtc/NV12Buffer";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_NV12Buffer_clazz(nullptr);
#ifndef org_webrtc_NV12Buffer_clazz_defined
#define org_webrtc_NV12Buffer_clazz_defined
inline jclass org_webrtc_NV12Buffer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_NV12Buffer,
      &g_org_webrtc_NV12Buffer_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static void JNI_NV12Buffer_CropAndScale(JNIEnv* env, jint cropX,
    jint cropY,
    jint cropWidth,
    jint cropHeight,
    jint scaleWidth,
    jint scaleHeight,
    const base::android::JavaParamRef<jobject>& src,
    jint srcWidth,
    jint srcHeight,
    jint srcStride,
    jint srcSliceHeight,
    const base::android::JavaParamRef<jobject>& dstY,
    jint dstStrideY,
    const base::android::JavaParamRef<jobject>& dstU,
    jint dstStrideU,
    const base::android::JavaParamRef<jobject>& dstV,
    jint dstStrideV);

JNI_GENERATOR_EXPORT void Java_org_webrtc_NV12Buffer_nativeCropAndScale(
    JNIEnv* env,
    jclass jcaller,
    jint cropX,
    jint cropY,
    jint cropWidth,
    jint cropHeight,
    jint scaleWidth,
    jint scaleHeight,
    jobject src,
    jint srcWidth,
    jint srcHeight,
    jint srcStride,
    jint srcSliceHeight,
    jobject dstY,
    jint dstStrideY,
    jobject dstU,
    jint dstStrideU,
    jobject dstV,
    jint dstStrideV) {
  return JNI_NV12Buffer_CropAndScale(env, cropX, cropY, cropWidth, cropHeight, scaleWidth,
      scaleHeight, base::android::JavaParamRef<jobject>(env, src), srcWidth, srcHeight, srcStride,
      srcSliceHeight, base::android::JavaParamRef<jobject>(env, dstY), dstStrideY,
      base::android::JavaParamRef<jobject>(env, dstU), dstStrideU,
      base::android::JavaParamRef<jobject>(env, dstV), dstStrideV);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_NV12Buffer_JNI
