/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video/frame_cadence_adapter.h"

#include <atomic>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/base/attributes.h"
#include "api/sequence_checker.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/task_queue/task_queue_base.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "api/video/video_frame.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/race_checker.h"
#include "rtc_base/rate_statistics.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/system/no_unique_address.h"
#include "rtc_base/task_utils/repeating_task.h"
#include "rtc_base/thread_annotations.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/clock.h"
#include "system_wrappers/include/metrics.h"
#include "system_wrappers/include/ntp_time.h"

namespace webrtc {
namespace {

class AdapterMode {
 public:
  virtual ~AdapterMode() = default;

  virtual void OnFrame(Timestamp post_time,
                       int frames_scheduled_for_processing,
                       const VideoFrame& frame) = 0;

  virtual absl::optional<uint32_t> GetInputFrameRateFps() = 0;

  virtual void UpdateFrameRate() = 0;
};

class PassthroughAdapterMode : public AdapterMode {
 public:
  PassthroughAdapterMode(Clock* clock,
                         FrameCadenceAdapterInterface::Callback* callback)
      : clock_(clock), callback_(callback) {
    sequence_checker_.Detach();
  }

  void OnFrame(Timestamp post_time,
               int frames_scheduled_for_processing,
               const VideoFrame& frame) override {
    RTC_DCHECK_RUN_ON(&sequence_checker_);
    callback_->OnFrame(post_time, frames_scheduled_for_processing, frame);
  }

  absl::optional<uint32_t> GetInputFrameRateFps() override {
    RTC_DCHECK_RUN_ON(&sequence_checker_);
    return input_framerate_.Rate(clock_->TimeInMilliseconds());
  }

  void UpdateFrameRate() override {
    RTC_DCHECK_RUN_ON(&sequence_checker_);
    input_framerate_.Update(1, clock_->TimeInMilliseconds());
  }

 private:
  Clock* const clock_;
  FrameCadenceAdapterInterface::Callback* const callback_;
  RTC_NO_UNIQUE_ADDRESS SequenceChecker sequence_checker_;

  RateStatistics input_framerate_ RTC_GUARDED_BY(sequence_checker_){
      FrameCadenceAdapterInterface::kFrameRateAveragingWindowSizeMs, 1000};
};

class ZeroHertzAdapterMode : public AdapterMode {
 public:
  ZeroHertzAdapterMode(TaskQueueBase* queue,
                       Clock* clock,
                       FrameCadenceAdapterInterface::Callback* callback,
                       double max_fps);


  void ReconfigureParameters(
      const FrameCadenceAdapterInterface::ZeroHertzModeParams& params);

  void UpdateLayerQualityConvergence(size_t spatial_index,
                                     bool quality_converged);

  void UpdateLayerStatus(size_t spatial_index, bool enabled);

  void OnFrame(Timestamp post_time,
               int frames_scheduled_for_processing,
               const VideoFrame& frame) override;
  absl::optional<uint32_t> GetInputFrameRateFps() override;
  void UpdateFrameRate() override {}

  void OnDiscardedFrame();


  void ProcessKeyFrameRequest();

 private:


  struct SpatialLayerTracker {


    absl::optional<bool> quality_converged;
  };

  struct ScheduledRepeat {
    ScheduledRepeat(Timestamp origin,
                    int64_t origin_timestamp_us,
                    int64_t origin_ntp_time_ms)
        : scheduled(origin),
          idle(false),
          origin(origin),
          origin_timestamp_us(origin_timestamp_us),
          origin_ntp_time_ms(origin_ntp_time_ms) {}

    Timestamp scheduled;


    bool idle;

    Timestamp origin;

    int64_t origin_timestamp_us;

    int64_t origin_ntp_time_ms;
  };




  bool HasQualityConverged() const RTC_RUN_ON(sequence_checker_);


  void ResetQualityConvergenceInfo() RTC_RUN_ON(sequence_checker_);

  void ProcessOnDelayedCadence() RTC_RUN_ON(sequence_checker_);




  void ScheduleRepeat(int frame_id, bool idle_repeat)
      RTC_RUN_ON(sequence_checker_);



  void ProcessRepeatedFrameOnDelayedCadence(int frame_id)
      RTC_RUN_ON(sequence_checker_);

  void SendFrameNow(const VideoFrame& frame) const
      RTC_RUN_ON(sequence_checker_);

