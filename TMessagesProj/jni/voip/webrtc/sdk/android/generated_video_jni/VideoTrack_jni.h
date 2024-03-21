// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoTrack

#ifndef org_webrtc_VideoTrack_JNI
#define org_webrtc_VideoTrack_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoTrack[];
const char kClassPath_org_webrtc_VideoTrack[] = "org/webrtc/VideoTrack";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoTrack_clazz(nullptr);
#ifndef org_webrtc_VideoTrack_clazz_defined
#define org_webrtc_VideoTrack_clazz_defined
inline jclass org_webrtc_VideoTrack_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoTrack,
      &g_org_webrtc_VideoTrack_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static void JNI_VideoTrack_AddSink(JNIEnv* env, jlong track,
    jlong nativeSink);

JNI_GENERATOR_EXPORT void Java_org_webrtc_VideoTrack_nativeAddSink(
    JNIEnv* env,
    jclass jcaller,
    jlong track,
    jlong nativeSink) {
  return JNI_VideoTrack_AddSink(env, track, nativeSink);
}

static void JNI_VideoTrack_RemoveSink(JNIEnv* env, jlong track,
    jlong nativeSink);

JNI_GENERATOR_EXPORT void Java_org_webrtc_VideoTrack_nativeRemoveSink(
    JNIEnv* env,
    jclass jcaller,
    jlong track,
    jlong nativeSink) {
  return JNI_VideoTrack_RemoveSink(env, track, nativeSink);
}

static jlong JNI_VideoTrack_WrapSink(JNIEnv* env, const base::android::JavaParamRef<jobject>& sink);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_VideoTrack_nativeWrapSink(
    JNIEnv* env,
    jclass jcaller,
    jobject sink) {
  return JNI_VideoTrack_WrapSink(env, base::android::JavaParamRef<jobject>(env, sink));
}

static void JNI_VideoTrack_FreeSink(JNIEnv* env, jlong sink);

JNI_GENERATOR_EXPORT void Java_org_webrtc_VideoTrack_nativeFreeSink(
    JNIEnv* env,
    jclass jcaller,
    jlong sink) {
  return JNI_VideoTrack_FreeSink(env, sink);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoTrack_JNI
