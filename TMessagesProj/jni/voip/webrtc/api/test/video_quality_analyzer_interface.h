/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_TEST_VIDEO_QUALITY_ANALYZER_INTERFACE_H_
#define API_TEST_VIDEO_QUALITY_ANALYZER_INTERFACE_H_

#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/array_view.h"
#include "api/test/stats_observer_interface.h"
#include "api/video/encoded_image.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/video_encoder.h"

namespace webrtc {


// tests. Interface has only one abstract method, which have to return frame id.
// Other methods have empty implementation by default, so user can override only
// required parts.
//
// VideoQualityAnalyzerInterface will be injected into WebRTC pipeline on both
// sides of the call. Here is video data flow in WebRTC pipeline
//
// Alice:
//  ___________       ________       _________
// |           |     |        |     |         |
// |   Frame   |-(A)→| WebRTC |-(B)→| Video   |-(C)┐
// | Generator |     | Stack  |     | Decoder |    |
//  ¯¯¯¯¯¯¯¯¯¯¯       ¯¯¯¯¯¯¯¯       ¯¯¯¯¯¯¯¯¯     |
//                                               __↓________
//                                              | Transport |
//                                              |     &     |
//                                              |  Network  |
//                                               ¯¯|¯¯¯¯¯¯¯¯
// Bob:                                            |
//  _______       ________       _________         |
// |       |     |        |     |         |        |
// | Video |←(F)-| WebRTC |←(E)-| Video   |←(D)----┘
// | Sink  |     | Stack  |     | Decoder |
//  ¯¯¯¯¯¯¯       ¯¯¯¯¯¯¯¯       ¯¯¯¯¯¯¯¯¯
// The analyzer will be injected in all points from A to F.
class VideoQualityAnalyzerInterface
    : public webrtc_pc_e2e::StatsObserverInterface {
 public:

  struct EncoderStats {
    std::string encoder_name = "unknown";



    uint32_t target_encode_bitrate = 0;
  };

  struct DecoderStats {
    std::string decoder_name = "unknown";


    absl::optional<int32_t> decode_time_ms = absl::nullopt;
  };

  ~VideoQualityAnalyzerInterface() override = default;







  virtual void Start(std::string test_case_name,
                     rtc::ArrayView<const std::string> peer_names,
                     int max_threads_count) {}



  virtual uint16_t OnFrameCaptured(absl::string_view peer_name,
                                   const std::string& stream_label,
                                   const VideoFrame& frame) = 0;


  virtual void OnFramePreEncode(absl::string_view peer_name,
                                const VideoFrame& frame) {}




  virtual void OnFrameEncoded(absl::string_view peer_name,
                              uint16_t frame_id,
                              const EncodedImage& encoded_image,
                              const EncoderStats& stats,
                              bool discarded) {}


  virtual void OnFrameDropped(absl::string_view peer_name,
                              EncodedImageCallback::DropReason reason) {}


  virtual void OnFramePreDecode(absl::string_view peer_name,
                                uint16_t frame_id,
                                const EncodedImage& encoded_image) {}


  virtual void OnFrameDecoded(absl::string_view peer_name,
                              const VideoFrame& frame,
                              const DecoderStats& stats) {}


  virtual void OnFrameRendered(absl::string_view peer_name,
                               const VideoFrame& frame) {}




  virtual void OnEncoderError(absl::string_view peer_name,
                              const VideoFrame& frame,
                              int32_t error_code) {}




  virtual void OnDecoderError(absl::string_view peer_name,
                              uint16_t frame_id,
                              int32_t error_code,
                              const DecoderStats& stats) {}


  void OnStatsReports(
      absl::string_view pc_label,
      const rtc::scoped_refptr<const RTCStatsReport>& report) override {}

  virtual void RegisterParticipantInCall(absl::string_view peer_name) {}


  virtual void UnregisterParticipantInCall(absl::string_view peer_name) {}


  virtual void Stop() {}



  virtual std::string GetStreamLabel(uint16_t frame_id) = 0;
};

}  // namespace webrtc

#endif  // API_TEST_VIDEO_QUALITY_ANALYZER_INTERFACE_H_