  TimeDelta RepeatDuration(bool idle_repeat) const
      RTC_RUN_ON(sequence_checker_);



  void MaybeStartRefreshFrameRequester() RTC_RUN_ON(sequence_checker_);

  TaskQueueBase* const queue_;
  Clock* const clock_;
  FrameCadenceAdapterInterface::Callback* const callback_;


  const double max_fps_;

  const TimeDelta frame_delay_ = TimeDelta::Seconds(1) / max_fps_;

  RTC_NO_UNIQUE_ADDRESS SequenceChecker sequence_checker_;

  std::deque<VideoFrame> queued_frames_ RTC_GUARDED_BY(sequence_checker_);


  int current_frame_id_ RTC_GUARDED_BY(sequence_checker_) = 0;

  absl::optional<ScheduledRepeat> scheduled_repeat_
      RTC_GUARDED_BY(sequence_checker_);

  std::vector<SpatialLayerTracker> layer_trackers_
      RTC_GUARDED_BY(sequence_checker_);


  RepeatingTaskHandle refresh_frame_requester_
      RTC_GUARDED_BY(sequence_checker_);

  ScopedTaskSafety safety_;
};

class FrameCadenceAdapterImpl : public FrameCadenceAdapterInterface {
 public:
  FrameCadenceAdapterImpl(Clock* clock,
                          TaskQueueBase* queue,
                          const FieldTrialsView& field_trials);
  ~FrameCadenceAdapterImpl();

  void Initialize(Callback* callback) override;
  void SetZeroHertzModeEnabled(
      absl::optional<ZeroHertzModeParams> params) override;
  absl::optional<uint32_t> GetInputFrameRateFps() override;
  void UpdateFrameRate() override;
  void UpdateLayerQualityConvergence(size_t spatial_index,
                                     bool quality_converged) override;
  void UpdateLayerStatus(size_t spatial_index, bool enabled) override;
  void ProcessKeyFrameRequest() override;

  void OnFrame(const VideoFrame& frame) override;
  void OnDiscardedFrame() override;
  void OnConstraintsChanged(
      const VideoTrackSourceConstraints& constraints) override;

 private:

  void OnFrameOnMainQueue(Timestamp post_time,
                          int frames_scheduled_for_processing,
                          const VideoFrame& frame) RTC_RUN_ON(queue_);





  bool IsZeroHertzScreenshareEnabled() const RTC_RUN_ON(queue_);

  void MaybeReconfigureAdapters(bool was_zero_hertz_enabled) RTC_RUN_ON(queue_);

  void MaybeReportFrameRateConstraintUmas() RTC_RUN_ON(queue_);

  Clock* const clock_;
  TaskQueueBase* const queue_;


  const bool zero_hertz_screenshare_enabled_;

  absl::optional<PassthroughAdapterMode> passthrough_adapter_;
  absl::optional<ZeroHertzAdapterMode> zero_hertz_adapter_;

  absl::optional<ZeroHertzModeParams> zero_hertz_params_;

  AdapterMode* current_adapter_mode_ = nullptr;

  absl::optional<Timestamp> zero_hertz_adapter_created_timestamp_
      RTC_GUARDED_BY(queue_);

  Callback* callback_ = nullptr;

  absl::optional<VideoTrackSourceConstraints> source_constraints_
      RTC_GUARDED_BY(queue_);


  rtc::RaceChecker incoming_frame_race_checker_;
  bool has_reported_screenshare_frame_rate_umas_ RTC_GUARDED_BY(queue_) = false;


  std::atomic<int> frames_scheduled_for_processing_{0};

  ScopedTaskSafetyDetached safety_;
};

ZeroHertzAdapterMode::ZeroHertzAdapterMode(
    TaskQueueBase* queue,
    Clock* clock,
    FrameCadenceAdapterInterface::Callback* callback,
    double max_fps)
    : queue_(queue), clock_(clock), callback_(callback), max_fps_(max_fps) {
  sequence_checker_.Detach();
  MaybeStartRefreshFrameRequester();
}

void ZeroHertzAdapterMode::ReconfigureParameters(
    const FrameCadenceAdapterInterface::ZeroHertzModeParams& params) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_INFO) << __func__ << " this " << this << " num_simulcast_layers "
                    << params.num_simulcast_layers;

  layer_trackers_.clear();
  layer_trackers_.resize(params.num_simulcast_layers,
                         SpatialLayerTracker{false});
}

