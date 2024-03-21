// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//     base/android/jni_generator/jni_generator.py
// For
//     org/webrtc/PeerConnection

#ifndef org_webrtc_PeerConnection_JNI
#define org_webrtc_PeerConnection_JNI

#include <jni.h>

#include "webrtc/sdk/android/src/jni/jni_generator_helper.h"


JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_PeerConnection[];
const char kClassPath_org_webrtc_PeerConnection[] = "org/webrtc/PeerConnection";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_PeerConnection_00024IceGatheringState[];
const char kClassPath_org_webrtc_PeerConnection_00024IceGatheringState[] =
    "org/webrtc/PeerConnection$IceGatheringState";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_PeerConnection_00024IceConnectionState[];
const char kClassPath_org_webrtc_PeerConnection_00024IceConnectionState[] =
    "org/webrtc/PeerConnection$IceConnectionState";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_PeerConnection_00024PeerConnectionState[];
const char kClassPath_org_webrtc_PeerConnection_00024PeerConnectionState[] =
    "org/webrtc/PeerConnection$PeerConnectionState";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_PeerConnection_00024SignalingState[];
const char kClassPath_org_webrtc_PeerConnection_00024SignalingState[] =
    "org/webrtc/PeerConnection$SignalingState";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_PeerConnection_00024Observer[];
const char kClassPath_org_webrtc_PeerConnection_00024Observer[] =
    "org/webrtc/PeerConnection$Observer";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_PeerConnection_00024IceServer[];
const char kClassPath_org_webrtc_PeerConnection_00024IceServer[] =
    "org/webrtc/PeerConnection$IceServer";

JNI_REGISTRATION_EXPORT extern const char kClassPath_org_webrtc_PeerConnection_00024AdapterType[];
const char kClassPath_org_webrtc_PeerConnection_00024AdapterType[] =
    "org/webrtc/PeerConnection$AdapterType";

JNI_REGISTRATION_EXPORT extern const char
    kClassPath_org_webrtc_PeerConnection_00024RTCConfiguration[];
const char kClassPath_org_webrtc_PeerConnection_00024RTCConfiguration[] =
    "org/webrtc/PeerConnection$RTCConfiguration";
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_org_webrtc_PeerConnection_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_clazz_defined
#define org_webrtc_PeerConnection_clazz_defined
inline jclass org_webrtc_PeerConnection_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_PeerConnection,
      &g_org_webrtc_PeerConnection_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024IceGatheringState_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024IceGatheringState_clazz_defined
#define org_webrtc_PeerConnection_00024IceGatheringState_clazz_defined
inline jclass org_webrtc_PeerConnection_00024IceGatheringState_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_PeerConnection_00024IceGatheringState,
      &g_org_webrtc_PeerConnection_00024IceGatheringState_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024IceConnectionState_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024IceConnectionState_clazz_defined
#define org_webrtc_PeerConnection_00024IceConnectionState_clazz_defined
inline jclass org_webrtc_PeerConnection_00024IceConnectionState_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_PeerConnection_00024IceConnectionState,
      &g_org_webrtc_PeerConnection_00024IceConnectionState_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024PeerConnectionState_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024PeerConnectionState_clazz_defined
#define org_webrtc_PeerConnection_00024PeerConnectionState_clazz_defined
inline jclass org_webrtc_PeerConnection_00024PeerConnectionState_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_PeerConnection_00024PeerConnectionState,
      &g_org_webrtc_PeerConnection_00024PeerConnectionState_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024SignalingState_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024SignalingState_clazz_defined
#define org_webrtc_PeerConnection_00024SignalingState_clazz_defined
inline jclass org_webrtc_PeerConnection_00024SignalingState_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_PeerConnection_00024SignalingState,
      &g_org_webrtc_PeerConnection_00024SignalingState_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024Observer_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024Observer_clazz_defined
#define org_webrtc_PeerConnection_00024Observer_clazz_defined
inline jclass org_webrtc_PeerConnection_00024Observer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_PeerConnection_00024Observer,
      &g_org_webrtc_PeerConnection_00024Observer_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024IceServer_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024IceServer_clazz_defined
#define org_webrtc_PeerConnection_00024IceServer_clazz_defined
inline jclass org_webrtc_PeerConnection_00024IceServer_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_PeerConnection_00024IceServer,
      &g_org_webrtc_PeerConnection_00024IceServer_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024AdapterType_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024AdapterType_clazz_defined
