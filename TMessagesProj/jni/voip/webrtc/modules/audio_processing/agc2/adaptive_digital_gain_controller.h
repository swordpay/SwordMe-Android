/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_AGC2_ADAPTIVE_DIGITAL_GAIN_CONTROLLER_H_
#define MODULES_AUDIO_PROCESSING_AGC2_ADAPTIVE_DIGITAL_GAIN_CONTROLLER_H_

#include <memory>

#include "absl/types/optional.h"
#include "modules/audio_processing/agc2/adaptive_digital_gain_applier.h"
#include "modules/audio_processing/agc2/noise_level_estimator.h"
#include "modules/audio_processing/agc2/saturation_protector.h"
#include "modules/audio_processing/agc2/speech_level_estimator.h"
#include "modules/audio_processing/include/audio_frame_view.h"
#include "modules/audio_processing/include/audio_processing.h"

namespace webrtc {
class ApmDataDumper;

// target level, which is determined by the given configuration.
class AdaptiveDigitalGainController {
 public:
  AdaptiveDigitalGainController(
      ApmDataDumper* apm_data_dumper,
      const AudioProcessing::Config::GainController2::AdaptiveDigital& config,
      int sample_rate_hz,
      int num_channels);
  AdaptiveDigitalGainController(const AdaptiveDigitalGainController&) = delete;
  AdaptiveDigitalGainController& operator=(
      const AdaptiveDigitalGainController&) = delete;
  ~AdaptiveDigitalGainController();

  void Initialize(int sample_rate_hz, int num_channels);



  void Process(AudioFrameView<float> frame,
               float speech_probability,
               float limiter_envelope);

  void HandleInputGainChange();


  absl::optional<float> GetSpeechLevelDbfsIfConfident() const;

 private:
  SpeechLevelEstimator speech_level_estimator_;
  AdaptiveDigitalGainApplier gain_controller_;
  ApmDataDumper* const apm_data_dumper_;
  std::unique_ptr<NoiseLevelEstimator> noise_level_estimator_;
  std::unique_ptr<SaturationProtector> saturation_protector_;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_AGC2_ADAPTIVE_DIGITAL_GAIN_CONTROLLER_H_
