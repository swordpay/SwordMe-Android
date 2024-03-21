// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/Metrics

#ifndef org_webrtc_Metrics_JNI
#define org_webrtc_Metrics_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_Metrics[];
const char kClassPath_org_webrtc_Metrics[] = "org/webrtc/Metrics";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_Metrics_00024HistogramInfo[];
const char kClassPath_org_webrtc_Metrics_00024HistogramInfo[] = "org/webrtc/Metrics$HistogramInfo";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_Metrics_clazz(nullptr);
#ifndef org_webrtc_Metrics_clazz_defined
#define org_webrtc_Metrics_clazz_defined
inline jclass org_webrtc_Metrics_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_Metrics,
      &g_org_webrtc_Metrics_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_Metrics_00024HistogramInfo_clazz(nullptr);
#ifndef org_webrtc_Metrics_00024HistogramInfo_clazz_defined
#define org_webrtc_Metrics_00024HistogramInfo_clazz_defined
inline jclass org_webrtc_Metrics_00024HistogramInfo_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_Metrics_00024HistogramInfo,
      &g_org_webrtc_Metrics_00024HistogramInfo_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static void JNI_Metrics_Enable(JNIEnv* env);

JNI_GENERATOR_EXPORT void Java_org_webrtc_Metrics_nativeEnable(
    JNIEnv* env,
    jclass jcaller) {
  return JNI_Metrics_Enable(env);
}

static base::android::ScopedJavaLocalRef<jobject> JNI_Metrics_GetAndReset(JNIEnv* env);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_Metrics_nativeGetAndReset(
    JNIEnv* env,
    jclass jcaller) {
  return JNI_Metrics_GetAndReset(env).Release();
}


static std::atomic<jmethodID> g_org_webrtc_Metrics_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Metrics_Constructor(JNIEnv* env) {
  jclass clazz = org_webrtc_Metrics_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_Metrics_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "()V",
          &g_org_webrtc_Metrics_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_Metrics_00024HistogramInfo_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_HistogramInfo_Constructor(JNIEnv* env,
    JniIntWrapper min,
    JniIntWrapper max,
    JniIntWrapper bucketCount) {
  jclass clazz = org_webrtc_Metrics_00024HistogramInfo_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_Metrics_00024HistogramInfo_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(III)V",
          &g_org_webrtc_Metrics_00024HistogramInfo_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(min), as_jint(max), as_jint(bucketCount));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_Metrics_00024HistogramInfo_addSample(nullptr);
static void Java_HistogramInfo_addSample(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper value,
    JniIntWrapper numEvents) {
  jclass clazz = org_webrtc_Metrics_00024HistogramInfo_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_Metrics_00024HistogramInfo_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "addSample",
          "(II)V",
          &g_org_webrtc_Metrics_00024HistogramInfo_addSample);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, as_jint(value), as_jint(numEvents));
}

static std::atomic<jmethodID> g_org_webrtc_Metrics_add(nullptr);
static void Java_Metrics_add(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jstring>& name,
    const base::android::JavaRef<jobject>& info) {
  jclass clazz = org_webrtc_Metrics_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_Metrics_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "add",
          "(Ljava/lang/String;Lorg/webrtc/Metrics$HistogramInfo;)V",
          &g_org_webrtc_Metrics_add);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, name.obj(), info.obj());
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_Metrics_JNI
