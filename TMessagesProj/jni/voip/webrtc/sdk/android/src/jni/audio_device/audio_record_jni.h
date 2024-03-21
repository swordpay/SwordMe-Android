/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SDK_ANDROID_SRC_JNI_AUDIO_DEVICE_AUDIO_RECORD_JNI_H_
#define SDK_ANDROID_SRC_JNI_AUDIO_DEVICE_AUDIO_RECORD_JNI_H_

#include <jni.h>

#include <memory>

#include "api/sequence_checker.h"
#include "modules/audio_device/audio_device_buffer.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "sdk/android/src/jni/audio_device/audio_device_module.h"

namespace webrtc {

namespace jni {

// AudioRecord interface. Most of the work is done by its Java counterpart in
// WebRtcAudioRecord.java. This class is created and lives on a thread in
// C++-land, but recorded audio buffers are delivered on a high-priority
// thread managed by the Java class.
//
// The Java class makes use of AudioEffect features (mainly AEC) which are
// first available in Jelly Bean. If it is instantiated running against earlier
// SDKs, the AEC provided by the APM in WebRTC must be used and enabled
// separately instead.
//
// An instance can be created on any thread, but must then be used on one and
// the same thread. All public methods must also be called on the same thread. A
// thread checker will RTC_DCHECK if any method is called on an invalid thread.
//
// This class uses AttachCurrentThreadIfNeeded to attach to a Java VM if needed.
// Additional thread checking guarantees that no other (possibly non attached)
// thread is used.
class AudioRecordJni : public AudioInput {
 public:
  static ScopedJavaLocalRef<jobject> CreateJavaWebRtcAudioRecord(
      JNIEnv* env,
      const JavaRef<jobject>& j_context,
      const JavaRef<jobject>& j_audio_manager);

  AudioRecordJni(JNIEnv* env,
                 const AudioParameters& audio_parameters,
                 int total_delay_ms,
                 const JavaRef<jobject>& j_webrtc_audio_record);
  ~AudioRecordJni() override;

  int32_t Init() override;
  int32_t Terminate() override;

  int32_t InitRecording() override;
  bool RecordingIsInitialized() const override;

  int32_t StartRecording() override;
  int32_t StopRecording() override;
  bool Recording() const override;

  void AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) override;

  bool IsAcousticEchoCancelerSupported() const override;
  bool IsNoiseSuppressorSupported() const override;

  int32_t EnableBuiltInAEC(bool enable) override;
  int32_t EnableBuiltInNS(bool enable) override;





  void CacheDirectBufferAddress(JNIEnv* env,
                                const JavaParamRef<jobject>& j_caller,
                                const JavaParamRef<jobject>& byte_buffer);






  void DataIsRecorded(JNIEnv* env,
                      const JavaParamRef<jobject>& j_caller,
                      int length);

 private:

  SequenceChecker thread_checker_;


  SequenceChecker thread_checker_java_;

  JNIEnv* env_ = nullptr;
  ScopedJavaGlobalRef<jobject> j_audio_record_;

  const AudioParameters audio_parameters_;



  const int total_delay_ms_;

  void* direct_buffer_address_;

  size_t direct_buffer_capacity_in_bytes_;




  size_t frames_per_buffer_;

  bool initialized_;

  bool recording_;


  AudioDeviceBuffer* audio_device_buffer_;
};

}  // namespace jni

}  // namespace webrtc

#endif  // SDK_ANDROID_SRC_JNI_AUDIO_DEVICE_AUDIO_RECORD_JNI_H_
