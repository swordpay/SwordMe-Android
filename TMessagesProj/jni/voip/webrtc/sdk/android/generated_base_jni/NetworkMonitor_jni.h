// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/NetworkMonitor

#ifndef org_webrtc_NetworkMonitor_JNI
#define org_webrtc_NetworkMonitor_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_NetworkMonitor[];
const char kClassPath_org_webrtc_NetworkMonitor[] = "org/webrtc/NetworkMonitor";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_NetworkMonitor_clazz(nullptr);
#ifndef org_webrtc_NetworkMonitor_clazz_defined
#define org_webrtc_NetworkMonitor_clazz_defined
inline jclass org_webrtc_NetworkMonitor_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_NetworkMonitor,
      &g_org_webrtc_NetworkMonitor_clazz);
}
#endif


namespace  webrtc {
namespace jni {

JNI_GENERATOR_EXPORT void Java_org_webrtc_NetworkMonitor_nativeNotifyConnectionTypeChanged(
    JNIEnv* env,
    jobject jcaller,
    jlong nativeAndroidNetworkMonitor) {
  AndroidNetworkMonitor* native =
      reinterpret_cast<AndroidNetworkMonitor*>(nativeAndroidNetworkMonitor);
  CHECK_NATIVE_PTR(env, jcaller, native, "NotifyConnectionTypeChanged");
  return native->NotifyConnectionTypeChanged(env, base::android::JavaParamRef<jobject>(env,
      jcaller));
}

JNI_GENERATOR_EXPORT void Java_org_webrtc_NetworkMonitor_nativeNotifyOfNetworkConnect(
    JNIEnv* env,
    jobject jcaller,
    jlong nativeAndroidNetworkMonitor,
    jobject networkInfo) {
  AndroidNetworkMonitor* native =
      reinterpret_cast<AndroidNetworkMonitor*>(nativeAndroidNetworkMonitor);
  CHECK_NATIVE_PTR(env, jcaller, native, "NotifyOfNetworkConnect");
  return native->NotifyOfNetworkConnect(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, networkInfo));
}

JNI_GENERATOR_EXPORT void Java_org_webrtc_NetworkMonitor_nativeNotifyOfNetworkDisconnect(
    JNIEnv* env,
    jobject jcaller,
    jlong nativeAndroidNetworkMonitor,
    jlong networkHandle) {
  AndroidNetworkMonitor* native =
      reinterpret_cast<AndroidNetworkMonitor*>(nativeAndroidNetworkMonitor);
  CHECK_NATIVE_PTR(env, jcaller, native, "NotifyOfNetworkDisconnect");
  return native->NotifyOfNetworkDisconnect(env, base::android::JavaParamRef<jobject>(env, jcaller),
      networkHandle);
}

JNI_GENERATOR_EXPORT void Java_org_webrtc_NetworkMonitor_nativeNotifyOfActiveNetworkList(
    JNIEnv* env,
    jobject jcaller,
    jlong nativeAndroidNetworkMonitor,
    jobjectArray networkInfos) {
  AndroidNetworkMonitor* native =
      reinterpret_cast<AndroidNetworkMonitor*>(nativeAndroidNetworkMonitor);
  CHECK_NATIVE_PTR(env, jcaller, native, "NotifyOfActiveNetworkList");
  return native->NotifyOfActiveNetworkList(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobjectArray>(env, networkInfos));
}

JNI_GENERATOR_EXPORT void Java_org_webrtc_NetworkMonitor_nativeNotifyOfNetworkPreference(
    JNIEnv* env,
    jobject jcaller,
    jlong nativeAndroidNetworkMonitor,
    jobject type,
    jint preference) {
  AndroidNetworkMonitor* native =
      reinterpret_cast<AndroidNetworkMonitor*>(nativeAndroidNetworkMonitor);
  CHECK_NATIVE_PTR(env, jcaller, native, "NotifyOfNetworkPreference");
  return native->NotifyOfNetworkPreference(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, type), preference);
}


static std::atomic<jmethodID> g_org_webrtc_NetworkMonitor_getInstance(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_NetworkMonitor_getInstance(JNIEnv* env) {
  jclass clazz = org_webrtc_NetworkMonitor_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_NetworkMonitor_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "getInstance",
          "()Lorg/webrtc/NetworkMonitor;",
          &g_org_webrtc_NetworkMonitor_getInstance);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_NetworkMonitor_startMonitoring(nullptr);
static void Java_NetworkMonitor_startMonitoring(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& applicationContext,
    jlong nativeObserver) {
  jclass clazz = org_webrtc_NetworkMonitor_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkMonitor_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "startMonitoring",
          "(Landroid/content/Context;J)V",
          &g_org_webrtc_NetworkMonitor_startMonitoring);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, applicationContext.obj(), nativeObserver);
}

static std::atomic<jmethodID> g_org_webrtc_NetworkMonitor_stopMonitoring(nullptr);
static void Java_NetworkMonitor_stopMonitoring(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, jlong nativeObserver) {
  jclass clazz = org_webrtc_NetworkMonitor_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkMonitor_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "stopMonitoring",
          "(J)V",
          &g_org_webrtc_NetworkMonitor_stopMonitoring);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, nativeObserver);
}

static std::atomic<jmethodID> g_org_webrtc_NetworkMonitor_networkBindingSupported(nullptr);
static jboolean Java_NetworkMonitor_networkBindingSupported(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_NetworkMonitor_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_NetworkMonitor_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "networkBindingSupported",
          "()Z",
          &g_org_webrtc_NetworkMonitor_networkBindingSupported);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_NetworkMonitor_androidSdkInt(nullptr);
static jint Java_NetworkMonitor_androidSdkInt(JNIEnv* env) {
  jclass clazz = org_webrtc_NetworkMonitor_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_NetworkMonitor_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "androidSdkInt",
          "()I",
          &g_org_webrtc_NetworkMonitor_androidSdkInt);

  jint ret =
      env->CallStaticIntMethod(clazz,
          call_context.base.method_id);
  return ret;
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_NetworkMonitor_JNI
