// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/JniHelper

#ifndef org_webrtc_JniHelper_JNI
#define org_webrtc_JniHelper_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_JniHelper[];
const char kClassPath_org_webrtc_JniHelper[] = "org/webrtc/JniHelper";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_JniHelper_clazz(nullptr);
#ifndef org_webrtc_JniHelper_clazz_defined
#define org_webrtc_JniHelper_clazz_defined
inline jclass org_webrtc_JniHelper_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_JniHelper,
      &g_org_webrtc_JniHelper_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_JniHelper_getStringBytes(nullptr);
static base::android::ScopedJavaLocalRef<jbyteArray> Java_JniHelper_getStringBytes(JNIEnv* env,
    const base::android::JavaRef<jstring>& s) {
  jclass clazz = org_webrtc_JniHelper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_JniHelper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getStringBytes",
          "(Ljava/lang/String;)[B",
          &g_org_webrtc_JniHelper_getStringBytes);

  jbyteArray ret =
      static_cast<jbyteArray>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, s.obj()));
  return base::android::ScopedJavaLocalRef<jbyteArray>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_JniHelper_getStringClass(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_JniHelper_getStringClass(JNIEnv* env) {
  jclass clazz = org_webrtc_JniHelper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_JniHelper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getStringClass",
          "()Ljava/lang/Object;",
          &g_org_webrtc_JniHelper_getStringClass);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_JniHelper_getKey(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_JniHelper_getKey(JNIEnv* env, const
    base::android::JavaRef<jobject>& entry) {
  jclass clazz = org_webrtc_JniHelper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_JniHelper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getKey",
          "(Ljava/util/Map$Entry;)Ljava/lang/Object;",
          &g_org_webrtc_JniHelper_getKey);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, entry.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_JniHelper_getValue(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_JniHelper_getValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& entry) {
  jclass clazz = org_webrtc_JniHelper_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_JniHelper_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getValue",
          "(Ljava/util/Map$Entry;)Ljava/lang/Object;",
          &g_org_webrtc_JniHelper_getValue);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, entry.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_JniHelper_JNI
