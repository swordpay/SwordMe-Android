/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_AGC2_BIQUAD_FILTER_H_
#define MODULES_AUDIO_PROCESSING_AGC2_BIQUAD_FILTER_H_

#include "api/array_view.h"

namespace webrtc {

//        b[0] + b[1] • z^(-1) + b[2] • z^(-2)
// H(z) = ------------------------------------
//          1 + a[1] • z^(-1) + a[2] • z^(-2)
class BiQuadFilter {
 public:


  struct Config {
    float b[3];  // b[0], b[1], b[2].
    float a[2];  // a[1], a[2].
  };

  explicit BiQuadFilter(const Config& config);
  BiQuadFilter(const BiQuadFilter&) = delete;
  BiQuadFilter& operator=(const BiQuadFilter&) = delete;
  ~BiQuadFilter();

  void SetConfig(const Config& config);

  void Reset();


  void Process(rtc::ArrayView<const float> x, rtc::ArrayView<float> y);

 private:
  Config config_;
  struct State {
    float b[2];
    float a[2];
  } state_;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_AGC2_BIQUAD_FILTER_H_
