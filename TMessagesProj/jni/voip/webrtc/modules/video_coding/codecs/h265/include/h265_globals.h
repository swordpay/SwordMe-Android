/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// order to compile the WebRTC codebase, even if this codec is not used.

#ifndef MODULES_VIDEO_CODING_CODECS_H265_INCLUDE_H265_GLOBALS_H_
#define MODULES_VIDEO_CODING_CODECS_H265_INCLUDE_H265_GLOBALS_H_

#ifndef DISABLE_H265

#include "modules/video_coding/codecs/h264/include/h264_globals.h"

namespace webrtc {

enum H265PacketizationTypes {
  kH265SingleNalu,  // This packet contains a single NAL unit.
  kH265AP,          // This packet contains aggregation Packet.


  kH265FU,          // This packet contains a FU (fragmentation


};

struct H265NaluInfo {
  uint8_t type;
  int vps_id;
  int sps_id;
  int pps_id;
};

enum class H265PacketizationMode {
  NonInterleaved = 0,  // Mode 1 - STAP-A, FU-A is allowed
  SingleNalUnit        // Mode 0 - only single NALU allowed
};

struct RTPVideoHeaderH265 {



  uint8_t nalu_type;
  H265PacketizationTypes packetization_type;
  H265NaluInfo nalus[kMaxNalusPerPacket];
  size_t nalus_length;

  H265PacketizationMode packetization_mode;
};

}  // namespace webrtc

#endif

#endif  // MODULES_VIDEO_CODING_CODECS_H265_INCLUDE_H265_GLOBALS_H_
