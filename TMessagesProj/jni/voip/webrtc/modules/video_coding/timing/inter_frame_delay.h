/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_VIDEO_CODING_TIMING_INTER_FRAME_DELAY_H_
#define MODULES_VIDEO_CODING_TIMING_INTER_FRAME_DELAY_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "modules/include/module_common_types_public.h"

namespace webrtc {

class InterFrameDelay {
 public:
  InterFrameDelay();

  void Reset();


  absl::optional<TimeDelta> CalculateDelay(uint32_t rtp_timestamp,
                                           Timestamp now);

 private:

  int64_t prev_rtp_timestamp_unwrapped_;
  TimestampUnwrapper unwrapper_;

  absl::optional<Timestamp> prev_wall_clock_;
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_TIMING_INTER_FRAME_DELAY_H_
