// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/BuiltinAudioDecoderFactoryFactory

#ifndef org_webrtc_BuiltinAudioDecoderFactoryFactory_JNI
#define org_webrtc_BuiltinAudioDecoderFactoryFactory_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_BuiltinAudioDecoderFactoryFactory[];
const char kClassPath_org_webrtc_BuiltinAudioDecoderFactoryFactory[] =
    "org/webrtc/BuiltinAudioDecoderFactoryFactory";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_BuiltinAudioDecoderFactoryFactory_clazz(nullptr);
#ifndef org_webrtc_BuiltinAudioDecoderFactoryFactory_clazz_defined
#define org_webrtc_BuiltinAudioDecoderFactoryFactory_clazz_defined
inline jclass org_webrtc_BuiltinAudioDecoderFactoryFactory_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_BuiltinAudioDecoderFactoryFactory,
      &g_org_webrtc_BuiltinAudioDecoderFactoryFactory_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jlong JNI_BuiltinAudioDecoderFactoryFactory_CreateBuiltinAudioDecoderFactory(JNIEnv* env);

JNI_GENERATOR_EXPORT jlong
    Java_org_webrtc_BuiltinAudioDecoderFactoryFactory_nativeCreateBuiltinAudioDecoderFactory(
    JNIEnv* env,
    jclass jcaller) {
  return JNI_BuiltinAudioDecoderFactoryFactory_CreateBuiltinAudioDecoderFactory(env);
}


}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_BuiltinAudioDecoderFactoryFactory_JNI