void ZeroHertzAdapterMode::UpdateLayerQualityConvergence(
    size_t spatial_index,
    bool quality_converged) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_LOG(LS_INFO) << __func__ << " this " << this << " layer " << spatial_index
                   << " quality has converged: " << quality_converged;
  if (spatial_index >= layer_trackers_.size())
    return;
  if (layer_trackers_[spatial_index].quality_converged.has_value())
    layer_trackers_[spatial_index].quality_converged = quality_converged;
}

void ZeroHertzAdapterMode::UpdateLayerStatus(size_t spatial_index,
                                             bool enabled) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  if (spatial_index >= layer_trackers_.size())
    return;
  if (enabled) {
    if (!layer_trackers_[spatial_index].quality_converged.has_value()) {

      layer_trackers_[spatial_index].quality_converged = false;
    }
  } else {
    layer_trackers_[spatial_index].quality_converged = absl::nullopt;
  }
  RTC_LOG(LS_INFO)
      << __func__ << " this " << this << " layer " << spatial_index
      << (enabled
              ? (layer_trackers_[spatial_index].quality_converged.has_value()
                     ? " enabled."
                     : " enabled and it's assumed quality has not converged.")
              : " disabled.");
}

void ZeroHertzAdapterMode::OnFrame(Timestamp post_time,
                                   int frames_scheduled_for_processing,
                                   const VideoFrame& frame) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_VERBOSE) << "ZeroHertzAdapterMode::" << __func__ << " this "
                       << this;
  refresh_frame_requester_.Stop();

  ResetQualityConvergenceInfo();

  if (scheduled_repeat_.has_value()) {
    RTC_DCHECK(queued_frames_.size() == 1);
    RTC_DLOG(LS_VERBOSE) << __func__ << " this " << this
                         << " cancel repeat and restart with original";
    queued_frames_.pop_front();
  }

  queued_frames_.push_back(frame);
  current_frame_id_++;
  scheduled_repeat_ = absl::nullopt;
  queue_->PostDelayedHighPrecisionTask(
      SafeTask(safety_.flag(),
               [this] {
                 RTC_DCHECK_RUN_ON(&sequence_checker_);
                 ProcessOnDelayedCadence();
               }),
      frame_delay_);
}

void ZeroHertzAdapterMode::OnDiscardedFrame() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_VERBOSE) << "ZeroHertzAdapterMode::" << __func__;




  MaybeStartRefreshFrameRequester();
}

absl::optional<uint32_t> ZeroHertzAdapterMode::GetInputFrameRateFps() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  return max_fps_;
}

void ZeroHertzAdapterMode::ProcessKeyFrameRequest() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);






  ResetQualityConvergenceInfo();


  if (!scheduled_repeat_.has_value() || !scheduled_repeat_->idle) {
    RTC_LOG(LS_INFO) << __func__ << " this " << this
                     << " not requesting refresh frame because of recently "
                        "incoming frame or short repeating.";
    return;
  }


  Timestamp now = clock_->CurrentTime();
  if (scheduled_repeat_->scheduled + RepeatDuration(/*idle_repeat=*/true) -
          now <=
      frame_delay_) {
    RTC_LOG(LS_INFO) << __func__ << " this " << this
                     << " not requesting refresh frame because of soon "
                        "happening idle repeat";
    return;
  }


  RTC_LOG(LS_INFO) << __func__ << " this " << this
                   << " not requesting refresh frame and scheduling a short "
                      "repeat due to key frame request";
  ScheduleRepeat(++current_frame_id_, /*idle_repeat=*/false);
  return;
}

bool ZeroHertzAdapterMode::HasQualityConverged() const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);




  const bool quality_converged =
      !layer_trackers_.empty() &&
      absl::c_all_of(layer_trackers_, [](const SpatialLayerTracker& tracker) {
        return tracker.quality_converged.value_or(true);
      });
  return quality_converged;
}

void ZeroHertzAdapterMode::ResetQualityConvergenceInfo() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_INFO) << __func__ << " this " << this;
  for (auto& layer_tracker : layer_trackers_) {
    if (layer_tracker.quality_converged.has_value())
      layer_tracker.quality_converged = false;
  }
}

void ZeroHertzAdapterMode::ProcessOnDelayedCadence() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DCHECK(!queued_frames_.empty());
  RTC_DLOG(LS_VERBOSE) << __func__ << " this " << this;

  SendFrameNow(queued_frames_.front());


  if (queued_frames_.size() > 1) {
    queued_frames_.pop_front();
    return;
  }



  ScheduleRepeat(current_frame_id_, HasQualityConverged());
}

