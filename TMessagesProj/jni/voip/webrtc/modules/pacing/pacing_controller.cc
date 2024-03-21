/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/pacing/pacing_controller.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "modules/pacing/bitrate_prober.h"
#include "modules/pacing/interval_budget.h"
#include "rtc_base/checks.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {
namespace {
// Time limit in milliseconds between packet bursts.
constexpr TimeDelta kDefaultMinPacketLimit = TimeDelta::Millis(5);
constexpr TimeDelta kCongestedPacketInterval = TimeDelta::Millis(500);
// TODO(sprang): Consider dropping this limit.
// The maximum debt level, in terms of time, capped when sending packets.
constexpr TimeDelta kMaxDebtInTime = TimeDelta::Millis(500);
constexpr TimeDelta kMaxElapsedTime = TimeDelta::Seconds(2);
constexpr TimeDelta kTargetPaddingDuration = TimeDelta::Millis(5);

bool IsDisabled(const FieldTrialsView& field_trials, absl::string_view key) {
  return absl::StartsWith(field_trials.Lookup(key), "Disabled");
}

bool IsEnabled(const FieldTrialsView& field_trials, absl::string_view key) {
  return absl::StartsWith(field_trials.Lookup(key), "Enabled");
}

}  // namespace

const TimeDelta PacingController::kMaxExpectedQueueLength =
    TimeDelta::Millis(2000);
const TimeDelta PacingController::kPausedProcessInterval =
    kCongestedPacketInterval;
const TimeDelta PacingController::kMinSleepTime = TimeDelta::Millis(1);
const TimeDelta PacingController::kMaxEarlyProbeProcessing =
    TimeDelta::Millis(1);

PacingController::PacingController(Clock* clock,
                                   PacketSender* packet_sender,
                                   const FieldTrialsView& field_trials)
    : clock_(clock),
      packet_sender_(packet_sender),
      field_trials_(field_trials),
      drain_large_queues_(
          !IsDisabled(field_trials_, "WebRTC-Pacer-DrainQueue")),
      send_padding_if_silent_(
          IsEnabled(field_trials_, "WebRTC-Pacer-PadInSilence")),
      pace_audio_(IsEnabled(field_trials_, "WebRTC-Pacer-BlockAudio")),
      ignore_transport_overhead_(
          IsEnabled(field_trials_, "WebRTC-Pacer-IgnoreTransportOverhead")),
      fast_retransmissions_(
          IsEnabled(field_trials_, "WebRTC-Pacer-FastRetransmissions")),
      min_packet_limit_(kDefaultMinPacketLimit),
      transport_overhead_per_packet_(DataSize::Zero()),
      send_burst_interval_(TimeDelta::Zero()),
      last_timestamp_(clock_->CurrentTime()),
      paused_(false),
      media_debt_(DataSize::Zero()),
      padding_debt_(DataSize::Zero()),
      pacing_rate_(DataRate::Zero()),
      adjusted_media_rate_(DataRate::Zero()),
      padding_rate_(DataRate::Zero()),
      prober_(field_trials_),
      probing_send_failure_(false),
      last_process_time_(clock->CurrentTime()),
      last_send_time_(last_process_time_),
      seen_first_packet_(false),
      packet_queue_(/*creation_time=*/last_process_time_),
      congested_(false),
      queue_time_limit_(kMaxExpectedQueueLength),
      account_for_audio_(false),
      include_overhead_(false) {
  if (!drain_large_queues_) {
    RTC_LOG(LS_WARNING) << "Pacer queues will not be drained,"
                           "pushback experiment must be enabled.";
  }
  FieldTrialParameter<int> min_packet_limit_ms("", min_packet_limit_.ms());
  ParseFieldTrial({&min_packet_limit_ms},
                  field_trials_.Lookup("WebRTC-Pacer-MinPacketLimitMs"));
  min_packet_limit_ = TimeDelta::Millis(min_packet_limit_ms.Get());
  UpdateBudgetWithElapsedTime(min_packet_limit_);
}

PacingController::~PacingController() = default;

void PacingController::CreateProbeCluster(DataRate bitrate, int cluster_id) {
  prober_.CreateProbeCluster({.at_time = CurrentTime(),
                              .target_data_rate = bitrate,
                              .target_duration = TimeDelta::Millis(15),
                              .target_probe_count = 5,
                              .id = cluster_id});
}

void PacingController::CreateProbeClusters(
    rtc::ArrayView<const ProbeClusterConfig> probe_cluster_configs) {
  for (const ProbeClusterConfig probe_cluster_config : probe_cluster_configs) {
    prober_.CreateProbeCluster(probe_cluster_config);
  }
}

