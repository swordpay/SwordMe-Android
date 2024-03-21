// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/lang/Enum

#ifndef java_lang_Enum_JNI
#define java_lang_Enum_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_lang_Enum[];
const char kClassPath_java_lang_Enum[] = "java/lang/Enum";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_lang_Enum_clazz(nullptr);
#ifndef java_lang_Enum_clazz_defined
#define java_lang_Enum_clazz_defined
inline jclass java_lang_Enum_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_lang_Enum, &g_java_lang_Enum_clazz);
}
#endif


namespace JNI_Enum {


static std::atomic<jmethodID> g_java_lang_Enum_name(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Enum_name(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_Enum_name(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "name",
          "()Ljava/lang/String;",
          &g_java_lang_Enum_name);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Enum_ordinal(nullptr);
[[maybe_unused]] static jint Java_Enum_ordinal(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static jint Java_Enum_ordinal(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "ordinal",
          "()I",
          &g_java_lang_Enum_ordinal);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Enum_toString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Enum_toString(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_Enum_toString(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "()Ljava/lang/String;",
          &g_java_lang_Enum_toString);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Enum_equals(nullptr);
[[maybe_unused]] static jboolean Java_Enum_equals(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Enum_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_lang_Enum_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Enum_hashCode(nullptr);
[[maybe_unused]] static jint Java_Enum_hashCode(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static jint Java_Enum_hashCode(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_lang_Enum_hashCode);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Enum_clone(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Enum_clone(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_Enum_clone(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "clone",
          "()Ljava/lang/Object;",
          &g_java_lang_Enum_clone);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Enum_compareToI_JLE(nullptr);
[[maybe_unused]] static jint Java_Enum_compareToI_JLE(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Enum_compareToI_JLE(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Enum;)I",
          &g_java_lang_Enum_compareToI_JLE);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Enum_getDeclaringClass(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jclass>
    Java_Enum_getDeclaringClass(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jclass> Java_Enum_getDeclaringClass(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDeclaringClass",
          "()Ljava/lang/Class;",
          &g_java_lang_Enum_getDeclaringClass);

  jclass ret =
      static_cast<jclass>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jclass>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Enum_valueOf(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Enum_valueOf(JNIEnv* env,
    const base::android::JavaRef<jclass>& p0,
    const base::android::JavaRef<jstring>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Enum_valueOf(JNIEnv* env, const
    base::android::JavaRef<jclass>& p0,
    const base::android::JavaRef<jstring>& p1) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Enum_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/Enum;",
          &g_java_lang_Enum_valueOf);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Enum_finalize(nullptr);
[[maybe_unused]] static void Java_Enum_finalize(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static void Java_Enum_finalize(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "finalize",
          "()V",
          &g_java_lang_Enum_finalize);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_java_lang_Enum_compareToI_JLO(nullptr);
[[maybe_unused]] static jint Java_Enum_compareToI_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Enum_compareToI_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Enum_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Enum_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Object;)I",
          &g_java_lang_Enum_compareToI_JLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

}  // namespace JNI_Enum

#endif  // java_lang_Enum_JNI