void ZeroHertzAdapterMode::ScheduleRepeat(int frame_id, bool idle_repeat) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_VERBOSE) << __func__ << " this " << this << " frame_id "
                       << frame_id;
  Timestamp now = clock_->CurrentTime();
  if (!scheduled_repeat_.has_value()) {
    scheduled_repeat_.emplace(now, queued_frames_.front().timestamp_us(),
                              queued_frames_.front().ntp_time_ms());
  }
  scheduled_repeat_->scheduled = now;
  scheduled_repeat_->idle = idle_repeat;

  TimeDelta repeat_delay = RepeatDuration(idle_repeat);
  queue_->PostDelayedHighPrecisionTask(
      SafeTask(safety_.flag(),
               [this, frame_id] {
                 RTC_DCHECK_RUN_ON(&sequence_checker_);
                 ProcessRepeatedFrameOnDelayedCadence(frame_id);
               }),
      repeat_delay);
}

void ZeroHertzAdapterMode::ProcessRepeatedFrameOnDelayedCadence(int frame_id) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_VERBOSE) << __func__ << " this " << this << " frame_id "
                       << frame_id;
  RTC_DCHECK(!queued_frames_.empty());

  if (frame_id != current_frame_id_)
    return;
  RTC_DCHECK(scheduled_repeat_.has_value());

  VideoFrame& frame = queued_frames_.front();

  VideoFrame::UpdateRect empty_update_rect;
  empty_update_rect.MakeEmptyUpdate();
  frame.set_update_rect(empty_update_rect);





  TimeDelta total_delay = clock_->CurrentTime() - scheduled_repeat_->origin;
  if (frame.timestamp_us() > 0) {
    frame.set_timestamp_us(scheduled_repeat_->origin_timestamp_us +
                           total_delay.us());
  }
  if (frame.ntp_time_ms()) {
    frame.set_ntp_time_ms(scheduled_repeat_->origin_ntp_time_ms +
                          total_delay.ms());
  }
  SendFrameNow(frame);

  ScheduleRepeat(frame_id, HasQualityConverged());
}

void ZeroHertzAdapterMode::SendFrameNow(const VideoFrame& frame) const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_VERBOSE) << __func__ << " this " << this << " timestamp "
                       << frame.timestamp() << " timestamp_us "
                       << frame.timestamp_us() << " ntp_time_ms "
                       << frame.ntp_time_ms();


  callback_->OnFrame(/*post_time=*/clock_->CurrentTime(),
                     /*frames_scheduled_for_processing=*/1, frame);
}

TimeDelta ZeroHertzAdapterMode::RepeatDuration(bool idle_repeat) const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  return idle_repeat
             ? FrameCadenceAdapterInterface::kZeroHertzIdleRepeatRatePeriod
             : frame_delay_;
}

void ZeroHertzAdapterMode::MaybeStartRefreshFrameRequester() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DLOG(LS_VERBOSE) << __func__;
  if (!refresh_frame_requester_.Running()) {
    refresh_frame_requester_ = RepeatingTaskHandle::DelayedStart(
        queue_,
        FrameCadenceAdapterInterface::kOnDiscardedFrameRefreshFramePeriod *
            frame_delay_,
        [this] {
          RTC_DLOG(LS_VERBOSE) << __func__ << " RequestRefreshFrame";
          if (callback_)
            callback_->RequestRefreshFrame();
          return frame_delay_;
        });
  }
}

FrameCadenceAdapterImpl::FrameCadenceAdapterImpl(
    Clock* clock,
    TaskQueueBase* queue,
    const FieldTrialsView& field_trials)
    : clock_(clock),
      queue_(queue),
      zero_hertz_screenshare_enabled_(
          !field_trials.IsDisabled("WebRTC-ZeroHertzScreenshare")) {}

FrameCadenceAdapterImpl::~FrameCadenceAdapterImpl() {
  RTC_DLOG(LS_VERBOSE) << __func__ << " this " << this;
}

void FrameCadenceAdapterImpl::Initialize(Callback* callback) {
  callback_ = callback;
  passthrough_adapter_.emplace(clock_, callback);
  current_adapter_mode_ = &passthrough_adapter_.value();
}

