// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/SessionDescription

#ifndef org_webrtc_SessionDescription_JNI
#define org_webrtc_SessionDescription_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_SessionDescription[];
const char kClassPath_org_webrtc_SessionDescription[] = "org/webrtc/SessionDescription";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_SessionDescription_00024Type[];
const char kClassPath_org_webrtc_SessionDescription_00024Type[] =
    "org/webrtc/SessionDescription$Type";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_SessionDescription_clazz(nullptr);
#ifndef org_webrtc_SessionDescription_clazz_defined
#define org_webrtc_SessionDescription_clazz_defined
inline jclass org_webrtc_SessionDescription_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_SessionDescription,
      &g_org_webrtc_SessionDescription_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_SessionDescription_00024Type_clazz(nullptr);
#ifndef org_webrtc_SessionDescription_00024Type_clazz_defined
#define org_webrtc_SessionDescription_00024Type_clazz_defined
inline jclass org_webrtc_SessionDescription_00024Type_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_SessionDescription_00024Type,
      &g_org_webrtc_SessionDescription_00024Type_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_SessionDescription_00024Type_fromCanonicalForm(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Type_fromCanonicalForm(JNIEnv* env, const
    base::android::JavaRef<jstring>& canonical) {
  jclass clazz = org_webrtc_SessionDescription_00024Type_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_SessionDescription_00024Type_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromCanonicalForm",
          "(Ljava/lang/String;)Lorg/webrtc/SessionDescription$Type;",
          &g_org_webrtc_SessionDescription_00024Type_fromCanonicalForm);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, canonical.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_SessionDescription_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_SessionDescription_Constructor(JNIEnv* env,
    const base::android::JavaRef<jobject>& type,
    const base::android::JavaRef<jstring>& description) {
  jclass clazz = org_webrtc_SessionDescription_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_SessionDescription_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Lorg/webrtc/SessionDescription$Type;Ljava/lang/String;)V",
          &g_org_webrtc_SessionDescription_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, type.obj(), description.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_SessionDescription_getDescription(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_SessionDescription_getDescription(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_SessionDescription_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_SessionDescription_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDescription",
          "()Ljava/lang/String;",
          &g_org_webrtc_SessionDescription_getDescription);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_SessionDescription_getTypeInCanonicalForm(nullptr);
static base::android::ScopedJavaLocalRef<jstring>
    Java_SessionDescription_getTypeInCanonicalForm(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_SessionDescription_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_SessionDescription_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTypeInCanonicalForm",
          "()Ljava/lang/String;",
          &g_org_webrtc_SessionDescription_getTypeInCanonicalForm);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_SessionDescription_JNI