void PacingController::Pause() {
  if (!paused_)
    RTC_LOG(LS_INFO) << "PacedSender paused.";
  paused_ = true;
  packet_queue_.SetPauseState(true, CurrentTime());
}

void PacingController::Resume() {
  if (paused_)
    RTC_LOG(LS_INFO) << "PacedSender resumed.";
  paused_ = false;
  packet_queue_.SetPauseState(false, CurrentTime());
}

bool PacingController::IsPaused() const {
  return paused_;
}

void PacingController::SetCongested(bool congested) {
  if (congested_ && !congested) {
    UpdateBudgetWithElapsedTime(UpdateTimeAndGetElapsed(CurrentTime()));
  }
  congested_ = congested;
}

bool PacingController::IsProbing() const {
  return prober_.is_probing();
}

Timestamp PacingController::CurrentTime() const {
  Timestamp time = clock_->CurrentTime();
  if (time < last_timestamp_) {
    RTC_LOG(LS_WARNING)
        << "Non-monotonic clock behavior observed. Previous timestamp: "
        << last_timestamp_.ms() << ", new timestamp: " << time.ms();
    RTC_DCHECK_GE(time, last_timestamp_);
    time = last_timestamp_;
  }
  last_timestamp_ = time;
  return time;
}

void PacingController::SetProbingEnabled(bool enabled) {
  RTC_CHECK(!seen_first_packet_);
  prober_.SetEnabled(enabled);
}

