// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/VideoSink

#ifndef org_webrtc_VideoSink_JNI
#define org_webrtc_VideoSink_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_VideoSink[];
const char kClassPath_org_webrtc_VideoSink[] = "org/webrtc/VideoSink";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_VideoSink_clazz(nullptr);
#ifndef org_webrtc_VideoSink_clazz_defined
#define org_webrtc_VideoSink_clazz_defined
inline jclass org_webrtc_VideoSink_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_VideoSink,
      &g_org_webrtc_VideoSink_clazz);
}
#endif


namespace  webrtc {
namespace jni {


static std::atomic<jmethodID> g_org_webrtc_VideoSink_onFrame(nullptr);
static void Java_VideoSink_onFrame(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& frame) {
  jclass clazz = org_webrtc_VideoSink_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_VideoSink_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onFrame",
          "(Lorg/webrtc/VideoFrame;)V",
          &g_org_webrtc_VideoSink_onFrame);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, frame.obj());
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_VideoSink_JNI
