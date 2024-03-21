// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/DataChannel

#ifndef org_webrtc_DataChannel_JNI
#define org_webrtc_DataChannel_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_DataChannel[];
const char kClassPath_org_webrtc_DataChannel[] = "org/webrtc/DataChannel";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_DataChannel_00024Init[];
const char kClassPath_org_webrtc_DataChannel_00024Init[] = "org/webrtc/DataChannel$Init";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_DataChannel_00024Buffer[];
const char kClassPath_org_webrtc_DataChannel_00024Buffer[] = "org/webrtc/DataChannel$Buffer";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_DataChannel_00024Observer[];
const char kClassPath_org_webrtc_DataChannel_00024Observer[] = "org/webrtc/DataChannel$Observer";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_DataChannel_00024State[];
const char kClassPath_org_webrtc_DataChannel_00024State[] = "org/webrtc/DataChannel$State";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_DataChannel_clazz(nullptr);
#ifndef org_webrtc_DataChannel_clazz_defined
#define org_webrtc_DataChannel_clazz_defined
inline jclass org_webrtc_DataChannel_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_DataChannel,
      &g_org_webrtc_DataChannel_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_DataChannel_00024Init_clazz(nullptr);
#ifndef org_webrtc_DataChannel_00024Init_clazz_defined
#define org_webrtc_DataChannel_00024Init_clazz_defined
inline jclass org_webrtc_DataChannel_00024Init_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_DataChannel_00024Init,
      &g_org_webrtc_DataChannel_00024Init_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_DataChannel_00024Buffer_clazz(nullptr);
#ifndef org_webrtc_DataChannel_00024Buffer_clazz_defined
#define org_webrtc_DataChannel_00024Buffer_clazz_defined
inline jclass org_webrtc_DataChannel_00024Buffer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_DataChannel_00024Buffer,
      &g_org_webrtc_DataChannel_00024Buffer_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_DataChannel_00024Observer_clazz(nullptr);
#ifndef org_webrtc_DataChannel_00024Observer_clazz_defined
#define org_webrtc_DataChannel_00024Observer_clazz_defined
inline jclass org_webrtc_DataChannel_00024Observer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_DataChannel_00024Observer,
      &g_org_webrtc_DataChannel_00024Observer_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_DataChannel_00024State_clazz(nullptr);
#ifndef org_webrtc_DataChannel_00024State_clazz_defined
#define org_webrtc_DataChannel_00024State_clazz_defined
inline jclass org_webrtc_DataChannel_00024State_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_DataChannel_00024State,
      &g_org_webrtc_DataChannel_00024State_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jlong JNI_DataChannel_RegisterObserver(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& observer);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_DataChannel_nativeRegisterObserver(
    JNIEnv* env,
    jobject jcaller,
    jobject observer) {
  return JNI_DataChannel_RegisterObserver(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, observer));
}

static void JNI_DataChannel_UnregisterObserver(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jlong observer);

JNI_GENERATOR_EXPORT void Java_org_webrtc_DataChannel_nativeUnregisterObserver(
    JNIEnv* env,
    jobject jcaller,
    jlong observer) {
  return JNI_DataChannel_UnregisterObserver(env, base::android::JavaParamRef<jobject>(env, jcaller),
      observer);
}

