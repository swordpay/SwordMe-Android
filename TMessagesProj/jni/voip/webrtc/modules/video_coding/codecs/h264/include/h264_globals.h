/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// order to compile the WebRTC codebase, even if this codec is not used.

#ifndef MODULES_VIDEO_CODING_CODECS_H264_INCLUDE_H264_GLOBALS_H_
#define MODULES_VIDEO_CODING_CODECS_H264_INCLUDE_H264_GLOBALS_H_

#include <string>

#include "modules/video_coding/codecs/interface/common_constants.h"
#include "rtc_base/checks.h"

namespace webrtc {

enum H264PacketizationTypes {
  kH264SingleNalu,  // This packet contains a single NAL unit.
  kH264StapA,       // This packet contains STAP-A (single time



  kH264FuA,         // This packet contains a FU-A (fragmentation


};

// Due to the structure containing this being initialized with zeroes
// in some places, and mode 1 being default, mode 1 needs to have the value
// zero. https://crbug.com/webrtc/6803
enum class H264PacketizationMode {
  NonInterleaved = 0,  // Mode 1 - STAP-A, FU-A is allowed
  SingleNalUnit        // Mode 0 - only single NALU allowed
};

// .cc file it should belong to.
// TODO(hta): Refactor. https://bugs.webrtc.org/6842
// TODO(jonasolsson): Use absl::string_view instead when that's available.
inline std::string ToString(H264PacketizationMode mode) {
  if (mode == H264PacketizationMode::NonInterleaved) {
    return "NonInterleaved";
  } else if (mode == H264PacketizationMode::SingleNalUnit) {
    return "SingleNalUnit";
  }
  RTC_DCHECK_NOTREACHED();
  return "";
}

struct NaluInfo {
  uint8_t type;
  int sps_id;
  int pps_id;
};

const size_t kMaxNalusPerPacket = 10;

struct RTPVideoHeaderH264 {





  uint8_t nalu_type;

  H264PacketizationTypes packetization_type;
  NaluInfo nalus[kMaxNalusPerPacket];
  size_t nalus_length;


  H264PacketizationMode packetization_mode;
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_CODECS_H264_INCLUDE_H264_GLOBALS_H_
