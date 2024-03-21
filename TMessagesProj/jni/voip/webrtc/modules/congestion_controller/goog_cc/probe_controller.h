/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_CONGESTION_CONTROLLER_GOOG_CC_PROBE_CONTROLLER_H_
#define MODULES_CONGESTION_CONTROLLER_GOOG_CC_PROBE_CONTROLLER_H_

#include <stdint.h>

#include <initializer_list>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/rtc_event_log/rtc_event_log.h"
#include "api/transport/network_control.h"
#include "api/transport/network_types.h"
#include "api/units/data_rate.h"
#include "api/units/timestamp.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace webrtc {

struct ProbeControllerConfig {
  explicit ProbeControllerConfig(const FieldTrialsView* key_value_config);
  ProbeControllerConfig(const ProbeControllerConfig&);
  ProbeControllerConfig& operator=(const ProbeControllerConfig&) = default;
  ~ProbeControllerConfig();





  FieldTrialParameter<double> first_exponential_probe_scale;
  FieldTrialOptional<double> second_exponential_probe_scale;
  FieldTrialParameter<double> further_exponential_probe_scale;
  FieldTrialParameter<double> further_probe_threshold;

  FieldTrialParameter<TimeDelta> alr_probing_interval;
  FieldTrialParameter<double> alr_probe_scale;

  FieldTrialParameter<TimeDelta> network_state_estimate_probing_interval;


  FieldTrialParameter<double> network_state_estimate_fast_rampup_rate;


  FieldTrialParameter<double> network_state_estimate_drop_down_rate;
  FieldTrialParameter<double> network_state_probe_scale;


  FieldTrialParameter<TimeDelta> network_state_probe_duration;

  FieldTrialOptional<double> first_allocation_probe_scale;
  FieldTrialOptional<double> second_allocation_probe_scale;
  FieldTrialFlag allocation_allow_further_probing;
  FieldTrialParameter<DataRate> allocation_probe_max;

  FieldTrialParameter<int> min_probe_packets_sent;

  FieldTrialParameter<TimeDelta> min_probe_duration;


  FieldTrialParameter<bool> limit_probe_target_rate_to_loss_bwe;


  FieldTrialParameter<double> skip_if_estimate_larger_than_fraction_of_max;
};

// capacity. There is also support for probing during a session when max
// bitrate is adjusted by an application.
class ProbeController {
 public:
  explicit ProbeController(const FieldTrialsView* key_value_config,
                           RtcEventLog* event_log);
  ~ProbeController();

  ProbeController(const ProbeController&) = delete;
  ProbeController& operator=(const ProbeController&) = delete;

  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig> SetBitrates(
      DataRate min_bitrate,
      DataRate start_bitrate,
      DataRate max_bitrate,
      Timestamp at_time);


  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig>
  OnMaxTotalAllocatedBitrate(DataRate max_total_allocated_bitrate,
                             Timestamp at_time);

  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig> OnNetworkAvailability(
      NetworkAvailability msg);

  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig> SetEstimatedBitrate(
      DataRate bitrate,
      bool bwe_limited_due_to_packet_loss,
      Timestamp at_time);

  void EnablePeriodicAlrProbing(bool enable);

  void SetAlrStartTimeMs(absl::optional<int64_t> alr_start_time);
  void SetAlrEndedTimeMs(int64_t alr_end_time);

  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig> RequestProbe(
      Timestamp at_time);

  void SetMaxBitrate(DataRate max_bitrate);
  void SetNetworkStateEstimate(webrtc::NetworkStateEstimate estimate);


  void Reset(Timestamp at_time);

  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig> Process(
      Timestamp at_time);

 private:
  enum class State {

    kInit,

    kWaitingForProbingResult,

    kProbingComplete,
  };

  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig>
  InitiateExponentialProbing(Timestamp at_time);
  ABSL_MUST_USE_RESULT std::vector<ProbeClusterConfig> InitiateProbing(
      Timestamp now,
      std::vector<DataRate> bitrates_to_probe,
      bool probe_further);
  bool TimeForAlrProbe(Timestamp at_time) const;
  bool TimeForNetworkStateProbe(Timestamp at_time) const;

  bool network_available_;
  bool bwe_limited_due_to_packet_loss_;
  State state_;
  DataRate min_bitrate_to_probe_further_ = DataRate::PlusInfinity();
  Timestamp time_last_probing_initiated_ = Timestamp::MinusInfinity();
  DataRate estimated_bitrate_ = DataRate::Zero();
  bool send_probe_on_next_process_interval_;
  absl::optional<webrtc::NetworkStateEstimate> network_estimate_;
  DataRate start_bitrate_ = DataRate::Zero();
  DataRate max_bitrate_ = DataRate::PlusInfinity();
  Timestamp last_bwe_drop_probing_time_ = Timestamp::Zero();
  absl::optional<Timestamp> alr_start_time_;
  absl::optional<Timestamp> alr_end_time_;
  bool enable_periodic_alr_probing_;
  Timestamp time_of_last_large_drop_ = Timestamp::MinusInfinity();
  DataRate bitrate_before_last_large_drop_ = DataRate::Zero();
  DataRate max_total_allocated_bitrate_ = DataRate::Zero();

  const bool in_rapid_recovery_experiment_;

  bool mid_call_probing_waiting_for_result_;
  DataRate mid_call_probing_bitrate_ = DataRate::Zero();
  DataRate mid_call_probing_succcess_threshold_ = DataRate::Zero();
  RtcEventLog* event_log_;

  int32_t next_probe_cluster_id_ = 1;

  ProbeControllerConfig config_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_PROBE_CONTROLLER_H_