void PacingController::SetPacingRates(DataRate pacing_rate,
                                      DataRate padding_rate) {
  static constexpr DataRate kMaxRate = DataRate::KilobitsPerSec(100'000);
  RTC_CHECK_GT(pacing_rate, DataRate::Zero());
  RTC_CHECK_GE(padding_rate, DataRate::Zero());
  if (padding_rate > pacing_rate) {
    RTC_LOG(LS_WARNING) << "Padding rate " << padding_rate.kbps()
                        << "kbps is higher than the pacing rate "
                        << pacing_rate.kbps() << "kbps, capping.";
    padding_rate = pacing_rate;
  }

  if (pacing_rate > kMaxRate || padding_rate > kMaxRate) {
    RTC_LOG(LS_WARNING) << "Very high pacing rates ( > " << kMaxRate.kbps()
                        << " kbps) configured: pacing = " << pacing_rate.kbps()
                        << " kbps, padding = " << padding_rate.kbps()
                        << " kbps.";
  }
  pacing_rate_ = pacing_rate;
  padding_rate_ = padding_rate;
  MaybeUpdateMediaRateDueToLongQueue(CurrentTime());

  RTC_LOG(LS_VERBOSE) << "bwe:pacer_updated pacing_kbps=" << pacing_rate_.kbps()
                      << " padding_budget_kbps=" << padding_rate.kbps();
}

void PacingController::EnqueuePacket(std::unique_ptr<RtpPacketToSend> packet) {
  RTC_DCHECK(pacing_rate_ > DataRate::Zero())
      << "SetPacingRate must be called before InsertPacket.";
  RTC_CHECK(packet->packet_type());

  prober_.OnIncomingPacket(DataSize::Bytes(packet->payload_size()));

  const Timestamp now = CurrentTime();
  if (packet_queue_.Empty()) {



    Timestamp target_process_time = now;
    Timestamp next_send_time = NextSendTime();
    if (next_send_time.IsFinite()) {


      target_process_time = std::min(now, next_send_time);
    }
    UpdateBudgetWithElapsedTime(UpdateTimeAndGetElapsed(target_process_time));
  }
  packet_queue_.Push(now, std::move(packet));
  seen_first_packet_ = true;

  MaybeUpdateMediaRateDueToLongQueue(now);
}

void PacingController::SetAccountForAudioPackets(bool account_for_audio) {
  account_for_audio_ = account_for_audio;
}

void PacingController::SetIncludeOverhead() {
  include_overhead_ = true;
}

void PacingController::SetTransportOverhead(DataSize overhead_per_packet) {
  if (ignore_transport_overhead_)
    return;
  transport_overhead_per_packet_ = overhead_per_packet;
}

void PacingController::SetSendBurstInterval(TimeDelta burst_interval) {
  send_burst_interval_ = burst_interval;
}

TimeDelta PacingController::ExpectedQueueTime() const {
  RTC_DCHECK_GT(adjusted_media_rate_, DataRate::Zero());
  return QueueSizeData() / adjusted_media_rate_;
}

size_t PacingController::QueueSizePackets() const {
  return rtc::checked_cast<size_t>(packet_queue_.SizeInPackets());
}

const std::array<int, kNumMediaTypes>&
PacingController::SizeInPacketsPerRtpPacketMediaType() const {
  return packet_queue_.SizeInPacketsPerRtpPacketMediaType();
}

DataSize PacingController::QueueSizeData() const {
  DataSize size = packet_queue_.SizeInPayloadBytes();
  if (include_overhead_) {
    size += static_cast<int64_t>(packet_queue_.SizeInPackets()) *
            transport_overhead_per_packet_;
  }
  return size;
}

DataSize PacingController::CurrentBufferLevel() const {
  return std::max(media_debt_, padding_debt_);
}

absl::optional<Timestamp> PacingController::FirstSentPacketTime() const {
  return first_sent_packet_time_;
}

Timestamp PacingController::OldestPacketEnqueueTime() const {
  return packet_queue_.OldestEnqueueTime();
}

TimeDelta PacingController::UpdateTimeAndGetElapsed(Timestamp now) {


  if (last_process_time_.IsMinusInfinity() || now < last_process_time_) {
    return TimeDelta::Zero();
  }
  TimeDelta elapsed_time = now - last_process_time_;
  last_process_time_ = now;
  if (elapsed_time > kMaxElapsedTime) {
    RTC_LOG(LS_WARNING) << "Elapsed time (" << elapsed_time.ms()
                        << " ms) longer than expected, limiting to "
                        << kMaxElapsedTime.ms();
    elapsed_time = kMaxElapsedTime;
  }
  return elapsed_time;
}

bool PacingController::ShouldSendKeepalive(Timestamp now) const {
  if (send_padding_if_silent_ || paused_ || congested_ || !seen_first_packet_) {


    if (now - last_send_time_ >= kCongestedPacketInterval) {
      return true;
    }
  }
  return false;
}

Timestamp PacingController::NextSendTime() const {
  const Timestamp now = CurrentTime();
  Timestamp next_send_time = Timestamp::PlusInfinity();

  if (paused_) {
    return last_send_time_ + kPausedProcessInterval;
  }

  if (prober_.is_probing() && !probing_send_failure_) {
    Timestamp probe_time = prober_.NextProbeTime(now);
    if (!probe_time.IsPlusInfinity()) {
      return probe_time.IsMinusInfinity() ? now : probe_time;
    }
  }


  Timestamp unpaced_send_time = NextUnpacedSendTime();
  if (unpaced_send_time.IsFinite()) {
    return unpaced_send_time;
  }

  if (congested_ || !seen_first_packet_) {

    return last_send_time_ + kCongestedPacketInterval;
  }

  if (adjusted_media_rate_ > DataRate::Zero() && !packet_queue_.Empty()) {



    TimeDelta drain_time = media_debt_ / adjusted_media_rate_;
    next_send_time =
        last_process_time_ +
        ((send_burst_interval_ > drain_time) ? TimeDelta::Zero() : drain_time);
  } else if (padding_rate_ > DataRate::Zero() && packet_queue_.Empty()) {



    RTC_DCHECK_GT(adjusted_media_rate_, DataRate::Zero());
    TimeDelta drain_time = std::max(media_debt_ / adjusted_media_rate_,
                                    padding_debt_ / padding_rate_);

    if (drain_time.IsZero() &&
        (!media_debt_.IsZero() || !padding_debt_.IsZero())) {


      drain_time = TimeDelta::Micros(1);
    }
    next_send_time = last_process_time_ + drain_time;
  } else {

    next_send_time = last_process_time_ + kPausedProcessInterval;
  }

  if (send_padding_if_silent_) {
    next_send_time =
        std::min(next_send_time, last_send_time_ + kPausedProcessInterval);
  }

  return next_send_time;
}

void PacingController::ProcessPackets() {
  const Timestamp now = CurrentTime();
  Timestamp target_send_time = now;

  if (ShouldSendKeepalive(now)) {
    DataSize keepalive_data_sent = DataSize::Zero();


    if (seen_first_packet_) {
      std::vector<std::unique_ptr<RtpPacketToSend>> keepalive_packets =
          packet_sender_->GeneratePadding(DataSize::Bytes(1));
      for (auto& packet : keepalive_packets) {
        keepalive_data_sent +=
            DataSize::Bytes(packet->payload_size() + packet->padding_size());
        packet_sender_->SendPacket(std::move(packet), PacedPacketInfo());
        for (auto& packet : packet_sender_->FetchFec()) {
          EnqueuePacket(std::move(packet));
        }
      }
    }
    OnPacketSent(RtpPacketMediaType::kPadding, keepalive_data_sent, now);
  }

  if (paused_) {
    return;
  }

  TimeDelta early_execute_margin =
      prober_.is_probing() ? kMaxEarlyProbeProcessing : TimeDelta::Zero();

  target_send_time = NextSendTime();
  if (now + early_execute_margin < target_send_time) {


    UpdateBudgetWithElapsedTime(UpdateTimeAndGetElapsed(now));
    return;
  }

  TimeDelta elapsed_time = UpdateTimeAndGetElapsed(target_send_time);

  if (elapsed_time > TimeDelta::Zero()) {
    UpdateBudgetWithElapsedTime(elapsed_time);
  }

  PacedPacketInfo pacing_info;
  DataSize recommended_probe_size = DataSize::Zero();
  bool is_probing = prober_.is_probing();
  if (is_probing) {


    pacing_info = prober_.CurrentCluster(now).value_or(PacedPacketInfo());
    if (pacing_info.probe_cluster_id != PacedPacketInfo::kNotAProbe) {
      recommended_probe_size = prober_.RecommendedMinProbeSize();
      RTC_DCHECK_GT(recommended_probe_size, DataSize::Zero());
    } else {

      is_probing = false;
    }
  }

  DataSize data_sent = DataSize::Zero();

  static constexpr int kMaxIterations = 1 << 16;
  int iteration = 0;
  int packets_sent = 0;
  int padding_packets_generated = 0;
  for (; iteration < kMaxIterations; ++iteration) {


    std::unique_ptr<RtpPacketToSend> rtp_packet =
        GetPendingPacket(pacing_info, target_send_time, now);
    if (rtp_packet == nullptr) {

      DataSize padding_to_add = PaddingToAdd(recommended_probe_size, data_sent);
      if (padding_to_add > DataSize::Zero()) {
        std::vector<std::unique_ptr<RtpPacketToSend>> padding_packets =
            packet_sender_->GeneratePadding(padding_to_add);
        if (!padding_packets.empty()) {
          padding_packets_generated += padding_packets.size();
          for (auto& packet : padding_packets) {
            EnqueuePacket(std::move(packet));
          }

          continue;
        } else {


          UpdatePaddingBudgetWithSentData(padding_to_add);
        }
      }

      break;
    } else {
      RTC_DCHECK(rtp_packet);
      RTC_DCHECK(rtp_packet->packet_type().has_value());
      const RtpPacketMediaType packet_type = *rtp_packet->packet_type();
      DataSize packet_size = DataSize::Bytes(rtp_packet->payload_size() +
                                             rtp_packet->padding_size());

      if (include_overhead_) {
        packet_size += DataSize::Bytes(rtp_packet->headers_size()) +
                       transport_overhead_per_packet_;
      }

      packet_sender_->SendPacket(std::move(rtp_packet), pacing_info);
      for (auto& packet : packet_sender_->FetchFec()) {
        EnqueuePacket(std::move(packet));
      }
      data_sent += packet_size;
      ++packets_sent;

      OnPacketSent(packet_type, packet_size, now);

      if (is_probing) {
        pacing_info.probe_cluster_bytes_sent += packet_size.bytes();


        if (data_sent >= recommended_probe_size) {
          break;
        }
      }


      target_send_time = NextSendTime();
      if (target_send_time > now) {

        if (!is_probing) {
          break;
        }
        target_send_time = now;
      }
      UpdateBudgetWithElapsedTime(UpdateTimeAndGetElapsed(target_send_time));
    }
  }

  if (iteration >= kMaxIterations) {


    RTC_LOG(LS_ERROR) << "PacingController exceeded max iterations in "
                         "send-loop: packets sent = "
                      << packets_sent << ", padding packets generated = "
                      << padding_packets_generated
                      << ", bytes sent = " << data_sent.bytes();
    last_send_time_ = now;
    last_process_time_ = now;
    return;
  }

  if (is_probing) {
    probing_send_failure_ = data_sent == DataSize::Zero();
    if (!probing_send_failure_) {
      prober_.ProbeSent(CurrentTime(), data_sent);
    }
  }



  MaybeUpdateMediaRateDueToLongQueue(CurrentTime());
}

DataSize PacingController::PaddingToAdd(DataSize recommended_probe_size,
                                        DataSize data_sent) const {
  if (!packet_queue_.Empty()) {

    return DataSize::Zero();
  }

  if (congested_) {

    return DataSize::Zero();
  }

  if (!seen_first_packet_) {


    return DataSize::Zero();
  }

  if (!recommended_probe_size.IsZero()) {
    if (recommended_probe_size > data_sent) {
      return recommended_probe_size - data_sent;
    }
    return DataSize::Zero();
  }

  if (padding_rate_ > DataRate::Zero() && padding_debt_ == DataSize::Zero()) {
    return kTargetPaddingDuration * padding_rate_;
  }
  return DataSize::Zero();
}

std::unique_ptr<RtpPacketToSend> PacingController::GetPendingPacket(
    const PacedPacketInfo& pacing_info,
    Timestamp target_send_time,
    Timestamp now) {
  const bool is_probe =
      pacing_info.probe_cluster_id != PacedPacketInfo::kNotAProbe;


  if (is_probe && pacing_info.probe_cluster_bytes_sent == 0) {
    auto padding = packet_sender_->GeneratePadding(DataSize::Bytes(1));


    if (!padding.empty()) {


      RTC_DCHECK_EQ(padding.size(), 1u);
      return std::move(padding[0]);
    }
  }

  if (packet_queue_.Empty()) {
    return nullptr;
  }


  if (NextUnpacedSendTime().IsInfinite() && !is_probe) {
    if (congested_) {

      return nullptr;
    }

    if (now <= target_send_time && send_burst_interval_.IsZero()) {





      TimeDelta flush_time = media_debt_ / adjusted_media_rate_;
      if (now + flush_time > target_send_time) {
        return nullptr;
      }
    }
  }

  return packet_queue_.Pop();
}

void PacingController::OnPacketSent(RtpPacketMediaType packet_type,
                                    DataSize packet_size,
                                    Timestamp send_time) {
  if (!first_sent_packet_time_ && packet_type != RtpPacketMediaType::kPadding) {
    first_sent_packet_time_ = send_time;
  }

  bool audio_packet = packet_type == RtpPacketMediaType::kAudio;
  if ((!audio_packet || account_for_audio_) && packet_size > DataSize::Zero()) {
    UpdateBudgetWithSentData(packet_size);
  }

  last_send_time_ = send_time;
}

void PacingController::UpdateBudgetWithElapsedTime(TimeDelta delta) {
  media_debt_ -= std::min(media_debt_, adjusted_media_rate_ * delta);
  padding_debt_ -= std::min(padding_debt_, padding_rate_ * delta);
}

void PacingController::UpdateBudgetWithSentData(DataSize size) {
  media_debt_ += size;
  media_debt_ = std::min(media_debt_, adjusted_media_rate_ * kMaxDebtInTime);
  UpdatePaddingBudgetWithSentData(size);
}

void PacingController::UpdatePaddingBudgetWithSentData(DataSize size) {
  padding_debt_ += size;
  padding_debt_ = std::min(padding_debt_, padding_rate_ * kMaxDebtInTime);
}

void PacingController::SetQueueTimeLimit(TimeDelta limit) {
  queue_time_limit_ = limit;
}

void PacingController::MaybeUpdateMediaRateDueToLongQueue(Timestamp now) {
  adjusted_media_rate_ = pacing_rate_;
  if (!drain_large_queues_) {
    return;
  }

  DataSize queue_size_data = QueueSizeData();
  if (queue_size_data > DataSize::Zero()) {



    packet_queue_.UpdateAverageQueueTime(now);
    TimeDelta avg_time_left =
        std::max(TimeDelta::Millis(1),
                 queue_time_limit_ - packet_queue_.AverageQueueTime());
    DataRate min_rate_needed = queue_size_data / avg_time_left;
    if (min_rate_needed > pacing_rate_) {
      adjusted_media_rate_ = min_rate_needed;
      RTC_LOG(LS_VERBOSE) << "bwe:large_pacing_queue pacing_rate_kbps="
                          << pacing_rate_.kbps();
    }
  }
}

Timestamp PacingController::NextUnpacedSendTime() const {
  if (!pace_audio_) {
    Timestamp leading_audio_send_time =
        packet_queue_.LeadingPacketEnqueueTime(RtpPacketMediaType::kAudio);
    if (leading_audio_send_time.IsFinite()) {
      return leading_audio_send_time;
    }
  }
  if (fast_retransmissions_) {
    Timestamp leading_retransmission_send_time =
        packet_queue_.LeadingPacketEnqueueTime(
            RtpPacketMediaType::kRetransmission);
    if (leading_retransmission_send_time.IsFinite()) {
      return leading_retransmission_send_time;
    }
  }
  return Timestamp::MinusInfinity();
}

}  // namespace webrtc
