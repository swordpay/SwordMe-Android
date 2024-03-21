// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/RefCounted

#ifndef org_webrtc_RefCounted_JNI
#define org_webrtc_RefCounted_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_RefCounted[];
const char kClassPath_org_webrtc_RefCounted[] = "org/webrtc/RefCounted";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_RefCounted_clazz(nullptr);
#ifndef org_webrtc_RefCounted_clazz_defined
#define org_webrtc_RefCounted_clazz_defined
inline jclass org_webrtc_RefCounted_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_RefCounted,
      &g_org_webrtc_RefCounted_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_RefCounted_retain(nullptr);
static void Java_RefCounted_retain(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RefCounted_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RefCounted_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "retain",
          "()V",
          &g_org_webrtc_RefCounted_retain);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_RefCounted_release(nullptr);
static void Java_RefCounted_release(JNIEnv* env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_RefCounted_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_RefCounted_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "release",
          "()V",
          &g_org_webrtc_RefCounted_release);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_RefCounted_JNI
