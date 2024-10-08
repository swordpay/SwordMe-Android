// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/H264Utils

#ifndef org_webrtc_H264Utils_JNI
#define org_webrtc_H264Utils_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_H264Utils[];
const char kClassPath_org_webrtc_H264Utils[] = "org/webrtc/H264Utils";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_H264Utils_clazz(nullptr);
#ifndef org_webrtc_H264Utils_clazz_defined
#define org_webrtc_H264Utils_clazz_defined
inline jclass org_webrtc_H264Utils_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_H264Utils,
      &g_org_webrtc_H264Utils_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jboolean JNI_H264Utils_IsSameH264Profile(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& params1,
    const base::android::JavaParamRef<jobject>& params2);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_H264Utils_nativeIsSameH264Profile(
    JNIEnv* env,
    jclass jcaller,
    jobject params1,
    jobject params2) {
  return JNI_H264Utils_IsSameH264Profile(env, base::android::JavaParamRef<jobject>(env, params1),
      base::android::JavaParamRef<jobject>(env, params2));
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_H264Utils_JNI
