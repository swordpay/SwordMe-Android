/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_AGC2_CLIPPING_PREDICTOR_H_
#define MODULES_AUDIO_PROCESSING_AGC2_CLIPPING_PREDICTOR_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "modules/audio_processing/include/audio_frame_view.h"
#include "modules/audio_processing/include/audio_processing.h"

namespace webrtc {

// 10 ms multi-channel frames and estimates an analog mic level decrease step
// to possibly avoid clipping when predicted. `Analyze()` and
// `EstimateClippedLevelStep()` can be called in any order.
class ClippingPredictor {
 public:
  virtual ~ClippingPredictor() = default;

  virtual void Reset() = 0;

  virtual void Analyze(const AudioFrameView<const float>& frame) = 0;







  virtual absl::optional<int> EstimateClippedLevelStep(
      int channel,
      int level,
      int default_step,
      int min_mic_level,
      int max_mic_level) const = 0;
};

// the following must hold for `config`:
// `window_length < reference_window_length + reference_window_delay`.
// Returns `nullptr` if `config.enabled` is false.
std::unique_ptr<ClippingPredictor> CreateClippingPredictor(
    int num_channels,
    const AudioProcessing::Config::GainController1::AnalogGainController::
        ClippingPredictor& config);

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_AGC2_CLIPPING_PREDICTOR_H_
