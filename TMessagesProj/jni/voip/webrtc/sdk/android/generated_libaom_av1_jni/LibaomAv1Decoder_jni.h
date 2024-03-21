// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/LibaomAv1Decoder

#ifndef org_webrtc_LibaomAv1Decoder_JNI
#define org_webrtc_LibaomAv1Decoder_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_LibaomAv1Decoder[];
const char kClassPath_org_webrtc_LibaomAv1Decoder[] = "org/webrtc/LibaomAv1Decoder";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_LibaomAv1Decoder_clazz(nullptr);
#ifndef org_webrtc_LibaomAv1Decoder_clazz_defined
#define org_webrtc_LibaomAv1Decoder_clazz_defined
inline jclass org_webrtc_LibaomAv1Decoder_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_LibaomAv1Decoder,
      &g_org_webrtc_LibaomAv1Decoder_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jlong JNI_LibaomAv1Decoder_CreateDecoder(JNIEnv* env);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_LibaomAv1Decoder_nativeCreateDecoder(
    JNIEnv* env,
    jclass jcaller) {
  return JNI_LibaomAv1Decoder_CreateDecoder(env);
}

static jboolean JNI_LibaomAv1Decoder_IsSupported(JNIEnv* env);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_LibaomAv1Decoder_nativeIsSupported(
    JNIEnv* env,
    jclass jcaller) {
  return JNI_LibaomAv1Decoder_IsSupported(env);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_LibaomAv1Decoder_JNI
