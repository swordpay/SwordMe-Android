// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/RtcCertificatePem

#ifndef org_webrtc_RtcCertificatePem_JNI
#define org_webrtc_RtcCertificatePem_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RtcCertificatePem[];
const char kClassPath_org_webrtc_RtcCertificatePem[] = "org/webrtc/RtcCertificatePem";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RtcCertificatePem_clazz(nullptr);
#ifndef org_webrtc_RtcCertificatePem_clazz_defined
#define org_webrtc_RtcCertificatePem_clazz_defined
inline jclass org_webrtc_RtcCertificatePem_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RtcCertificatePem,
      &g_org_webrtc_RtcCertificatePem_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static base::android::ScopedJavaLocalRef<jobject> JNI_RtcCertificatePem_GenerateCertificate(JNIEnv*
    env, const base::android::JavaParamRef<jobject>& keyType,
    jlong expires);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_RtcCertificatePem_nativeGenerateCertificate(
    JNIEnv* env,
    jclass jcaller,
    jobject keyType,
    jlong expires) {
  return JNI_RtcCertificatePem_GenerateCertificate(env, base::android::JavaParamRef<jobject>(env,
      keyType), expires).Release();
}


static std::atomic<jmethodID> g_org_webrtc_RtcCertificatePem_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RtcCertificatePem_Constructor(JNIEnv* env,
    const base::android::JavaRef<jstring>& privateKey,
    const base::android::JavaRef<jstring>& certificate) {
  jclass clazz = org_webrtc_RtcCertificatePem_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_RtcCertificatePem_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;Ljava/lang/String;)V",
          &g_org_webrtc_RtcCertificatePem_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, privateKey.obj(), certificate.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtcCertificatePem_getPrivateKey(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_RtcCertificatePem_getPrivateKey(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtcCertificatePem_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtcCertificatePem_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getPrivateKey",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtcCertificatePem_getPrivateKey);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_RtcCertificatePem_getCertificate(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_RtcCertificatePem_getCertificate(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RtcCertificatePem_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RtcCertificatePem_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCertificate",
          "()Ljava/lang/String;",
          &g_org_webrtc_RtcCertificatePem_getCertificate);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_RtcCertificatePem_JNI
