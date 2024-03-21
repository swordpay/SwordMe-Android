// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/lang/Double

#ifndef java_lang_Double_JNI
#define java_lang_Double_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_lang_Double[];
const char kClassPath_java_lang_Double[] = "java/lang/Double";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_lang_Double_clazz(nullptr);
#ifndef java_lang_Double_clazz_defined
#define java_lang_Double_clazz_defined
inline jclass java_lang_Double_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_lang_Double, &g_java_lang_Double_clazz);
}
#endif


namespace JNI_Double {

enum Java_Double_constant_fields {
  BYTES = 8,
  MAX_EXPONENT = 1023,
  MIN_EXPONENT = -1022,
  SIZE = 64,
};


}  // namespace JNI_Double
// Step 3: Method stubs.
namespace JNI_Double {


static std::atomic<jmethodID> g_java_lang_Double_toStringJLS_D(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Double_toStringJLS_D(JNIEnv*
    env, jdouble p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Double_toStringJLS_D(JNIEnv* env, jdouble p0)
    {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toString",
          "(D)Ljava/lang/String;",
          &g_java_lang_Double_toStringJLS_D);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Double_toHexString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Double_toHexString(JNIEnv*
    env, jdouble p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Double_toHexString(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toHexString",
          "(D)Ljava/lang/String;",
          &g_java_lang_Double_toHexString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Double_valueOfJLD_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Double_valueOfJLD_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Double_valueOfJLD_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/String;)Ljava/lang/Double;",
          &g_java_lang_Double_valueOfJLD_JLS);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Double_valueOfJLD_D(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Double_valueOfJLD_D(JNIEnv*
    env, jdouble p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Double_valueOfJLD_D(JNIEnv* env, jdouble p0)
    {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(D)Ljava/lang/Double;",
          &g_java_lang_Double_valueOfJLD_D);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Double_parseDouble(nullptr);
[[maybe_unused]] static jdouble Java_Double_parseDouble(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jdouble Java_Double_parseDouble(JNIEnv* env, const base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseDouble",
          "(Ljava/lang/String;)D",
          &g_java_lang_Double_parseDouble);

  jdouble ret =
      env->CallStaticDoubleMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_isNaNZ_D(nullptr);
[[maybe_unused]] static jboolean Java_Double_isNaNZ_D(JNIEnv* env, jdouble p0);
static jboolean Java_Double_isNaNZ_D(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "isNaN",
          "(D)Z",
          &g_java_lang_Double_isNaNZ_D);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_isInfiniteZ_D(nullptr);
[[maybe_unused]] static jboolean Java_Double_isInfiniteZ_D(JNIEnv* env, jdouble p0);
static jboolean Java_Double_isInfiniteZ_D(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "isInfinite",
          "(D)Z",
          &g_java_lang_Double_isInfiniteZ_D);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_isFinite(nullptr);
[[maybe_unused]] static jboolean Java_Double_isFinite(JNIEnv* env, jdouble p0);
static jboolean Java_Double_isFinite(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "isFinite",
          "(D)Z",
          &g_java_lang_Double_isFinite);

  jboolean ret =
      env->CallStaticBooleanMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_isNaNZ(nullptr);
[[maybe_unused]] static jboolean Java_Double_isNaNZ(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jboolean Java_Double_isNaNZ(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "isNaN",
          "()Z",
          &g_java_lang_Double_isNaNZ);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_isInfiniteZ(nullptr);
[[maybe_unused]] static jboolean Java_Double_isInfiniteZ(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jboolean Java_Double_isInfiniteZ(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "isInfinite",
          "()Z",
          &g_java_lang_Double_isInfiniteZ);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_toStringJLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Double_toStringJLS(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_Double_toStringJLS(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "()Ljava/lang/String;",
          &g_java_lang_Double_toStringJLS);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Double_byteValue(nullptr);
[[maybe_unused]] static jbyte Java_Double_byteValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jbyte Java_Double_byteValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "byteValue",
          "()B",
          &g_java_lang_Double_byteValue);

  jbyte ret =
      env->CallByteMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_shortValue(nullptr);
[[maybe_unused]] static jshort Java_Double_shortValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jshort Java_Double_shortValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "shortValue",
          "()S",
          &g_java_lang_Double_shortValue);

  jshort ret =
      env->CallShortMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_intValue(nullptr);
[[maybe_unused]] static jint Java_Double_intValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_Double_intValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "intValue",
          "()I",
          &g_java_lang_Double_intValue);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_longValue(nullptr);
[[maybe_unused]] static jlong Java_Double_longValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jlong Java_Double_longValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "longValue",
          "()J",
          &g_java_lang_Double_longValue);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_floatValue(nullptr);
[[maybe_unused]] static jfloat Java_Double_floatValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jfloat Java_Double_floatValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "floatValue",
          "()F",
          &g_java_lang_Double_floatValue);

  jfloat ret =
      env->CallFloatMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_doubleValue(nullptr);
[[maybe_unused]] static jdouble Java_Double_doubleValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jdouble Java_Double_doubleValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "doubleValue",
          "()D",
          &g_java_lang_Double_doubleValue);

  jdouble ret =
      env->CallDoubleMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_hashCodeI(nullptr);
[[maybe_unused]] static jint Java_Double_hashCodeI(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_Double_hashCodeI(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_lang_Double_hashCodeI);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_hashCodeI_D(nullptr);
[[maybe_unused]] static jint Java_Double_hashCodeI_D(JNIEnv* env, jdouble p0);
static jint Java_Double_hashCodeI_D(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "hashCode",
          "(D)I",
          &g_java_lang_Double_hashCodeI_D);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_equals(nullptr);
[[maybe_unused]] static jboolean Java_Double_equals(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Double_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_lang_Double_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_doubleToLongBits(nullptr);
[[maybe_unused]] static jlong Java_Double_doubleToLongBits(JNIEnv* env, jdouble p0);
static jlong Java_Double_doubleToLongBits(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "doubleToLongBits",
          "(D)J",
          &g_java_lang_Double_doubleToLongBits);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_doubleToRawLongBits(nullptr);
[[maybe_unused]] static jlong Java_Double_doubleToRawLongBits(JNIEnv* env, jdouble p0);
static jlong Java_Double_doubleToRawLongBits(JNIEnv* env, jdouble p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "doubleToRawLongBits",
          "(D)J",
          &g_java_lang_Double_doubleToRawLongBits);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_longBitsToDouble(nullptr);
[[maybe_unused]] static jdouble Java_Double_longBitsToDouble(JNIEnv* env, jlong p0);
static jdouble Java_Double_longBitsToDouble(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "longBitsToDouble",
          "(J)D",
          &g_java_lang_Double_longBitsToDouble);

  jdouble ret =
      env->CallStaticDoubleMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_compareToI_JLD(nullptr);
[[maybe_unused]] static jint Java_Double_compareToI_JLD(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Double_compareToI_JLD(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Double;)I",
          &g_java_lang_Double_compareToI_JLD);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_compare(nullptr);
[[maybe_unused]] static jint Java_Double_compare(JNIEnv* env, jdouble p0,
    jdouble p1);
static jint Java_Double_compare(JNIEnv* env, jdouble p0,
    jdouble p1) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "compare",
          "(DD)I",
          &g_java_lang_Double_compare);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_sum(nullptr);
[[maybe_unused]] static jdouble Java_Double_sum(JNIEnv* env, jdouble p0,
    jdouble p1);
static jdouble Java_Double_sum(JNIEnv* env, jdouble p0,
    jdouble p1) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "sum",
          "(DD)D",
          &g_java_lang_Double_sum);

  jdouble ret =
      env->CallStaticDoubleMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_max(nullptr);
[[maybe_unused]] static jdouble Java_Double_max(JNIEnv* env, jdouble p0,
    jdouble p1);
static jdouble Java_Double_max(JNIEnv* env, jdouble p0,
    jdouble p1) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "max",
          "(DD)D",
          &g_java_lang_Double_max);

  jdouble ret =
      env->CallStaticDoubleMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_min(nullptr);
[[maybe_unused]] static jdouble Java_Double_min(JNIEnv* env, jdouble p0,
    jdouble p1);
static jdouble Java_Double_min(JNIEnv* env, jdouble p0,
    jdouble p1) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "min",
          "(DD)D",
          &g_java_lang_Double_min);

  jdouble ret =
      env->CallStaticDoubleMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_compareToI_JLO(nullptr);
[[maybe_unused]] static jint Java_Double_compareToI_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Double_compareToI_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Double_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Object;)I",
          &g_java_lang_Double_compareToI_JLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Double_ConstructorJLD_D(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Double_ConstructorJLD_D(JNIEnv* env, jdouble p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Double_ConstructorJLD_D(JNIEnv* env, jdouble
    p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(D)V",
          &g_java_lang_Double_ConstructorJLD_D);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Double_ConstructorJLD_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Double_ConstructorJLD_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Double_ConstructorJLD_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Double_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Double_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;)V",
          &g_java_lang_Double_ConstructorJLD_JLS);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_Double

#endif  // java_lang_Double_JNI
