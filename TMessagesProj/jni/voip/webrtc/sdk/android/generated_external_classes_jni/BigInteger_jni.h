// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/math/BigInteger

#ifndef java_math_BigInteger_JNI
#define java_math_BigInteger_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_math_BigInteger[];
const char kClassPath_java_math_BigInteger[] = "java/math/BigInteger";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_math_BigInteger_clazz(nullptr);
#ifndef java_math_BigInteger_clazz_defined
#define java_math_BigInteger_clazz_defined
inline jclass java_math_BigInteger_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_math_BigInteger,
      &g_java_math_BigInteger_clazz);
}
#endif


namespace JNI_BigInteger {


static std::atomic<jmethodID> g_java_math_BigInteger_probablePrime(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_probablePrime(JNIEnv* env, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_probablePrime(JNIEnv* env,
    JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "probablePrime",
          "(ILjava/util/Random;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_probablePrime);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_nextProbablePrime(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_nextProbablePrime(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_nextProbablePrime(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "nextProbablePrime",
          "()Ljava/math/BigInteger;",
          &g_java_math_BigInteger_nextProbablePrime);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_valueOf(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_valueOf(JNIEnv*
    env, jlong p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_valueOf(JNIEnv* env, jlong p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(J)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_valueOf);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_add(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_add(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_add(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "add",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_add);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_subtract(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_subtract(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_subtract(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "subtract",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_subtract);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_multiply(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_multiply(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_multiply(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "multiply",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_multiply);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_divide(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_divide(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_divide(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "divide",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_divide);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_divideAndRemainder(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_BigInteger_divideAndRemainder(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobjectArray> Java_BigInteger_divideAndRemainder(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "divideAndRemainder",
          "(Ljava/math/BigInteger;)[Ljava/math/BigInteger;",
          &g_java_math_BigInteger_divideAndRemainder);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj()));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_remainder(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_remainder(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_remainder(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "remainder",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_remainder);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_pow(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_pow(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_pow(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "pow",
          "(I)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_pow);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_gcd(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_gcd(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_gcd(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "gcd",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_gcd);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_abs(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_abs(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_abs(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "abs",
          "()Ljava/math/BigInteger;",
          &g_java_math_BigInteger_abs);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_negate(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_negate(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_negate(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "negate",
          "()Ljava/math/BigInteger;",
          &g_java_math_BigInteger_negate);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_signum(nullptr);
[[maybe_unused]] static jint Java_BigInteger_signum(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_signum(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "signum",
          "()I",
          &g_java_math_BigInteger_signum);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_mod(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_mod(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_mod(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "mod",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_mod);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_modPow(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_modPow(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_modPow(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "modPow",
          "(Ljava/math/BigInteger;Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_modPow);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_modInverse(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_modInverse(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_modInverse(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "modInverse",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_modInverse);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_shiftLeft(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_shiftLeft(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_shiftLeft(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "shiftLeft",
          "(I)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_shiftLeft);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_shiftRight(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_shiftRight(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_shiftRight(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "shiftRight",
          "(I)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_shiftRight);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_and(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_and(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_and(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "and",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_and);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_or(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_or(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_or(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "or",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_or);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_xor(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_xor(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_xor(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "xor",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_xor);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_not(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_not(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_not(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "not",
          "()Ljava/math/BigInteger;",
          &g_java_math_BigInteger_not);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_andNot(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_andNot(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_andNot(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "andNot",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_andNot);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_testBit(nullptr);
[[maybe_unused]] static jboolean Java_BigInteger_testBit(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static jboolean Java_BigInteger_testBit(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "testBit",
          "(I)Z",
          &g_java_math_BigInteger_testBit);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_setBit(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_setBit(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_setBit(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "setBit",
          "(I)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_setBit);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_clearBit(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_clearBit(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_clearBit(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "clearBit",
          "(I)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_clearBit);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_flipBit(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_flipBit(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_flipBit(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "flipBit",
          "(I)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_flipBit);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_getLowestSetBit(nullptr);
[[maybe_unused]] static jint Java_BigInteger_getLowestSetBit(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_getLowestSetBit(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getLowestSetBit",
          "()I",
          &g_java_math_BigInteger_getLowestSetBit);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_bitLength(nullptr);
[[maybe_unused]] static jint Java_BigInteger_bitLength(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_bitLength(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "bitLength",
          "()I",
          &g_java_math_BigInteger_bitLength);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_bitCount(nullptr);
[[maybe_unused]] static jint Java_BigInteger_bitCount(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_bitCount(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "bitCount",
          "()I",
          &g_java_math_BigInteger_bitCount);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_isProbablePrime(nullptr);
[[maybe_unused]] static jboolean Java_BigInteger_isProbablePrime(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static jboolean Java_BigInteger_isProbablePrime(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "isProbablePrime",
          "(I)Z",
          &g_java_math_BigInteger_isProbablePrime);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_compareToI_JMBI(nullptr);
[[maybe_unused]] static jint Java_BigInteger_compareToI_JMBI(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_BigInteger_compareToI_JMBI(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/math/BigInteger;)I",
          &g_java_math_BigInteger_compareToI_JMBI);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_equals(nullptr);
[[maybe_unused]] static jboolean Java_BigInteger_equals(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_BigInteger_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_math_BigInteger_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_min(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_min(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_min(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "min",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_min);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_max(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_max(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_max(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "max",
          "(Ljava/math/BigInteger;)Ljava/math/BigInteger;",
          &g_java_math_BigInteger_max);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_hashCode(nullptr);
[[maybe_unused]] static jint Java_BigInteger_hashCode(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_hashCode(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_math_BigInteger_hashCode);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_toStringJLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_BigInteger_toStringJLS_I(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jstring> Java_BigInteger_toStringJLS_I(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "(I)Ljava/lang/String;",
          &g_java_math_BigInteger_toStringJLS_I);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_toStringJLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_BigInteger_toStringJLS(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_BigInteger_toStringJLS(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "()Ljava/lang/String;",
          &g_java_math_BigInteger_toStringJLS);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_toByteArray(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jbyteArray>
    Java_BigInteger_toByteArray(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jbyteArray> Java_BigInteger_toByteArray(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toByteArray",
          "()[B",
          &g_java_math_BigInteger_toByteArray);

  jbyteArray ret =
      static_cast<jbyteArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jbyteArray>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_intValue(nullptr);
[[maybe_unused]] static jint Java_BigInteger_intValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_intValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "intValue",
          "()I",
          &g_java_math_BigInteger_intValue);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_longValue(nullptr);
[[maybe_unused]] static jlong Java_BigInteger_longValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jlong Java_BigInteger_longValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "longValue",
          "()J",
          &g_java_math_BigInteger_longValue);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_floatValue(nullptr);
[[maybe_unused]] static jfloat Java_BigInteger_floatValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jfloat Java_BigInteger_floatValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "floatValue",
          "()F",
          &g_java_math_BigInteger_floatValue);

  jfloat ret =
      env->CallFloatMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_doubleValue(nullptr);
[[maybe_unused]] static jdouble Java_BigInteger_doubleValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jdouble Java_BigInteger_doubleValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "doubleValue",
          "()D",
          &g_java_math_BigInteger_doubleValue);

  jdouble ret =
      env->CallDoubleMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_longValueExact(nullptr);
[[maybe_unused]] static jlong Java_BigInteger_longValueExact(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jlong Java_BigInteger_longValueExact(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "longValueExact",
          "()J",
          &g_java_math_BigInteger_longValueExact);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_intValueExact(nullptr);
[[maybe_unused]] static jint Java_BigInteger_intValueExact(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_BigInteger_intValueExact(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "intValueExact",
          "()I",
          &g_java_math_BigInteger_intValueExact);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_shortValueExact(nullptr);
[[maybe_unused]] static jshort Java_BigInteger_shortValueExact(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jshort Java_BigInteger_shortValueExact(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "shortValueExact",
          "()S",
          &g_java_math_BigInteger_shortValueExact);

  jshort ret =
      env->CallShortMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_byteValueExact(nullptr);
[[maybe_unused]] static jbyte Java_BigInteger_byteValueExact(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jbyte Java_BigInteger_byteValueExact(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "byteValueExact",
          "()B",
          &g_java_math_BigInteger_byteValueExact);

  jbyte ret =
      env->CallByteMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_compareToI_JLO(nullptr);
[[maybe_unused]] static jint Java_BigInteger_compareToI_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_BigInteger_compareToI_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_math_BigInteger_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Object;)I",
          &g_java_math_BigInteger_compareToI_JLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_math_BigInteger_ConstructorJMBI_AB(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_ConstructorJMBI_AB(JNIEnv* env, const base::android::JavaRef<jbyteArray>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_ConstructorJMBI_AB(JNIEnv* env,
    const base::android::JavaRef<jbyteArray>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "([B)V",
          &g_java_math_BigInteger_ConstructorJMBI_AB);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_ConstructorJMBI_I_AB(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_ConstructorJMBI_I_AB(JNIEnv* env, JniIntWrapper p0,
    const base::android::JavaRef<jbyteArray>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_ConstructorJMBI_I_AB(JNIEnv* env,
    JniIntWrapper p0,
    const base::android::JavaRef<jbyteArray>& p1) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(I[B)V",
          &g_java_math_BigInteger_ConstructorJMBI_I_AB);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(p0), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_ConstructorJMBI_JLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_ConstructorJMBI_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_ConstructorJMBI_JLS_I(JNIEnv* env,
    const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;I)V",
          &g_java_math_BigInteger_ConstructorJMBI_JLS_I);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_ConstructorJMBI_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_ConstructorJMBI_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_ConstructorJMBI_JLS(JNIEnv* env,
    const base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;)V",
          &g_java_math_BigInteger_ConstructorJMBI_JLS);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_ConstructorJMBI_I_JUR(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_ConstructorJMBI_I_JUR(JNIEnv* env, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_ConstructorJMBI_I_JUR(JNIEnv* env,
    JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(ILjava/util/Random;)V",
          &g_java_math_BigInteger_ConstructorJMBI_I_JUR);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(p0), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_math_BigInteger_ConstructorJMBI_I_I_JUR(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_BigInteger_ConstructorJMBI_I_I_JUR(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1,
    const base::android::JavaRef<jobject>& p2);
static base::android::ScopedJavaLocalRef<jobject> Java_BigInteger_ConstructorJMBI_I_I_JUR(JNIEnv*
    env, JniIntWrapper p0,
    JniIntWrapper p1,
    const base::android::JavaRef<jobject>& p2) {
  jclass clazz = java_math_BigInteger_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_math_BigInteger_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(IILjava/util/Random;)V",
          &g_java_math_BigInteger_ConstructorJMBI_I_I_JUR);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1), p2.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_BigInteger

#endif  // java_math_BigInteger_JNI
