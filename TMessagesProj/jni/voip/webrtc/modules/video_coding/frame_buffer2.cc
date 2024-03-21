/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_coding/frame_buffer2.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/video/encoded_image.h"
#include "api/video/video_timing.h"
#include "modules/video_coding/frame_helpers.h"
#include "modules/video_coding/include/video_coding_defines.h"
#include "modules/video_coding/timing/jitter_estimator.h"
#include "modules/video_coding/timing/timing.h"
#include "rtc_base/checks.h"
#include "rtc_base/experiments/rtt_mult_experiment.h"
#include "rtc_base/logging.h"
#include "rtc_base/numerics/sequence_number_util.h"
#include "rtc_base/trace_event.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {
namespace video_coding {

namespace {
// Max number of frames the buffer will hold.
constexpr size_t kMaxFramesBuffered = 800;

// low-latency renderer is used.
constexpr size_t kZeroPlayoutDelayDefaultMaxDecodeQueueSize = 8;

constexpr int kMaxFramesHistory = 1 << 13;

// still be rendered.
constexpr int kMaxAllowedFrameDelayMs = 5;

constexpr int64_t kLogNonDecodedIntervalMs = 5000;
}  // namespace

FrameBuffer::FrameBuffer(Clock* clock,
                         VCMTiming* timing,
                         const FieldTrialsView& field_trials)
    : decoded_frames_history_(kMaxFramesHistory),
      clock_(clock),
      callback_queue_(nullptr),
      jitter_estimator_(clock, field_trials),
      timing_(timing),
      stopped_(false),
      protection_mode_(kProtectionNack),
      last_log_non_decoded_ms_(-kLogNonDecodedIntervalMs),
      rtt_mult_settings_(RttMultExperiment::GetRttMultValue()),
      zero_playout_delay_max_decode_queue_size_(
          "max_decode_queue_size",
          kZeroPlayoutDelayDefaultMaxDecodeQueueSize) {
  ParseFieldTrial({&zero_playout_delay_max_decode_queue_size_},
                  field_trials.Lookup("WebRTC-ZeroPlayoutDelay"));
  callback_checker_.Detach();
}

FrameBuffer::~FrameBuffer() {
  RTC_DCHECK_RUN_ON(&construction_checker_);
}

void FrameBuffer::NextFrame(int64_t max_wait_time_ms,
                            bool keyframe_required,
                            TaskQueueBase* callback_queue,
                            NextFrameCallback handler) {
  RTC_DCHECK_RUN_ON(&callback_checker_);
  RTC_DCHECK(callback_queue->IsCurrent());
  TRACE_EVENT0("webrtc", "FrameBuffer::NextFrame");
  int64_t latest_return_time_ms =
      clock_->TimeInMilliseconds() + max_wait_time_ms;

  MutexLock lock(&mutex_);
  if (stopped_) {
    return;
  }
  latest_return_time_ms_ = latest_return_time_ms;
  keyframe_required_ = keyframe_required;
  frame_handler_ = handler;
  callback_queue_ = callback_queue;
  StartWaitForNextFrameOnQueue();
}

void FrameBuffer::StartWaitForNextFrameOnQueue() {
  RTC_DCHECK(callback_queue_);
  RTC_DCHECK(!callback_task_.Running());
  int64_t wait_ms = FindNextFrame(clock_->CurrentTime());
  callback_task_ = RepeatingTaskHandle::DelayedStart(
      callback_queue_, TimeDelta::Millis(wait_ms),
      [this] {
        RTC_DCHECK_RUN_ON(&callback_checker_);


        std::unique_ptr<EncodedFrame> frame;
        NextFrameCallback frame_handler;
        {
          MutexLock lock(&mutex_);
          if (!frames_to_decode_.empty()) {

            frame = GetNextFrame();
            timing_->SetLastDecodeScheduledTimestamp(clock_->CurrentTime());
          } else if (clock_->TimeInMilliseconds() < latest_return_time_ms_) {



            int64_t wait_ms = FindNextFrame(clock_->CurrentTime());
            return TimeDelta::Millis(wait_ms);
          }
          frame_handler = std::move(frame_handler_);
          CancelCallback();
        }

        frame_handler(std::move(frame));
        return TimeDelta::Zero();  // Ignored.
      },
      TaskQueueBase::DelayPrecision::kHigh);
}

int64_t FrameBuffer::FindNextFrame(Timestamp now) {
  int64_t wait_ms = latest_return_time_ms_ - now.ms();
  frames_to_decode_.clear();


  for (auto frame_it = frames_.begin();
       frame_it != frames_.end() && frame_it->first <= last_continuous_frame_;
       ++frame_it) {
    if (!frame_it->second.continuous ||
        frame_it->second.num_missing_decodable > 0) {
      continue;
    }

    EncodedFrame* frame = frame_it->second.frame.get();

    if (keyframe_required_ && !frame->is_keyframe())
      continue;

    auto last_decoded_frame_timestamp =
        decoded_frames_history_.GetLastDecodedFrameTimestamp();



    if (last_decoded_frame_timestamp &&
        AheadOf(*last_decoded_frame_timestamp, frame->Timestamp())) {
      continue;
    }

    std::vector<FrameMap::iterator> current_superframe;
    current_superframe.push_back(frame_it);
    bool last_layer_completed = frame_it->second.frame->is_last_spatial_layer;
    FrameMap::iterator next_frame_it = frame_it;
    while (!last_layer_completed) {
      ++next_frame_it;

      if (next_frame_it == frames_.end() || !next_frame_it->second.frame) {
        break;
      }

      if (next_frame_it->second.frame->Timestamp() != frame->Timestamp() ||
          !next_frame_it->second.continuous) {
        break;
      }

      if (next_frame_it->second.num_missing_decodable > 0) {
        bool has_inter_layer_dependency = false;
        for (size_t i = 0; i < EncodedFrame::kMaxFrameReferences &&
                           i < next_frame_it->second.frame->num_references;
             ++i) {
          if (next_frame_it->second.frame->references[i] >= frame_it->first) {
            has_inter_layer_dependency = true;
            break;
          }
        }




        if (!has_inter_layer_dependency ||
            next_frame_it->second.num_missing_decodable > 1) {
          break;
        }
      }

      current_superframe.push_back(next_frame_it);
      last_layer_completed = next_frame_it->second.frame->is_last_spatial_layer;
    }



    if (!last_layer_completed) {
      continue;
    }

    frames_to_decode_ = std::move(current_superframe);

    absl::optional<Timestamp> render_time = frame->RenderTimestamp();
    if (!render_time) {
      render_time = timing_->RenderTime(frame->Timestamp(), now);
      frame->SetRenderTime(render_time->ms());
    }
    bool too_many_frames_queued =
        frames_.size() > zero_playout_delay_max_decode_queue_size_ ? true
                                                                   : false;
    wait_ms =
        timing_->MaxWaitingTime(*render_time, now, too_many_frames_queued).ms();





    if (wait_ms < -kMaxAllowedFrameDelayMs)
      continue;

    break;
  }
  wait_ms = std::min<int64_t>(wait_ms, latest_return_time_ms_ - now.ms());
  wait_ms = std::max<int64_t>(wait_ms, 0);
  return wait_ms;
}

std::unique_ptr<EncodedFrame> FrameBuffer::GetNextFrame() {
  RTC_DCHECK_RUN_ON(&callback_checker_);
  Timestamp now = clock_->CurrentTime();

  std::vector<std::unique_ptr<EncodedFrame>> frames_out;

  RTC_DCHECK(!frames_to_decode_.empty());
  bool superframe_delayed_by_retransmission = false;
  DataSize superframe_size = DataSize::Zero();
  const EncodedFrame& first_frame = *frames_to_decode_[0]->second.frame;
  absl::optional<Timestamp> render_time = first_frame.RenderTimestamp();
  int64_t receive_time_ms = first_frame.ReceivedTime();

  if (!render_time || FrameHasBadRenderTiming(*render_time, now) ||
      TargetVideoDelayIsTooLarge(timing_->TargetVideoDelay())) {
    RTC_LOG(LS_WARNING) << "Resetting jitter estimator and timing module due "
                           "to bad render timing for rtp_timestamp="
                        << first_frame.Timestamp();
    jitter_estimator_.Reset();
    timing_->Reset();
    render_time = timing_->RenderTime(first_frame.Timestamp(), now);
  }

  for (FrameMap::iterator& frame_it : frames_to_decode_) {
    RTC_DCHECK(frame_it != frames_.end());
    std::unique_ptr<EncodedFrame> frame = std::move(frame_it->second.frame);

    frame->SetRenderTime(render_time->ms());

    superframe_delayed_by_retransmission |= frame->delayed_by_retransmission();
    receive_time_ms = std::max(receive_time_ms, frame->ReceivedTime());
    superframe_size += DataSize::Bytes(frame->size());

    PropagateDecodability(frame_it->second);
    decoded_frames_history_.InsertDecoded(frame_it->first, frame->Timestamp());

    frames_.erase(frames_.begin(), ++frame_it);

    frames_out.emplace_back(std::move(frame));
  }

  if (!superframe_delayed_by_retransmission) {
    auto frame_delay = inter_frame_delay_.CalculateDelay(
        first_frame.Timestamp(), Timestamp::Millis(receive_time_ms));

    if (frame_delay) {
      jitter_estimator_.UpdateEstimate(*frame_delay, superframe_size);
    }

    float rtt_mult = protection_mode_ == kProtectionNackFEC ? 0.0 : 1.0;
    absl::optional<TimeDelta> rtt_mult_add_cap_ms = absl::nullopt;
    if (rtt_mult_settings_.has_value()) {
      rtt_mult = rtt_mult_settings_->rtt_mult_setting;
      rtt_mult_add_cap_ms =
          TimeDelta::Millis(rtt_mult_settings_->rtt_mult_add_cap_ms);
    }
    timing_->SetJitterDelay(
        jitter_estimator_.GetJitterEstimate(rtt_mult, rtt_mult_add_cap_ms));
    timing_->UpdateCurrentDelay(*render_time, now);
  } else {
    if (RttMultExperiment::RttMultEnabled())
      jitter_estimator_.FrameNacked();
  }

  if (frames_out.size() == 1) {
    return std::move(frames_out[0]);
  } else {
    return CombineAndDeleteFrames(std::move(frames_out));
  }
}

void FrameBuffer::SetProtectionMode(VCMVideoProtection mode) {
  TRACE_EVENT0("webrtc", "FrameBuffer::SetProtectionMode");
  MutexLock lock(&mutex_);
  protection_mode_ = mode;
}

void FrameBuffer::Stop() {
  TRACE_EVENT0("webrtc", "FrameBuffer::Stop");
  MutexLock lock(&mutex_);
  if (stopped_)
    return;
  stopped_ = true;

  CancelCallback();
}

void FrameBuffer::Clear() {
  MutexLock lock(&mutex_);
  ClearFramesAndHistory();
}

int FrameBuffer::Size() {
  MutexLock lock(&mutex_);
  return frames_.size();
}

void FrameBuffer::UpdateRtt(int64_t rtt_ms) {
  MutexLock lock(&mutex_);
  jitter_estimator_.UpdateRtt(TimeDelta::Millis(rtt_ms));
}

bool FrameBuffer::ValidReferences(const EncodedFrame& frame) const {
  for (size_t i = 0; i < frame.num_references; ++i) {
    if (frame.references[i] >= frame.Id())
      return false;

    for (size_t j = i + 1; j < frame.num_references; ++j) {
      if (frame.references[i] == frame.references[j])
        return false;
    }
  }

  return true;
}

void FrameBuffer::CancelCallback() {

  frame_handler_ = {};
  callback_task_.Stop();
  callback_queue_ = nullptr;
  callback_checker_.Detach();
}

int64_t FrameBuffer::InsertFrame(std::unique_ptr<EncodedFrame> frame) {
  TRACE_EVENT0("webrtc", "FrameBuffer::InsertFrame");
  RTC_DCHECK(frame);

  MutexLock lock(&mutex_);

  int64_t last_continuous_frame_id = last_continuous_frame_.value_or(-1);

  if (!ValidReferences(*frame)) {
    RTC_LOG(LS_WARNING) << "Frame " << frame->Id()
                        << " has invalid frame references, dropping frame.";
    return last_continuous_frame_id;
  }

  if (frames_.size() >= kMaxFramesBuffered) {
    if (frame->is_keyframe()) {
      RTC_LOG(LS_WARNING) << "Inserting keyframe " << frame->Id()
                          << " but buffer is full, clearing"
                             " buffer and inserting the frame.";
      ClearFramesAndHistory();
    } else {
      RTC_LOG(LS_WARNING) << "Frame " << frame->Id()
                          << " could not be inserted due to the frame "
                             "buffer being full, dropping frame.";
      return last_continuous_frame_id;
    }
  }

  auto last_decoded_frame = decoded_frames_history_.GetLastDecodedFrameId();
  auto last_decoded_frame_timestamp =
      decoded_frames_history_.GetLastDecodedFrameTimestamp();
  if (last_decoded_frame && frame->Id() <= *last_decoded_frame) {
    if (AheadOf(frame->Timestamp(), *last_decoded_frame_timestamp) &&
        frame->is_keyframe()) {





      RTC_LOG(LS_WARNING)
          << "A jump in frame id was detected, clearing buffer.";
      ClearFramesAndHistory();
      last_continuous_frame_id = -1;
    } else {
      RTC_LOG(LS_WARNING) << "Frame " << frame->Id() << " inserted after frame "
                          << *last_decoded_frame
                          << " was handed off for decoding, dropping frame.";
      return last_continuous_frame_id;
    }
  }



  if (!frames_.empty() && frame->Id() < frames_.begin()->first &&
      frames_.rbegin()->first < frame->Id()) {
    RTC_LOG(LS_WARNING) << "A jump in frame id was detected, clearing buffer.";
    ClearFramesAndHistory();
    last_continuous_frame_id = -1;
  }

  auto info = frames_.emplace(frame->Id(), FrameInfo()).first;

  if (info->second.frame) {
    return last_continuous_frame_id;
  }

  if (!UpdateFrameInfoWithIncomingFrame(*frame, info))
    return last_continuous_frame_id;

  if (!frame->delayed_by_retransmission() && frame->ReceivedTime() >= 0)
    timing_->IncomingTimestamp(frame->Timestamp(),
                               Timestamp::Millis(frame->ReceivedTime()));


  info->second.frame = std::move(frame);

  if (info->second.num_missing_continuous == 0) {
    info->second.continuous = true;
    PropagateContinuity(info);
    last_continuous_frame_id = *last_continuous_frame_;


    if (callback_queue_) {
      callback_queue_->PostTask([this] {
        MutexLock lock(&mutex_);
        if (!callback_task_.Running())
          return;
        RTC_CHECK(frame_handler_);
        callback_task_.Stop();
        StartWaitForNextFrameOnQueue();
      });
    }
  }

  return last_continuous_frame_id;
}

void FrameBuffer::PropagateContinuity(FrameMap::iterator start) {
  TRACE_EVENT0("webrtc", "FrameBuffer::PropagateContinuity");
  RTC_DCHECK(start->second.continuous);

  std::queue<FrameMap::iterator> continuous_frames;
  continuous_frames.push(start);

  while (!continuous_frames.empty()) {
    auto frame = continuous_frames.front();
    continuous_frames.pop();

    if (!last_continuous_frame_ || *last_continuous_frame_ < frame->first) {
      last_continuous_frame_ = frame->first;
    }


    for (size_t d = 0; d < frame->second.dependent_frames.size(); ++d) {
      auto frame_ref = frames_.find(frame->second.dependent_frames[d]);
      RTC_DCHECK(frame_ref != frames_.end());

      if (frame_ref != frames_.end()) {
        --frame_ref->second.num_missing_continuous;
        if (frame_ref->second.num_missing_continuous == 0) {
          frame_ref->second.continuous = true;
          continuous_frames.push(frame_ref);
        }
      }
    }
  }
}

void FrameBuffer::PropagateDecodability(const FrameInfo& info) {
  TRACE_EVENT0("webrtc", "FrameBuffer::PropagateDecodability");
  for (size_t d = 0; d < info.dependent_frames.size(); ++d) {
    auto ref_info = frames_.find(info.dependent_frames[d]);
    RTC_DCHECK(ref_info != frames_.end());

    if (ref_info != frames_.end()) {
      RTC_DCHECK_GT(ref_info->second.num_missing_decodable, 0U);
      --ref_info->second.num_missing_decodable;
    }
  }
}

bool FrameBuffer::UpdateFrameInfoWithIncomingFrame(const EncodedFrame& frame,
                                                   FrameMap::iterator info) {
  TRACE_EVENT0("webrtc", "FrameBuffer::UpdateFrameInfoWithIncomingFrame");
  auto last_decoded_frame = decoded_frames_history_.GetLastDecodedFrameId();
  RTC_DCHECK(!last_decoded_frame || *last_decoded_frame < info->first);








  struct Dependency {
    int64_t frame_id;
    bool continuous;
  };
  std::vector<Dependency> not_yet_fulfilled_dependencies;

  for (size_t i = 0; i < frame.num_references; ++i) {

    if (last_decoded_frame && frame.references[i] <= *last_decoded_frame) {


      if (!decoded_frames_history_.WasDecoded(frame.references[i])) {
        int64_t now_ms = clock_->TimeInMilliseconds();
        if (last_log_non_decoded_ms_ + kLogNonDecodedIntervalMs < now_ms) {
          RTC_LOG(LS_WARNING)
              << "Frame " << frame.Id()
              << " depends on a non-decoded frame more previous than the last "
                 "decoded frame, dropping frame.";
          last_log_non_decoded_ms_ = now_ms;
        }
        return false;
      }
    } else {
      auto ref_info = frames_.find(frame.references[i]);
      bool ref_continuous =
          ref_info != frames_.end() && ref_info->second.continuous;
      not_yet_fulfilled_dependencies.push_back(
          {frame.references[i], ref_continuous});
    }
  }

  info->second.num_missing_continuous = not_yet_fulfilled_dependencies.size();
  info->second.num_missing_decodable = not_yet_fulfilled_dependencies.size();

  for (const Dependency& dep : not_yet_fulfilled_dependencies) {
    if (dep.continuous)
      --info->second.num_missing_continuous;

    frames_[dep.frame_id].dependent_frames.push_back(frame.Id());
  }

  return true;
}

void FrameBuffer::ClearFramesAndHistory() {
  TRACE_EVENT0("webrtc", "FrameBuffer::ClearFramesAndHistory");
  frames_.clear();
  last_continuous_frame_.reset();
  frames_to_decode_.clear();
  decoded_frames_history_.Clear();
}

// NextFrame and GetNextFrame with methods returning multiple frames.
std::unique_ptr<EncodedFrame> FrameBuffer::CombineAndDeleteFrames(
    std::vector<std::unique_ptr<EncodedFrame>> frames) const {
  RTC_DCHECK(!frames.empty());
  absl::InlinedVector<std::unique_ptr<EncodedFrame>, 4> inlined;
  for (auto& frame : frames) {
    inlined.push_back(std::move(frame));
  }
  return webrtc::CombineAndDeleteFrames(std::move(inlined));
}

FrameBuffer::FrameInfo::FrameInfo() = default;
FrameBuffer::FrameInfo::FrameInfo(FrameInfo&&) = default;
FrameBuffer::FrameInfo::~FrameInfo() = default;

}  // namespace video_coding
}  // namespace webrtc
