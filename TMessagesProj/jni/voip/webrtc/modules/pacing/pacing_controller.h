/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_PACING_CONTROLLER_H_
#define MODULES_PACING_PACING_CONTROLLER_H_

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/function_view.h"
#include "api/transport/field_trial_based_config.h"
#include "api/transport/network_types.h"
#include "modules/pacing/bitrate_prober.h"
#include "modules/pacing/interval_budget.h"
#include "modules/pacing/prioritized_packet_queue.h"
#include "modules/pacing/rtp_packet_pacer.h"
#include "modules/rtp_rtcp/include/rtp_packet_sender.h"
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {

// logic of determining which packets to send when, but the actual timing of
// the processing is done externally (e.g. RtpPacketPacer). Furthermore, the
// forwarding of packets when they are ready to be sent is also handled
// externally, via the PacingController::PacketSender interface.
class PacingController {
 public:
  class PacketSender {
   public:
    virtual ~PacketSender() = default;
    virtual void SendPacket(std::unique_ptr<RtpPacketToSend> packet,
                            const PacedPacketInfo& cluster_info) = 0;

    virtual std::vector<std::unique_ptr<RtpPacketToSend>> FetchFec() = 0;
    virtual std::vector<std::unique_ptr<RtpPacketToSend>> GeneratePadding(
        DataSize size) = 0;


    virtual void OnAbortedRetransmissions(
        uint32_t ssrc,
        rtc::ArrayView<const uint16_t> sequence_numbers) {}
    virtual absl::optional<uint32_t> GetRtxSsrcForMedia(uint32_t ssrc) const {
      return absl::nullopt;
    }
  };




  static const TimeDelta kMaxExpectedQueueLength;



  static const TimeDelta kPausedProcessInterval;

  static const TimeDelta kMinSleepTime;



  static const TimeDelta kMaxEarlyProbeProcessing;

  PacingController(Clock* clock,
                   PacketSender* packet_sender,
                   const FieldTrialsView& field_trials);

  ~PacingController();


  void EnqueuePacket(std::unique_ptr<RtpPacketToSend> packet);

  void CreateProbeCluster(DataRate bitrate, int cluster_id);
  void CreateProbeClusters(
      rtc::ArrayView<const ProbeClusterConfig> probe_cluster_configs);

  void Pause();   // Temporarily pause all sending.
  void Resume();  // Resume sending packets.
  bool IsPaused() const;

  void SetCongested(bool congested);

  void SetPacingRates(DataRate pacing_rate, DataRate padding_rate);
  DataRate pacing_rate() const { return adjusted_media_rate_; }




  void SetAccountForAudioPackets(bool account_for_audio);
  void SetIncludeOverhead();

  void SetTransportOverhead(DataSize overhead_per_packet);



  void SetSendBurstInterval(TimeDelta burst_interval);

  Timestamp OldestPacketEnqueueTime() const;

  size_t QueueSizePackets() const;


  const std::array<int, kNumMediaTypes>& SizeInPacketsPerRtpPacketMediaType()
      const;

  DataSize QueueSizeData() const;

  DataSize CurrentBufferLevel() const;

  absl::optional<Timestamp> FirstSentPacketTime() const;


  TimeDelta ExpectedQueueTime() const;

  void SetQueueTimeLimit(TimeDelta limit);



  void SetProbingEnabled(bool enabled);

  Timestamp NextSendTime() const;


  void ProcessPackets();

  bool IsProbing() const;

 private:
  TimeDelta UpdateTimeAndGetElapsed(Timestamp now);
  bool ShouldSendKeepalive(Timestamp now) const;

  void UpdateBudgetWithElapsedTime(TimeDelta delta);
  void UpdateBudgetWithSentData(DataSize size);
  void UpdatePaddingBudgetWithSentData(DataSize size);

  DataSize PaddingToAdd(DataSize recommended_probe_size,
                        DataSize data_sent) const;

  std::unique_ptr<RtpPacketToSend> GetPendingPacket(
      const PacedPacketInfo& pacing_info,
      Timestamp target_send_time,
      Timestamp now);
  void OnPacketSent(RtpPacketMediaType packet_type,
                    DataSize packet_size,
                    Timestamp send_time);
  void MaybeUpdateMediaRateDueToLongQueue(Timestamp now);

  Timestamp CurrentTime() const;




  Timestamp NextUnpacedSendTime() const;

  Clock* const clock_;
  PacketSender* const packet_sender_;
  const FieldTrialsView& field_trials_;

  const bool drain_large_queues_;
  const bool send_padding_if_silent_;
  const bool pace_audio_;
  const bool ignore_transport_overhead_;
  const bool fast_retransmissions_;

  TimeDelta min_packet_limit_;
  DataSize transport_overhead_per_packet_;
  TimeDelta send_burst_interval_;


  mutable Timestamp last_timestamp_;
  bool paused_;

  DataSize media_debt_;
  DataSize padding_debt_;

  DataRate pacing_rate_;


  DataRate adjusted_media_rate_;


  DataRate padding_rate_;

  BitrateProber prober_;
  bool probing_send_failure_;

  Timestamp last_process_time_;
  Timestamp last_send_time_;
  absl::optional<Timestamp> first_sent_packet_time_;
  bool seen_first_packet_;

  PrioritizedPacketQueue packet_queue_;

  bool congested_;

  TimeDelta queue_time_limit_;
  bool account_for_audio_;
  bool include_overhead_;
};
}  // namespace webrtc

#endif  // MODULES_PACING_PACING_CONTROLLER_H_