void FrameCadenceAdapterImpl::SetZeroHertzModeEnabled(
    absl::optional<ZeroHertzModeParams> params) {
  RTC_DCHECK_RUN_ON(queue_);
  bool was_zero_hertz_enabled = zero_hertz_params_.has_value();
  if (params.has_value() && !was_zero_hertz_enabled)
    has_reported_screenshare_frame_rate_umas_ = false;
  zero_hertz_params_ = params;
  MaybeReconfigureAdapters(was_zero_hertz_enabled);
}

absl::optional<uint32_t> FrameCadenceAdapterImpl::GetInputFrameRateFps() {
  RTC_DCHECK_RUN_ON(queue_);
  return current_adapter_mode_->GetInputFrameRateFps();
}

void FrameCadenceAdapterImpl::UpdateFrameRate() {
  RTC_DCHECK_RUN_ON(queue_);



  passthrough_adapter_->UpdateFrameRate();
}

void FrameCadenceAdapterImpl::UpdateLayerQualityConvergence(
    size_t spatial_index,
    bool quality_converged) {
  if (zero_hertz_adapter_.has_value())
    zero_hertz_adapter_->UpdateLayerQualityConvergence(spatial_index,
                                                       quality_converged);
}

void FrameCadenceAdapterImpl::UpdateLayerStatus(size_t spatial_index,
                                                bool enabled) {
  if (zero_hertz_adapter_.has_value())
    zero_hertz_adapter_->UpdateLayerStatus(spatial_index, enabled);
}

void FrameCadenceAdapterImpl::ProcessKeyFrameRequest() {
  RTC_DCHECK_RUN_ON(queue_);
  if (zero_hertz_adapter_)
    zero_hertz_adapter_->ProcessKeyFrameRequest();
}

void FrameCadenceAdapterImpl::OnFrame(const VideoFrame& frame) {


  RTC_DCHECK_RUNS_SERIALIZED(&incoming_frame_race_checker_);
  RTC_DLOG(LS_VERBOSE) << "FrameCadenceAdapterImpl::" << __func__ << " this "
                       << this;

  Timestamp post_time = clock_->CurrentTime();
  frames_scheduled_for_processing_.fetch_add(1, std::memory_order_relaxed);
  queue_->PostTask(SafeTask(safety_.flag(), [this, post_time, frame] {
    RTC_DCHECK_RUN_ON(queue_);
    if (zero_hertz_adapter_created_timestamp_.has_value()) {
      TimeDelta time_until_first_frame =
          clock_->CurrentTime() - *zero_hertz_adapter_created_timestamp_;
      zero_hertz_adapter_created_timestamp_ = absl::nullopt;
      RTC_HISTOGRAM_COUNTS_10000(
          "WebRTC.Screenshare.ZeroHz.TimeUntilFirstFrameMs",
          time_until_first_frame.ms());
    }

    const int frames_scheduled_for_processing =
        frames_scheduled_for_processing_.fetch_sub(1,
                                                   std::memory_order_relaxed);
    OnFrameOnMainQueue(post_time, frames_scheduled_for_processing,
                       std::move(frame));
    MaybeReportFrameRateConstraintUmas();
  }));
}

void FrameCadenceAdapterImpl::OnDiscardedFrame() {
  callback_->OnDiscardedFrame();
  queue_->PostTask(SafeTask(safety_.flag(), [this] {
    RTC_DCHECK_RUN_ON(queue_);
    if (zero_hertz_adapter_) {
      zero_hertz_adapter_->OnDiscardedFrame();
    }
  }));
}

void FrameCadenceAdapterImpl::OnConstraintsChanged(
    const VideoTrackSourceConstraints& constraints) {
  RTC_LOG(LS_INFO) << __func__ << " this " << this << " min_fps "
                   << constraints.min_fps.value_or(-1) << " max_fps "
                   << constraints.max_fps.value_or(-1);
  queue_->PostTask(SafeTask(safety_.flag(), [this, constraints] {
    RTC_DCHECK_RUN_ON(queue_);
    bool was_zero_hertz_enabled = IsZeroHertzScreenshareEnabled();
    source_constraints_ = constraints;
    MaybeReconfigureAdapters(was_zero_hertz_enabled);
  }));
}

void FrameCadenceAdapterImpl::OnFrameOnMainQueue(
    Timestamp post_time,
    int frames_scheduled_for_processing,
    const VideoFrame& frame) {
  RTC_DCHECK_RUN_ON(queue_);
  current_adapter_mode_->OnFrame(post_time, frames_scheduled_for_processing,
                                 frame);
}

