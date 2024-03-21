// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoDecoderFactory

#ifndef org_webrtc_VideoDecoderFactory_JNI
#define org_webrtc_VideoDecoderFactory_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoDecoderFactory[];
const char kClassPath_org_webrtc_VideoDecoderFactory[] = "org/webrtc/VideoDecoderFactory";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoDecoderFactory_clazz(nullptr);
#ifndef org_webrtc_VideoDecoderFactory_clazz_defined
#define org_webrtc_VideoDecoderFactory_clazz_defined
inline jclass org_webrtc_VideoDecoderFactory_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoDecoderFactory,
      &g_org_webrtc_VideoDecoderFactory_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_VideoDecoderFactory_createDecoder(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoDecoderFactory_createDecoder(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& info) {
  jclass clazz = org_webrtc_VideoDecoderFactory_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoDecoderFactory_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "createDecoder",
          "(Lorg/webrtc/VideoCodecInfo;)Lorg/webrtc/VideoDecoder;",
          &g_org_webrtc_VideoDecoderFactory_createDecoder);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, info.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoDecoderFactory_getSupportedCodecs(nullptr);
static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_VideoDecoderFactory_getSupportedCodecs(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_VideoDecoderFactory_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoDecoderFactory_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSupportedCodecs",
          "()[Lorg/webrtc/VideoCodecInfo;",
          &g_org_webrtc_VideoDecoderFactory_getSupportedCodecs);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoDecoderFactory_JNI
