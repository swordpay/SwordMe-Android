/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_DEVICE_ANDROID_OPENSLES_RECORDER_H_
#define MODULES_AUDIO_DEVICE_ANDROID_OPENSLES_RECORDER_H_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

#include <memory>

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
// will RTC_DCHECK if any method is called on an invalid thread. Recorded audio
// buffers are provided on a dedicated internal thread managed by the OpenSL
// ES layer.
//
// The existing design forces the user to call InitRecording() after
// StopRecording() to be able to call StartRecording() again. This is inline
// with how the Java-based implementation works.
//
// As of API level 21, lower latency audio input is supported on select devices.
// To take advantage of this feature, first confirm that lower latency output is
// available. The capability for lower latency output is a prerequisite for the
// lower latency input feature. Then, create an AudioRecorder with the same
// sample rate and buffer size as would be used for output. OpenSL ES interfaces
// for input effects preclude the lower latency path.
// See https://developer.android.com/ndk/guides/audio/opensl-prog-notes.html
// for more details.
class OpenSLESRecorder {
 public:






  static const int kNumOfOpenSLESBuffers = 2;

  explicit OpenSLESRecorder(AudioManager* audio_manager);
  ~OpenSLESRecorder();

  int Init();
  int Terminate();

  int InitRecording();
  bool RecordingIsInitialized() const { return initialized_; }

  int StartRecording();
  int StopRecording();
  bool Recording() const { return recording_; }

  void AttachAudioBuffer(AudioDeviceBuffer* audio_buffer);

  int EnableBuiltInAEC(bool enable);
  int EnableBuiltInAGC(bool enable);
  int EnableBuiltInNS(bool enable);

 private:



  bool ObtainEngineInterface();

  bool CreateAudioRecorder();
  void DestroyAudioRecorder();


  void AllocateDataBuffers();



  static void SimpleBufferQueueCallback(SLAndroidSimpleBufferQueueItf caller,
                                        void* context);
  void ReadBufferQueue();





  bool EnqueueAudioBuffer();

  SLuint32 GetRecordState() const;

  SLAndroidSimpleBufferQueueState GetBufferQueueState() const;

  SLuint32 GetBufferCount();


  void LogBufferState() const;


  SequenceChecker thread_checker_;



  SequenceChecker thread_checker_opensles_;




  AudioManager* const audio_manager_;


  const AudioParameters audio_parameters_;


  AudioDeviceBuffer* audio_device_buffer_;



  SLDataFormat_PCM pcm_format_;

  bool initialized_;
  bool recording_;


  SLEngineItf engine_;


  webrtc::ScopedSLObjectItf recorder_object_;


  SLRecordItf recorder_;




  SLAndroidSimpleBufferQueueItf simple_buffer_queue_;


  std::unique_ptr<FineAudioBuffer> fine_audio_buffer_;




  std::unique_ptr<std::unique_ptr<SLint16[]>[]> audio_buffers_;


  int buffer_index_;

  uint32_t last_rec_time_;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_ANDROID_OPENSLES_RECORDER_H_
