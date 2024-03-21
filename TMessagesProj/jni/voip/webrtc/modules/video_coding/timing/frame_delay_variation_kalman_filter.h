/*
 *  Copyright (c) 2022 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_VIDEO_CODING_TIMING_FRAME_DELAY_VARIATION_KALMAN_FILTER_H_
#define MODULES_VIDEO_CODING_TIMING_FRAME_DELAY_VARIATION_KALMAN_FILTER_H_

#include "api/units/data_size.h"
#include "api/units/time_delta.h"

namespace webrtc {

// https://en.wikipedia.org/wiki/Kalman_filter) to estimate the frame delay
// variation (i.e., the difference in transmission time between a frame and the
// prior frame) for a frame, given its size variation in bytes (i.e., the
// difference in size between a frame and the prior frame). The idea is that,
// given a fixed link bandwidth, a larger frame (in bytes) would take
// proportionally longer to arrive than a correspondingly smaller frame. Using
// the variations of frame delay and frame size, the underlying bandwidth and
// queuing delay variation of the network link can be estimated.
//
// The filter takes as input the frame delay variation, the difference between
// the actual inter-frame arrival time and the expected inter-frame arrival time
// (based on RTP timestamp), and frame size variation, the inter-frame size
// delta for a single frame. The frame delay variation is seen as the
// measurement and the frame size variation is used in the observation model.
// The hidden state of the filter is the link bandwidth and queuing delay
// buildup. The estimated state can be used to get the expected frame delay
// variation for a frame, given its frame size variation. This information can
// then be used to estimate the frame delay variation coming from network
// jitter.
//
// Mathematical details:
//  * The state (`x` in Wikipedia notation) is a 2x1 vector comprising the
//    reciprocal of link bandwidth [1 / bytes per ms] and the
//    link queuing delay buildup [ms].
//  * The state transition matrix (`F`) is the 2x2 identity matrix, meaning that
//    link bandwidth and link queuing delay buildup are modeled as independent.
//  * The measurement (`z`) is the (scalar) frame delay variation [ms].
//  * The observation matrix (`H`) is a 1x2 vector set as
//    `{frame_size_variation [bytes], 1.0}`.
//  * The state estimate covariance (`P`) is a symmetric 2x2 matrix.
//  * The process noise covariance (`Q`) is a constant 2x2 diagonal matrix
//    [(1 / bytes per ms)^2, ms^2].
//  * The observation noise covariance (`r`) is a scalar [ms^2] that is
//    determined externally to this class.
class FrameDelayVariationKalmanFilter {
 public:
  FrameDelayVariationKalmanFilter();
  ~FrameDelayVariationKalmanFilter() = default;




















  void PredictAndUpdate(double frame_delay_variation_ms,
                        double frame_size_variation_bytes,
                        double max_frame_size_bytes,
                        double var_noise);


  double GetFrameDelayVariationEstimateSizeBased(
      double frame_size_variation_bytes) const;


  double GetFrameDelayVariationEstimateTotal(
      double frame_size_variation_bytes) const;

 private:

  double estimate_[2];
  double estimate_cov_[2][2];  // Estimate covariance.


  double process_noise_cov_diag_[2];
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_TIMING_FRAME_DELAY_VARIATION_KALMAN_FILTER_H_
