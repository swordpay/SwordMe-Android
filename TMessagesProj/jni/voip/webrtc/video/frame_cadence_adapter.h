/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_FRAME_CADENCE_ADAPTER_H_
#define VIDEO_FRAME_CADENCE_ADAPTER_H_

#include <memory>

#include "absl/base/attributes.h"
#include "api/field_trials_view.h"
#include "api/task_queue/task_queue_base.h"
#include "api/units/time_delta.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/thread_annotations.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {

// With the exception of the constructor and the methods overridden in
// VideoSinkInterface, the rest of the interface to this class (including dtor)
// needs to happen on the queue passed in Create.
class FrameCadenceAdapterInterface
    : public rtc::VideoSinkInterface<VideoFrame> {
 public:



  static constexpr int64_t kFrameRateAveragingWindowSizeMs = (1000 / 30) * 90;



  static constexpr TimeDelta kZeroHertzIdleRepeatRatePeriod =
      TimeDelta::Millis(1000);


  static constexpr int kOnDiscardedFrameRefreshFramePeriod = 3;

  struct ZeroHertzModeParams {

    size_t num_simulcast_layers = 0;
  };

  class Callback {
   public:
    virtual ~Callback() = default;











    virtual void OnFrame(Timestamp post_time,
                         int frames_scheduled_for_processing,
                         const VideoFrame& frame) = 0;

    virtual void OnDiscardedFrame() = 0;

    virtual void RequestRefreshFrame() = 0;
  };




  static std::unique_ptr<FrameCadenceAdapterInterface> Create(
      Clock* clock,
      TaskQueueBase* queue,
      const FieldTrialsView& field_trials);

  virtual void Initialize(Callback* callback) = 0;



  virtual void SetZeroHertzModeEnabled(
      absl::optional<ZeroHertzModeParams> params) = 0;


  virtual absl::optional<uint32_t> GetInputFrameRateFps() = 0;


  virtual void UpdateFrameRate() = 0;



  virtual void UpdateLayerQualityConvergence(size_t spatial_index,
                                             bool converged) = 0;

  virtual void UpdateLayerStatus(size_t spatial_index, bool enabled) = 0;


  virtual void ProcessKeyFrameRequest() = 0;
};

}  // namespace webrtc

#endif  // VIDEO_FRAME_CADENCE_ADAPTER_H_
