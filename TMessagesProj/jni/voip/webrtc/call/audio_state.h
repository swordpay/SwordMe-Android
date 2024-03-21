/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef CALL_AUDIO_STATE_H_
#define CALL_AUDIO_STATE_H_

#include "api/audio/audio_mixer.h"
#include "api/scoped_refptr.h"
#include "modules/async_audio_processing/async_audio_processing.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

class AudioTransport;

// webrtc::Call for audio processing purposes.
class AudioState : public rtc::RefCountInterface {
 public:
  struct Config {
    Config();
    ~Config();


    rtc::scoped_refptr<AudioMixer> audio_mixer;

    rtc::scoped_refptr<webrtc::AudioProcessing> audio_processing;

    rtc::scoped_refptr<webrtc::AudioDeviceModule> audio_device_module;

    rtc::scoped_refptr<AsyncAudioProcessing::Factory>
        async_audio_processing_factory;
  };

  virtual AudioProcessing* audio_processing() = 0;
  virtual AudioTransport* audio_transport() = 0;




  virtual void SetPlayout(bool enabled) = 0;



  virtual void SetRecording(bool enabled) = 0;

  virtual void SetStereoChannelSwapping(bool enable) = 0;

  static rtc::scoped_refptr<AudioState> Create(
      const AudioState::Config& config);

  ~AudioState() override {}
};
}  // namespace webrtc

#endif  // CALL_AUDIO_STATE_H_
