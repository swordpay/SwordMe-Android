// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/RtpParameters

#ifndef org_webrtc_RtpParameters_JNI
#define org_webrtc_RtpParameters_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RtpParameters[];
const char kClassPath_org_webrtc_RtpParameters[] = "org/webrtc/RtpParameters";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_RtpParameters_00024DegradationPreference[];
const char kClassPath_org_webrtc_RtpParameters_00024DegradationPreference[] =
    "org/webrtc/RtpParameters$DegradationPreference";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RtpParameters_00024Encoding[];
const char kClassPath_org_webrtc_RtpParameters_00024Encoding[] =
    "org/webrtc/RtpParameters$Encoding";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RtpParameters_00024Codec[];
const char kClassPath_org_webrtc_RtpParameters_00024Codec[] = "org/webrtc/RtpParameters$Codec";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RtpParameters_00024Rtcp[];
const char kClassPath_org_webrtc_RtpParameters_00024Rtcp[] = "org/webrtc/RtpParameters$Rtcp";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_RtpParameters_00024HeaderExtension[];
const char kClassPath_org_webrtc_RtpParameters_00024HeaderExtension[] =
    "org/webrtc/RtpParameters$HeaderExtension";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RtpParameters_clazz(nullptr);
#ifndef org_webrtc_RtpParameters_clazz_defined
#define org_webrtc_RtpParameters_clazz_defined
inline jclass org_webrtc_RtpParameters_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RtpParameters,
      &g_org_webrtc_RtpParameters_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_RtpParameters_00024DegradationPreference_clazz(nullptr);
#ifndef org_webrtc_RtpParameters_00024DegradationPreference_clazz_defined
#define org_webrtc_RtpParameters_00024DegradationPreference_clazz_defined
inline jclass org_webrtc_RtpParameters_00024DegradationPreference_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_RtpParameters_00024DegradationPreference,
      &g_org_webrtc_RtpParameters_00024DegradationPreference_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RtpParameters_00024Encoding_clazz(nullptr);
#ifndef org_webrtc_RtpParameters_00024Encoding_clazz_defined
#define org_webrtc_RtpParameters_00024Encoding_clazz_defined
inline jclass org_webrtc_RtpParameters_00024Encoding_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RtpParameters_00024Encoding,
      &g_org_webrtc_RtpParameters_00024Encoding_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RtpParameters_00024Codec_clazz(nullptr);
#ifndef org_webrtc_RtpParameters_00024Codec_clazz_defined
#define org_webrtc_RtpParameters_00024Codec_clazz_defined
inline jclass org_webrtc_RtpParameters_00024Codec_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RtpParameters_00024Codec,
      &g_org_webrtc_RtpParameters_00024Codec_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RtpParameters_00024Rtcp_clazz(nullptr);
#ifndef org_webrtc_RtpParameters_00024Rtcp_clazz_defined
#define org_webrtc_RtpParameters_00024Rtcp_clazz_defined
inline jclass org_webrtc_RtpParameters_00024Rtcp_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RtpParameters_00024Rtcp,
      &g_org_webrtc_RtpParameters_00024Rtcp_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_RtpParameters_00024HeaderExtension_clazz(nullptr);
