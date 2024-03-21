// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoEncoder

#ifndef org_webrtc_VideoEncoder_JNI
#define org_webrtc_VideoEncoder_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoder[];
const char kClassPath_org_webrtc_VideoEncoder[] = "org/webrtc/VideoEncoder";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoder_00024Settings[];
const char kClassPath_org_webrtc_VideoEncoder_00024Settings[] = "org/webrtc/VideoEncoder$Settings";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoder_00024Capabilities[];
const char kClassPath_org_webrtc_VideoEncoder_00024Capabilities[] =
    "org/webrtc/VideoEncoder$Capabilities";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoder_00024EncodeInfo[];
const char kClassPath_org_webrtc_VideoEncoder_00024EncodeInfo[] =
    "org/webrtc/VideoEncoder$EncodeInfo";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_VideoEncoder_00024BitrateAllocation[];
const char kClassPath_org_webrtc_VideoEncoder_00024BitrateAllocation[] =
    "org/webrtc/VideoEncoder$BitrateAllocation";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits[];
const char kClassPath_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits[] =
    "org/webrtc/VideoEncoder$ResolutionBitrateLimits";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_VideoEncoder_00024RateControlParameters[];
const char kClassPath_org_webrtc_VideoEncoder_00024RateControlParameters[] =
    "org/webrtc/VideoEncoder$RateControlParameters";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoEncoder_00024EncoderInfo[];
const char kClassPath_org_webrtc_VideoEncoder_00024EncoderInfo[] =
    "org/webrtc/VideoEncoder$EncoderInfo";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoEncoder_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_clazz_defined
#define org_webrtc_VideoEncoder_clazz_defined
inline jclass org_webrtc_VideoEncoder_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoder,
      &g_org_webrtc_VideoEncoder_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoEncoder_00024Settings_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024Settings_clazz_defined
#define org_webrtc_VideoEncoder_00024Settings_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024Settings_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoder_00024Settings,
      &g_org_webrtc_VideoEncoder_00024Settings_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoder_00024Capabilities_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024Capabilities_clazz_defined
#define org_webrtc_VideoEncoder_00024Capabilities_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024Capabilities_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoder_00024Capabilities,
      &g_org_webrtc_VideoEncoder_00024Capabilities_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoder_00024EncodeInfo_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024EncodeInfo_clazz_defined
#define org_webrtc_VideoEncoder_00024EncodeInfo_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024EncodeInfo_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoder_00024EncodeInfo,
      &g_org_webrtc_VideoEncoder_00024EncodeInfo_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoder_00024BitrateAllocation_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024BitrateAllocation_clazz_defined
#define org_webrtc_VideoEncoder_00024BitrateAllocation_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024BitrateAllocation_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoder_00024BitrateAllocation,
      &g_org_webrtc_VideoEncoder_00024BitrateAllocation_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz_defined
#define org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits,
      &g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoder_00024RateControlParameters_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024RateControlParameters_clazz_defined
#define org_webrtc_VideoEncoder_00024RateControlParameters_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024RateControlParameters_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_VideoEncoder_00024RateControlParameters,
      &g_org_webrtc_VideoEncoder_00024RateControlParameters_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_VideoEncoder_00024EncoderInfo_clazz(nullptr);
