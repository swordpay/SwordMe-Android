// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/MediaStream

#ifndef org_webrtc_MediaStream_JNI
#define org_webrtc_MediaStream_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_MediaStream[];
const char kClassPath_org_webrtc_MediaStream[] = "org/webrtc/MediaStream";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_MediaStream_clazz(nullptr);
#ifndef org_webrtc_MediaStream_clazz_defined
#define org_webrtc_MediaStream_clazz_defined
inline jclass org_webrtc_MediaStream_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_MediaStream,
      &g_org_webrtc_MediaStream_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jboolean JNI_MediaStream_AddAudioTrackToNativeStream(JNIEnv* env, jlong stream,
    jlong nativeAudioTrack);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_MediaStream_nativeAddAudioTrackToNativeStream(
    JNIEnv* env,
    jclass jcaller,
    jlong stream,
    jlong nativeAudioTrack) {
  return JNI_MediaStream_AddAudioTrackToNativeStream(env, stream, nativeAudioTrack);
}

static jboolean JNI_MediaStream_AddVideoTrackToNativeStream(JNIEnv* env, jlong stream,
    jlong nativeVideoTrack);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_MediaStream_nativeAddVideoTrackToNativeStream(
    JNIEnv* env,
    jclass jcaller,
    jlong stream,
    jlong nativeVideoTrack) {
  return JNI_MediaStream_AddVideoTrackToNativeStream(env, stream, nativeVideoTrack);
}

static jboolean JNI_MediaStream_RemoveAudioTrack(JNIEnv* env, jlong stream,
    jlong nativeAudioTrack);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_MediaStream_nativeRemoveAudioTrack(
    JNIEnv* env,
    jclass jcaller,
    jlong stream,
    jlong nativeAudioTrack) {
  return JNI_MediaStream_RemoveAudioTrack(env, stream, nativeAudioTrack);
}

static jboolean JNI_MediaStream_RemoveVideoTrack(JNIEnv* env, jlong stream,
    jlong nativeVideoTrack);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_MediaStream_nativeRemoveVideoTrack(
    JNIEnv* env,
    jclass jcaller,
    jlong stream,
    jlong nativeVideoTrack) {
  return JNI_MediaStream_RemoveVideoTrack(env, stream, nativeVideoTrack);
}

static base::android::ScopedJavaLocalRef<jstring> JNI_MediaStream_GetId(JNIEnv* env, jlong stream);

JNI_GENERATOR_EXPORT jstring Java_org_webrtc_MediaStream_nativeGetId(
    JNIEnv* env,
    jclass jcaller,
    jlong stream) {
  return JNI_MediaStream_GetId(env, stream).Release();
}


static std::atomic<jmethodID> g_org_webrtc_MediaStream_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_MediaStream_Constructor(JNIEnv* env, jlong
    nativeStream) {
  jclass clazz = org_webrtc_MediaStream_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_MediaStream_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(J)V",
          &g_org_webrtc_MediaStream_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, nativeStream);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_MediaStream_dispose(nullptr);
static void Java_MediaStream_dispose(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_MediaStream_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_MediaStream_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "dispose",
          "()V",
          &g_org_webrtc_MediaStream_dispose);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_MediaStream_addNativeAudioTrack(nullptr);
static void Java_MediaStream_addNativeAudioTrack(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, jlong nativeTrack) {
  jclass clazz = org_webrtc_MediaStream_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_MediaStream_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "addNativeAudioTrack",
          "(J)V",
          &g_org_webrtc_MediaStream_addNativeAudioTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, nativeTrack);
}

static std::atomic<jmethodID> g_org_webrtc_MediaStream_addNativeVideoTrack(nullptr);
static void Java_MediaStream_addNativeVideoTrack(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, jlong nativeTrack) {
  jclass clazz = org_webrtc_MediaStream_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_MediaStream_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "addNativeVideoTrack",
          "(J)V",
          &g_org_webrtc_MediaStream_addNativeVideoTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, nativeTrack);
}

static std::atomic<jmethodID> g_org_webrtc_MediaStream_removeAudioTrack(nullptr);
static void Java_MediaStream_removeAudioTrack(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, jlong nativeTrack) {
  jclass clazz = org_webrtc_MediaStream_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_MediaStream_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "removeAudioTrack",
          "(J)V",
          &g_org_webrtc_MediaStream_removeAudioTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, nativeTrack);
}

static std::atomic<jmethodID> g_org_webrtc_MediaStream_removeVideoTrack(nullptr);
static void Java_MediaStream_removeVideoTrack(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, jlong nativeTrack) {
  jclass clazz = org_webrtc_MediaStream_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_MediaStream_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "removeVideoTrack",
          "(J)V",
          &g_org_webrtc_MediaStream_removeVideoTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, nativeTrack);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_MediaStream_JNI
