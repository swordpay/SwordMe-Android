/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VIDEO_ENCODED_FRAME_H_
#define API_VIDEO_ENCODED_FRAME_H_

#include <stddef.h>
#include <stdint.h>

#include "absl/types/optional.h"
#include "api/units/timestamp.h"
#include "modules/video_coding/encoded_frame.h"

namespace webrtc {

// TODO(philipel): Move transport specific info out of EncodedFrame.
// NOTE: This class is still under development and may change without notice.
class EncodedFrame : public webrtc::VCMEncodedFrame {
 public:
  static const uint8_t kMaxFrameReferences = 5;

  EncodedFrame() = default;
  EncodedFrame(const EncodedFrame&) = default;
  virtual ~EncodedFrame() {}


  virtual int64_t ReceivedTime() const = 0;


  absl::optional<webrtc::Timestamp> ReceivedTimestamp() const;


  virtual int64_t RenderTime() const = 0;


  absl::optional<webrtc::Timestamp> RenderTimestamp() const;



  virtual bool delayed_by_retransmission() const;

  bool is_keyframe() const { return num_references == 0; }

  void SetId(int64_t id) { id_ = id; }
  int64_t Id() const { return id_; }


  size_t num_references = 0;
  int64_t references[kMaxFrameReferences];


  bool is_last_spatial_layer = true;

 private:


  int64_t id_ = -1;
};

}  // namespace webrtc

#endif  // API_VIDEO_ENCODED_FRAME_H_
