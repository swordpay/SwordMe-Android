/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_VIDEO_CODING_INCLUDE_VIDEO_CODEC_INTERFACE_H_
#define MODULES_VIDEO_CODING_INCLUDE_VIDEO_CODEC_INTERFACE_H_

#include <vector>

#include "absl/base/attributes.h"
#include "absl/types/optional.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/video_decoder.h"
#include "api/video_codecs/video_encoder.h"
#include "common_video/generic_frame_descriptor/generic_frame_info.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#ifndef DISABLE_H265
#include "modules/video_coding/codecs/h265/include/h265_globals.h"
#endif
#include "modules/video_coding/codecs/vp9/include/vp9_globals.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

// with a copy-constructor. See below.
// Hack alert - the code assumes that thisstruct is memset when constructed.
struct CodecSpecificInfoVP8 {
  bool nonReference;
  uint8_t temporalIdx;
  bool layerSync;
  int8_t keyIdx;  // Negative value to skip keyIdx.








  bool useExplicitDependencies;
  static constexpr size_t kBuffersCount = 3;
  size_t referencedBuffers[kBuffersCount];
  size_t referencedBuffersCount;
  size_t updatedBuffers[kBuffersCount];
  size_t updatedBuffersCount;
};
static_assert(std::is_pod<CodecSpecificInfoVP8>::value, "");

struct CodecSpecificInfoVP9 {
  bool first_frame_in_picture;  // First frame, increment picture_id.
  bool inter_pic_predicted;     // This layer frame is dependent on previously

  bool flexible_mode;
  bool ss_data_available;
  bool non_ref_for_inter_layer_pred;

  uint8_t temporal_idx;
  bool temporal_up_switch;
  bool inter_layer_predicted;  // Frame is dependent on directly lower spatial

  uint8_t gof_idx;

  size_t num_spatial_layers;  // Always populated.
  size_t first_active_layer;
  bool spatial_layer_resolution_present;
  uint16_t width[kMaxVp9NumberOfSpatialLayers];
  uint16_t height[kMaxVp9NumberOfSpatialLayers];
  GofInfoVP9 gof;

  uint8_t num_ref_pics;
  uint8_t p_diff[kMaxVp9RefPics];

  ABSL_DEPRECATED("") bool end_of_picture;
};
static_assert(std::is_pod<CodecSpecificInfoVP9>::value, "");

struct CodecSpecificInfoH264 {
  H264PacketizationMode packetization_mode;
  uint8_t temporal_idx;
  bool base_layer_sync;
  bool idr_frame;
};

#ifndef DISABLE_H265
struct CodecSpecificInfoH265 {
  H265PacketizationMode packetization_mode;
  bool idr_frame;
};
#endif

static_assert(std::is_pod<CodecSpecificInfoH264>::value, "");

union CodecSpecificInfoUnion {
  CodecSpecificInfoVP8 VP8;
  CodecSpecificInfoVP9 VP9;
  CodecSpecificInfoH264 H264;
#ifndef DISABLE_H265
  CodecSpecificInfoH265 H265;
#endif
};
static_assert(std::is_pod<CodecSpecificInfoUnion>::value, "");

// must be fitted with a copy-constructor. This is because it is copied
// in the copy-constructor of VCMEncodedFrame.
struct RTC_EXPORT CodecSpecificInfo {
  CodecSpecificInfo();
  CodecSpecificInfo(const CodecSpecificInfo&);
  ~CodecSpecificInfo();

  VideoCodecType codecType;
  CodecSpecificInfoUnion codecSpecific;
  bool end_of_picture = true;
  absl::optional<GenericFrameInfo> generic_frame_info;
  absl::optional<FrameDependencyStructure> template_structure;
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_INCLUDE_VIDEO_CODEC_INTERFACE_H_
