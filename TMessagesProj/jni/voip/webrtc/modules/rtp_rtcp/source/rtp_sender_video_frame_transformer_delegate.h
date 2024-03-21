/*
 *  Copyright (c) 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_RTP_RTCP_SOURCE_RTP_SENDER_VIDEO_FRAME_TRANSFORMER_DELEGATE_H_
#define MODULES_RTP_RTCP_SOURCE_RTP_SENDER_VIDEO_FRAME_TRANSFORMER_DELEGATE_H_

#include <memory>

#include "api/frame_transformer_interface.h"
#include "api/scoped_refptr.h"
#include "api/sequence_checker.h"
#include "api/task_queue/task_queue_base.h"
#include "api/task_queue/task_queue_factory.h"
#include "api/video/video_layers_allocation.h"
#include "rtc_base/synchronization/mutex.h"

namespace webrtc {

class RTPSenderVideo;

// RTPSenderVideo to send the transformed frames. Ensures thread-safe access to
// the sender.
class RTPSenderVideoFrameTransformerDelegate : public TransformedFrameCallback {
 public:
  RTPSenderVideoFrameTransformerDelegate(
      RTPSenderVideo* sender,
      rtc::scoped_refptr<FrameTransformerInterface> frame_transformer,
      uint32_t ssrc,
      TaskQueueFactory* send_transport_queue);

  void Init();

  bool TransformFrame(int payload_type,
                      absl::optional<VideoCodecType> codec_type,
                      uint32_t rtp_timestamp,
                      const EncodedImage& encoded_image,
                      RTPVideoHeader video_header,
                      absl::optional<int64_t> expected_retransmission_time_ms);


  void OnTransformedFrame(
      std::unique_ptr<TransformableFrameInterface> frame) override;

  void SendVideo(std::unique_ptr<TransformableFrameInterface> frame) const
      RTC_RUN_ON(encoder_queue_);


  void SetVideoStructureUnderLock(
      const FrameDependencyStructure* video_structure);



  void SetVideoLayersAllocationUnderLock(VideoLayersAllocation allocation);



  void Reset();

 protected:
  ~RTPSenderVideoFrameTransformerDelegate() override = default;

 private:
  mutable Mutex sender_lock_;
  RTPSenderVideo* sender_ RTC_GUARDED_BY(sender_lock_);
  rtc::scoped_refptr<FrameTransformerInterface> frame_transformer_;
  const uint32_t ssrc_;
  TaskQueueBase* encoder_queue_ = nullptr;
  TaskQueueFactory* task_queue_factory_;


  std::unique_ptr<TaskQueueBase, TaskQueueDeleter> owned_encoder_queue_;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_RTP_SENDER_VIDEO_FRAME_TRANSFORMER_DELEGATE_H_
