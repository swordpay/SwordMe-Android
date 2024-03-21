/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_VIDEO_STREAM_DECODER_IMPL_H_
#define VIDEO_VIDEO_STREAM_DECODER_IMPL_H_

#include <map>
#include <memory>
#include <utility>

#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/sequence_checker.h"
#include "api/transport/field_trial_based_config.h"
#include "api/video/video_stream_decoder.h"
#include "modules/video_coding/frame_buffer2.h"
#include "modules/video_coding/timing/timing.h"
#include "rtc_base/memory/always_valid_pointer.h"
#include "rtc_base/platform_thread.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/task_queue.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {

class VideoStreamDecoderImpl : public VideoStreamDecoderInterface {
 public:
  VideoStreamDecoderImpl(
      VideoStreamDecoderInterface::Callbacks* callbacks,
      VideoDecoderFactory* decoder_factory,
      TaskQueueFactory* task_queue_factory,
      std::map<int, std::pair<SdpVideoFormat, int>> decoder_settings,
      const FieldTrialsView* field_trials);

  ~VideoStreamDecoderImpl() override;

  void OnFrame(std::unique_ptr<EncodedFrame> frame) override;

  void SetMinPlayoutDelay(TimeDelta min_delay) override;
  void SetMaxPlayoutDelay(TimeDelta max_delay) override;

 private:
  class DecodeCallbacks : public DecodedImageCallback {
   public:
    explicit DecodeCallbacks(VideoStreamDecoderImpl* video_stream_decoder_impl);
    int32_t Decoded(VideoFrame& decodedImage) override;
    int32_t Decoded(VideoFrame& decodedImage, int64_t decode_time_ms) override;
    void Decoded(VideoFrame& decodedImage,
                 absl::optional<int32_t> decode_time_ms,
                 absl::optional<uint8_t> qp) override;

   private:
    VideoStreamDecoderImpl* const video_stream_decoder_impl_;
  };

  enum DecodeResult {
    kOk,
    kOkRequestKeyframe,
    kDecodeFailure,
  };

  struct FrameInfo {
    int64_t timestamp = -1;
    int64_t decode_start_time_ms;
    int64_t render_time_us;
    VideoContentType content_type;
  };

  void SaveFrameInfo(const EncodedFrame& frame) RTC_RUN_ON(bookkeeping_queue_);
  FrameInfo* GetFrameInfo(int64_t timestamp) RTC_RUN_ON(bookkeeping_queue_);
  void StartNextDecode() RTC_RUN_ON(bookkeeping_queue_);
  void OnNextFrameCallback(std::unique_ptr<EncodedFrame> frame)
      RTC_RUN_ON(bookkeeping_queue_);
  void OnDecodedFrameCallback(VideoFrame& decodedImage,  // NOLINT
                              absl::optional<int32_t> decode_time_ms,
                              absl::optional<uint8_t> qp);

  VideoDecoder* GetDecoder(int payload_type) RTC_RUN_ON(decode_queue_);
  VideoStreamDecoderImpl::DecodeResult DecodeFrame(
      std::unique_ptr<EncodedFrame> frame) RTC_RUN_ON(decode_queue_);

  AlwaysValidPointer<const FieldTrialsView, FieldTrialBasedConfig>
      field_trials_;
  VCMTiming timing_;
  DecodeCallbacks decode_callbacks_;


  static constexpr int kFrameInfoMemory = 8;
  std::array<FrameInfo, kFrameInfoMemory> frame_info_
      RTC_GUARDED_BY(bookkeeping_queue_);
  int next_frame_info_index_ RTC_GUARDED_BY(bookkeeping_queue_);
  VideoStreamDecoderInterface::Callbacks* const callbacks_
      RTC_PT_GUARDED_BY(bookkeeping_queue_);
  int64_t last_continuous_frame_id_ RTC_GUARDED_BY(bookkeeping_queue_) = -1;
  bool keyframe_required_ RTC_GUARDED_BY(bookkeeping_queue_);

  absl::optional<int> current_payload_type_ RTC_GUARDED_BY(decode_queue_);
  VideoDecoderFactory* const decoder_factory_ RTC_PT_GUARDED_BY(decode_queue_);
  std::map<int, std::pair<SdpVideoFormat, int>> decoder_settings_
      RTC_GUARDED_BY(decode_queue_);








  Mutex shut_down_mutex_;
  bool shut_down_ RTC_GUARDED_BY(shut_down_mutex_);
  video_coding::FrameBuffer frame_buffer_ RTC_GUARDED_BY(bookkeeping_queue_);
  rtc::TaskQueue bookkeeping_queue_;
  std::unique_ptr<VideoDecoder> decoder_ RTC_GUARDED_BY(decode_queue_);
  rtc::TaskQueue decode_queue_;
};

}  // namespace webrtc

#endif  // VIDEO_VIDEO_STREAM_DECODER_IMPL_H_
