/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_AUDIO_DEVICE_INCLUDE_TEST_AUDIO_DEVICE_H_
#define MODULES_AUDIO_DEVICE_INCLUDE_TEST_AUDIO_DEVICE_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "api/array_view.h"
#include "api/scoped_refptr.h"
#include "api/task_queue/task_queue_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "rtc_base/buffer.h"

namespace webrtc {

// capturer and a renderer. It will use 10ms audio frames.
class TestAudioDeviceModule : public AudioDeviceModule {
 public:


  static size_t SamplesPerFrame(int sampling_frequency_in_hz);

  class Capturer {
   public:
    virtual ~Capturer() {}


    virtual int SamplingFrequency() const = 0;

    virtual int NumChannels() const = 0;



    virtual bool Capture(rtc::BufferT<int16_t>* buffer) = 0;
  };

  class Renderer {
   public:
    virtual ~Renderer() {}


    virtual int SamplingFrequency() const = 0;

    virtual int NumChannels() const = 0;


    virtual bool Render(rtc::ArrayView<const int16_t> data) = 0;
  };


  class PulsedNoiseCapturer : public Capturer {
   public:
    ~PulsedNoiseCapturer() override {}

    virtual void SetMaxAmplitude(int16_t amplitude) = 0;
  };

  ~TestAudioDeviceModule() override {}







  static rtc::scoped_refptr<TestAudioDeviceModule> Create(
      TaskQueueFactory* task_queue_factory,
      std::unique_ptr<Capturer> capturer,
      std::unique_ptr<Renderer> renderer,
      float speed = 1);



  static std::unique_ptr<PulsedNoiseCapturer> CreatePulsedNoiseCapturer(
      int16_t max_amplitude,
      int sampling_frequency_in_hz,
      int num_channels = 1);

  static std::unique_ptr<Renderer> CreateDiscardRenderer(
      int sampling_frequency_in_hz,
      int num_channels = 1);



  static std::unique_ptr<Capturer> CreateWavFileReader(
      absl::string_view filename,
      int sampling_frequency_in_hz,
      int num_channels = 1);




  static std::unique_ptr<Capturer> CreateWavFileReader(
      absl::string_view filename,
      bool repeat = false);

  static std::unique_ptr<Renderer> CreateWavFileWriter(
      absl::string_view filename,
      int sampling_frequency_in_hz,
      int num_channels = 1);



  static std::unique_ptr<Renderer> CreateBoundedWavFileWriter(
      absl::string_view filename,
      int sampling_frequency_in_hz,
      int num_channels = 1);

  int32_t Init() override = 0;
  int32_t RegisterAudioCallback(AudioTransport* callback) override = 0;

  int32_t StartPlayout() override = 0;
  int32_t StopPlayout() override = 0;
  int32_t StartRecording() override = 0;
  int32_t StopRecording() override = 0;

  bool Playing() const override = 0;
  bool Recording() const override = 0;

  virtual void WaitForRecordingEnd() = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_INCLUDE_TEST_AUDIO_DEVICE_H_
