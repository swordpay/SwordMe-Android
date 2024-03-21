// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoEncoderFactory

#ifndef org_webrtc_VideoEncoderFactory_JNI
#define org_webrtc_VideoEncoderFactory_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoderFactory[];
const char kClassPath_org_webrtc_VideoEncoderFactory[] = "org/webrtc/VideoEncoderFactory";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector[];
const char kClassPath_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector[] =
    "org/webrtc/VideoEncoderFactory$VideoEncoderSelector";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoEncoderFactory_clazz(nullptr);
#ifndef org_webrtc_VideoEncoderFactory_clazz_defined
#define org_webrtc_VideoEncoderFactory_clazz_defined
inline jclass org_webrtc_VideoEncoderFactory_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoderFactory,
      &g_org_webrtc_VideoEncoderFactory_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(nullptr);
#ifndef org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz_defined
#define org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz_defined
inline jclass org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector,
      &g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_onCurrentEncoder(nullptr);
static void Java_VideoEncoderSelector_onCurrentEncoder(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& info) {
  jclass clazz = org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onCurrentEncoder",
          "(Lorg/webrtc/VideoCodecInfo;)V",
          &g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_onCurrentEncoder);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, info.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_onAvailableBitrate(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_VideoEncoderSelector_onAvailableBitrate(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, JniIntWrapper kbps) {
  jclass clazz = org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onAvailableBitrate",
          "(I)Lorg/webrtc/VideoCodecInfo;",
          &g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_onAvailableBitrate);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(kbps));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_onEncoderBroken(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoderSelector_onEncoderBroken(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onEncoderBroken",
          "()Lorg/webrtc/VideoCodecInfo;",
          &g_org_webrtc_VideoEncoderFactory_00024VideoEncoderSelector_onEncoderBroken);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderFactory_createEncoder(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoderFactory_createEncoder(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& info) {
  jclass clazz = org_webrtc_VideoEncoderFactory_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "createEncoder",
          "(Lorg/webrtc/VideoCodecInfo;)Lorg/webrtc/VideoEncoder;",
          &g_org_webrtc_VideoEncoderFactory_createEncoder);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, info.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderFactory_getSupportedCodecs(nullptr);
static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_VideoEncoderFactory_getSupportedCodecs(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_VideoEncoderFactory_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSupportedCodecs",
          "()[Lorg/webrtc/VideoCodecInfo;",
          &g_org_webrtc_VideoEncoderFactory_getSupportedCodecs);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderFactory_getImplementations(nullptr);
static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_VideoEncoderFactory_getImplementations(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_VideoEncoderFactory_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getImplementations",
          "()[Lorg/webrtc/VideoCodecInfo;",
          &g_org_webrtc_VideoEncoderFactory_getImplementations);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoderFactory_getEncoderSelector(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_VideoEncoderFactory_getEncoderSelector(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_VideoEncoderFactory_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoderFactory_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEncoderSelector",
          "()Lorg/webrtc/VideoEncoderFactory$VideoEncoderSelector;",
          &g_org_webrtc_VideoEncoderFactory_getEncoderSelector);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoEncoderFactory_JNI
