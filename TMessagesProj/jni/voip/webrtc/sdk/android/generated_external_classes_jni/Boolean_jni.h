// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/lang/Boolean

#ifndef java_lang_Boolean_JNI
#define java_lang_Boolean_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_lang_Boolean[];
const char kClassPath_java_lang_Boolean[] = "java/lang/Boolean";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_lang_Boolean_clazz(nullptr);
#ifndef java_lang_Boolean_clazz_defined
#define java_lang_Boolean_clazz_defined
inline jclass java_lang_Boolean_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_lang_Boolean, &g_java_lang_Boolean_clazz);
}
#endif


namespace JNI_Boolean {


static std::atomic<jmethodID> g_java_lang_Boolean_parseBoolean(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_parseBoolean(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jboolean Java_Boolean_parseBoolean(JNIEnv* env, const base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseBoolean",
          "(Ljava/lang/String;)Z",
          &g_java_lang_Boolean_parseBoolean);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_booleanValue(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_booleanValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jboolean Java_Boolean_booleanValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "booleanValue",
          "()Z",
          &g_java_lang_Boolean_booleanValue);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_valueOfJLB_Z(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Boolean_valueOfJLB_Z(JNIEnv*
    env, jboolean p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Boolean_valueOfJLB_Z(JNIEnv* env, jboolean
    p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Z)Ljava/lang/Boolean;",
          &g_java_lang_Boolean_valueOfJLB_Z);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Boolean_valueOfJLB_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Boolean_valueOfJLB_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Boolean_valueOfJLB_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/String;)Ljava/lang/Boolean;",
          &g_java_lang_Boolean_valueOfJLB_JLS);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Boolean_toStringJLS_Z(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Boolean_toStringJLS_Z(JNIEnv* env, jboolean p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Boolean_toStringJLS_Z(JNIEnv* env, jboolean
    p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toString",
          "(Z)Ljava/lang/String;",
          &g_java_lang_Boolean_toStringJLS_Z);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Boolean_toStringJLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Boolean_toStringJLS(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_Boolean_toStringJLS(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Boolean_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "()Ljava/lang/String;",
          &g_java_lang_Boolean_toStringJLS);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Boolean_hashCodeI(nullptr);
[[maybe_unused]] static jint Java_Boolean_hashCodeI(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_Boolean_hashCodeI(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Boolean_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_lang_Boolean_hashCodeI);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_hashCodeI_Z(nullptr);
[[maybe_unused]] static jint Java_Boolean_hashCodeI_Z(JNIEnv* env, jboolean p0);
static jint Java_Boolean_hashCodeI_Z(JNIEnv* env, jboolean p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "hashCode",
          "(Z)I",
          &g_java_lang_Boolean_hashCodeI_Z);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_equals(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_equals(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Boolean_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_lang_Boolean_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_getBoolean(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_getBoolean(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jboolean Java_Boolean_getBoolean(JNIEnv* env, const base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getBoolean",
          "(Ljava/lang/String;)Z",
          &g_java_lang_Boolean_getBoolean);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_compareToI_JLB(nullptr);
[[maybe_unused]] static jint Java_Boolean_compareToI_JLB(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Boolean_compareToI_JLB(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Boolean_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Boolean;)I",
          &g_java_lang_Boolean_compareToI_JLB);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_compare(nullptr);
[[maybe_unused]] static jint Java_Boolean_compare(JNIEnv* env, jboolean p0,
    jboolean p1);
static jint Java_Boolean_compare(JNIEnv* env, jboolean p0,
    jboolean p1) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "compare",
          "(ZZ)I",
          &g_java_lang_Boolean_compare);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_logicalAnd(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_logicalAnd(JNIEnv* env, jboolean p0,
    jboolean p1);
static jboolean Java_Boolean_logicalAnd(JNIEnv* env, jboolean p0,
    jboolean p1) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "logicalAnd",
          "(ZZ)Z",
          &g_java_lang_Boolean_logicalAnd);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_logicalOr(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_logicalOr(JNIEnv* env, jboolean p0,
    jboolean p1);
static jboolean Java_Boolean_logicalOr(JNIEnv* env, jboolean p0,
    jboolean p1) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "logicalOr",
          "(ZZ)Z",
          &g_java_lang_Boolean_logicalOr);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_logicalXor(nullptr);
[[maybe_unused]] static jboolean Java_Boolean_logicalXor(JNIEnv* env, jboolean p0,
    jboolean p1);
static jboolean Java_Boolean_logicalXor(JNIEnv* env, jboolean p0,
    jboolean p1) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "logicalXor",
          "(ZZ)Z",
          &g_java_lang_Boolean_logicalXor);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_compareToI_JLO(nullptr);
[[maybe_unused]] static jint Java_Boolean_compareToI_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Boolean_compareToI_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Boolean_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Object;)I",
          &g_java_lang_Boolean_compareToI_JLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Boolean_ConstructorJLB_Z(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Boolean_ConstructorJLB_Z(JNIEnv* env, jboolean p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Boolean_ConstructorJLB_Z(JNIEnv* env,
    jboolean p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Z)V",
          &g_java_lang_Boolean_ConstructorJLB_Z);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Boolean_ConstructorJLB_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Boolean_ConstructorJLB_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Boolean_ConstructorJLB_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Boolean_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Boolean_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;)V",
          &g_java_lang_Boolean_ConstructorJLB_JLS);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_Boolean

#endif  // java_lang_Boolean_JNI
