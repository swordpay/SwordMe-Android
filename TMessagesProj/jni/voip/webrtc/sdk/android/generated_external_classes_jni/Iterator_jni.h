// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/util/Iterator

#ifndef java_util_Iterator_JNI
#define java_util_Iterator_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_util_Iterator[];
const char kClassPath_java_util_Iterator[] = "java/util/Iterator";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_util_Iterator_clazz(nullptr);
#ifndef java_util_Iterator_clazz_defined
#define java_util_Iterator_clazz_defined
inline jclass java_util_Iterator_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_util_Iterator,
      &g_java_util_Iterator_clazz);
}
#endif


namespace JNI_Iterator {


static std::atomic<jmethodID> g_java_util_Iterator_hasNext(nullptr);
[[maybe_unused]] static jboolean Java_Iterator_hasNext(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jboolean Java_Iterator_hasNext(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Iterator_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Iterator_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "hasNext",
          "()Z",
          &g_java_util_Iterator_hasNext);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_util_Iterator_next(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_Iterator_next(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_Iterator_next(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Iterator_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Iterator_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "next",
          "()Ljava/lang/Object;",
          &g_java_util_Iterator_next);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_Iterator_remove(nullptr);
[[maybe_unused]] static void Java_Iterator_remove(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static void Java_Iterator_remove(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_Iterator_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Iterator_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "remove",
          "()V",
          &g_java_util_Iterator_remove);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_java_util_Iterator_forEachRemaining(nullptr);
[[maybe_unused]] static void Java_Iterator_forEachRemaining(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static void Java_Iterator_forEachRemaining(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_Iterator_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_Iterator_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "forEachRemaining",
          "(Ljava/util/function/Consumer;)V",
          &g_java_util_Iterator_forEachRemaining);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

}  // namespace JNI_Iterator

#endif  // java_util_Iterator_JNI
