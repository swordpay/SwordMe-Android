// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/lang/Long

#ifndef java_lang_Long_JNI
#define java_lang_Long_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_lang_Long[];
const char kClassPath_java_lang_Long[] = "java/lang/Long";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_lang_Long_clazz(nullptr);
#ifndef java_lang_Long_clazz_defined
#define java_lang_Long_clazz_defined
inline jclass java_lang_Long_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_lang_Long, &g_java_lang_Long_clazz);
}
#endif


namespace JNI_Long {

enum Java_Long_constant_fields {
  BYTES = 8,
  SIZE = 64,
};


}  // namespace JNI_Long
// Step 3: Method stubs.
namespace JNI_Long {


static std::atomic<jmethodID> g_java_lang_Long_toStringJLS_J_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Long_toStringJLS_J_I(JNIEnv*
    env, jlong p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toStringJLS_J_I(JNIEnv* env, jlong p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toString",
          "(JI)Ljava/lang/String;",
          &g_java_lang_Long_toStringJLS_J_I);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0, as_jint(p1)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_toUnsignedStringJLS_J_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Long_toUnsignedStringJLS_J_I(JNIEnv* env, jlong p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toUnsignedStringJLS_J_I(JNIEnv* env,
    jlong p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toUnsignedString",
          "(JI)Ljava/lang/String;",
          &g_java_lang_Long_toUnsignedStringJLS_J_I);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0, as_jint(p1)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_toHexString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Long_toHexString(JNIEnv*
    env, jlong p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toHexString(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toHexString",
          "(J)Ljava/lang/String;",
          &g_java_lang_Long_toHexString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_toOctalString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Long_toOctalString(JNIEnv*
    env, jlong p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toOctalString(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toOctalString",
          "(J)Ljava/lang/String;",
          &g_java_lang_Long_toOctalString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_toBinaryString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Long_toBinaryString(JNIEnv*
    env, jlong p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toBinaryString(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toBinaryString",
          "(J)Ljava/lang/String;",
          &g_java_lang_Long_toBinaryString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_toStringJLS_J(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Long_toStringJLS_J(JNIEnv*
    env, jlong p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toStringJLS_J(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toString",
          "(J)Ljava/lang/String;",
          &g_java_lang_Long_toStringJLS_J);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_toUnsignedStringJLS_J(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Long_toUnsignedStringJLS_J(JNIEnv* env, jlong p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toUnsignedStringJLS_J(JNIEnv* env, jlong
    p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toUnsignedString",
          "(J)Ljava/lang/String;",
          &g_java_lang_Long_toUnsignedStringJLS_J);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_parseLongJ_JLS_I(nullptr);
[[maybe_unused]] static jlong Java_Long_parseLongJ_JLS_I(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static jlong Java_Long_parseLongJ_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseLong",
          "(Ljava/lang/String;I)J",
          &g_java_lang_Long_parseLongJ_JLS_I);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_parseLongJ_JLS(nullptr);
[[maybe_unused]] static jlong Java_Long_parseLongJ_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jlong Java_Long_parseLongJ_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseLong",
          "(Ljava/lang/String;)J",
          &g_java_lang_Long_parseLongJ_JLS);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_parseUnsignedLongJ_JLS_I(nullptr);
[[maybe_unused]] static jlong Java_Long_parseUnsignedLongJ_JLS_I(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static jlong Java_Long_parseUnsignedLongJ_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>&
    p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseUnsignedLong",
          "(Ljava/lang/String;I)J",
          &g_java_lang_Long_parseUnsignedLongJ_JLS_I);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_parseUnsignedLongJ_JLS(nullptr);
[[maybe_unused]] static jlong Java_Long_parseUnsignedLongJ_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jlong Java_Long_parseUnsignedLongJ_JLS(JNIEnv* env, const base::android::JavaRef<jstring>&
    p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseUnsignedLong",
          "(Ljava/lang/String;)J",
          &g_java_lang_Long_parseUnsignedLongJ_JLS);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_valueOfJLLO_JLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Long_valueOfJLLO_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_valueOfJLLO_JLS_I(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/String;I)Ljava/lang/Long;",
          &g_java_lang_Long_valueOfJLLO_JLS_I);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_valueOfJLLO_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Long_valueOfJLLO_JLS(JNIEnv*
    env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_valueOfJLLO_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/String;)Ljava/lang/Long;",
          &g_java_lang_Long_valueOfJLLO_JLS);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_valueOfJLLO_J(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Long_valueOfJLLO_J(JNIEnv*
    env, jlong p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_valueOfJLLO_J(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(J)Ljava/lang/Long;",
          &g_java_lang_Long_valueOfJLLO_J);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_decode(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Long_decode(JNIEnv* env,
    const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_decode(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "decode",
          "(Ljava/lang/String;)Ljava/lang/Long;",
          &g_java_lang_Long_decode);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_byteValue(nullptr);
[[maybe_unused]] static jbyte Java_Long_byteValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jbyte Java_Long_byteValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "byteValue",
          "()B",
          &g_java_lang_Long_byteValue);

  jbyte ret =
      env->CallByteMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_shortValue(nullptr);
[[maybe_unused]] static jshort Java_Long_shortValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jshort Java_Long_shortValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "shortValue",
          "()S",
          &g_java_lang_Long_shortValue);

  jshort ret =
      env->CallShortMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_intValue(nullptr);
[[maybe_unused]] static jint Java_Long_intValue(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static jint Java_Long_intValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "intValue",
          "()I",
          &g_java_lang_Long_intValue);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_longValue(nullptr);
[[maybe_unused]] static jlong Java_Long_longValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jlong Java_Long_longValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "longValue",
          "()J",
          &g_java_lang_Long_longValue);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_floatValue(nullptr);
[[maybe_unused]] static jfloat Java_Long_floatValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jfloat Java_Long_floatValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "floatValue",
          "()F",
          &g_java_lang_Long_floatValue);

  jfloat ret =
      env->CallFloatMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_doubleValue(nullptr);
[[maybe_unused]] static jdouble Java_Long_doubleValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jdouble Java_Long_doubleValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "doubleValue",
          "()D",
          &g_java_lang_Long_doubleValue);

  jdouble ret =
      env->CallDoubleMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_toStringJLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Long_toStringJLS(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_Long_toStringJLS(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "()Ljava/lang/String;",
          &g_java_lang_Long_toStringJLS);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_hashCodeI(nullptr);
[[maybe_unused]] static jint Java_Long_hashCodeI(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static jint Java_Long_hashCodeI(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_lang_Long_hashCodeI);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_hashCodeI_J(nullptr);
[[maybe_unused]] static jint Java_Long_hashCodeI_J(JNIEnv* env, jlong p0);
static jint Java_Long_hashCodeI_J(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "hashCode",
          "(J)I",
          &g_java_lang_Long_hashCodeI_J);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_equals(nullptr);
[[maybe_unused]] static jboolean Java_Long_equals(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Long_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_lang_Long_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_getLongJLLO_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Long_getLongJLLO_JLS(JNIEnv*
    env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_getLongJLLO_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getLong",
          "(Ljava/lang/String;)Ljava/lang/Long;",
          &g_java_lang_Long_getLongJLLO_JLS);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_getLongJLLO_JLS_J(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Long_getLongJLLO_JLS_J(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    jlong p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_getLongJLLO_JLS_J(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getLong",
          "(Ljava/lang/String;J)Ljava/lang/Long;",
          &g_java_lang_Long_getLongJLLO_JLS_J);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_getLongJLLO_JLS_JLLO(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Long_getLongJLLO_JLS_JLLO(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_getLongJLLO_JLS_JLLO(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getLong",
          "(Ljava/lang/String;Ljava/lang/Long;)Ljava/lang/Long;",
          &g_java_lang_Long_getLongJLLO_JLS_JLLO);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_compareToI_JLLO(nullptr);
[[maybe_unused]] static jint Java_Long_compareToI_JLLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Long_compareToI_JLLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Long;)I",
          &g_java_lang_Long_compareToI_JLLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_compare(nullptr);
[[maybe_unused]] static jint Java_Long_compare(JNIEnv* env, jlong p0,
    jlong p1);
static jint Java_Long_compare(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "compare",
          "(JJ)I",
          &g_java_lang_Long_compare);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_compareUnsigned(nullptr);
[[maybe_unused]] static jint Java_Long_compareUnsigned(JNIEnv* env, jlong p0,
    jlong p1);
static jint Java_Long_compareUnsigned(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "compareUnsigned",
          "(JJ)I",
          &g_java_lang_Long_compareUnsigned);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_divideUnsigned(nullptr);
[[maybe_unused]] static jlong Java_Long_divideUnsigned(JNIEnv* env, jlong p0,
    jlong p1);
static jlong Java_Long_divideUnsigned(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "divideUnsigned",
          "(JJ)J",
          &g_java_lang_Long_divideUnsigned);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_remainderUnsigned(nullptr);
[[maybe_unused]] static jlong Java_Long_remainderUnsigned(JNIEnv* env, jlong p0,
    jlong p1);
static jlong Java_Long_remainderUnsigned(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "remainderUnsigned",
          "(JJ)J",
          &g_java_lang_Long_remainderUnsigned);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_highestOneBit(nullptr);
[[maybe_unused]] static jlong Java_Long_highestOneBit(JNIEnv* env, jlong p0);
static jlong Java_Long_highestOneBit(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "highestOneBit",
          "(J)J",
          &g_java_lang_Long_highestOneBit);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_lowestOneBit(nullptr);
[[maybe_unused]] static jlong Java_Long_lowestOneBit(JNIEnv* env, jlong p0);
static jlong Java_Long_lowestOneBit(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "lowestOneBit",
          "(J)J",
          &g_java_lang_Long_lowestOneBit);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_numberOfLeadingZeros(nullptr);
[[maybe_unused]] static jint Java_Long_numberOfLeadingZeros(JNIEnv* env, jlong p0);
static jint Java_Long_numberOfLeadingZeros(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "numberOfLeadingZeros",
          "(J)I",
          &g_java_lang_Long_numberOfLeadingZeros);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_numberOfTrailingZeros(nullptr);
[[maybe_unused]] static jint Java_Long_numberOfTrailingZeros(JNIEnv* env, jlong p0);
static jint Java_Long_numberOfTrailingZeros(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "numberOfTrailingZeros",
          "(J)I",
          &g_java_lang_Long_numberOfTrailingZeros);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_bitCount(nullptr);
[[maybe_unused]] static jint Java_Long_bitCount(JNIEnv* env, jlong p0);
static jint Java_Long_bitCount(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "bitCount",
          "(J)I",
          &g_java_lang_Long_bitCount);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_rotateLeft(nullptr);
[[maybe_unused]] static jlong Java_Long_rotateLeft(JNIEnv* env, jlong p0,
    JniIntWrapper p1);
static jlong Java_Long_rotateLeft(JNIEnv* env, jlong p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "rotateLeft",
          "(JI)J",
          &g_java_lang_Long_rotateLeft);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_rotateRight(nullptr);
[[maybe_unused]] static jlong Java_Long_rotateRight(JNIEnv* env, jlong p0,
    JniIntWrapper p1);
static jlong Java_Long_rotateRight(JNIEnv* env, jlong p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "rotateRight",
          "(JI)J",
          &g_java_lang_Long_rotateRight);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_reverse(nullptr);
[[maybe_unused]] static jlong Java_Long_reverse(JNIEnv* env, jlong p0);
static jlong Java_Long_reverse(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "reverse",
          "(J)J",
          &g_java_lang_Long_reverse);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_signum(nullptr);
[[maybe_unused]] static jint Java_Long_signum(JNIEnv* env, jlong p0);
static jint Java_Long_signum(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "signum",
          "(J)I",
          &g_java_lang_Long_signum);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_reverseBytes(nullptr);
[[maybe_unused]] static jlong Java_Long_reverseBytes(JNIEnv* env, jlong p0);
static jlong Java_Long_reverseBytes(JNIEnv* env, jlong p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "reverseBytes",
          "(J)J",
          &g_java_lang_Long_reverseBytes);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_sum(nullptr);
[[maybe_unused]] static jlong Java_Long_sum(JNIEnv* env, jlong p0,
    jlong p1);
static jlong Java_Long_sum(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "sum",
          "(JJ)J",
          &g_java_lang_Long_sum);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_max(nullptr);
[[maybe_unused]] static jlong Java_Long_max(JNIEnv* env, jlong p0,
    jlong p1);
static jlong Java_Long_max(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "max",
          "(JJ)J",
          &g_java_lang_Long_max);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_min(nullptr);
[[maybe_unused]] static jlong Java_Long_min(JNIEnv* env, jlong p0,
    jlong p1);
static jlong Java_Long_min(JNIEnv* env, jlong p0,
    jlong p1) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "min",
          "(JJ)J",
          &g_java_lang_Long_min);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, p0, p1);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_compareToI_JLO(nullptr);
[[maybe_unused]] static jint Java_Long_compareToI_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Long_compareToI_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Long_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Object;)I",
          &g_java_lang_Long_compareToI_JLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Long_ConstructorJLLO_J(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Long_ConstructorJLLO_J(JNIEnv* env, jlong p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_ConstructorJLLO_J(JNIEnv* env, jlong p0)
    {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(J)V",
          &g_java_lang_Long_ConstructorJLLO_J);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Long_ConstructorJLLO_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Long_ConstructorJLLO_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Long_ConstructorJLLO_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Long_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Long_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;)V",
          &g_java_lang_Long_ConstructorJLLO_JLS);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_Long

#endif  // java_lang_Long_JNI
