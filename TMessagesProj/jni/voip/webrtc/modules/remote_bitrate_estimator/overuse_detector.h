/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_REMOTE_BITRATE_ESTIMATOR_OVERUSE_DETECTOR_H_
#define MODULES_REMOTE_BITRATE_ESTIMATOR_OVERUSE_DETECTOR_H_

#include <stdint.h>

#include "api/field_trials_view.h"
#include "api/network_state_predictor.h"

namespace webrtc {

bool AdaptiveThresholdExperimentIsDisabled(
    const FieldTrialsView& key_value_config);

class OveruseDetector {
 public:
  explicit OveruseDetector(const FieldTrialsView* key_value_config);
  virtual ~OveruseDetector();

  OveruseDetector(const OveruseDetector&) = delete;
  OveruseDetector& operator=(const OveruseDetector&) = delete;






  BandwidthUsage Detect(double offset,
                        double timestamp_delta,
                        int num_of_deltas,
                        int64_t now_ms);

  BandwidthUsage State() const;

 private:
  void UpdateThreshold(double modified_offset, int64_t now_ms);
  void InitializeExperiment(const FieldTrialsView& key_value_config);

  bool in_experiment_;
  double k_up_;
  double k_down_;
  double overusing_time_threshold_;
  double threshold_;
  int64_t last_update_ms_;
  double prev_offset_;
  double time_over_using_;
  int overuse_counter_;
  BandwidthUsage hypothesis_;
};
}  // namespace webrtc

#endif  // MODULES_REMOTE_BITRATE_ESTIMATOR_OVERUSE_DETECTOR_H_
