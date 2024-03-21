// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/lang/Integer

#ifndef java_lang_Integer_JNI
#define java_lang_Integer_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_lang_Integer[];
const char kClassPath_java_lang_Integer[] = "java/lang/Integer";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_lang_Integer_clazz(nullptr);
#ifndef java_lang_Integer_clazz_defined
#define java_lang_Integer_clazz_defined
inline jclass java_lang_Integer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_lang_Integer, &g_java_lang_Integer_clazz);
}
#endif


namespace JNI_Integer {

enum Java_Integer_constant_fields {
  BYTES = 4,
  MAX_VALUE = 2147483647,
  MIN_VALUE = -2147483648,
  SIZE = 32,
};


}  // namespace JNI_Integer
// Step 3: Method stubs.
namespace JNI_Integer {


static std::atomic<jmethodID> g_java_lang_Integer_toStringJLS_I_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Integer_toStringJLS_I_I(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toStringJLS_I_I(JNIEnv* env,
    JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toString",
          "(II)Ljava/lang/String;",
          &g_java_lang_Integer_toStringJLS_I_I);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_toUnsignedStringJLS_I_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Integer_toUnsignedStringJLS_I_I(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toUnsignedStringJLS_I_I(JNIEnv* env,
    JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toUnsignedString",
          "(II)Ljava/lang/String;",
          &g_java_lang_Integer_toUnsignedStringJLS_I_I);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_toHexString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toHexString(JNIEnv*
    env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toHexString(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toHexString",
          "(I)Ljava/lang/String;",
          &g_java_lang_Integer_toHexString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_toOctalString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Integer_toOctalString(JNIEnv* env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toOctalString(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toOctalString",
          "(I)Ljava/lang/String;",
          &g_java_lang_Integer_toOctalString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_toBinaryString(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Integer_toBinaryString(JNIEnv* env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toBinaryString(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toBinaryString",
          "(I)Ljava/lang/String;",
          &g_java_lang_Integer_toBinaryString);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_toStringJLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Integer_toStringJLS_I(JNIEnv* env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toStringJLS_I(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toString",
          "(I)Ljava/lang/String;",
          &g_java_lang_Integer_toStringJLS_I);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_toUnsignedStringJLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring>
    Java_Integer_toUnsignedStringJLS_I(JNIEnv* env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toUnsignedStringJLS_I(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toUnsignedString",
          "(I)Ljava/lang/String;",
          &g_java_lang_Integer_toUnsignedStringJLS_I);

  jstring ret =
      static_cast<jstring>(env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0)));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_parseIntI_JLS_I(nullptr);
[[maybe_unused]] static jint Java_Integer_parseIntI_JLS_I(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static jint Java_Integer_parseIntI_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseInt",
          "(Ljava/lang/String;I)I",
          &g_java_lang_Integer_parseIntI_JLS_I);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_parseIntI_JLS(nullptr);
[[maybe_unused]] static jint Java_Integer_parseIntI_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jint Java_Integer_parseIntI_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseInt",
          "(Ljava/lang/String;)I",
          &g_java_lang_Integer_parseIntI_JLS);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_parseUnsignedIntI_JLS_I(nullptr);
[[maybe_unused]] static jint Java_Integer_parseUnsignedIntI_JLS_I(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static jint Java_Integer_parseUnsignedIntI_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>&
    p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseUnsignedInt",
          "(Ljava/lang/String;I)I",
          &g_java_lang_Integer_parseUnsignedIntI_JLS_I);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_parseUnsignedIntI_JLS(nullptr);
[[maybe_unused]] static jint Java_Integer_parseUnsignedIntI_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0);
static jint Java_Integer_parseUnsignedIntI_JLS(JNIEnv* env, const base::android::JavaRef<jstring>&
    p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "parseUnsignedInt",
          "(Ljava/lang/String;)I",
          &g_java_lang_Integer_parseUnsignedIntI_JLS);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_valueOfJLI_JLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_valueOfJLI_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_valueOfJLI_JLS_I(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/String;I)Ljava/lang/Integer;",
          &g_java_lang_Integer_valueOfJLI_JLS_I);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_valueOfJLI_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_valueOfJLI_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_valueOfJLI_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(Ljava/lang/String;)Ljava/lang/Integer;",
          &g_java_lang_Integer_valueOfJLI_JLS);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_valueOfJLI_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Integer_valueOfJLI_I(JNIEnv*
    env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_valueOfJLI_I(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "valueOf",
          "(I)Ljava/lang/Integer;",
          &g_java_lang_Integer_valueOfJLI_I);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_byteValue(nullptr);
[[maybe_unused]] static jbyte Java_Integer_byteValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jbyte Java_Integer_byteValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "byteValue",
          "()B",
          &g_java_lang_Integer_byteValue);

  jbyte ret =
      env->CallByteMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_shortValue(nullptr);
[[maybe_unused]] static jshort Java_Integer_shortValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jshort Java_Integer_shortValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "shortValue",
          "()S",
          &g_java_lang_Integer_shortValue);

  jshort ret =
      env->CallShortMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_intValue(nullptr);
[[maybe_unused]] static jint Java_Integer_intValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_Integer_intValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "intValue",
          "()I",
          &g_java_lang_Integer_intValue);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_longValue(nullptr);
[[maybe_unused]] static jlong Java_Integer_longValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jlong Java_Integer_longValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "longValue",
          "()J",
          &g_java_lang_Integer_longValue);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_floatValue(nullptr);
[[maybe_unused]] static jfloat Java_Integer_floatValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jfloat Java_Integer_floatValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "floatValue",
          "()F",
          &g_java_lang_Integer_floatValue);

