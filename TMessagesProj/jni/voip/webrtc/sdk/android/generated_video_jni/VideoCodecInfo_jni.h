// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoCodecInfo

#ifndef org_webrtc_VideoCodecInfo_JNI
#define org_webrtc_VideoCodecInfo_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoCodecInfo[];
const char kClassPath_org_webrtc_VideoCodecInfo[] = "org/webrtc/VideoCodecInfo";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoCodecInfo_clazz(nullptr);
#ifndef org_webrtc_VideoCodecInfo_clazz_defined
#define org_webrtc_VideoCodecInfo_clazz_defined
inline jclass org_webrtc_VideoCodecInfo_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoCodecInfo,
      &g_org_webrtc_VideoCodecInfo_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_VideoCodecInfo_Constructor(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoCodecInfo_Constructor(JNIEnv* env, const
    base::android::JavaRef<jstring>& name,
    const base::android::JavaRef<jobject>& params) {
  jclass clazz = org_webrtc_VideoCodecInfo_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_VideoCodecInfo_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "<init>",
          "(Ljava/lang/String;Ljava/util/Map;)V",
          &g_org_webrtc_VideoCodecInfo_Constructor);

  jobject ret =
      env->NewObject(clazz,
          call_context.base.method_id, name.obj(), params.obj());
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoCodecInfo_getName(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_VideoCodecInfo_getName(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoCodecInfo_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoCodecInfo_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getName",
          "()Ljava/lang/String;",
          &g_org_webrtc_VideoCodecInfo_getName);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_VideoCodecInfo_getParams(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_VideoCodecInfo_getParams(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_VideoCodecInfo_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoCodecInfo_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getParams",
          "()Ljava/util/Map;",
          &g_org_webrtc_VideoCodecInfo_getParams);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoCodecInfo_JNI
