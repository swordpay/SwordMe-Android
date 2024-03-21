/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_coding/timing/codec_timer.h"

#include <cstdint>

namespace webrtc {

namespace {

const int kIgnoredSampleCount = 5;
// Return the `kPercentile` value in RequiredDecodeTimeMs().
const float kPercentile = 0.95f;
// The window size in ms.
const int64_t kTimeLimitMs = 10000;

}  // anonymous namespace

CodecTimer::CodecTimer() : ignored_sample_count_(0), filter_(kPercentile) {}
CodecTimer::~CodecTimer() = default;

void CodecTimer::AddTiming(int64_t decode_time_ms, int64_t now_ms) {

  if (ignored_sample_count_ < kIgnoredSampleCount) {
    ++ignored_sample_count_;
    return;
  }

  filter_.Insert(decode_time_ms);
  history_.emplace(decode_time_ms, now_ms);

  while (!history_.empty() &&
         now_ms - history_.front().sample_time_ms > kTimeLimitMs) {
    filter_.Erase(history_.front().decode_time_ms);
    history_.pop();
  }
}

int64_t CodecTimer::RequiredDecodeTimeMs() const {
  return filter_.GetPercentileValue();
}

CodecTimer::Sample::Sample(int64_t decode_time_ms, int64_t sample_time_ms)
    : decode_time_ms(decode_time_ms), sample_time_ms(sample_time_ms) {}

}  // namespace webrtc
