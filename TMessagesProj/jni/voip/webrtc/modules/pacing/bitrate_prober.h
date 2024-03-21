/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_BITRATE_PROBER_H_
#define MODULES_PACING_BITRATE_PROBER_H_

#include <stddef.h>
#include <stdint.h>

#include <queue>

#include "api/transport/field_trial_based_config.h"
#include "api/transport/network_types.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace webrtc {
class RtcEventLog;

struct BitrateProberConfig {
  explicit BitrateProberConfig(const FieldTrialsView* key_value_config);
  BitrateProberConfig(const BitrateProberConfig&) = default;
  BitrateProberConfig& operator=(const BitrateProberConfig&) = default;
  ~BitrateProberConfig() = default;

  FieldTrialParameter<TimeDelta> min_probe_delta;

  FieldTrialParameter<TimeDelta> max_probe_delay;




  FieldTrialParameter<DataSize> min_packet_size;
};

// on being protected by the caller.
class BitrateProber {
 public:
  explicit BitrateProber(const FieldTrialsView& field_trials);
  ~BitrateProber();

  void SetEnabled(bool enable);



  bool is_probing() const { return probing_state_ == ProbingState::kActive; }



  void OnIncomingPacket(DataSize packet_size);

  void CreateProbeCluster(const ProbeClusterConfig& cluster_config);




  Timestamp NextProbeTime(Timestamp now) const;

  absl::optional<PacedPacketInfo> CurrentCluster(Timestamp now);



  DataSize RecommendedMinProbeSize() const;



  void ProbeSent(Timestamp now, DataSize size);

 private:
  enum class ProbingState {

    kDisabled,

    kInactive,


    kActive,


    kSuspended,
  };


  struct ProbeCluster {
    PacedPacketInfo pace_info;

    int sent_probes = 0;
    int sent_bytes = 0;
    Timestamp requested_at = Timestamp::MinusInfinity();
    Timestamp started_at = Timestamp::MinusInfinity();
    int retries = 0;
  };

  Timestamp CalculateNextProbeTime(const ProbeCluster& cluster) const;

  ProbingState probing_state_;



  std::queue<ProbeCluster> clusters_;

  Timestamp next_probe_time_;

  int total_probe_count_;
  int total_failed_probe_count_;

  BitrateProberConfig config_;
};

}  // namespace webrtc

#endif  // MODULES_PACING_BITRATE_PROBER_H_
