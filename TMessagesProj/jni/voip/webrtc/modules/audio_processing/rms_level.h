/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_RMS_LEVEL_H_
#define MODULES_AUDIO_PROCESSING_RMS_LEVEL_H_

#include <stddef.h>
#include <stdint.h>

#include "absl/types/optional.h"
#include "api/array_view.h"

namespace webrtc {

// full-scale) of audio data. The computation follows RFC 6465:
// https://tools.ietf.org/html/rfc6465
// with the intent that it can provide the RTP audio level indication.
//
// The expected approach is to provide constant-sized chunks of audio to
// Analyze(). When enough chunks have been accumulated to form a packet, call
// Average() to get the audio level indicator for the RTP header.
class RmsLevel {
 public:
  struct Levels {
    int average;
    int peak;
  };

  enum : int { kMinLevelDb = 127, kInaudibleButNotMuted = 126 };

  RmsLevel();
  ~RmsLevel();


  void Reset();

  void Analyze(rtc::ArrayView<const int16_t> data);
  void Analyze(rtc::ArrayView<const float> data);


  void AnalyzeMuted(size_t length);




  int Average();


  Levels AverageAndPeak();

 private:


  void CheckBlockSize(size_t block_size);

  float sum_square_;
  size_t sample_count_;
  float max_sum_square_;
  absl::optional<size_t> block_size_;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_RMS_LEVEL_H_
