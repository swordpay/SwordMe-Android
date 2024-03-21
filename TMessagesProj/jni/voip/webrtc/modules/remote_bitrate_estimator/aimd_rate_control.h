/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_REMOTE_BITRATE_ESTIMATOR_AIMD_RATE_CONTROL_H_
#define MODULES_REMOTE_BITRATE_ESTIMATOR_AIMD_RATE_CONTROL_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/transport/network_types.h"
#include "api/units/data_rate.h"
#include "api/units/timestamp.h"
#include "modules/congestion_controller/goog_cc/link_capacity_estimator.h"
#include "modules/remote_bitrate_estimator/include/bwe_defines.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace webrtc {
// A rate control implementation based on additive increases of
// bitrate when no over-use is detected and multiplicative decreases when
// over-uses are detected. When we think the available bandwidth has changes or
// is unknown, we will switch to a "slow-start mode" where we increase
// multiplicatively.
class AimdRateControl {
 public:
  explicit AimdRateControl(const FieldTrialsView* key_value_config);
  AimdRateControl(const FieldTrialsView* key_value_config, bool send_side);
  ~AimdRateControl();



  bool ValidEstimate() const;
  void SetStartBitrate(DataRate start_bitrate);
  void SetMinBitrate(DataRate min_bitrate);
  TimeDelta GetFeedbackInterval() const;




  bool TimeToReduceFurther(Timestamp at_time,
                           DataRate estimated_throughput) const;

  bool InitialTimeToReduceFurther(Timestamp at_time) const;

  DataRate LatestEstimate() const;
  void SetRtt(TimeDelta rtt);
  DataRate Update(const RateControlInput* input, Timestamp at_time);
  void SetInApplicationLimitedRegion(bool in_alr);
  void SetEstimate(DataRate bitrate, Timestamp at_time);
  void SetNetworkStateEstimate(
      const absl::optional<NetworkStateEstimate>& estimate);

  double GetNearMaxIncreaseRateBpsPerSecond() const;

  TimeDelta GetExpectedBandwidthPeriod() const;

 private:
  enum class RateControlState { kRcHold, kRcIncrease, kRcDecrease };

  friend class GoogCcStatePrinter;







  void ChangeBitrate(const RateControlInput& input, Timestamp at_time);

  DataRate ClampBitrate(DataRate new_bitrate) const;
  DataRate MultiplicativeRateIncrease(Timestamp at_time,
                                      Timestamp last_ms,
                                      DataRate current_bitrate) const;
  DataRate AdditiveRateIncrease(Timestamp at_time, Timestamp last_time) const;
  void UpdateChangePeriod(Timestamp at_time);
  void ChangeState(const RateControlInput& input, Timestamp at_time);

  DataRate min_configured_bitrate_;
  DataRate max_configured_bitrate_;
  DataRate current_bitrate_;
  DataRate latest_estimated_throughput_;
  LinkCapacityEstimator link_capacity_;
  absl::optional<NetworkStateEstimate> network_estimate_;
  RateControlState rate_control_state_;
  Timestamp time_last_bitrate_change_;
  Timestamp time_last_bitrate_decrease_;
  Timestamp time_first_throughput_estimate_;
  bool bitrate_is_initialized_;
  double beta_;
  bool in_alr_;
  TimeDelta rtt_;
  const bool send_side_;
  const bool in_experiment_;


  const bool no_bitrate_increase_in_alr_;


  const bool estimate_bounded_backoff_;


  FieldTrialFlag disable_estimate_bounded_increase_{"Disabled"};
  FieldTrialParameter<double> estimate_bounded_increase_ratio_{"ratio", 1.0};
  FieldTrialParameter<bool> ignore_throughput_limit_if_network_estimate_{
      "ignore_acked", false};
  FieldTrialParameter<bool> increase_to_network_estimate_{"immediate_incr",
                                                          false};
  FieldTrialParameter<bool> ignore_network_estimate_decrease_{"ignore_decr",
                                                              false};
  absl::optional<DataRate> last_decrease_;
  FieldTrialOptional<TimeDelta> initial_backoff_interval_;
  FieldTrialFlag link_capacity_fix_;
};
}  // namespace webrtc

#endif  // MODULES_REMOTE_BITRATE_ESTIMATOR_AIMD_RATE_CONTROL_H_
