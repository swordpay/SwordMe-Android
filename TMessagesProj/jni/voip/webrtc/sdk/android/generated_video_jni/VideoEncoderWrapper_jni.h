// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoEncoderWrapper

#ifndef org_webrtc_VideoEncoderWrapper_JNI
#define org_webrtc_VideoEncoderWrapper_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoderWrapper[];
const char kClassPath_org_webrtc_VideoEncoderWrapper[] = "org/webrtc/VideoEncoderWrapper";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoEncoderWrapper_clazz(nullptr);
#ifndef org_webrtc_VideoEncoderWrapper_clazz_defined
#define org_webrtc_VideoEncoderWrapper_clazz_defined
inline jclass org_webrtc_VideoEncoderWrapper_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoderWrapper,
      &g_org_webrtc_VideoEncoderWrapper_clazz);
}
#endif


namespace  webrtc {
namespace jni {

JNI_GENERATOR_EXPORT void Java_org_webrtc_VideoEncoderWrapper_nativeOnEncodedFrame(
    JNIEnv* env,
    jclass jcaller,
    jlong nativeVideoEncoderWrapper,
    jobject frame) {
  VideoEncoderWrapper* native = reinterpret_cast<VideoEncoderWrapper*>(nativeVideoEncoderWrapper);
  CHECK_NATIVE_PTR(env, jcaller, native, "OnEncodedFrame");
  return native->OnEncodedFrame(env, base::android::JavaParamRef<jobject>(env, frame));
}


static std::atomic<jmethodID> g_org_webrtc_VideoEncoderWrapper_getScalingSettingsOn(nullptr);
static jboolean Java_VideoEncoderWrapper_getScalingSettingsOn(JNIEnv* env, const
    base::android::JavaRef<jobject>& scalingSettings) {
  jclass clazz = org_webrtc_VideoEncoderWrapper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoderWrapper_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getScalingSettingsOn",
          "(Lorg/webrtc/VideoEncoder$ScalingSettings;)Z",
          &g_org_webrtc_VideoEncoderWrapper_getScalingSettingsOn);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, scalingSettings.obj());
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderWrapper_getScalingSettingsLow(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_VideoEncoderWrapper_getScalingSettingsLow(JNIEnv* env, const
    base::android::JavaRef<jobject>& scalingSettings) {
  jclass clazz = org_webrtc_VideoEncoderWrapper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoderWrapper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getScalingSettingsLow",
          "(Lorg/webrtc/VideoEncoder$ScalingSettings;)Ljava/lang/Integer;",
          &g_org_webrtc_VideoEncoderWrapper_getScalingSettingsLow);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, scalingSettings.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderWrapper_getScalingSettingsHigh(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_VideoEncoderWrapper_getScalingSettingsHigh(JNIEnv* env, const
    base::android::JavaRef<jobject>& scalingSettings) {
  jclass clazz = org_webrtc_VideoEncoderWrapper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoderWrapper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getScalingSettingsHigh",
          "(Lorg/webrtc/VideoEncoder$ScalingSettings;)Ljava/lang/Integer;",
          &g_org_webrtc_VideoEncoderWrapper_getScalingSettingsHigh);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, scalingSettings.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderWrapper_createEncoderCallback(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_VideoEncoderWrapper_createEncoderCallback(JNIEnv* env, jlong nativeEncoder) {
  jclass clazz = org_webrtc_VideoEncoderWrapper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoderWrapper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "createEncoderCallback",
          "(J)Lorg/webrtc/VideoEncoder$Callback;",
          &g_org_webrtc_VideoEncoderWrapper_createEncoderCallback);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, nativeEncoder);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoEncoderWrapper_JNI
