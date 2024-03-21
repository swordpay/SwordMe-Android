// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     java/util/ArrayList

#ifndef java_util_ArrayList_JNI
#define java_util_ArrayList_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_java_util_ArrayList[];
const char kClassPath_java_util_ArrayList[] = "java/util/ArrayList";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_util_ArrayList_clazz(nullptr);
#ifndef java_util_ArrayList_clazz_defined
#define java_util_ArrayList_clazz_defined
inline jclass java_util_ArrayList_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_util_ArrayList,
      &g_java_util_ArrayList_clazz);
}
#endif


namespace JNI_ArrayList {


static std::atomic<jmethodID> g_java_util_ArrayList_trimToSize(nullptr);
[[maybe_unused]] static void Java_ArrayList_trimToSize(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static void Java_ArrayList_trimToSize(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "trimToSize",
          "()V",
          &g_java_util_ArrayList_trimToSize);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_java_util_ArrayList_ensureCapacity(nullptr);
[[maybe_unused]] static void Java_ArrayList_ensureCapacity(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static void Java_ArrayList_ensureCapacity(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "ensureCapacity",
          "(I)V",
          &g_java_util_ArrayList_ensureCapacity);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
}

static std::atomic<jmethodID> g_java_util_ArrayList_size(nullptr);
[[maybe_unused]] static jint Java_ArrayList_size(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj);
static jint Java_ArrayList_size(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "size",
          "()I",
          &g_java_util_ArrayList_size);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_isEmpty(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_isEmpty(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static jboolean Java_ArrayList_isEmpty(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "isEmpty",
          "()Z",
          &g_java_util_ArrayList_isEmpty);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_contains(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_contains(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_contains(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "contains",
          "(Ljava/lang/Object;)Z",
          &g_java_util_ArrayList_contains);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_indexOf(nullptr);
[[maybe_unused]] static jint Java_ArrayList_indexOf(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_ArrayList_indexOf(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "indexOf",
          "(Ljava/lang/Object;)I",
          &g_java_util_ArrayList_indexOf);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_lastIndexOf(nullptr);
[[maybe_unused]] static jint Java_ArrayList_lastIndexOf(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jint Java_ArrayList_lastIndexOf(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "lastIndexOf",
          "(Ljava/lang/Object;)I",
          &g_java_util_ArrayList_lastIndexOf);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_clone(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_clone(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_clone(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "clone",
          "()Ljava/lang/Object;",
          &g_java_util_ArrayList_clone);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_toArrayLJLO(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_ArrayList_toArrayLJLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobjectArray> Java_ArrayList_toArrayLJLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toArray",
          "()[Ljava/lang/Object;",
          &g_java_util_ArrayList_toArrayLJLO);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_toArrayLJUT_LJUT(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_ArrayList_toArrayLJUT_LJUT(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobjectArray>& p0);
static base::android::ScopedJavaLocalRef<jobjectArray> Java_ArrayList_toArrayLJUT_LJUT(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobjectArray>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toArray",
          "([Ljava/lang/Object;)[Ljava/lang/Object;",
          &g_java_util_ArrayList_toArrayLJUT_LJUT);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, p0.obj()));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_get(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_get(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_get(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "get",
          "(I)Ljava/lang/Object;",
          &g_java_util_ArrayList_get);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_set(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_set(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_set(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "set",
          "(ILjava/lang/Object;)Ljava/lang/Object;",
          &g_java_util_ArrayList_set);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0), p1.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_addZ_JUE(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_addZ_JUE(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_addZ_JUE(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "add",
          "(Ljava/lang/Object;)Z",
          &g_java_util_ArrayList_addZ_JUE);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_addV_I_JUE(nullptr);
[[maybe_unused]] static void Java_ArrayList_addV_I_JUE(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1);
static void Java_ArrayList_addV_I_JUE(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "add",
          "(ILjava/lang/Object;)V",
          &g_java_util_ArrayList_addV_I_JUE);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0), p1.obj());
}

static std::atomic<jmethodID> g_java_util_ArrayList_removeJUE_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_removeJUE_I(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_removeJUE_I(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "remove",
          "(I)Ljava/lang/Object;",
          &g_java_util_ArrayList_removeJUE_I);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_removeZ_JLO(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_removeZ_JLO(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_removeZ_JLO(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "remove",
          "(Ljava/lang/Object;)Z",
          &g_java_util_ArrayList_removeZ_JLO);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_clear(nullptr);
[[maybe_unused]] static void Java_ArrayList_clear(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj);
static void Java_ArrayList_clear(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "clear",
          "()V",
          &g_java_util_ArrayList_clear);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_java_util_ArrayList_addAllZ_JUC(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_addAllZ_JUC(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_addAllZ_JUC(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "addAll",
          "(Ljava/util/Collection;)Z",
          &g_java_util_ArrayList_addAllZ_JUC);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_addAllZ_I_JUC(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_addAllZ_I_JUC(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1);
static jboolean Java_ArrayList_addAllZ_I_JUC(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, JniIntWrapper p0,
    const base::android::JavaRef<jobject>& p1) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "addAll",
          "(ILjava/util/Collection;)Z",
          &g_java_util_ArrayList_addAllZ_I_JUC);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0), p1.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_removeRange(nullptr);
[[maybe_unused]] static void Java_ArrayList_removeRange(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    JniIntWrapper p1);
static void Java_ArrayList_removeRange(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "removeRange",
          "(II)V",
          &g_java_util_ArrayList_removeRange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0), as_jint(p1));
}

static std::atomic<jmethodID> g_java_util_ArrayList_removeAll(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_removeAll(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_removeAll(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "removeAll",
          "(Ljava/util/Collection;)Z",
          &g_java_util_ArrayList_removeAll);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_retainAll(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_retainAll(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_retainAll(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "retainAll",
          "(Ljava/util/Collection;)Z",
          &g_java_util_ArrayList_retainAll);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_listIteratorJULII_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_listIteratorJULII_I(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_listIteratorJULII_I(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj, JniIntWrapper p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "listIterator",
          "(I)Ljava/util/ListIterator;",
          &g_java_util_ArrayList_listIteratorJULII_I);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_listIteratorJULII(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_listIteratorJULII(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_listIteratorJULII(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "listIterator",
          "()Ljava/util/ListIterator;",
          &g_java_util_ArrayList_listIteratorJULII);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_iterator(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_iterator(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_iterator(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "iterator",
          "()Ljava/util/Iterator;",
          &g_java_util_ArrayList_iterator);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_subList(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_subList(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    JniIntWrapper p1);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_subList(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper p0,
    JniIntWrapper p1) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "subList",
          "(II)Ljava/util/List;",
          &g_java_util_ArrayList_subList);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(p0), as_jint(p1));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_forEach(nullptr);
[[maybe_unused]] static void Java_ArrayList_forEach(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static void Java_ArrayList_forEach(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "forEach",
          "(Ljava/util/function/Consumer;)V",
          &g_java_util_ArrayList_forEach);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

static std::atomic<jmethodID> g_java_util_ArrayList_spliterator(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_spliterator(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_spliterator(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "spliterator",
          "()Ljava/util/Spliterator;",
          &g_java_util_ArrayList_spliterator);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_removeIf(nullptr);
[[maybe_unused]] static jboolean Java_ArrayList_removeIf(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static jboolean Java_ArrayList_removeIf(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "removeIf",
          "(Ljava/util/function/Predicate;)Z",
          &g_java_util_ArrayList_removeIf);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
  return ret;
}

static std::atomic<jmethodID> g_java_util_ArrayList_replaceAll(nullptr);
[[maybe_unused]] static void Java_ArrayList_replaceAll(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& p0);
static void Java_ArrayList_replaceAll(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "replaceAll",
          "(Ljava/util/function/UnaryOperator;)V",
          &g_java_util_ArrayList_replaceAll);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

static std::atomic<jmethodID> g_java_util_ArrayList_sort(nullptr);
[[maybe_unused]] static void Java_ArrayList_sort(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& p0);
static void Java_ArrayList_sort(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      java_util_ArrayList_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "sort",
          "(Ljava/util/Comparator;)V",
          &g_java_util_ArrayList_sort);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, p0.obj());
}

static std::atomic<jmethodID> g_java_util_ArrayList_ConstructorJUALI_I(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_ConstructorJUALI_I(JNIEnv* env, JniIntWrapper p0);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_ConstructorJUALI_I(JNIEnv* env,
    JniIntWrapper p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(I)V",
          &g_java_util_ArrayList_ConstructorJUALI_I);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(p0));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_ConstructorJUALI(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_ConstructorJUALI(JNIEnv* env);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_ConstructorJUALI(JNIEnv* env) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "()V",
          &g_java_util_ArrayList_ConstructorJUALI);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_java_util_ArrayList_ConstructorJUALI_JUC(nullptr);
[[maybe_unused]] static base::android::ScopedJavaLocalRef<jobject>
    Java_ArrayList_ConstructorJUALI_JUC(JNIEnv* env, const base::android::JavaRef<jobject>& p0);
static base::android::ScopedJavaLocalRef<jobject> Java_ArrayList_ConstructorJUALI_JUC(JNIEnv* env,
    const base::android::JavaRef<jobject>& p0) {
  jclass clazz = java_util_ArrayList_clazz(env);
  CHECK_CLAZZ(env, clazz,
      java_util_ArrayList_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/util/Collection;)V",
          &g_java_util_ArrayList_ConstructorJUALI_JUC);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, p0.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace JNI_ArrayList

#endif  // java_util_ArrayList_JNI
