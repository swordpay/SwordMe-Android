/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VIDEO_VIDEO_SINK_INTERFACE_H_
#define API_VIDEO_VIDEO_SINK_INTERFACE_H_

#include "absl/types/optional.h"
#include "api/video_track_source_constraints.h"
#include "rtc_base/checks.h"

namespace rtc {

template <typename VideoFrameT>
class VideoSinkInterface {
 public:
  virtual ~VideoSinkInterface() = default;

  virtual void OnFrame(const VideoFrameT& frame) = 0;


  virtual void OnDiscardedFrame() {}


  virtual void OnConstraintsChanged(
      const webrtc::VideoTrackSourceConstraints& constraints) {}
};

}  // namespace rtc

#endif  // API_VIDEO_VIDEO_SINK_INTERFACE_H_
