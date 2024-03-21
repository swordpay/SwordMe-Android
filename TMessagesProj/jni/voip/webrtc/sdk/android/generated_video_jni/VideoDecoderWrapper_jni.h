// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoDecoderWrapper

#ifndef org_webrtc_VideoDecoderWrapper_JNI
#define org_webrtc_VideoDecoderWrapper_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoDecoderWrapper[];
const char kClassPath_org_webrtc_VideoDecoderWrapper[] = "org/webrtc/VideoDecoderWrapper";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoDecoderWrapper_clazz(nullptr);
#ifndef org_webrtc_VideoDecoderWrapper_clazz_defined
#define org_webrtc_VideoDecoderWrapper_clazz_defined
inline jclass org_webrtc_VideoDecoderWrapper_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoDecoderWrapper,
      &g_org_webrtc_VideoDecoderWrapper_clazz);
}
#endif


namespace  webrtc {
namespace jni {

JNI_GENERATOR_EXPORT void Java_org_webrtc_VideoDecoderWrapper_nativeOnDecodedFrame(
    JNIEnv* env,
    jclass jcaller,
    jlong nativeVideoDecoderWrapper,
    jobject frame,
    jobject decodeTimeMs,
    jobject qp) {
  VideoDecoderWrapper* native = reinterpret_cast<VideoDecoderWrapper*>(nativeVideoDecoderWrapper);
  CHECK_NATIVE_PTR(env, jcaller, native, "OnDecodedFrame");
  return native->OnDecodedFrame(env, base::android::JavaParamRef<jobject>(env, frame),
      base::android::JavaParamRef<jobject>(env, decodeTimeMs),
      base::android::JavaParamRef<jobject>(env, qp));
}


static std::atomic<jmethodID> g_org_webrtc_VideoDecoderWrapper_createDecoderCallback(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_VideoDecoderWrapper_createDecoderCallback(JNIEnv* env, jlong nativeDecoder) {
  jclass clazz = org_webrtc_VideoDecoderWrapper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoDecoderWrapper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "createDecoderCallback",
          "(J)Lorg/webrtc/VideoDecoder$Callback;",
          &g_org_webrtc_VideoDecoderWrapper_createDecoderCallback);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, nativeDecoder);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoDecoderWrapper_JNI
