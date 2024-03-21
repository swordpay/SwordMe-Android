// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/util/Map

#ifndef java_util_Map_JNI
#define java_util_Map_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_util_Map[];
const char kClassPath_java_util_Map[] = "java/util/Map";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_util_Map_clazz(nullptr);
#ifndef java_util_Map_clazz_defined
#define java_util_Map_clazz_defined
inline jclass java_util_Map_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_util_Map, &g_java_util_Map_clazz);
}
#endif


namespace JNI_Map {


static std::atomic<jmethodID> g_java_util_Map_size(nullptr);
[[maybe_unused]] static jint Java_Map_size(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static jint Java_Map_size(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "size",
          "()I",
          &g_java_util_Map_size);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_isEmpty(nullptr);
[[maybe_unused]] static jboolean Java_Map_isEmpty(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jboolean Java_Map_isEmpty(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "isEmpty",
          "()Z",
          &g_java_util_Map_isEmpty);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_containsKey(nullptr);
[[maybe_unused]] static jboolean Java_Map_containsKey(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Map_containsKey(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "containsKey",
          "(Ljava/lang/Object;)Z",
          &g_java_util_Map_containsKey);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_containsValue(nullptr);
[[maybe_unused]] static jboolean Java_Map_containsValue(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Map_containsValue(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "containsValue",
          "(Ljava/lang/Object;)Z",
          &g_java_util_Map_containsValue);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_get(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_get(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_get(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "get",
          "(Ljava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_Map_get);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_put(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_put(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_put(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "put",
          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_Map_put);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_removeJUV_JLO(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_removeJUV_JLO(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_removeJUV_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "remove",
          "(Ljava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_Map_removeJUV_JLO);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_putAll(nullptr);
[[maybe_unused]] static void Java_Map_putAll(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& p0);
static void Java_Map_putAll(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "putAll",
          "(Ljava/util/Map;)V",
          &g_java_util_Map_putAll);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

static std::atomic<jmethodID> g_java_util_Map_clear(nullptr);
[[maybe_unused]] static void Java_Map_clear(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static void Java_Map_clear(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "clear",
          "()V",
          &g_java_util_Map_clear);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_java_util_Map_keySet(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_keySet(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_keySet(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "keySet",
          "()Ljava/util/Set;",
          &g_java_util_Map_keySet);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_values(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_values(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_values(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "values",
          "()Ljava/util/Collection;",
          &g_java_util_Map_values);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_entrySet(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_entrySet(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_entrySet(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "entrySet",
          "()Ljava/util/Set;",
          &g_java_util_Map_entrySet);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_equals(nullptr);
[[maybe_unused]] static jboolean Java_Map_equals(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_Map_equals(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "equals",
          "(Ljava/lang/Object;)Z",
          &g_java_util_Map_equals);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_hashCode(nullptr);
[[maybe_unused]] static jint Java_Map_hashCode(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static jint Java_Map_hashCode(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hashCode",
          "()I",
          &g_java_util_Map_hashCode);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_getOrDefault(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_getOrDefault(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_getOrDefault(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getOrDefault",
          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_Map_getOrDefault);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_forEach(nullptr);
[[maybe_unused]] static void Java_Map_forEach(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& p0);
static void Java_Map_forEach(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "forEach",
          "(Ljava/util/function/BiConsumer;)V",
          &g_java_util_Map_forEach);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

static std::atomic<jmethodID> g_java_util_Map_replaceAll(nullptr);
[[maybe_unused]] static void Java_Map_replaceAll(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& p0);
static void Java_Map_replaceAll(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "replaceAll",
          "(Ljava/util/function/BiFunction;)V",
          &g_java_util_Map_replaceAll);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

static std::atomic<jmethodID> g_java_util_Map_putIfAbsent(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_putIfAbsent(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_putIfAbsent(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "putIfAbsent",
          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_Map_putIfAbsent);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_removeZ_JLO_JLO(nullptr);
[[maybe_unused]] static jboolean Java_Map_removeZ_JLO_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static jboolean Java_Map_removeZ_JLO_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "remove",
          "(Ljava/lang/Object;Ljava/lang/Object;)Z",
          &g_java_util_Map_removeZ_JLO_JLO);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_replaceZ_JUK_JUV_JUV(nullptr);
[[maybe_unused]] static jboolean Java_Map_replaceZ_JUK_JUV_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2);
static jboolean Java_Map_replaceZ_JUK_JUV_JUV(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "replace",
          "(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Z",
          &g_java_util_Map_replaceZ_JUK_JUV_JUV);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_Map_replaceJUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_replaceJUV_JUK_JUV(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_replaceJUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "replace",
          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_Map_replaceJUV_JUK_JUV);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_computeIfAbsent(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_computeIfAbsent(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_computeIfAbsent(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "computeIfAbsent",
          "(Ljava/lang/Object;Ljava/util/function/Function;)Ljava/lang/Object;",
          &g_java_util_Map_computeIfAbsent);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_computeIfPresent(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_computeIfPresent(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_computeIfPresent(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "computeIfPresent",
          "(Ljava/lang/Object;Ljava/util/function/BiFunction;)Ljava/lang/Object;",
          &g_java_util_Map_computeIfPresent);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_compute(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_compute(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_compute(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "compute",
          "(Ljava/lang/Object;Ljava/util/function/BiFunction;)Ljava/lang/Object;",
          &g_java_util_Map_compute);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_merge(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_merge(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_merge(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "merge",
          "(Ljava/lang/Object;Ljava/lang/Object;Ljava/util/function/BiFunction;)Ljava/lang/Object;",
          &g_java_util_Map_merge);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_ofJUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofJUV(JNIEnv* env);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofJUV(JNIEnv* env) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
          "()Ljava/util/Map;",
          &g_java_util_Map_ofJUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_ofJUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofJUV_JUK_JUV(JNIEnv*
    env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofJUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV(JNIEnv* env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofJUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv*
    env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj(), p8.obj(), p9.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj(), p8.obj(), p9.obj(), p10.obj(), p11.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj(), p8.obj(), p9.obj(), p10.obj(), p11.obj(), p12.obj(), p13.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env,
    const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13,
    const base::android::JavaRef<jobject>& p14,
    const base::android::JavaRef<jobject>& p15);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv* env,
    const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13,
    const base::android::JavaRef<jobject>& p14,
    const base::android::JavaRef<jobject>& p15) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
          &g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj(), p8.obj(), p9.obj(), p10.obj(), p11.obj(), p12.obj(), p13.obj(),
              p14.obj(), p15.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv*
    env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13,
    const base::android::JavaRef<jobject>& p14,
    const base::android::JavaRef<jobject>& p15,
    const base::android::JavaRef<jobject>& p16,
    const base::android::JavaRef<jobject>& p17);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv*
    env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13,
    const base::android::JavaRef<jobject>& p14,
    const base::android::JavaRef<jobject>& p15,
    const base::android::JavaRef<jobject>& p16,
    const base::android::JavaRef<jobject>& p17) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
&g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj(), p8.obj(), p9.obj(), p10.obj(), p11.obj(), p12.obj(), p13.obj(),
              p14.obj(), p15.obj(), p16.obj(), p17.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv*
    env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13,
    const base::android::JavaRef<jobject>& p14,
    const base::android::JavaRef<jobject>& p15,
    const base::android::JavaRef<jobject>& p16,
    const base::android::JavaRef<jobject>& p17,
    const base::android::JavaRef<jobject>& p18,
    const base::android::JavaRef<jobject>& p19);
static base::android::ScopedJavaLocalRef<jobject>
    Java_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV(JNIEnv*
    env, const base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1,
    const base::android::JavaRef<jobject>& p2,
    const base::android::JavaRef<jobject>& p3,
    const base::android::JavaRef<jobject>& p4,
    const base::android::JavaRef<jobject>& p5,
    const base::android::JavaRef<jobject>& p6,
    const base::android::JavaRef<jobject>& p7,
    const base::android::JavaRef<jobject>& p8,
    const base::android::JavaRef<jobject>& p9,
    const base::android::JavaRef<jobject>& p10,
    const base::android::JavaRef<jobject>& p11,
    const base::android::JavaRef<jobject>& p12,
    const base::android::JavaRef<jobject>& p13,
    const base::android::JavaRef<jobject>& p14,
    const base::android::JavaRef<jobject>& p15,
    const base::android::JavaRef<jobject>& p16,
    const base::android::JavaRef<jobject>& p17,
    const base::android::JavaRef<jobject>& p18,
    const base::android::JavaRef<jobject>& p19) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "of",
"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map;",
&g_java_util_Map_ofJUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV_JUK_JUV);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj(), p2.obj(), p3.obj(), p4.obj(), p5.obj(),
              p6.obj(), p7.obj(), p8.obj(), p9.obj(), p10.obj(), p11.obj(), p12.obj(), p13.obj(),
              p14.obj(), p15.obj(), p16.obj(), p17.obj(), p18.obj(), p19.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_ofEntries(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofEntries(JNIEnv* env,
    const base::android::JavaRef<jobjectArray>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_ofEntries(JNIEnv* env, const
    base::android::JavaRef<jobjectArray>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "ofEntries",
          "([Ljava/util/Map$Entry;)Ljava/util/Map;",
          &g_java_util_Map_ofEntries);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_entry(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_entry(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_entry(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "entry",
          "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/util/Map$Entry;",
          &g_java_util_Map_entry);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj(), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Map_copyOf(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Map_copyOf(JNIEnv* env,
    const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_Map_copyOf(JNIEnv* env, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Map_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_Map_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "copyOf",
          "(Ljava/util/Map;)Ljava/util/Map;",
          &g_java_util_Map_copyOf);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_Map

#endif  // java_util_Map_JNI
