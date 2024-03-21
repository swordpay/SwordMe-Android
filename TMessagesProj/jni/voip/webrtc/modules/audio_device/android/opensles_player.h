/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_DEVICE_ANDROID_OPENSLES_PLAYER_H_
#define MODULES_AUDIO_DEVICE_ANDROID_OPENSLES_PLAYER_H_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

#include "api/sequence_checker.h"
#include "modules/audio_device/android/audio_common.h"
#include "modules/audio_device/android/audio_manager.h"
#include "modules/audio_device/android/opensles_common.h"
#include "modules/audio_device/audio_device_generic.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "modules/utility/include/helpers_android.h"

namespace webrtc {

class FineAudioBuffer;

// C based OpenSL ES API. No calls from C/C++ to Java using JNI is done.
//
// An instance must be created and destroyed on one and the same thread.
// All public methods must also be called on the same thread. A thread checker
// will RTC_DCHECK if any method is called on an invalid thread. Decoded audio
// buffers are requested on a dedicated internal thread managed by the OpenSL
// ES layer.
//
// The existing design forces the user to call InitPlayout() after Stoplayout()
// to be able to call StartPlayout() again. This is inline with how the Java-
// based implementation works.
//
// OpenSL ES is a native C API which have no Dalvik-related overhead such as
// garbage collection pauses and it supports reduced audio output latency.
// If the device doesn't claim this feature but supports API level 9 (Android
// platform version 2.3) or later, then we can still use the OpenSL ES APIs but
// the output latency may be higher.
class OpenSLESPlayer {
 public:






  static const int kNumOfOpenSLESBuffers = 2;

  explicit OpenSLESPlayer(AudioManager* audio_manager);
  ~OpenSLESPlayer();

  int Init();
  int Terminate();

  int InitPlayout();
  bool PlayoutIsInitialized() const { return initialized_; }

  int StartPlayout();
  int StopPlayout();
  bool Playing() const { return playing_; }

  int SpeakerVolumeIsAvailable(bool& available);
  int SetSpeakerVolume(uint32_t volume);
  int SpeakerVolume(uint32_t& volume) const;
  int MaxSpeakerVolume(uint32_t& maxVolume) const;
  int MinSpeakerVolume(uint32_t& minVolume) const;

  void AttachAudioBuffer(AudioDeviceBuffer* audioBuffer);

 private:



  static void SimpleBufferQueueCallback(SLAndroidSimpleBufferQueueItf caller,
                                        void* context);
  void FillBufferQueue();






  void EnqueuePlayoutData(bool silence);


  void AllocateDataBuffers();



  bool ObtainEngineInterface();

  bool CreateMix();
  void DestroyMix();


  bool CreateAudioPlayer();
  void DestroyAudioPlayer();

  SLuint32 GetPlayState() const;


  SequenceChecker thread_checker_;



  SequenceChecker thread_checker_opensles_;




  AudioManager* audio_manager_;


  const AudioParameters audio_parameters_;


  AudioDeviceBuffer* audio_device_buffer_;

  bool initialized_;
  bool playing_;



  SLDataFormat_PCM pcm_format_;


  std::unique_ptr<SLint16[]> audio_buffers_[kNumOfOpenSLESBuffers];










  std::unique_ptr<FineAudioBuffer> fine_audio_buffer_;


  int buffer_index_;


  SLEngineItf engine_;

  webrtc::ScopedSLObjectItf output_mix_;


  webrtc::ScopedSLObjectItf player_object_;


  SLPlayItf player_;



  SLAndroidSimpleBufferQueueItf simple_buffer_queue_;


  SLVolumeItf volume_;

  uint32_t last_play_time_;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_ANDROID_OPENSLES_PLAYER_H_
