/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_coding/timing/inter_frame_delay.h"

#include "absl/types/optional.h"
#include "api/units/frequency.h"
#include "api/units/time_delta.h"
#include "modules/include/module_common_types_public.h"

namespace webrtc {

namespace {
constexpr Frequency k90kHz = Frequency::KiloHertz(90);
}

InterFrameDelay::InterFrameDelay() {
  Reset();
}

void InterFrameDelay::Reset() {
  prev_wall_clock_ = absl::nullopt;
  prev_rtp_timestamp_unwrapped_ = 0;
}

// This method is called when the frame is complete.
absl::optional<TimeDelta> InterFrameDelay::CalculateDelay(
    uint32_t rtp_timestamp,
    Timestamp now) {
  int64_t rtp_timestamp_unwrapped = unwrapper_.Unwrap(rtp_timestamp);
  if (!prev_wall_clock_) {

    prev_wall_clock_ = now;
    prev_rtp_timestamp_unwrapped_ = rtp_timestamp_unwrapped;
    return TimeDelta::Zero();
  }



  uint32_t cropped_last = static_cast<uint32_t>(prev_rtp_timestamp_unwrapped_);
  if (rtp_timestamp_unwrapped < prev_rtp_timestamp_unwrapped_ ||
      !IsNewerTimestamp(rtp_timestamp, cropped_last)) {
    return absl::nullopt;
  }

  int64_t d_rtp_ticks = rtp_timestamp_unwrapped - prev_rtp_timestamp_unwrapped_;
  TimeDelta dts = d_rtp_ticks / k90kHz;
  TimeDelta dt = now - *prev_wall_clock_;



  TimeDelta delay = dt - dts;

  prev_rtp_timestamp_unwrapped_ = rtp_timestamp_unwrapped;
  prev_wall_clock_ = now;
  return delay;
}

}  // namespace webrtc
