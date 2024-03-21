/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SYSTEM_WRAPPERS_INCLUDE_RTP_TO_NTP_ESTIMATOR_H_
#define SYSTEM_WRAPPERS_INCLUDE_RTP_TO_NTP_ESTIMATOR_H_

#include <stdint.h>

#include <list>

#include "absl/types/optional.h"
#include "modules/include/module_common_types_public.h"
#include "rtc_base/checks.h"
#include "system_wrappers/include/ntp_time.h"

namespace webrtc {

// The class needs to be trained with (at least 2) RTP/NTP timestamp pairs from
// RTCP sender reports before the convertion can be done.
class RtpToNtpEstimator {
 public:
  static constexpr int kMaxInvalidSamples = 3;

  RtpToNtpEstimator() = default;
  RtpToNtpEstimator(const RtpToNtpEstimator&) = delete;
  RtpToNtpEstimator& operator=(const RtpToNtpEstimator&) = delete;
  ~RtpToNtpEstimator() = default;

  enum UpdateResult { kInvalidMeasurement, kSameMeasurement, kNewMeasurement };

  UpdateResult UpdateMeasurements(NtpTime ntp, uint32_t rtp_timestamp);


  NtpTime Estimate(uint32_t rtp_timestamp) const;

  double EstimatedFrequencyKhz() const;

 private:



  struct Parameters {
    double slope;
    double offset;
  };

  struct RtcpMeasurement {
    NtpTime ntp_time;
    int64_t unwrapped_rtp_timestamp;
  };

  void UpdateParameters();

  int consecutive_invalid_samples_ = 0;
  std::list<RtcpMeasurement> measurements_;
  absl::optional<Parameters> params_;
  mutable TimestampUnwrapper unwrapper_;
};
}  // namespace webrtc

#endif  // SYSTEM_WRAPPERS_INCLUDE_RTP_TO_NTP_ESTIMATOR_H_
