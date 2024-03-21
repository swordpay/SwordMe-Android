// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/AudioTrack

#ifndef org_webrtc_AudioTrack_JNI
#define org_webrtc_AudioTrack_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_AudioTrack[];
const char kClassPath_org_webrtc_AudioTrack[] = "org/webrtc/AudioTrack";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_AudioTrack_clazz(nullptr);
#ifndef org_webrtc_AudioTrack_clazz_defined
#define org_webrtc_AudioTrack_clazz_defined
inline jclass org_webrtc_AudioTrack_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_AudioTrack,
      &g_org_webrtc_AudioTrack_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static void JNI_AudioTrack_SetVolume(JNIEnv* env, jlong track,
    jdouble volume);

JNI_GENERATOR_EXPORT void Java_org_webrtc_AudioTrack_nativeSetVolume(
    JNIEnv* env,
    jclass jcaller,
    jlong track,
    jdouble volume) {
  return JNI_AudioTrack_SetVolume(env, track, volume);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_AudioTrack_JNI
