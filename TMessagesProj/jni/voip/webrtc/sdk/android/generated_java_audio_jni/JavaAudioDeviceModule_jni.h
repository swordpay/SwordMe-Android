// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/audio/JavaAudioDeviceModule

#ifndef org_webrtc_audio_JavaAudioDeviceModule_JNI
#define org_webrtc_audio_JavaAudioDeviceModule_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_audio_JavaAudioDeviceModule[];
const char kClassPath_org_webrtc_audio_JavaAudioDeviceModule[] =
    "org/webrtc/audio/JavaAudioDeviceModule";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_audio_JavaAudioDeviceModule_clazz(nullptr);
#ifndef org_webrtc_audio_JavaAudioDeviceModule_clazz_defined
#define org_webrtc_audio_JavaAudioDeviceModule_clazz_defined
inline jclass org_webrtc_audio_JavaAudioDeviceModule_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_audio_JavaAudioDeviceModule,
      &g_org_webrtc_audio_JavaAudioDeviceModule_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jlong JNI_JavaAudioDeviceModule_CreateAudioDeviceModule(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& context,
    const base::android::JavaParamRef<jobject>& audioManager,
    const base::android::JavaParamRef<jobject>& audioInput,
    const base::android::JavaParamRef<jobject>& audioOutput,
    jint inputSampleRate,
    jint outputSampleRate,
    jboolean useStereoInput,
    jboolean useStereoOutput);

JNI_GENERATOR_EXPORT jlong
    Java_org_webrtc_audio_JavaAudioDeviceModule_nativeCreateAudioDeviceModule(
    JNIEnv* env,
    jclass jcaller,
    jobject context,
    jobject audioManager,
    jobject audioInput,
    jobject audioOutput,
    jint inputSampleRate,
    jint outputSampleRate,
    jboolean useStereoInput,
    jboolean useStereoOutput) {
  return JNI_JavaAudioDeviceModule_CreateAudioDeviceModule(env,
      base::android::JavaParamRef<jobject>(env, context), base::android::JavaParamRef<jobject>(env,
      audioManager), base::android::JavaParamRef<jobject>(env, audioInput),
      base::android::JavaParamRef<jobject>(env, audioOutput), inputSampleRate, outputSampleRate,
      useStereoInput, useStereoOutput);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_audio_JavaAudioDeviceModule_JNI