#ifndef org_webrtc_VideoEncoder_00024EncoderInfo_clazz_defined
#define org_webrtc_VideoEncoder_00024EncoderInfo_clazz_defined
inline jclass org_webrtc_VideoEncoder_00024EncoderInfo_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoEncoder_00024EncoderInfo,
      &g_org_webrtc_VideoEncoder_00024EncoderInfo_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_00024Settings_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Settings_Constructor(JNIEnv* env,
    JniIntWrapper numberOfCores,
    JniIntWrapper width,
    JniIntWrapper height,
    JniIntWrapper startBitrate,
    JniIntWrapper maxFramerate,
    JniIntWrapper numberOfSimulcastStreams,
    jboolean automaticResizeOn,
    const base::android::JavaRef<jobject>& capabilities) {
  jclass clazz = org_webrtc_VideoEncoder_00024Settings_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoder_00024Settings_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(IIIIIIZLorg/webrtc/VideoEncoder$Capabilities;)V",
          &g_org_webrtc_VideoEncoder_00024Settings_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(numberOfCores), as_jint(width), as_jint(height),
              as_jint(startBitrate), as_jint(maxFramerate), as_jint(numberOfSimulcastStreams),
              automaticResizeOn, capabilities.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_00024Capabilities_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Capabilities_Constructor(JNIEnv* env,
    jboolean lossNotification) {
  jclass clazz = org_webrtc_VideoEncoder_00024Capabilities_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoder_00024Capabilities_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Z)V",
          &g_org_webrtc_VideoEncoder_00024Capabilities_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, lossNotification);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_00024EncodeInfo_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_EncodeInfo_Constructor(JNIEnv* env, const
    base::android::JavaRef<jobjectArray>& frameTypes) {
  jclass clazz = org_webrtc_VideoEncoder_00024EncodeInfo_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoder_00024EncodeInfo_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "([Lorg/webrtc/EncodedImage$FrameType;)V",
          &g_org_webrtc_VideoEncoder_00024EncodeInfo_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, frameTypes.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_00024BitrateAllocation_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_BitrateAllocation_Constructor(JNIEnv* env,
    const base::android::JavaRef<jobjectArray>& bitratesBbs) {
  jclass clazz = org_webrtc_VideoEncoder_00024BitrateAllocation_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoder_00024BitrateAllocation_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "([[I)V",
          &g_org_webrtc_VideoEncoder_00024BitrateAllocation_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, bitratesBbs.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getFrameSizePixels(nullptr);
static jint Java_ResolutionBitrateLimits_getFrameSizePixels(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getFrameSizePixels",
          "()I",
          &g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getFrameSizePixels);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getMinStartBitrateBps(nullptr);
static jint Java_ResolutionBitrateLimits_getMinStartBitrateBps(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMinStartBitrateBps",
          "()I",
          &g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getMinStartBitrateBps);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getMinBitrateBps(nullptr);
static jint Java_ResolutionBitrateLimits_getMinBitrateBps(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMinBitrateBps",
          "()I",
          &g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getMinBitrateBps);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getMaxBitrateBps(nullptr);
static jint Java_ResolutionBitrateLimits_getMaxBitrateBps(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMaxBitrateBps",
          "()I",
          &g_org_webrtc_VideoEncoder_00024ResolutionBitrateLimits_getMaxBitrateBps);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024RateControlParameters_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RateControlParameters_Constructor(JNIEnv*
    env, const base::android::JavaRef<jobject>& bitrate,
    jdouble framerateFps) {
  jclass clazz = org_webrtc_VideoEncoder_00024RateControlParameters_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoEncoder_00024RateControlParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Lorg/webrtc/VideoEncoder$BitrateAllocation;D)V",
          &g_org_webrtc_VideoEncoder_00024RateControlParameters_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, bitrate.obj(), framerateFps);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024EncoderInfo_getRequestedResolutionAlignment(nullptr);
static jint Java_EncoderInfo_getRequestedResolutionAlignment(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_00024EncoderInfo_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_00024EncoderInfo_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRequestedResolutionAlignment",
          "()I",
          &g_org_webrtc_VideoEncoder_00024EncoderInfo_getRequestedResolutionAlignment);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_VideoEncoder_00024EncoderInfo_getApplyAlignmentToAllSimulcastLayers(nullptr);
static jboolean Java_EncoderInfo_getApplyAlignmentToAllSimulcastLayers(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_00024EncoderInfo_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_00024EncoderInfo_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getApplyAlignmentToAllSimulcastLayers",
          "()Z",
          &g_org_webrtc_VideoEncoder_00024EncoderInfo_getApplyAlignmentToAllSimulcastLayers);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_createNativeVideoEncoder(nullptr);
static jlong Java_VideoEncoder_createNativeVideoEncoder(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "createNativeVideoEncoder",
          "()J",
          &g_org_webrtc_VideoEncoder_createNativeVideoEncoder);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_isHardwareEncoder(nullptr);
static jboolean Java_VideoEncoder_isHardwareEncoder(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "isHardwareEncoder",
          "()Z",
          &g_org_webrtc_VideoEncoder_isHardwareEncoder);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_initEncode(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoder_initEncode(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& settings,
    const base::android::JavaRef<jobject>& encodeCallback) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "initEncode",
"(Lorg/webrtc/VideoEncoder$Settings;Lorg/webrtc/VideoEncoder$Callback;)Lorg/webrtc/VideoCodecStatus;",
          &g_org_webrtc_VideoEncoder_initEncode);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, settings.obj(), encodeCallback.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_release(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoder_release(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "release",
          "()Lorg/webrtc/VideoCodecStatus;",
          &g_org_webrtc_VideoEncoder_release);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_encode(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoder_encode(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& frame,
    const base::android::JavaRef<jobject>& info) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "encode",
"(Lorg/webrtc/VideoFrame;Lorg/webrtc/VideoEncoder$EncodeInfo;)Lorg/webrtc/VideoCodecStatus;",
          &g_org_webrtc_VideoEncoder_encode);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, frame.obj(), info.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_setRates(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoder_setRates(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& rcParameters) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "setRates",
          "(Lorg/webrtc/VideoEncoder$RateControlParameters;)Lorg/webrtc/VideoCodecStatus;",
          &g_org_webrtc_VideoEncoder_setRates);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id, rcParameters.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_getScalingSettings(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoder_getScalingSettings(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getScalingSettings",
          "()Lorg/webrtc/VideoEncoder$ScalingSettings;",
          &g_org_webrtc_VideoEncoder_getScalingSettings);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_getResolutionBitrateLimits(nullptr);
static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_VideoEncoder_getResolutionBitrateLimits(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getResolutionBitrateLimits",
          "()[Lorg/webrtc/VideoEncoder$ResolutionBitrateLimits;",
          &g_org_webrtc_VideoEncoder_getResolutionBitrateLimits);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_getImplementationName(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_VideoEncoder_getImplementationName(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getImplementationName",
          "()Ljava/lang/String;",
          &g_org_webrtc_VideoEncoder_getImplementationName);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoEncoder_getEncoderInfo(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoEncoder_getEncoderInfo(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoEncoder_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoEncoder_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEncoderInfo",
          "()Lorg/webrtc/VideoEncoder$EncoderInfo;",
          &g_org_webrtc_VideoEncoder_getEncoderInfo);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoEncoder_JNI