#define org_webrtc_PeerConnection_00024AdapterType_clazz_defined
inline jclass org_webrtc_PeerConnection_00024AdapterType_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_org_webrtc_PeerConnection_00024AdapterType,
      &g_org_webrtc_PeerConnection_00024AdapterType_clazz);
}
#endif
// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_clazz(nullptr);
#ifndef org_webrtc_PeerConnection_00024RTCConfiguration_clazz_defined
#define org_webrtc_PeerConnection_00024RTCConfiguration_clazz_defined
inline jclass org_webrtc_PeerConnection_00024RTCConfiguration_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env,
      kClassPath_org_webrtc_PeerConnection_00024RTCConfiguration,
      &g_org_webrtc_PeerConnection_00024RTCConfiguration_clazz);
}
#endif


namespace  webrtc {
namespace jni {

static jlong JNI_PeerConnection_GetNativePeerConnection(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_PeerConnection_nativeGetNativePeerConnection(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetNativePeerConnection(env, base::android::JavaParamRef<jobject>(env,
      jcaller));
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_GetLocalDescription(JNIEnv*
    env, const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeGetLocalDescription(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetLocalDescription(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_GetRemoteDescription(JNIEnv*
    env, const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeGetRemoteDescription(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetRemoteDescription(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_GetCertificate(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeGetCertificate(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetCertificate(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_CreateDataChannel(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jstring>& label,
    const base::android::JavaParamRef<jobject>& init);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeCreateDataChannel(
    JNIEnv* env,
    jobject jcaller,
    jstring label,
    jobject init) {
  return JNI_PeerConnection_CreateDataChannel(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jstring>(env, label),
      base::android::JavaParamRef<jobject>(env, init)).Release();
}

static void JNI_PeerConnection_CreateOffer(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller,
    const base::android::JavaParamRef<jobject>& observer,
    const base::android::JavaParamRef<jobject>& constraints);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeCreateOffer(
    JNIEnv* env,
    jobject jcaller,
    jobject observer,
    jobject constraints) {
  return JNI_PeerConnection_CreateOffer(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, observer), base::android::JavaParamRef<jobject>(env,
      constraints));
}

static void JNI_PeerConnection_CreateAnswer(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller,
    const base::android::JavaParamRef<jobject>& observer,
    const base::android::JavaParamRef<jobject>& constraints);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeCreateAnswer(
    JNIEnv* env,
    jobject jcaller,
    jobject observer,
    jobject constraints) {
  return JNI_PeerConnection_CreateAnswer(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, observer), base::android::JavaParamRef<jobject>(env,
      constraints));
}

static void JNI_PeerConnection_SetLocalDescriptionAutomatically(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& observer);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeSetLocalDescriptionAutomatically(
    JNIEnv* env,
    jobject jcaller,
    jobject observer) {
  return JNI_PeerConnection_SetLocalDescriptionAutomatically(env,
      base::android::JavaParamRef<jobject>(env, jcaller), base::android::JavaParamRef<jobject>(env,
      observer));
}

static void JNI_PeerConnection_SetLocalDescription(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& observer,
    const base::android::JavaParamRef<jobject>& sdp);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeSetLocalDescription(
    JNIEnv* env,
    jobject jcaller,
    jobject observer,
    jobject sdp) {
  return JNI_PeerConnection_SetLocalDescription(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobject>(env, observer),
      base::android::JavaParamRef<jobject>(env, sdp));
}

static void JNI_PeerConnection_SetRemoteDescription(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& observer,
    const base::android::JavaParamRef<jobject>& sdp);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeSetRemoteDescription(
    JNIEnv* env,
    jobject jcaller,
    jobject observer,
    jobject sdp) {
  return JNI_PeerConnection_SetRemoteDescription(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobject>(env, observer),
      base::android::JavaParamRef<jobject>(env, sdp));
}

static void JNI_PeerConnection_RestartIce(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeRestartIce(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_RestartIce(env, base::android::JavaParamRef<jobject>(env, jcaller));
}

static void JNI_PeerConnection_SetAudioPlayout(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jboolean playout);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeSetAudioPlayout(
    JNIEnv* env,
    jobject jcaller,
    jboolean playout) {
  return JNI_PeerConnection_SetAudioPlayout(env, base::android::JavaParamRef<jobject>(env, jcaller),
      playout);
}

static void JNI_PeerConnection_SetAudioRecording(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jboolean recording);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeSetAudioRecording(
    JNIEnv* env,
    jobject jcaller,
    jboolean recording) {
  return JNI_PeerConnection_SetAudioRecording(env, base::android::JavaParamRef<jobject>(env,
      jcaller), recording);
}

static jboolean JNI_PeerConnection_SetBitrate(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& min,
    const base::android::JavaParamRef<jobject>& current,
    const base::android::JavaParamRef<jobject>& max);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeSetBitrate(
    JNIEnv* env,
    jobject jcaller,
    jobject min,
    jobject current,
    jobject max) {
  return JNI_PeerConnection_SetBitrate(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, min), base::android::JavaParamRef<jobject>(env,
      current), base::android::JavaParamRef<jobject>(env, max));
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_SignalingState(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeSignalingState(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_SignalingState(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_IceConnectionState(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeIceConnectionState(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_IceConnectionState(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_ConnectionState(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeConnectionState(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_ConnectionState(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_IceGatheringState(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeIceGatheringState(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_IceGatheringState(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static void JNI_PeerConnection_Close(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeClose(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_Close(env, base::android::JavaParamRef<jobject>(env, jcaller));
}

static jlong JNI_PeerConnection_CreatePeerConnectionObserver(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& observer);

JNI_GENERATOR_EXPORT jlong Java_org_webrtc_PeerConnection_nativeCreatePeerConnectionObserver(
    JNIEnv* env,
    jclass jcaller,
    jobject observer) {
  return JNI_PeerConnection_CreatePeerConnectionObserver(env,
      base::android::JavaParamRef<jobject>(env, observer));
}

static void JNI_PeerConnection_FreeOwnedPeerConnection(JNIEnv* env, jlong ownedPeerConnection);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeFreeOwnedPeerConnection(
    JNIEnv* env,
    jclass jcaller,
    jlong ownedPeerConnection) {
  return JNI_PeerConnection_FreeOwnedPeerConnection(env, ownedPeerConnection);
}

static jboolean JNI_PeerConnection_SetConfiguration(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& config);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeSetConfiguration(
    JNIEnv* env,
    jobject jcaller,
    jobject config) {
  return JNI_PeerConnection_SetConfiguration(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobject>(env, config));
}

static jboolean JNI_PeerConnection_AddIceCandidate(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jstring>& sdpMid,
    jint sdpMLineIndex,
    const base::android::JavaParamRef<jstring>& iceCandidateSdp);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeAddIceCandidate(
    JNIEnv* env,
    jobject jcaller,
    jstring sdpMid,
    jint sdpMLineIndex,
    jstring iceCandidateSdp) {
  return JNI_PeerConnection_AddIceCandidate(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jstring>(env, sdpMid), sdpMLineIndex,
      base::android::JavaParamRef<jstring>(env, iceCandidateSdp));
}

static void JNI_PeerConnection_AddIceCandidateWithObserver(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jstring>& sdpMid,
    jint sdpMLineIndex,
    const base::android::JavaParamRef<jstring>& iceCandidateSdp,
    const base::android::JavaParamRef<jobject>& observer);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeAddIceCandidateWithObserver(
    JNIEnv* env,
    jobject jcaller,
    jstring sdpMid,
    jint sdpMLineIndex,
    jstring iceCandidateSdp,
    jobject observer) {
  return JNI_PeerConnection_AddIceCandidateWithObserver(env,
      base::android::JavaParamRef<jobject>(env, jcaller), base::android::JavaParamRef<jstring>(env,
      sdpMid), sdpMLineIndex, base::android::JavaParamRef<jstring>(env, iceCandidateSdp),
      base::android::JavaParamRef<jobject>(env, observer));
}

static jboolean JNI_PeerConnection_RemoveIceCandidates(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobjectArray>& candidates);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeRemoveIceCandidates(
    JNIEnv* env,
    jobject jcaller,
    jobjectArray candidates) {
  return JNI_PeerConnection_RemoveIceCandidates(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobjectArray>(env, candidates));
}

static jboolean JNI_PeerConnection_AddLocalStream(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jlong stream);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeAddLocalStream(
    JNIEnv* env,
    jobject jcaller,
    jlong stream) {
  return JNI_PeerConnection_AddLocalStream(env, base::android::JavaParamRef<jobject>(env, jcaller),
      stream);
}

static void JNI_PeerConnection_RemoveLocalStream(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jlong stream);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeRemoveLocalStream(
    JNIEnv* env,
    jobject jcaller,
    jlong stream) {
  return JNI_PeerConnection_RemoveLocalStream(env, base::android::JavaParamRef<jobject>(env,
      jcaller), stream);
}

static jboolean JNI_PeerConnection_OldGetStats(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& observer,
    jlong nativeTrack);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeOldGetStats(
    JNIEnv* env,
    jobject jcaller,
    jobject observer,
    jlong nativeTrack) {
  return JNI_PeerConnection_OldGetStats(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, observer), nativeTrack);
}

static void JNI_PeerConnection_NewGetStats(JNIEnv* env, const base::android::JavaParamRef<jobject>&
    jcaller,
    const base::android::JavaParamRef<jobject>& callback);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeNewGetStats(
    JNIEnv* env,
    jobject jcaller,
    jobject callback) {
  return JNI_PeerConnection_NewGetStats(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jobject>(env, callback));
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_CreateSender(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jstring>& kind,
    const base::android::JavaParamRef<jstring>& stream_id);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeCreateSender(
    JNIEnv* env,
    jobject jcaller,
    jstring kind,
    jstring stream_id) {
  return JNI_PeerConnection_CreateSender(env, base::android::JavaParamRef<jobject>(env, jcaller),
      base::android::JavaParamRef<jstring>(env, kind), base::android::JavaParamRef<jstring>(env,
      stream_id)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_GetSenders(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeGetSenders(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetSenders(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_GetReceivers(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeGetReceivers(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetReceivers(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_GetTransceivers(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeGetTransceivers(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_GetTransceivers(env, base::android::JavaParamRef<jobject>(env,
      jcaller)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_AddTrack(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jlong track,
    const base::android::JavaParamRef<jobject>& streamIds);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeAddTrack(
    JNIEnv* env,
    jobject jcaller,
    jlong track,
    jobject streamIds) {
  return JNI_PeerConnection_AddTrack(env, base::android::JavaParamRef<jobject>(env, jcaller), track,
      base::android::JavaParamRef<jobject>(env, streamIds)).Release();
}

static jboolean JNI_PeerConnection_RemoveTrack(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jlong sender);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeRemoveTrack(
    JNIEnv* env,
    jobject jcaller,
    jlong sender) {
  return JNI_PeerConnection_RemoveTrack(env, base::android::JavaParamRef<jobject>(env, jcaller),
      sender);
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_AddTransceiverWithTrack(JNIEnv*
    env, const base::android::JavaParamRef<jobject>& jcaller,
    jlong track,
    const base::android::JavaParamRef<jobject>& init);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeAddTransceiverWithTrack(
    JNIEnv* env,
    jobject jcaller,
    jlong track,
    jobject init) {
  return JNI_PeerConnection_AddTransceiverWithTrack(env, base::android::JavaParamRef<jobject>(env,
      jcaller), track, base::android::JavaParamRef<jobject>(env, init)).Release();
}

static base::android::ScopedJavaLocalRef<jobject> JNI_PeerConnection_AddTransceiverOfType(JNIEnv*
    env, const base::android::JavaParamRef<jobject>& jcaller,
    const base::android::JavaParamRef<jobject>& mediaType,
    const base::android::JavaParamRef<jobject>& init);

JNI_GENERATOR_EXPORT jobject Java_org_webrtc_PeerConnection_nativeAddTransceiverOfType(
    JNIEnv* env,
    jobject jcaller,
    jobject mediaType,
    jobject init) {
  return JNI_PeerConnection_AddTransceiverOfType(env, base::android::JavaParamRef<jobject>(env,
      jcaller), base::android::JavaParamRef<jobject>(env, mediaType),
      base::android::JavaParamRef<jobject>(env, init)).Release();
}

static jboolean JNI_PeerConnection_StartRtcEventLog(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller,
    jint file_descriptor,
    jint max_size_bytes);

JNI_GENERATOR_EXPORT jboolean Java_org_webrtc_PeerConnection_nativeStartRtcEventLog(
    JNIEnv* env,
    jobject jcaller,
    jint file_descriptor,
    jint max_size_bytes) {
  return JNI_PeerConnection_StartRtcEventLog(env, base::android::JavaParamRef<jobject>(env,
      jcaller), file_descriptor, max_size_bytes);
}

static void JNI_PeerConnection_StopRtcEventLog(JNIEnv* env, const
    base::android::JavaParamRef<jobject>& jcaller);

JNI_GENERATOR_EXPORT void Java_org_webrtc_PeerConnection_nativeStopRtcEventLog(
    JNIEnv* env,
    jobject jcaller) {
  return JNI_PeerConnection_StopRtcEventLog(env, base::android::JavaParamRef<jobject>(env,
      jcaller));
}


static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024IceGatheringState_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceGatheringState_fromNativeIndex(JNIEnv*
    env, JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_PeerConnection_00024IceGatheringState_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_PeerConnection_00024IceGatheringState_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/PeerConnection$IceGatheringState;",
          &g_org_webrtc_PeerConnection_00024IceGatheringState_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024IceConnectionState_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceConnectionState_fromNativeIndex(JNIEnv*
    env, JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_PeerConnection_00024IceConnectionState_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_PeerConnection_00024IceConnectionState_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/PeerConnection$IceConnectionState;",
          &g_org_webrtc_PeerConnection_00024IceConnectionState_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024PeerConnectionState_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_PeerConnectionState_fromNativeIndex(JNIEnv*
    env, JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_PeerConnection_00024PeerConnectionState_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_PeerConnection_00024PeerConnectionState_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/PeerConnection$PeerConnectionState;",
          &g_org_webrtc_PeerConnection_00024PeerConnectionState_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024SignalingState_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_SignalingState_fromNativeIndex(JNIEnv* env,
    JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_PeerConnection_00024SignalingState_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_PeerConnection_00024SignalingState_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/PeerConnection$SignalingState;",
          &g_org_webrtc_PeerConnection_00024SignalingState_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onSignalingChange(nullptr);
static void Java_Observer_onSignalingChange(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& newState) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onSignalingChange",
          "(Lorg/webrtc/PeerConnection$SignalingState;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onSignalingChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, newState.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onIceConnectionChange(nullptr);
static void Java_Observer_onIceConnectionChange(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& newState) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onIceConnectionChange",
          "(Lorg/webrtc/PeerConnection$IceConnectionState;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onIceConnectionChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, newState.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onStandardizedIceConnectionChange(nullptr);
static void Java_Observer_onStandardizedIceConnectionChange(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& newState) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onStandardizedIceConnectionChange",
          "(Lorg/webrtc/PeerConnection$IceConnectionState;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onStandardizedIceConnectionChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, newState.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onConnectionChange(nullptr);
static void Java_Observer_onConnectionChange(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& newState) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onConnectionChange",
          "(Lorg/webrtc/PeerConnection$PeerConnectionState;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onConnectionChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, newState.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onIceConnectionReceivingChange(nullptr);
static void Java_Observer_onIceConnectionReceivingChange(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, jboolean receiving) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onIceConnectionReceivingChange",
          "(Z)V",
          &g_org_webrtc_PeerConnection_00024Observer_onIceConnectionReceivingChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, receiving);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onIceGatheringChange(nullptr);
static void Java_Observer_onIceGatheringChange(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& newState) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onIceGatheringChange",
          "(Lorg/webrtc/PeerConnection$IceGatheringState;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onIceGatheringChange);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, newState.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onIceCandidate(nullptr);
static void Java_Observer_onIceCandidate(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& candidate) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onIceCandidate",
          "(Lorg/webrtc/IceCandidate;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onIceCandidate);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, candidate.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onIceCandidateError(nullptr);
static void Java_Observer_onIceCandidateError(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobject>& event) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onIceCandidateError",
          "(Lorg/webrtc/IceCandidateErrorEvent;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onIceCandidateError);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, event.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onIceCandidatesRemoved(nullptr);
static void Java_Observer_onIceCandidatesRemoved(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj, const base::android::JavaRef<jobjectArray>& candidates) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onIceCandidatesRemoved",
          "([Lorg/webrtc/IceCandidate;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onIceCandidatesRemoved);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, candidates.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onSelectedCandidatePairChanged(nullptr);
static void Java_Observer_onSelectedCandidatePairChanged(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj, const base::android::JavaRef<jobject>& event) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onSelectedCandidatePairChanged",
          "(Lorg/webrtc/CandidatePairChangeEvent;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onSelectedCandidatePairChanged);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, event.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onAddStream(nullptr);
static void Java_Observer_onAddStream(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& stream) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onAddStream",
          "(Lorg/webrtc/MediaStream;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onAddStream);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, stream.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onRemoveStream(nullptr);
static void Java_Observer_onRemoveStream(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& stream) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onRemoveStream",
          "(Lorg/webrtc/MediaStream;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onRemoveStream);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, stream.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onDataChannel(nullptr);
static void Java_Observer_onDataChannel(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& dataChannel) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onDataChannel",
          "(Lorg/webrtc/DataChannel;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onDataChannel);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, dataChannel.obj());
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024Observer_onRenegotiationNeeded(nullptr);
static void Java_Observer_onRenegotiationNeeded(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onRenegotiationNeeded",
          "()V",
          &g_org_webrtc_PeerConnection_00024Observer_onRenegotiationNeeded);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onAddTrack(nullptr);
static void Java_Observer_onAddTrack(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& receiver,
    const base::android::JavaRef<jobjectArray>& mediaStreams) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onAddTrack",
          "(Lorg/webrtc/RtpReceiver;[Lorg/webrtc/MediaStream;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onAddTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, receiver.obj(), mediaStreams.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onRemoveTrack(nullptr);
static void Java_Observer_onRemoveTrack(JNIEnv* env, const base::android::JavaRef<jobject>& obj,
    const base::android::JavaRef<jobject>& receiver) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onRemoveTrack",
          "(Lorg/webrtc/RtpReceiver;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onRemoveTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, receiver.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024Observer_onTrack(nullptr);
static void Java_Observer_onTrack(JNIEnv* env, const base::android::JavaRef<jobject>& obj, const
    base::android::JavaRef<jobject>& transceiver) {
  jclass clazz = org_webrtc_PeerConnection_00024Observer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024Observer_clazz(env));

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "onTrack",
          "(Lorg/webrtc/RtpTransceiver;)V",
          &g_org_webrtc_PeerConnection_00024Observer_onTrack);

     env->CallVoidMethod(obj.obj(),
          call_context.base.method_id, transceiver.obj());
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024IceServer_getUrls(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceServer_getUrls(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getUrls",
          "()Ljava/util/List;",
          &g_org_webrtc_PeerConnection_00024IceServer_getUrls);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024IceServer_getUsername(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_IceServer_getUsername(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getUsername",
          "()Ljava/lang/String;",
          &g_org_webrtc_PeerConnection_00024IceServer_getUsername);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024IceServer_getPassword(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_IceServer_getPassword(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getPassword",
          "()Ljava/lang/String;",
          &g_org_webrtc_PeerConnection_00024IceServer_getPassword);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024IceServer_getTlsCertPolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceServer_getTlsCertPolicy(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTlsCertPolicy",
          "()Lorg/webrtc/PeerConnection$TlsCertPolicy;",
          &g_org_webrtc_PeerConnection_00024IceServer_getTlsCertPolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024IceServer_getHostname(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_IceServer_getHostname(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getHostname",
          "()Ljava/lang/String;",
          &g_org_webrtc_PeerConnection_00024IceServer_getHostname);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024IceServer_getTlsAlpnProtocols(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceServer_getTlsAlpnProtocols(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTlsAlpnProtocols",
          "()Ljava/util/List;",
          &g_org_webrtc_PeerConnection_00024IceServer_getTlsAlpnProtocols);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024IceServer_getTlsEllipticCurves(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_IceServer_getTlsEllipticCurves(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024IceServer_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024IceServer_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTlsEllipticCurves",
          "()Ljava/util/List;",
          &g_org_webrtc_PeerConnection_00024IceServer_getTlsEllipticCurves);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024AdapterType_fromNativeIndex(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_AdapterType_fromNativeIndex(JNIEnv* env,
    JniIntWrapper nativeIndex) {
  jclass clazz = org_webrtc_PeerConnection_00024AdapterType_clazz(env);
  CHECK_CLAZZ(env, clazz,
      org_webrtc_PeerConnection_00024AdapterType_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_STATIC>(
          env,
          clazz,
          "fromNativeIndex",
          "(I)Lorg/webrtc/PeerConnection$AdapterType;",
          &g_org_webrtc_PeerConnection_00024AdapterType_fromNativeIndex);

  jobject ret =
      env->CallStaticObjectMethod(clazz,
          call_context.base.method_id, as_jint(nativeIndex));
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceTransportsType(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getIceTransportsType(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceTransportsType",
          "()Lorg/webrtc/PeerConnection$IceTransportsType;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceTransportsType);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceServers(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getIceServers(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceServers",
          "()Ljava/util/List;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceServers);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getBundlePolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getBundlePolicy(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getBundlePolicy",
          "()Lorg/webrtc/PeerConnection$BundlePolicy;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getBundlePolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getTurnPortPrunePolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getTurnPortPrunePolicy(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTurnPortPrunePolicy",
          "()Lorg/webrtc/PeerConnection$PortPrunePolicy;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getTurnPortPrunePolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getCertificate(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getCertificate(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCertificate",
          "()Lorg/webrtc/RtcCertificatePem;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getCertificate);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getRtcpMuxPolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getRtcpMuxPolicy(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getRtcpMuxPolicy",
          "()Lorg/webrtc/PeerConnection$RtcpMuxPolicy;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getRtcpMuxPolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getTcpCandidatePolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getTcpCandidatePolicy(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTcpCandidatePolicy",
          "()Lorg/webrtc/PeerConnection$TcpCandidatePolicy;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getTcpCandidatePolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getCandidateNetworkPolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getCandidateNetworkPolicy(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCandidateNetworkPolicy",
          "()Lorg/webrtc/PeerConnection$CandidateNetworkPolicy;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getCandidateNetworkPolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getAudioJitterBufferMaxPackets(nullptr);
static jint Java_RTCConfiguration_getAudioJitterBufferMaxPackets(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getAudioJitterBufferMaxPackets",
          "()I",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getAudioJitterBufferMaxPackets);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getAudioJitterBufferFastAccelerate(nullptr);
static jboolean Java_RTCConfiguration_getAudioJitterBufferFastAccelerate(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getAudioJitterBufferFastAccelerate",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getAudioJitterBufferFastAccelerate);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceConnectionReceivingTimeout(nullptr);
static jint Java_RTCConfiguration_getIceConnectionReceivingTimeout(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceConnectionReceivingTimeout",
          "()I",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceConnectionReceivingTimeout);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceBackupCandidatePairPingInterval(nullptr);
static jint Java_RTCConfiguration_getIceBackupCandidatePairPingInterval(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceBackupCandidatePairPingInterval",
          "()I",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceBackupCandidatePairPingInterval);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_00024RTCConfiguration_getKeyType(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getKeyType(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getKeyType",
          "()Lorg/webrtc/PeerConnection$KeyType;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getKeyType);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getContinualGatheringPolicy(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getContinualGatheringPolicy(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getContinualGatheringPolicy",
          "()Lorg/webrtc/PeerConnection$ContinualGatheringPolicy;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getContinualGatheringPolicy);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCandidatePoolSize(nullptr);
static jint Java_RTCConfiguration_getIceCandidatePoolSize(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceCandidatePoolSize",
          "()I",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCandidatePoolSize);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getPruneTurnPorts(nullptr);
static jboolean Java_RTCConfiguration_getPruneTurnPorts(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getPruneTurnPorts",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getPruneTurnPorts);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getPresumeWritableWhenFullyRelayed(nullptr);
static jboolean Java_RTCConfiguration_getPresumeWritableWhenFullyRelayed(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getPresumeWritableWhenFullyRelayed",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getPresumeWritableWhenFullyRelayed);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getSurfaceIceCandidatesOnIceTransportTypeChanged(nullptr);
static jboolean Java_RTCConfiguration_getSurfaceIceCandidatesOnIceTransportTypeChanged(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSurfaceIceCandidatesOnIceTransportTypeChanged",
          "()Z",
&g_org_webrtc_PeerConnection_00024RTCConfiguration_getSurfaceIceCandidatesOnIceTransportTypeChanged);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCheckIntervalStrongConnectivity(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getIceCheckIntervalStrongConnectivity(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceCheckIntervalStrongConnectivity",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCheckIntervalStrongConnectivity);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCheckIntervalWeakConnectivity(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getIceCheckIntervalWeakConnectivity(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceCheckIntervalWeakConnectivity",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCheckIntervalWeakConnectivity);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCheckMinInterval(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getIceCheckMinInterval(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceCheckMinInterval",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceCheckMinInterval);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceUnwritableTimeout(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getIceUnwritableTimeout(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceUnwritableTimeout",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceUnwritableTimeout);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceUnwritableMinChecks(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getIceUnwritableMinChecks(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getIceUnwritableMinChecks",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getIceUnwritableMinChecks);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getStunCandidateKeepaliveInterval(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getStunCandidateKeepaliveInterval(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getStunCandidateKeepaliveInterval",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getStunCandidateKeepaliveInterval);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getStableWritableConnectionPingIntervalMs(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getStableWritableConnectionPingIntervalMs(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getStableWritableConnectionPingIntervalMs",
          "()Ljava/lang/Integer;",
&g_org_webrtc_PeerConnection_00024RTCConfiguration_getStableWritableConnectionPingIntervalMs);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getDisableIPv6OnWifi(nullptr);
static jboolean Java_RTCConfiguration_getDisableIPv6OnWifi(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDisableIPv6OnWifi",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getDisableIPv6OnWifi);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getMaxIPv6Networks(nullptr);
static jint Java_RTCConfiguration_getMaxIPv6Networks(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getMaxIPv6Networks",
          "()I",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getMaxIPv6Networks);

  jint ret =
      env->CallIntMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getTurnCustomizer(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getTurnCustomizer(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTurnCustomizer",
          "()Lorg/webrtc/TurnCustomizer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getTurnCustomizer);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getDisableIpv6(nullptr);
static jboolean Java_RTCConfiguration_getDisableIpv6(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getDisableIpv6",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getDisableIpv6);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getEnableDscp(nullptr);
static jboolean Java_RTCConfiguration_getEnableDscp(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEnableDscp",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getEnableDscp);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getEnableCpuOveruseDetection(nullptr);
static jboolean Java_RTCConfiguration_getEnableCpuOveruseDetection(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEnableCpuOveruseDetection",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getEnableCpuOveruseDetection);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getSuspendBelowMinBitrate(nullptr);
static jboolean Java_RTCConfiguration_getSuspendBelowMinBitrate(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSuspendBelowMinBitrate",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getSuspendBelowMinBitrate);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getScreencastMinBitrate(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getScreencastMinBitrate(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getScreencastMinBitrate",
          "()Ljava/lang/Integer;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getScreencastMinBitrate);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getCombinedAudioVideoBwe(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getCombinedAudioVideoBwe(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCombinedAudioVideoBwe",
          "()Ljava/lang/Boolean;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getCombinedAudioVideoBwe);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getNetworkPreference(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getNetworkPreference(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNetworkPreference",
          "()Lorg/webrtc/PeerConnection$AdapterType;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getNetworkPreference);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getSdpSemantics(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getSdpSemantics(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getSdpSemantics",
          "()Lorg/webrtc/PeerConnection$SdpSemantics;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getSdpSemantics);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getActiveResetSrtpParams(nullptr);
static jboolean Java_RTCConfiguration_getActiveResetSrtpParams(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getActiveResetSrtpParams",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getActiveResetSrtpParams);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getAllowCodecSwitching(nullptr);
static base::android::ScopedJavaLocalRef<jobject>
    Java_RTCConfiguration_getAllowCodecSwitching(JNIEnv* env, const base::android::JavaRef<jobject>&
    obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getAllowCodecSwitching",
          "()Ljava/lang/Boolean;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getAllowCodecSwitching);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getCryptoOptions(nullptr);
static base::android::ScopedJavaLocalRef<jobject> Java_RTCConfiguration_getCryptoOptions(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getCryptoOptions",
          "()Lorg/webrtc/CryptoOptions;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getCryptoOptions);

  jobject ret =
      env->CallObjectMethod(obj.obj(),
          call_context.base.method_id);
  return base::android::ScopedJavaLocalRef<jobject>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getTurnLoggingId(nullptr);
static base::android::ScopedJavaLocalRef<jstring> Java_RTCConfiguration_getTurnLoggingId(JNIEnv*
    env, const base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), NULL);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getTurnLoggingId",
          "()Ljava/lang/String;",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getTurnLoggingId);

  jstring ret =
      static_cast<jstring>(env->CallObjectMethod(obj.obj(),
          call_context.base.method_id));
  return base::android::ScopedJavaLocalRef<jstring>(env, ret);
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getEnableImplicitRollback(nullptr);
static jboolean Java_RTCConfiguration_getEnableImplicitRollback(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getEnableImplicitRollback",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getEnableImplicitRollback);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID>
    g_org_webrtc_PeerConnection_00024RTCConfiguration_getOfferExtmapAllowMixed(nullptr);
static jboolean Java_RTCConfiguration_getOfferExtmapAllowMixed(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_00024RTCConfiguration_clazz(env), false);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getOfferExtmapAllowMixed",
          "()Z",
          &g_org_webrtc_PeerConnection_00024RTCConfiguration_getOfferExtmapAllowMixed);

  jboolean ret =
      env->CallBooleanMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

static std::atomic<jmethodID> g_org_webrtc_PeerConnection_getNativeOwnedPeerConnection(nullptr);
static jlong Java_PeerConnection_getNativeOwnedPeerConnection(JNIEnv* env, const
    base::android::JavaRef<jobject>& obj) {
  jclass clazz = org_webrtc_PeerConnection_clazz(env);
  CHECK_CLAZZ(env, obj.obj(),
      org_webrtc_PeerConnection_clazz(env), 0);

  jni_generator::JniJavaCallContextChecked call_context;
  call_context.Init<
      base::android::MethodID::TYPE_INSTANCE>(
          env,
          clazz,
          "getNativeOwnedPeerConnection",
          "()J",
          &g_org_webrtc_PeerConnection_getNativeOwnedPeerConnection);

  jlong ret =
      env->CallLongMethod(obj.obj(),
          call_context.base.method_id);
  return ret;
}

}  // namespace jni
}  // namespace  webrtc

#endif  // org_webrtc_PeerConnection_JNI
