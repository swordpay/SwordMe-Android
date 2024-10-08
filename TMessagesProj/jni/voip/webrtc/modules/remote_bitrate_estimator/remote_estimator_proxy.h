/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_REMOTE_BITRATE_ESTIMATOR_REMOTE_ESTIMATOR_PROXY_H_
#define MODULES_REMOTE_BITRATE_ESTIMATOR_REMOTE_ESTIMATOR_PROXY_H_

#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/rtp_headers.h"
#include "api/transport/network_control.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "modules/remote_bitrate_estimator/packet_arrival_map.h"
#include "modules/rtp_rtcp/source/rtcp_packet.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/numerics/sequence_number_util.h"
#include "rtc_base/synchronization/mutex.h"

namespace webrtc {

// receive side. It buffers a number of receive timestamps and then sends
// transport feedback messages back too the send side.
class RemoteEstimatorProxy {
 public:


  using TransportFeedbackSender = std::function<void(
      std::vector<std::unique_ptr<rtcp::RtcpPacket>> packets)>;
  RemoteEstimatorProxy(TransportFeedbackSender feedback_sender,
                       const FieldTrialsView* key_value_config,
                       NetworkStateEstimator* network_state_estimator);
  ~RemoteEstimatorProxy();

  struct Packet {
    Timestamp arrival_time;
    DataSize size;
    uint32_t ssrc;
    absl::optional<uint32_t> absolute_send_time_24bits;
    absl::optional<uint16_t> transport_sequence_number;
    absl::optional<FeedbackRequest> feedback_request;
  };
  void IncomingPacket(Packet packet);

  void IncomingPacket(int64_t arrival_time_ms,
                      size_t payload_size,
                      const RTPHeader& header);


  TimeDelta Process(Timestamp now);

  void OnBitrateChanged(int bitrate);
  void SetSendPeriodicFeedback(bool send_periodic_feedback);
  void SetTransportOverhead(DataSize overhead_per_packet);

 private:
  struct TransportWideFeedbackConfig {
    FieldTrialParameter<TimeDelta> back_window{"wind", TimeDelta::Millis(500)};
    FieldTrialParameter<TimeDelta> min_interval{"min", TimeDelta::Millis(50)};
    FieldTrialParameter<TimeDelta> max_interval{"max", TimeDelta::Millis(250)};
    FieldTrialParameter<TimeDelta> default_interval{"def",
                                                    TimeDelta::Millis(100)};
    FieldTrialParameter<double> bandwidth_fraction{"frac", 0.05};
    explicit TransportWideFeedbackConfig(
        const FieldTrialsView* key_value_config) {
      ParseFieldTrial({&back_window, &min_interval, &max_interval,
                       &default_interval, &bandwidth_fraction},
                      key_value_config->Lookup(
                          "WebRTC-Bwe-TransportWideFeedbackIntervals"));
    }
  };

  void MaybeCullOldPackets(int64_t sequence_number, Timestamp arrival_time)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(&lock_);
  void SendPeriodicFeedbacks() RTC_EXCLUSIVE_LOCKS_REQUIRED(&lock_);
  void SendFeedbackOnRequest(int64_t sequence_number,
                             const FeedbackRequest& feedback_request)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(&lock_);












  std::unique_ptr<rtcp::TransportFeedback> MaybeBuildFeedbackPacket(
      bool include_timestamps,
      int64_t begin_sequence_number_inclusive,
      int64_t end_sequence_number_exclusive,
      bool is_periodic_update) RTC_EXCLUSIVE_LOCKS_REQUIRED(&lock_);

  const TransportFeedbackSender feedback_sender_;
  const TransportWideFeedbackConfig send_config_;
  Timestamp last_process_time_;

  Mutex lock_;

  NetworkStateEstimator* const network_state_estimator_
      RTC_PT_GUARDED_BY(&lock_);
  uint32_t media_ssrc_ RTC_GUARDED_BY(&lock_);
  uint8_t feedback_packet_count_ RTC_GUARDED_BY(&lock_);
  SeqNumUnwrapper<uint16_t> unwrapper_ RTC_GUARDED_BY(&lock_);
  DataSize packet_overhead_ RTC_GUARDED_BY(&lock_);


  absl::optional<int64_t> periodic_window_start_seq_ RTC_GUARDED_BY(&lock_);

  PacketArrivalTimeMap packet_arrival_times_ RTC_GUARDED_BY(&lock_);

  TimeDelta send_interval_ RTC_GUARDED_BY(&lock_);
  bool send_periodic_feedback_ RTC_GUARDED_BY(&lock_);

  uint32_t previous_abs_send_time_ RTC_GUARDED_BY(&lock_);
  Timestamp abs_send_timestamp_ RTC_GUARDED_BY(&lock_);
};

}  // namespace webrtc

#endif  //  MODULES_REMOTE_BITRATE_ESTIMATOR_REMOTE_ESTIMATOR_PROXY_H_
