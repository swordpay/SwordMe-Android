// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/AddIceObserver

#ifndef org_webrtc_AddIceObserver_JNI
#define org_webrtc_AddIceObserver_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_AddIceObserver[];
const char kClassPath_org_webrtc_AddIceObserver[] = "org/webrtc/AddIceObserver";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_AddIceObserver_clazz(nullptr);
#ifndef org_webrtc_AddIceObserver_clazz_defined
#define org_webrtc_AddIceObserver_clazz_defined
inline jclass org_webrtc_AddIceObserver_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_AddIceObserver,
      &g_org_webrtc_AddIceObserver_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_AddIceObserver_onAddSuccess(nullptr);
static void Java_AddIceObserver_onAddSuccess(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_AddIceObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_AddIceObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onAddSuccess",
          "()V",
          &g_org_webrtc_AddIceObserver_onAddSuccess);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_AddIceObserver_onAddFailure(nullptr);
static void Java_AddIceObserver_onAddFailure(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jstring>& error) {
  jclass clazz = org_webrtc_AddIceObserver_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_AddIceObserver_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onAddFailure",
          "(Ljava/lang/String;)V",
          &g_org_webrtc_AddIceObserver_onAddFailure);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, error.obj());
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_AddIceObserver_JNI