static base::android::ScopedJavaLocalRef<jstring> JNI_DataChannel_Label(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jstring Java_org_webrtc_DataChannel_nativeLabel(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_DataChannel_Label(env, base::android::JavaParamRef<jobject>(env, jcaller)).Release();
}

static jint JNI_DataChannel_Id(JNIEnv* env, const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jint Java_org_webrtc_DataChannel_nativeId(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_DataChannel_Id(env, base::android::JavaParamRef<jobject>(env, jcaller));
}

static base::android::ScopedJavaLocalRef<jobject> JNI_DataChannel_State(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_DataChannel_nativeState(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_DataChannel_State(env, base::android::JavaParamRef<jobject>(env, jcaller)).Release();
}

static jlong JNI_DataChannel_BufferedAmount(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_DataChannel_nativeBufferedAmount(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_DataChannel_BufferedAmount(env, base::android::JavaParamRef<jobject>(env, jcaller));
}

static void JNI_DataChannel_Close(JNIEnv* env, const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT void Java_org_webrtc_DataChannel_nativeClose(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_DataChannel_Close(env, base::android::JavaParamRef<jobject>(env, jcaller));
}

static jboolean JNI_DataChannel_Send(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller,
    const base::android::JavaParamRef<jbyteArray>& data,
    jboolean binary);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_DataChannel_nativeSend(
    JNIEnv* env,
    jobject jcaller,
    jbyteArray data,
    jboolean binary) {
  return JNI_DataChannel_Send(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jbyteArray>(env, data), binary);
}


static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Init_getOrdered(nullptr);
static jboolean Java_Init_getOrdered(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_00024Init_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Init_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getOrdered",
          "()Z",
          &g_org_webrtc_DataChannel_00024Init_getOrdered);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Init_getMaxRetransmitTimeMs(nullptr);
static jint Java_Init_getMaxRetransmitTimeMs(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_DataChannel_00024Init_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Init_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMaxRetransmitTimeMs",
          "()I",
          &g_org_webrtc_DataChannel_00024Init_getMaxRetransmitTimeMs);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Init_getMaxRetransmits(nullptr);
static jint Java_Init_getMaxRetransmits(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_00024Init_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Init_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMaxRetransmits",
          "()I",
          &g_org_webrtc_DataChannel_00024Init_getMaxRetransmits);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Init_getProtocol(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_Init_getProtocol(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_00024Init_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Init_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getProtocol",
          "()Ljava/lang/String;",
          &g_org_webrtc_DataChannel_00024Init_getProtocol);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Init_getNegotiated(nullptr);
static jboolean Java_Init_getNegotiated(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_00024Init_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Init_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNegotiated",
          "()Z",
          &g_org_webrtc_DataChannel_00024Init_getNegotiated);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Init_getId(nullptr);
static jint Java_Init_getId(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_00024Init_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Init_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getId",
          "()I",
          &g_org_webrtc_DataChannel_00024Init_getId);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Buffer_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Buffer_Constructor(JNIEnv* env, const
    base::android::JavaRef<jobject>& data,
    jboolean binary) {
  jclass clazz = org_webrtc_DataChannel_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_DataChannel_00024Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/nio/ByteBuffer;Z)V",
          &g_org_webrtc_DataChannel_00024Buffer_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, data.obj(), binary);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_DataChannel_00024Observer_onBufferedAmountChange(nullptr);
static void Java_Observer_onBufferedAmountChange(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, jlong previousAmount) {
  jclass clazz = org_webrtc_DataChannel_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onBufferedAmountChange",
          "(J)V",
          &g_org_webrtc_DataChannel_00024Observer_onBufferedAmountChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, previousAmount);
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Observer_onStateChange(nullptr);
static void Java_Observer_onStateChange(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onStateChange",
          "()V",
          &g_org_webrtc_DataChannel_00024Observer_onStateChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024Observer_onMessage(nullptr);
static void Java_Observer_onMessage(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& buffer) {
  jclass clazz = org_webrtc_DataChannel_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onMessage",
          "(Lorg/webrtc/DataChannel$Buffer;)V",
          &g_org_webrtc_DataChannel_00024Observer_onMessage);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, buffer.obj());
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_00024State_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_State_fromNativeIndex(JNIEnv* env,
    JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_DataChannel_00024State_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_DataChannel_00024State_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/DataChannel$State;",
          &g_org_webrtc_DataChannel_00024State_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_DataChannel_Constructor(JNIEnv* env, jlong
    nativeDataChannel) {
  jclass clazz = org_webrtc_DataChannel_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_DataChannel_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(J)V",
          &g_org_webrtc_DataChannel_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, nativeDataChannel);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_DataChannel_getNativeDataChannel(nullptr);
static jlong Java_DataChannel_getNativeDataChannel(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_DataChannel_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_DataChannel_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNativeDataChannel",
          "()J",
          &g_org_webrtc_DataChannel_getNativeDataChannel);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_DataChannel_JNI
