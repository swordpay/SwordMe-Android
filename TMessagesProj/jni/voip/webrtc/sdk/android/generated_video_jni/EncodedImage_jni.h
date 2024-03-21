// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/EncodedImage

#ifndef org_webrtc_EncodedImage_JNI
#define org_webrtc_EncodedImage_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_EncodedImage[];
const char kClassPath_org_webrtc_EncodedImage[] = "org/webrtc/EncodedImage";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_EncodedImage_00024FrameType[];
const char kClassPath_org_webrtc_EncodedImage_00024FrameType[] =
    "org/webrtc/EncodedImage$FrameType";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_EncodedImage_clazz(nullptr);
#ifndef org_webrtc_EncodedImage_clazz_defined
#define org_webrtc_EncodedImage_clazz_defined
inline jclass org_webrtc_EncodedImage_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_EncodedImage,
      &g_org_webrtc_EncodedImage_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_EncodedImage_00024FrameType_clazz(nullptr);
#ifndef org_webrtc_EncodedImage_00024FrameType_clazz_defined
#define org_webrtc_EncodedImage_00024FrameType_clazz_defined
inline jclass org_webrtc_EncodedImage_00024FrameType_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_EncodedImage_00024FrameType,
      &g_org_webrtc_EncodedImage_00024FrameType_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_EncodedImage_00024FrameType_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_FrameType_fromNativeIndex(JNIEnv* env,
    JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_EncodedImage_00024FrameType_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_EncodedImage_00024FrameType_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/EncodedImage$FrameType;",
          &g_org_webrtc_EncodedImage_00024FrameType_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_EncodedImage_Constructor(JNIEnv* env, const
    base::android::JavaRef<jobject>& buffer,
    const base::android::JavaRef<jobject>& releaseCallback,
    JniIntWrapper encodedWidth,
    JniIntWrapper encodedHeight,
    jlong captureTimeNs,
    const base::android::JavaRef<jobject>& frameType,
    JniIntWrapper rotation,
    const base::android::JavaRef<jobject>& qp) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_EncodedImage_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
"(Ljava/nio/ByteBuffer;Ljava/lang/Runnable;IIJLorg/webrtc/EncodedImage$FrameType;ILjava/lang/Integer;)V",
          &g_org_webrtc_EncodedImage_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, buffer.obj(), releaseCallback.obj(), as_jint(encodedWidth),
              as_jint(encodedHeight), captureTimeNs, frameType.obj(), as_jint(rotation), qp.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getBuffer(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_EncodedImage_getBuffer(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getBuffer",
          "()Ljava/nio/ByteBuffer;",
          &g_org_webrtc_EncodedImage_getBuffer);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getEncodedWidth(nullptr);
static jint Java_EncodedImage_getEncodedWidth(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEncodedWidth",
          "()I",
          &g_org_webrtc_EncodedImage_getEncodedWidth);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getEncodedHeight(nullptr);
static jint Java_EncodedImage_getEncodedHeight(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEncodedHeight",
          "()I",
          &g_org_webrtc_EncodedImage_getEncodedHeight);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getCaptureTimeNs(nullptr);
static jlong Java_EncodedImage_getCaptureTimeNs(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCaptureTimeNs",
          "()J",
          &g_org_webrtc_EncodedImage_getCaptureTimeNs);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getFrameType(nullptr);
static jint Java_EncodedImage_getFrameType(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getFrameType",
          "()I",
          &g_org_webrtc_EncodedImage_getFrameType);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getRotation(nullptr);
static jint Java_EncodedImage_getRotation(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRotation",
          "()I",
          &g_org_webrtc_EncodedImage_getRotation);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_EncodedImage_getQp(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_EncodedImage_getQp(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_EncodedImage_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_EncodedImage_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getQp",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_EncodedImage_getQp);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_EncodedImage_JNI
