// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/SdpObserver

#ifndef org_webrtc_SdpObserver_JNI
#define org_webrtc_SdpObserver_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_SdpObserver[];
const char kClassPath_org_webrtc_SdpObserver[] = "org/webrtc/SdpObserver";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_SdpObserver_clazz(nullptr);
#ifndef org_webrtc_SdpObserver_clazz_defined
#define org_webrtc_SdpObserver_clazz_defined
inline jclass org_webrtc_SdpObserver_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_SdpObserver,
      &g_org_webrtc_SdpObserver_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_SdpObserver_onCreateSuccess(nullptr);
static void Java_SdpObserver_onCreateSuccess(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& sdp) {
  jclass clazz = org_webrtc_SdpObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_SdpObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onCreateSuccess",
          "(Lorg/webrtc/SessionDescription;)V",
          &g_org_webrtc_SdpObserver_onCreateSuccess);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, sdp.obj());
}

static std::atomic<jmethodID> g_org_webrtc_SdpObserver_onSetSuccess(nullptr);
static void Java_SdpObserver_onSetSuccess(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_SdpObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_SdpObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onSetSuccess",
          "()V",
          &g_org_webrtc_SdpObserver_onSetSuccess);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_SdpObserver_onCreateFailure(nullptr);
static void Java_SdpObserver_onCreateFailure(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jstring>& error) {
  jclass clazz = org_webrtc_SdpObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_SdpObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onCreateFailure",
          "(Ljava/lang/String;)V",
          &g_org_webrtc_SdpObserver_onCreateFailure);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, error.obj());
}

static std::atomic<jmethodID> g_org_webrtc_SdpObserver_onSetFailure(nullptr);
static void Java_SdpObserver_onSetFailure(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jstring>& error) {
  jclass clazz = org_webrtc_SdpObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_SdpObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onSetFailure",
          "(Ljava/lang/String;)V",
          &g_org_webrtc_SdpObserver_onSetFailure);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, error.obj());
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_SdpObserver_JNI
