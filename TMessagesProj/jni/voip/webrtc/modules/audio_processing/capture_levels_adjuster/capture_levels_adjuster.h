/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_AUDIO_PROCESSING_CAPTURE_LEVELS_ADJUSTER_CAPTURE_LEVELS_ADJUSTER_H_
#define MODULES_AUDIO_PROCESSING_CAPTURE_LEVELS_ADJUSTER_CAPTURE_LEVELS_ADJUSTER_H_

#include <stddef.h>

#include "modules/audio_processing/audio_buffer.h"
#include "modules/audio_processing/capture_levels_adjuster/audio_samples_scaler.h"

namespace webrtc {

// processing is done using a combination of explicitly specified gains
// and an emulated analog gain functionality where a specified analog level
// results in an additional gain. The pre-adjustment is achieved by combining
// the gain value `pre_gain` and the level `emulated_analog_mic_gain_level` to
// form a combined gain of `pre_gain`*`emulated_analog_mic_gain_level`/255 which
// is multiplied to each sample. The intention of the
// `emulated_analog_mic_gain_level` is to be controlled by the analog AGC
// functionality and to produce an emulated analog mic gain equal to
// `emulated_analog_mic_gain_level`/255. The post level adjustment is achieved
// by multiplying each sample with the value of `post_gain`. Any changes in the
// gains take are done smoothly over one frame and the scaled samples are
// clamped to fit into the allowed S16 sample range.
class CaptureLevelsAdjuster {
 public:




  CaptureLevelsAdjuster(bool emulated_analog_mic_gain_enabled,
                        int emulated_analog_mic_gain_level,
                        float pre_gain,
                        float post_gain);
  CaptureLevelsAdjuster(const CaptureLevelsAdjuster&) = delete;
  CaptureLevelsAdjuster& operator=(const CaptureLevelsAdjuster&) = delete;


  void ApplyPreLevelAdjustment(AudioBuffer& audio_buffer);


  void ApplyPostLevelAdjustment(AudioBuffer& audio_buffer);


  void SetPreGain(float pre_gain);



  float GetPreAdjustmentGain() const { return pre_adjustment_gain_; }


  void SetPostGain(float post_gain);


  void SetAnalogMicGainLevel(int level);

  int GetAnalogMicGainLevel() const { return emulated_analog_mic_gain_level_; }

 private:


  void UpdatePreAdjustmentGain();

  const bool emulated_analog_mic_gain_enabled_;
  int emulated_analog_mic_gain_level_;
  float pre_gain_;
  float pre_adjustment_gain_;
  AudioSamplesScaler pre_scaler_;
  AudioSamplesScaler post_scaler_;
};
}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_CAPTURE_LEVELS_ADJUSTER_CAPTURE_LEVELS_ADJUSTER_H_