#ifndef org_webrtc_RtpParameters_00024HeaderExtension_clazz_defined
#define org_webrtc_RtpParameters_00024HeaderExtension_clazz_defined
inline jclass org_webrtc_RtpParameters_00024HeaderExtension_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RtpParameters_00024HeaderExtension,
      &g_org_webrtc_RtpParameters_00024HeaderExtension_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID>
    g_org_webrtc_RtpParameters_00024DegradationPreference_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_DegradationPreference_fromNativeIndex(JNIEnv*
    env, JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_RtpParameters_00024DegradationPreference_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtpParameters_00024DegradationPreference_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/RtpParameters$DegradationPreference;",
          &g_org_webrtc_RtpParameters_00024DegradationPreference_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_Constructor(JNIEnv* env, const
    base::android::JavaRef<jstring>& rid,
    jboolean active,
    jdouble bitratePriority,
    JniIntWrapper networkPriority,
    const base::android::JavaRef<jobject>& maxBitrateBps,
    const base::android::JavaRef<jobject>& minBitrateBps,
    const base::android::JavaRef<jobject>& maxFramerate,
    const base::android::JavaRef<jobject>& numTemporalLayers,
    const base::android::JavaRef<jobject>& scaleResolutionDownBy,
    const base::android::JavaRef<jobject>& ssrc,
    jboolean adaptiveAudioPacketTime) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
"(Ljava/lang/String;ZDILjava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Double;Ljava/lang/Long;Z)V",
          &g_org_webrtc_RtpParameters_00024Encoding_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, rid.obj(), active, bitratePriority, as_jint(networkPriority),
              maxBitrateBps.obj(), minBitrateBps.obj(), maxFramerate.obj(), numTemporalLayers.obj(),
              scaleResolutionDownBy.obj(), ssrc.obj(), adaptiveAudioPacketTime);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getRid(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_Encoding_getRid(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRid",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtpParameters_00024Encoding_getRid);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getActive(nullptr);
static jboolean Java_Encoding_getActive(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getActive",
          "()Z",
          &g_org_webrtc_RtpParameters_00024Encoding_getActive);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getBitratePriority(nullptr);
static jdouble Java_Encoding_getBitratePriority(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getBitratePriority",
          "()D",
          &g_org_webrtc_RtpParameters_00024Encoding_getBitratePriority);

  jdouble ret =
      env->CallDoubleMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getNetworkPriority(nullptr);
static jint Java_Encoding_getNetworkPriority(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNetworkPriority",
          "()I",
          &g_org_webrtc_RtpParameters_00024Encoding_getNetworkPriority);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getMaxBitrateBps(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_getMaxBitrateBps(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMaxBitrateBps",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_RtpParameters_00024Encoding_getMaxBitrateBps);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getMinBitrateBps(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_getMinBitrateBps(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMinBitrateBps",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_RtpParameters_00024Encoding_getMinBitrateBps);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getMaxFramerate(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_getMaxFramerate(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMaxFramerate",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_RtpParameters_00024Encoding_getMaxFramerate);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_RtpParameters_00024Encoding_getNumTemporalLayers(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_getNumTemporalLayers(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNumTemporalLayers",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_RtpParameters_00024Encoding_getNumTemporalLayers);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_RtpParameters_00024Encoding_getScaleResolutionDownBy(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_getScaleResolutionDownBy(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getScaleResolutionDownBy",
          "()Ljava/lang/Double;",
          &g_org_webrtc_RtpParameters_00024Encoding_getScaleResolutionDownBy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getSsrc(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Encoding_getSsrc(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSsrc",
          "()Ljava/lang/Long;",
          &g_org_webrtc_RtpParameters_00024Encoding_getSsrc);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Encoding_getAdaptivePTime(nullptr);
static jboolean Java_Encoding_getAdaptivePTime(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Encoding_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Encoding_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getAdaptivePTime",
          "()Z",
          &g_org_webrtc_RtpParameters_00024Encoding_getAdaptivePTime);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Codec_Constructor(JNIEnv* env, JniIntWrapper
    payloadType,
    const base::android::JavaRef<jstring>& name,
    const base::android::JavaRef<jobject>& kind,
    const base::android::JavaRef<jobject>& clockRate,
    const base::android::JavaRef<jobject>& numChannels,
    const base::android::JavaRef<jobject>& parameters) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtpParameters_00024Codec_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
"(ILjava/lang/String;Lorg/webrtc/MediaStreamTrack$MediaType;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Map;)V",
          &g_org_webrtc_RtpParameters_00024Codec_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, as_jint(payloadType), name.obj(), kind.obj(),
              clockRate.obj(), numChannels.obj(), parameters.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_getPayloadType(nullptr);
static jint Java_Codec_getPayloadType(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Codec_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getPayloadType",
          "()I",
          &g_org_webrtc_RtpParameters_00024Codec_getPayloadType);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_getName(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_Codec_getName(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Codec_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getName",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtpParameters_00024Codec_getName);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_getKind(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Codec_getKind(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Codec_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getKind",
          "()Lorg/webrtc/MediaStreamTrack$MediaType;",
          &g_org_webrtc_RtpParameters_00024Codec_getKind);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_getClockRate(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Codec_getClockRate(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Codec_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getClockRate",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_RtpParameters_00024Codec_getClockRate);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_getNumChannels(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Codec_getNumChannels(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Codec_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNumChannels",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_RtpParameters_00024Codec_getNumChannels);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Codec_getParameters(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Codec_getParameters(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Codec_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Codec_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getParameters",
          "()Ljava/util/Map;",
          &g_org_webrtc_RtpParameters_00024Codec_getParameters);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Rtcp_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_Rtcp_Constructor(JNIEnv* env, const
    base::android::JavaRef<jstring>& cname,
    jboolean reducedSize) {
  jclass clazz = org_webrtc_RtpParameters_00024Rtcp_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtpParameters_00024Rtcp_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;Z)V",
          &g_org_webrtc_RtpParameters_00024Rtcp_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, cname.obj(), reducedSize);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Rtcp_getCname(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_Rtcp_getCname(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Rtcp_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Rtcp_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCname",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtpParameters_00024Rtcp_getCname);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024Rtcp_getReducedSize(nullptr);
static jboolean Java_Rtcp_getReducedSize(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024Rtcp_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024Rtcp_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getReducedSize",
          "()Z",
          &g_org_webrtc_RtpParameters_00024Rtcp_getReducedSize);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024HeaderExtension_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_HeaderExtension_Constructor(JNIEnv* env,
    const base::android::JavaRef<jstring>& uri,
    JniIntWrapper id,
    jboolean encrypted) {
  jclass clazz = org_webrtc_RtpParameters_00024HeaderExtension_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtpParameters_00024HeaderExtension_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;IZ)V",
          &g_org_webrtc_RtpParameters_00024HeaderExtension_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, uri.obj(), as_jint(id), encrypted);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024HeaderExtension_getUri(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_HeaderExtension_getUri(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024HeaderExtension_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024HeaderExtension_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getUri",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtpParameters_00024HeaderExtension_getUri);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024HeaderExtension_getId(nullptr);
static jint Java_HeaderExtension_getId(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024HeaderExtension_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024HeaderExtension_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getId",
          "()I",
          &g_org_webrtc_RtpParameters_00024HeaderExtension_getId);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_00024HeaderExtension_getEncrypted(nullptr);
static jboolean Java_HeaderExtension_getEncrypted(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_00024HeaderExtension_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_00024HeaderExtension_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEncrypted",
          "()Z",
          &g_org_webrtc_RtpParameters_00024HeaderExtension_getEncrypted);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RtpParameters_Constructor(JNIEnv* env, const
    base::android::JavaRef<jstring>& transactionId,
    const base::android::JavaRef<jobject>& degradationPreference,
    const base::android::JavaRef<jobject>& rtcp,
    const base::android::JavaRef<jobject>& headerExtensions,
    const base::android::JavaRef<jobject>& encodings,
    const base::android::JavaRef<jobject>& codecs) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
"(Ljava/lang/String;Lorg/webrtc/RtpParameters$DegradationPreference;Lorg/webrtc/RtpParameters$Rtcp;Ljava/util/List;Ljava/util/List;Ljava/util/List;)V",
          &g_org_webrtc_RtpParameters_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, transactionId.obj(), degradationPreference.obj(), rtcp.obj(),
              headerExtensions.obj(), encodings.obj(), codecs.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_getTransactionId(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_RtpParameters_getTransactionId(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTransactionId",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtpParameters_getTransactionId);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_getDegradationPreference(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RtpParameters_getDegradationPreference(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDegradationPreference",
          "()Lorg/webrtc/RtpParameters$DegradationPreference;",
          &g_org_webrtc_RtpParameters_getDegradationPreference);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_getRtcp(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RtpParameters_getRtcp(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRtcp",
          "()Lorg/webrtc/RtpParameters$Rtcp;",
          &g_org_webrtc_RtpParameters_getRtcp);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_getHeaderExtensions(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RtpParameters_getHeaderExtensions(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getHeaderExtensions",
          "()Ljava/util/List;",
          &g_org_webrtc_RtpParameters_getHeaderExtensions);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_getEncodings(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RtpParameters_getEncodings(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEncodings",
          "()Ljava/util/List;",
          &g_org_webrtc_RtpParameters_getEncodings);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtpParameters_getCodecs(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RtpParameters_getCodecs(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtpParameters_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtpParameters_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCodecs",
          "()Ljava/util/List;",
          &g_org_webrtc_RtpParameters_getCodecs);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_RtpParameters_JNI
