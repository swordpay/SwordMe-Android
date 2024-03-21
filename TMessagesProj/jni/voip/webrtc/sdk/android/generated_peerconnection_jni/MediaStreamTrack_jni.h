// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/MediaStreamTrack

#ifndef org_webrtc_MediaStreamTrack_JNI
#define org_webrtc_MediaStreamTrack_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_MediaStreamTrack[];
const char kClassPath_org_webrtc_MediaStreamTrack[] = "org/webrtc/MediaStreamTrack";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_MediaStreamTrack_00024State[];
const char kClassPath_org_webrtc_MediaStreamTrack_00024State[] =
    "org/webrtc/MediaStreamTrack$State";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_MediaStreamTrack_00024MediaType[];
const char kClassPath_org_webrtc_MediaStreamTrack_00024MediaType[] =
    "org/webrtc/MediaStreamTrack$MediaType";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_MediaStreamTrack_clazz(nullptr);
#ifndef org_webrtc_MediaStreamTrack_clazz_defined
#define org_webrtc_MediaStreamTrack_clazz_defined
inline jclass org_webrtc_MediaStreamTrack_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_MediaStreamTrack,
      &g_org_webrtc_MediaStreamTrack_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_MediaStreamTrack_00024State_clazz(nullptr);
#ifndef org_webrtc_MediaStreamTrack_00024State_clazz_defined
#define org_webrtc_MediaStreamTrack_00024State_clazz_defined
inline jclass org_webrtc_MediaStreamTrack_00024State_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_MediaStreamTrack_00024State,
      &g_org_webrtc_MediaStreamTrack_00024State_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_MediaStreamTrack_00024MediaType_clazz(nullptr);
#ifndef org_webrtc_MediaStreamTrack_00024MediaType_clazz_defined
#define org_webrtc_MediaStreamTrack_00024MediaType_clazz_defined
inline jclass org_webrtc_MediaStreamTrack_00024MediaType_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_MediaStreamTrack_00024MediaType,
      &g_org_webrtc_MediaStreamTrack_00024MediaType_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static base::android::ScopedJavaLocalRef<jstring> JNI_MediaStreamTrack_GetId(JNIEnv* env, jlong
    track);

JNI_GENERATOR_EXPORT jstring Java_org_webrtc_MediaStreamTrack_nativeGetId(
    JNIEnv* env,
    jclass jcaller,
    jlong track) {
  return JNI_MediaStreamTrack_GetId(env, track).Release();
}

static base::android::ScopedJavaLocalRef<jstring> JNI_MediaStreamTrack_GetKind(JNIEnv* env, jlong
    track);

JNI_GENERATOR_EXPORT jstring Java_org_webrtc_MediaStreamTrack_nativeGetKind(
    JNIEnv* env,
    jclass jcaller,
    jlong track) {
  return JNI_MediaStreamTrack_GetKind(env, track).Release();
}

static jboolean JNI_MediaStreamTrack_GetEnabled(JNIEnv* env, jlong track);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_MediaStreamTrack_nativeGetEnabled(
    JNIEnv* env,
    jclass jcaller,
    jlong track) {
  return JNI_MediaStreamTrack_GetEnabled(env, track);
}

static jboolean JNI_MediaStreamTrack_SetEnabled(JNIEnv* env, jlong track,
    jboolean enabled);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_MediaStreamTrack_nativeSetEnabled(
    JNIEnv* env,
    jclass jcaller,
    jlong track,
    jboolean enabled) {
  return JNI_MediaStreamTrack_SetEnabled(env, track, enabled);
}

static base::android::ScopedJavaLocalRef<jobject> JNI_MediaStreamTrack_GetState(JNIEnv* env, jlong
    track);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_MediaStreamTrack_nativeGetState(
    JNIEnv* env,
    jclass jcaller,
    jlong track) {
  return JNI_MediaStreamTrack_GetState(env, track).Release();
}


static std::atomic<jmethodID> g_org_webrtc_MediaStreamTrack_00024State_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_State_fromNativeIndex(JNIEnv* env,
    JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_MediaStreamTrack_00024State_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_MediaStreamTrack_00024State_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/MediaStreamTrack$State;",
          &g_org_webrtc_MediaStreamTrack_00024State_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_MediaStreamTrack_00024MediaType_getNative(nullptr);
static jint Java_MediaType_getNative(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_MediaStreamTrack_00024MediaType_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_MediaStreamTrack_00024MediaType_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNative",
          "()I",
          &g_org_webrtc_MediaStreamTrack_00024MediaType_getNative);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_MediaStreamTrack_00024MediaType_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_MediaType_fromNativeIndex(JNIEnv* env,
    JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_MediaStreamTrack_00024MediaType_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_MediaStreamTrack_00024MediaType_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/MediaStreamTrack$MediaType;",
          &g_org_webrtc_MediaStreamTrack_00024MediaType_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_MediaStreamTrack_JNI
