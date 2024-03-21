/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SDK_ANDROID_SRC_JNI_AUDIO_DEVICE_AAUDIO_PLAYER_H_
#define SDK_ANDROID_SRC_JNI_AUDIO_DEVICE_AAUDIO_PLAYER_H_

#include <aaudio/AAudio.h>

#include <memory>

#include "absl/types/optional.h"
#include "api/sequence_checker.h"
#include "api/task_queue/task_queue_base.h"
#include "modules/audio_device/audio_device_buffer.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "rtc_base/thread_annotations.h"
#include "sdk/android/src/jni/audio_device/aaudio_wrapper.h"
#include "sdk/android/src/jni/audio_device/audio_device_module.h"

namespace webrtc {

class AudioDeviceBuffer;
class FineAudioBuffer;

namespace jni {

// using the C based AAudio API.
//
// An instance must be created and destroyed on one and the same thread.
// All public methods must also be called on the same thread. A thread checker
// will DCHECK if any method is called on an invalid thread. Audio buffers
// are requested on a dedicated high-priority thread owned by AAudio.
//
// The existing design forces the user to call InitPlayout() after StopPlayout()
// to be able to call StartPlayout() again. This is in line with how the Java-
// based implementation works.
//
// An audio stream can be disconnected, e.g. when an audio device is removed.
// This implementation will restart the audio stream using the new preferred
// device if such an event happens.
//
// Also supports automatic buffer-size adjustment based on underrun detections
// where the internal AAudio buffer can be increased when needed. It will
// reduce the risk of underruns (~glitches) at the expense of an increased
// latency.
class AAudioPlayer final : public AudioOutput, public AAudioObserverInterface {
 public:
  explicit AAudioPlayer(const AudioParameters& audio_parameters);
  ~AAudioPlayer() override;

  int Init() override;
  int Terminate() override;

  int InitPlayout() override;
  bool PlayoutIsInitialized() const override;

  int StartPlayout() override;
  int StopPlayout() override;
  bool Playing() const override;

  void AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) override;

  bool SpeakerVolumeIsAvailable() override;
  int SetSpeakerVolume(uint32_t volume) override;
  absl::optional<uint32_t> SpeakerVolume() const override;
  absl::optional<uint32_t> MaxSpeakerVolume() const override;
  absl::optional<uint32_t> MinSpeakerVolume() const override;

 protected:




  aaudio_data_callback_result_t OnDataCallback(void* audio_data,
                                               int32_t num_frames) override;


  void OnErrorCallback(aaudio_result_t error) override;

 private:

  int GetPlayoutUnderrunCount() override { return 0; }

  void HandleStreamDisconnected();


  SequenceChecker main_thread_checker_;



  SequenceChecker thread_checker_aaudio_;

  TaskQueueBase* main_thread_;




  AAudioWrapper aaudio_;










  std::unique_ptr<FineAudioBuffer> fine_audio_buffer_;

  int32_t underrun_count_ = 0;

  bool first_data_callback_ = true;


  AudioDeviceBuffer* audio_device_buffer_ RTC_GUARDED_BY(main_thread_checker_) =
      nullptr;

  bool initialized_ RTC_GUARDED_BY(main_thread_checker_) = false;
  bool playing_ RTC_GUARDED_BY(main_thread_checker_) = false;


  double latency_millis_ RTC_GUARDED_BY(thread_checker_aaudio_) = 0;
};

}  // namespace jni

}  // namespace webrtc

#endif  // SDK_ANDROID_SRC_JNI_AUDIO_DEVICE_AAUDIO_PLAYER_H_
