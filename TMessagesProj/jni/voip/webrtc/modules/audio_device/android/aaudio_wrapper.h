/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_DEVICE_ANDROID_AAUDIO_WRAPPER_H_
#define MODULES_AUDIO_DEVICE_ANDROID_AAUDIO_WRAPPER_H_

#include <aaudio/AAudio.h>

#include "api/sequence_checker.h"
#include "modules/audio_device/include/audio_device_defines.h"

namespace webrtc {

class AudioManager;

// The interface also contains an error callback method for notifications of
// e.g. device changes.
class AAudioObserverInterface {
 public:



  virtual aaudio_data_callback_result_t OnDataCallback(void* audio_data,
                                                       int32_t num_frames) = 0;



  virtual void OnErrorCallback(aaudio_result_t error) = 0;

 protected:
  virtual ~AAudioObserverInterface() {}
};

// where the underlying resources (AAudioStreamBuilder and AAudioStream) are
// encapsulated. User must set the direction (in or out) at construction since
// it defines the stream type and the direction of the data flow in the
// AAudioObserverInterface.
//
// AAudio is a new Android C API introduced in the Android O (26) release.
// It is designed for high-performance audio applications that require low
// latency. Applications communicate with AAudio by reading and writing data
// to streams.
//
// Each stream is attached to a single audio device, where each audio device
// has a unique ID. The ID can be used to bind an audio stream to a specific
// audio device but this implementation lets AAudio choose the default primary
// device instead (device selection takes place in Java). A stream can only
// move data in one direction. When a stream is opened, Android checks to
// ensure that the audio device and stream direction agree.
class AAudioWrapper {
 public:
  AAudioWrapper(AudioManager* audio_manager,
                aaudio_direction_t direction,
                AAudioObserverInterface* observer);
  ~AAudioWrapper();

  bool Init();
  bool Start();
  bool Stop();






  double EstimateLatencyMillis() const;


  bool IncreaseOutputBufferSize();



  void ClearInputStream(void* audio_data, int32_t num_frames);

  AAudioObserverInterface* observer() const;
  AudioParameters audio_parameters() const;
  int32_t samples_per_frame() const;
  int32_t buffer_size_in_frames() const;
  int32_t buffer_capacity_in_frames() const;
  int32_t device_id() const;
  int32_t xrun_count() const;
  int32_t format() const;
  int32_t sample_rate() const;
  int32_t channel_count() const;
  int32_t frames_per_callback() const;
  aaudio_sharing_mode_t sharing_mode() const;
  aaudio_performance_mode_t performance_mode() const;
  aaudio_stream_state_t stream_state() const;
  int64_t frames_written() const;
  int64_t frames_read() const;
  aaudio_direction_t direction() const { return direction_; }
  AAudioStream* stream() const { return stream_; }
  int32_t frames_per_burst() const { return frames_per_burst_; }

 private:
  void SetStreamConfiguration(AAudioStreamBuilder* builder);
  bool OpenStream(AAudioStreamBuilder* builder);
  void CloseStream();
  void LogStreamConfiguration();
  void LogStreamState();
  bool VerifyStreamConfiguration();
  bool OptimizeBuffers();

  SequenceChecker thread_checker_;
  SequenceChecker aaudio_thread_checker_;
  AudioParameters audio_parameters_;
  const aaudio_direction_t direction_;
  AAudioObserverInterface* observer_ = nullptr;
  AAudioStream* stream_ = nullptr;
  int32_t frames_per_burst_ = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_ANDROID_AAUDIO_WRAPPER_H_
