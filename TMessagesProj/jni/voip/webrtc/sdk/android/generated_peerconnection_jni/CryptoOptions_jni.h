// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/CryptoOptions

#ifndef org_webrtc_CryptoOptions_JNI
#define org_webrtc_CryptoOptions_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_CryptoOptions[];
const char kClassPath_org_webrtc_CryptoOptions[] = "org/webrtc/CryptoOptions";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_CryptoOptions_00024Srtp[];
const char kClassPath_org_webrtc_CryptoOptions_00024Srtp[] = "org/webrtc/CryptoOptions$Srtp";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_CryptoOptions_00024SFrame[];
const char kClassPath_org_webrtc_CryptoOptions_00024SFrame[] = "org/webrtc/CryptoOptions$SFrame";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_CryptoOptions_clazz(nullptr);
#ifndef org_webrtc_CryptoOptions_clazz_defined
#define org_webrtc_CryptoOptions_clazz_defined
inline jclass org_webrtc_CryptoOptions_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_CryptoOptions,
      &g_org_webrtc_CryptoOptions_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_CryptoOptions_00024Srtp_clazz(nullptr);
#ifndef org_webrtc_CryptoOptions_00024Srtp_clazz_defined
#define org_webrtc_CryptoOptions_00024Srtp_clazz_defined
inline jclass org_webrtc_CryptoOptions_00024Srtp_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_CryptoOptions_00024Srtp,
      &g_org_webrtc_CryptoOptions_00024Srtp_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_CryptoOptions_00024SFrame_clazz(nullptr);
#ifndef org_webrtc_CryptoOptions_00024SFrame_clazz_defined
#define org_webrtc_CryptoOptions_00024SFrame_clazz_defined
inline jclass org_webrtc_CryptoOptions_00024SFrame_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_CryptoOptions_00024SFrame,
      &g_org_webrtc_CryptoOptions_00024SFrame_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID>
    g_org_webrtc_CryptoOptions_00024Srtp_getEnableGcmCryptoSuites(nullptr);
static jboolean Java_Srtp_getEnableGcmCryptoSuites(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_CryptoOptions_00024Srtp_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_CryptoOptions_00024Srtp_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEnableGcmCryptoSuites",
          "()Z",
          &g_org_webrtc_CryptoOptions_00024Srtp_getEnableGcmCryptoSuites);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_CryptoOptions_00024Srtp_getEnableAes128Sha1_32CryptoCipher(nullptr);
static jboolean Java_Srtp_getEnableAes128Sha1_32CryptoCipher(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_CryptoOptions_00024Srtp_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_CryptoOptions_00024Srtp_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEnableAes128Sha1_32CryptoCipher",
          "()Z",
          &g_org_webrtc_CryptoOptions_00024Srtp_getEnableAes128Sha1_32CryptoCipher);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_CryptoOptions_00024Srtp_getEnableEncryptedRtpHeaderExtensions(nullptr);
static jboolean Java_Srtp_getEnableEncryptedRtpHeaderExtensions(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_CryptoOptions_00024Srtp_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_CryptoOptions_00024Srtp_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEnableEncryptedRtpHeaderExtensions",
          "()Z",
          &g_org_webrtc_CryptoOptions_00024Srtp_getEnableEncryptedRtpHeaderExtensions);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_CryptoOptions_00024SFrame_getRequireFrameEncryption(nullptr);
static jboolean Java_SFrame_getRequireFrameEncryption(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_CryptoOptions_00024SFrame_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_CryptoOptions_00024SFrame_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRequireFrameEncryption",
          "()Z",
          &g_org_webrtc_CryptoOptions_00024SFrame_getRequireFrameEncryption);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_CryptoOptions_getSrtp(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_CryptoOptions_getSrtp(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_CryptoOptions_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_CryptoOptions_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSrtp",
          "()Lorg/webrtc/CryptoOptions$Srtp;",
          &g_org_webrtc_CryptoOptions_getSrtp);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_CryptoOptions_getSFrame(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_CryptoOptions_getSFrame(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_CryptoOptions_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_CryptoOptions_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSFrame",
          "()Lorg/webrtc/CryptoOptions$SFrame;",
          &g_org_webrtc_CryptoOptions_getSFrame);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_CryptoOptions_JNI