  jfloat ret =
      env->CallFloatMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_doubleValue(nullptr);
[[maybe_unused]] static jdouble Java_Integer_doubleValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jdouble Java_Integer_doubleValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "doubleValue",
          "()D",
          &g_java_lang_Integer_doubleValue);

  jdouble ret =
      env->CallDoubleMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_toStringJLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toStringJLS(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jstring> Java_Integer_toStringJLS(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toString",
          "()Ljava/lang/String;",
          &g_java_lang_Integer_toStringJLS);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_hashCodeI(nullptr);
[[maybe_unused]] static jint Java_Integer_hashCodeI(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jint Java_Integer_hashCodeI(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_lang_Integer_hashCodeI);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_hashCodeI_I(nullptr);
[[maybe_unused]] static jint Java_Integer_hashCodeI_I(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_hashCodeI_I(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "hashCode",
          "(I)I",
          &g_java_lang_Integer_hashCodeI_I);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_equals(nullptr);
[[maybe_unused]] static jboolean Java_Integer_equals(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Integer_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_lang_Integer_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_getIntegerJLI_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_getIntegerJLI_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_getIntegerJLI_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getInteger",
          "(Ljava/lang/String;)Ljava/lang/Integer;",
          &g_java_lang_Integer_getIntegerJLI_JLS);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_getIntegerJLI_JLS_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_getIntegerJLI_JLS_I(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_getIntegerJLI_JLS_I(JNIEnv* env,
    const base::android::JavaRef<jstring>& p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getInteger",
          "(Ljava/lang/String;I)Ljava/lang/Integer;",
          &g_java_lang_Integer_getIntegerJLI_JLS_I);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), as_jint(p1));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_getIntegerJLI_JLS_JLI(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_getIntegerJLI_JLS_JLI(JNIEnv* env, const base::android::JavaRef<jstring>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_getIntegerJLI_JLS_JLI(JNIEnv* env,
    const base::android::JavaRef<jstring>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getInteger",
          "(Ljava/lang/String;Ljava/lang/Integer;)Ljava/lang/Integer;",
          &g_java_lang_Integer_getIntegerJLI_JLS_JLI);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_decode(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Integer_decode(JNIEnv* env,
    const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_decode(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "decode",
          "(Ljava/lang/String;)Ljava/lang/Integer;",
          &g_java_lang_Integer_decode);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_compareToI_JLI(nullptr);
[[maybe_unused]] static jint Java_Integer_compareToI_JLI(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Integer_compareToI_JLI(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Integer;)I",
          &g_java_lang_Integer_compareToI_JLI);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_compare(nullptr);
[[maybe_unused]] static jint Java_Integer_compare(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_compare(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "compare",
          "(II)I",
          &g_java_lang_Integer_compare);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_compareUnsigned(nullptr);
[[maybe_unused]] static jint Java_Integer_compareUnsigned(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_compareUnsigned(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "compareUnsigned",
          "(II)I",
          &g_java_lang_Integer_compareUnsigned);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_toUnsignedLong(nullptr);
[[maybe_unused]] static jlong Java_Integer_toUnsignedLong(JNIEnv* env, JniIntWrapper p0);
static jlong Java_Integer_toUnsignedLong(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "toUnsignedLong",
          "(I)J",
          &g_java_lang_Integer_toUnsignedLong);

  jlong ret =
      env->CallStaticLongMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_divideUnsigned(nullptr);
[[maybe_unused]] static jint Java_Integer_divideUnsigned(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_divideUnsigned(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "divideUnsigned",
          "(II)I",
          &g_java_lang_Integer_divideUnsigned);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_remainderUnsigned(nullptr);
[[maybe_unused]] static jint Java_Integer_remainderUnsigned(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_remainderUnsigned(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "remainderUnsigned",
          "(II)I",
          &g_java_lang_Integer_remainderUnsigned);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_highestOneBit(nullptr);
[[maybe_unused]] static jint Java_Integer_highestOneBit(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_highestOneBit(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "highestOneBit",
          "(I)I",
          &g_java_lang_Integer_highestOneBit);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_lowestOneBit(nullptr);
[[maybe_unused]] static jint Java_Integer_lowestOneBit(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_lowestOneBit(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "lowestOneBit",
          "(I)I",
          &g_java_lang_Integer_lowestOneBit);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_numberOfLeadingZeros(nullptr);
[[maybe_unused]] static jint Java_Integer_numberOfLeadingZeros(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_numberOfLeadingZeros(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "numberOfLeadingZeros",
          "(I)I",
          &g_java_lang_Integer_numberOfLeadingZeros);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_numberOfTrailingZeros(nullptr);
[[maybe_unused]] static jint Java_Integer_numberOfTrailingZeros(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_numberOfTrailingZeros(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "numberOfTrailingZeros",
          "(I)I",
          &g_java_lang_Integer_numberOfTrailingZeros);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_bitCount(nullptr);
[[maybe_unused]] static jint Java_Integer_bitCount(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_bitCount(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "bitCount",
          "(I)I",
          &g_java_lang_Integer_bitCount);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_rotateLeft(nullptr);
[[maybe_unused]] static jint Java_Integer_rotateLeft(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_rotateLeft(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "rotateLeft",
          "(II)I",
          &g_java_lang_Integer_rotateLeft);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_rotateRight(nullptr);
[[maybe_unused]] static jint Java_Integer_rotateRight(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_rotateRight(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "rotateRight",
          "(II)I",
          &g_java_lang_Integer_rotateRight);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_reverse(nullptr);
[[maybe_unused]] static jint Java_Integer_reverse(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_reverse(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "reverse",
          "(I)I",
          &g_java_lang_Integer_reverse);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_signum(nullptr);
[[maybe_unused]] static jint Java_Integer_signum(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_signum(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "signum",
          "(I)I",
          &g_java_lang_Integer_signum);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_reverseBytes(nullptr);
[[maybe_unused]] static jint Java_Integer_reverseBytes(JNIEnv* env, JniIntWrapper p0);
static jint Java_Integer_reverseBytes(JNIEnv* env, JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "reverseBytes",
          "(I)I",
          &g_java_lang_Integer_reverseBytes);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_sum(nullptr);
[[maybe_unused]] static jint Java_Integer_sum(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_sum(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "sum",
          "(II)I",
          &g_java_lang_Integer_sum);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_max(nullptr);
[[maybe_unused]] static jint Java_Integer_max(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_max(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "max",
          "(II)I",
          &g_java_lang_Integer_max);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_min(nullptr);
[[maybe_unused]] static jint Java_Integer_min(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1);
static jint Java_Integer_min(JNIEnv* env, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "min",
          "(II)I",
          &g_java_lang_Integer_min);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_compareToI_JLO(nullptr);
[[maybe_unused]] static jint Java_Integer_compareToI_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_Integer_compareToI_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_lang_Integer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compareTo",
          "(Ljava/lang/Object;)I",
          &g_java_lang_Integer_compareToI_JLO);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_lang_Integer_ConstructorJLI_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_ConstructorJLI_I(JNIEnv* env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_ConstructorJLI_I(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(I)V",
          &g_java_lang_Integer_ConstructorJLI_I);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_lang_Integer_ConstructorJLI_JLS(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Integer_ConstructorJLI_JLS(JNIEnv* env, const base::android::JavaRef<jstring>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Integer_ConstructorJLI_JLS(JNIEnv* env, const
    base::android::JavaRef<jstring>& p0) {
  jclass clazz = java_lang_Integer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_lang_Integer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;)V",
          &g_java_lang_Integer_ConstructorJLI_JLS);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_Integer

#endif  // java_lang_Integer_JNI
