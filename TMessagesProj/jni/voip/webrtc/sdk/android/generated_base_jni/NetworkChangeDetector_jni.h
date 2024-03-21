// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/NetworkChangeDetector

#ifndef org_webrtc_NetworkChangeDetector_JNI
#define org_webrtc_NetworkChangeDetector_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_NetworkChangeDetector[];
const char kClassPath_org_webrtc_NetworkChangeDetector[] = "org/webrtc/NetworkChangeDetector";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_NetworkChangeDetector_00024IPAddress[];
const char kClassPath_org_webrtc_NetworkChangeDetector_00024IPAddress[] =
    "org/webrtc/NetworkChangeDetector$IPAddress";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_NetworkChangeDetector_00024NetworkInformation[];
const char kClassPath_org_webrtc_NetworkChangeDetector_00024NetworkInformation[] =
    "org/webrtc/NetworkChangeDetector$NetworkInformation";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_NetworkChangeDetector_clazz(nullptr);
#ifndef org_webrtc_NetworkChangeDetector_clazz_defined
#define org_webrtc_NetworkChangeDetector_clazz_defined
inline jclass org_webrtc_NetworkChangeDetector_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_NetworkChangeDetector,
      &g_org_webrtc_NetworkChangeDetector_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_NetworkChangeDetector_00024IPAddress_clazz(nullptr);
#ifndef org_webrtc_NetworkChangeDetector_00024IPAddress_clazz_defined
#define org_webrtc_NetworkChangeDetector_00024IPAddress_clazz_defined
inline jclass org_webrtc_NetworkChangeDetector_00024IPAddress_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_NetworkChangeDetector_00024IPAddress,
      &g_org_webrtc_NetworkChangeDetector_00024IPAddress_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(nullptr);
#ifndef org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz_defined
#define org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz_defined
inline jclass org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_NetworkChangeDetector_00024NetworkInformation,
      &g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_NetworkChangeDetector_00024IPAddress_getAddress(nullptr);
static base::android::ScopedJavaLocalRef<jbyteArray> Java_IPAddress_getAddress(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_NetworkChangeDetector_00024IPAddress_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkChangeDetector_00024IPAddress_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getAddress",
          "()[B",
          &g_org_webrtc_NetworkChangeDetector_00024IPAddress_getAddress);

  jbyteArray ret =
      static_cast<jbyteArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jbyteArray>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getIpAddresses(nullptr);
static base::android::ScopedJavaLocalRef<jobjectArray>
    Java_NetworkInformation_getIpAddresses(JNIEnv* env, const base::android::JavaRef<jobject>& obj)
    {
  jclass clazz = org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIpAddresses",
          "()[Lorg/webrtc/NetworkChangeDetector$IPAddress;",
          &g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getIpAddresses);

  jobjectArray ret =
      static_cast<jobjectArray>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getConnectionType(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_NetworkInformation_getConnectionType(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getConnectionType",
          "()Lorg/webrtc/NetworkChangeDetector$ConnectionType;",
          &g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getConnectionType);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getUnderlyingConnectionTypeForVpn(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_NetworkInformation_getUnderlyingConnectionTypeForVpn(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getUnderlyingConnectionTypeForVpn",
          "()Lorg/webrtc/NetworkChangeDetector$ConnectionType;",
&g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getUnderlyingConnectionTypeForVpn);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getHandle(nullptr);
static jlong Java_NetworkInformation_getHandle(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getHandle",
          "()J",
          &g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getHandle);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getName(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_NetworkInformation_getName(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkChangeDetector_00024NetworkInformation_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getName",
          "()Ljava/lang/String;",
          &g_org_webrtc_NetworkChangeDetector_00024NetworkInformation_getName);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_NetworkChangeDetector_JNI