bool FrameCadenceAdapterImpl::IsZeroHertzScreenshareEnabled() const {
  RTC_DCHECK_RUN_ON(queue_);
  return zero_hertz_screenshare_enabled_ && source_constraints_.has_value() &&
         source_constraints_->max_fps.value_or(-1) > 0 &&
         source_constraints_->min_fps.value_or(-1) == 0 &&
         zero_hertz_params_.has_value();
}

void FrameCadenceAdapterImpl::MaybeReconfigureAdapters(
    bool was_zero_hertz_enabled) {
  RTC_DCHECK_RUN_ON(queue_);
  bool is_zero_hertz_enabled = IsZeroHertzScreenshareEnabled();
  if (is_zero_hertz_enabled) {
    if (!was_zero_hertz_enabled) {
      zero_hertz_adapter_.emplace(queue_, clock_, callback_,
                                  source_constraints_->max_fps.value());
      RTC_LOG(LS_INFO) << "Zero hertz mode activated.";
      zero_hertz_adapter_created_timestamp_ = clock_->CurrentTime();
    }
    zero_hertz_adapter_->ReconfigureParameters(zero_hertz_params_.value());
    current_adapter_mode_ = &zero_hertz_adapter_.value();
  } else {
    if (was_zero_hertz_enabled)
      zero_hertz_adapter_ = absl::nullopt;
    current_adapter_mode_ = &passthrough_adapter_.value();
  }
}

void FrameCadenceAdapterImpl::MaybeReportFrameRateConstraintUmas() {
  RTC_DCHECK_RUN_ON(queue_);
  if (has_reported_screenshare_frame_rate_umas_)
    return;
  has_reported_screenshare_frame_rate_umas_ = true;
  if (!zero_hertz_params_.has_value())
    return;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Screenshare.FrameRateConstraints.Exists",
                        source_constraints_.has_value());
  if (!source_constraints_.has_value())
    return;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Screenshare.FrameRateConstraints.Min.Exists",
                        source_constraints_->min_fps.has_value());
  if (source_constraints_->min_fps.has_value()) {
    RTC_HISTOGRAM_COUNTS_100(
        "WebRTC.Screenshare.FrameRateConstraints.Min.Value",
        source_constraints_->min_fps.value());
  }
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Screenshare.FrameRateConstraints.Max.Exists",
                        source_constraints_->max_fps.has_value());
  if (source_constraints_->max_fps.has_value()) {
    RTC_HISTOGRAM_COUNTS_100(
        "WebRTC.Screenshare.FrameRateConstraints.Max.Value",
        source_constraints_->max_fps.value());
  }
  if (!source_constraints_->min_fps.has_value()) {
    if (source_constraints_->max_fps.has_value()) {
      RTC_HISTOGRAM_COUNTS_100(
          "WebRTC.Screenshare.FrameRateConstraints.MinUnset.Max",
          source_constraints_->max_fps.value());
    }
  } else if (source_constraints_->max_fps.has_value()) {
    if (source_constraints_->min_fps.value() <
        source_constraints_->max_fps.value()) {
      RTC_HISTOGRAM_COUNTS_100(
          "WebRTC.Screenshare.FrameRateConstraints.MinLessThanMax.Min",
          source_constraints_->min_fps.value());
      RTC_HISTOGRAM_COUNTS_100(
          "WebRTC.Screenshare.FrameRateConstraints.MinLessThanMax.Max",
          source_constraints_->max_fps.value());
    }



    constexpr int kMaxBucketCount =
        60 * /*max min_fps=*/60 + /*max max_fps=*/60 - 1;
    RTC_HISTOGRAM_ENUMERATION_SPARSE(
        "WebRTC.Screenshare.FrameRateConstraints.60MinPlusMaxMinusOne",
        source_constraints_->min_fps.value() * 60 +
            source_constraints_->max_fps.value() - 1,
        /*boundary=*/kMaxBucketCount);
  }
}

}  // namespace

std::unique_ptr<FrameCadenceAdapterInterface>
FrameCadenceAdapterInterface::Create(Clock* clock,
                                     TaskQueueBase* queue,
                                     const FieldTrialsView& field_trials) {
  return std::make_unique<FrameCadenceAdapterImpl>(clock, queue, field_trials);
}

}  // namespace webrtc
