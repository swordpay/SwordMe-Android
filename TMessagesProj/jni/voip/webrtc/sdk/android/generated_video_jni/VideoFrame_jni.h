// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoFrame

#ifndef org_webrtc_VideoFrame_JNI
#define org_webrtc_VideoFrame_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoFrame[];
const char kClassPath_org_webrtc_VideoFrame[] = "org/webrtc/VideoFrame";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoFrame_00024Buffer[];
const char kClassPath_org_webrtc_VideoFrame_00024Buffer[] = "org/webrtc/VideoFrame$Buffer";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoFrame_00024I420Buffer[];
const char kClassPath_org_webrtc_VideoFrame_00024I420Buffer[] = "org/webrtc/VideoFrame$I420Buffer";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoFrame_clazz(nullptr);
#ifndef org_webrtc_VideoFrame_clazz_defined
#define org_webrtc_VideoFrame_clazz_defined
inline jclass org_webrtc_VideoFrame_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoFrame,
      &g_org_webrtc_VideoFrame_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoFrame_00024Buffer_clazz(nullptr);
#ifndef org_webrtc_VideoFrame_00024Buffer_clazz_defined
#define org_webrtc_VideoFrame_00024Buffer_clazz_defined
inline jclass org_webrtc_VideoFrame_00024Buffer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoFrame_00024Buffer,
      &g_org_webrtc_VideoFrame_00024Buffer_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoFrame_00024I420Buffer_clazz(nullptr);
#ifndef org_webrtc_VideoFrame_00024I420Buffer_clazz_defined
#define org_webrtc_VideoFrame_00024I420Buffer_clazz_defined
inline jclass org_webrtc_VideoFrame_00024I420Buffer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoFrame_00024I420Buffer,
      &g_org_webrtc_VideoFrame_00024I420Buffer_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_getBufferType(nullptr);
static jint Java_Buffer_getBufferType(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getBufferType",
          "()I",
          &g_org_webrtc_VideoFrame_00024Buffer_getBufferType);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_getWidth(nullptr);
static jint Java_Buffer_getWidth(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getWidth",
          "()I",
          &g_org_webrtc_VideoFrame_00024Buffer_getWidth);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_getHeight(nullptr);
static jint Java_Buffer_getHeight(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getHeight",
          "()I",
          &g_org_webrtc_VideoFrame_00024Buffer_getHeight);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_toI420(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Buffer_toI420(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "toI420",
          "()Lorg/webrtc/VideoFrame$I420Buffer;",
          &g_org_webrtc_VideoFrame_00024Buffer_toI420);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_retain(nullptr);
static void Java_Buffer_retain(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "retain",
          "()V",
          &g_org_webrtc_VideoFrame_00024Buffer_retain);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_release(nullptr);
static void Java_Buffer_release(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "release",
          "()V",
          &g_org_webrtc_VideoFrame_00024Buffer_release);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024Buffer_cropAndScale(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Buffer_cropAndScale(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, JniIntWrapper cropX,
    JniIntWrapper cropY,
    JniIntWrapper cropWidth,
    JniIntWrapper cropHeight,
    JniIntWrapper scaleWidth,
    JniIntWrapper scaleHeight) {
  jclass clazz = org_webrtc_VideoFrame_00024Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "cropAndScale",
          "(IIIIII)Lorg/webrtc/VideoFrame$Buffer;",
          &g_org_webrtc_VideoFrame_00024Buffer_cropAndScale);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, as_jint(cropX), as_jint(cropY), as_jint(cropWidth),
              as_jint(cropHeight), as_jint(scaleWidth), as_jint(scaleHeight));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024I420Buffer_getDataY(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_I420Buffer_getDataY(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024I420Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024I420Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDataY",
          "()Ljava/nio/ByteBuffer;",
          &g_org_webrtc_VideoFrame_00024I420Buffer_getDataY);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024I420Buffer_getDataU(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_I420Buffer_getDataU(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024I420Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024I420Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDataU",
          "()Ljava/nio/ByteBuffer;",
          &g_org_webrtc_VideoFrame_00024I420Buffer_getDataU);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024I420Buffer_getDataV(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_I420Buffer_getDataV(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024I420Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024I420Buffer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDataV",
          "()Ljava/nio/ByteBuffer;",
          &g_org_webrtc_VideoFrame_00024I420Buffer_getDataV);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024I420Buffer_getStrideY(nullptr);
static jint Java_I420Buffer_getStrideY(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024I420Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024I420Buffer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getStrideY",
          "()I",
          &g_org_webrtc_VideoFrame_00024I420Buffer_getStrideY);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024I420Buffer_getStrideU(nullptr);
static jint Java_I420Buffer_getStrideU(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024I420Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024I420Buffer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getStrideU",
          "()I",
          &g_org_webrtc_VideoFrame_00024I420Buffer_getStrideU);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_00024I420Buffer_getStrideV(nullptr);
static jint Java_I420Buffer_getStrideV(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_00024I420Buffer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_00024I420Buffer_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getStrideV",
          "()I",
          &g_org_webrtc_VideoFrame_00024I420Buffer_getStrideV);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoFrame_Constructor(JNIEnv* env, const
    base::android::JavaRef<jobject>& buffer,
    JniIntWrapper rotation,
    jlong timestampNs) {
  jclass clazz = org_webrtc_VideoFrame_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoFrame_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Lorg/webrtc/VideoFrame$Buffer;IJ)V",
          &g_org_webrtc_VideoFrame_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, buffer.obj(), as_jint(rotation), timestampNs);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_getBuffer(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoFrame_getBuffer(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getBuffer",
          "()Lorg/webrtc/VideoFrame$Buffer;",
          &g_org_webrtc_VideoFrame_getBuffer);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_getRotation(nullptr);
static jint Java_VideoFrame_getRotation(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRotation",
          "()I",
          &g_org_webrtc_VideoFrame_getRotation);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_getTimestampNs(nullptr);
static jlong Java_VideoFrame_getTimestampNs(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = org_webrtc_VideoFrame_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTimestampNs",
          "()J",
          &g_org_webrtc_VideoFrame_getTimestampNs);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoFrame_release(nullptr);
static void Java_VideoFrame_release(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoFrame_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoFrame_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "release",
          "()V",
          &g_org_webrtc_VideoFrame_release);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoFrame_JNI
