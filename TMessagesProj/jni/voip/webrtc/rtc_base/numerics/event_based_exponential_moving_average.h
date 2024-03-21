/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_NUMERICS_EVENT_BASED_EXPONENTIAL_MOVING_AVERAGE_H_
#define RTC_BASE_NUMERICS_EVENT_BASED_EXPONENTIAL_MOVING_AVERAGE_H_

#include <cmath>
#include <cstdint>
#include <limits>
#include "absl/types/optional.h"

namespace rtc {

/**
 * This class implements exponential moving average for time series
 * estimating both value, variance and variance of estimator based on
 * https://en.wikipedia.org/w/index.php?title=Moving_average&section=9#Application_to_measuring_computer_performance
 * with the additions from nisse@ added to
 * https://en.wikipedia.org/wiki/Talk:Moving_average.
 *
 * A sample gets exponentially less weight so that it's 50%
 * after `half_time` time units.
 */
class EventBasedExponentialMovingAverage {
 public:


  explicit EventBasedExponentialMovingAverage(int half_time);

  void AddSample(int64_t now, int value);

  double GetAverage() const { return value_; }
  double GetVariance() const { return sample_variance_; }








  double GetConfidenceInterval() const;

  void Reset();


  void SetHalfTime(int half_time);

 private:
  double tau_;
  double value_ = std::nan("uninit");
  double sample_variance_ = std::numeric_limits<double>::infinity();

  double estimator_variance_ = 1;
  absl::optional<int64_t> last_observation_timestamp_;
};

}  // namespace rtc

#endif  // RTC_BASE_NUMERICS_EVENT_BASED_EXPONENTIAL_MOVING_AVERAGE_H_
