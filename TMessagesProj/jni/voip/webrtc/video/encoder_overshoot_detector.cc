/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video/encoder_overshoot_detector.h"

#include <algorithm>

namespace webrtc {
namespace {
// The buffer level for media-rate utilization is allowed to go below zero,
// down to
// -(`kMaxMediaUnderrunFrames` / `target_framerate_fps_`) * `target_bitrate_`.
static constexpr double kMaxMediaUnderrunFrames = 5.0;
}  // namespace

EncoderOvershootDetector::EncoderOvershootDetector(int64_t window_size_ms)
    : window_size_ms_(window_size_ms),
      time_last_update_ms_(-1),
      sum_network_utilization_factors_(0.0),
      sum_media_utilization_factors_(0.0),
      target_bitrate_(DataRate::Zero()),
      target_framerate_fps_(0),
      network_buffer_level_bits_(0),
      media_buffer_level_bits_(0) {}

EncoderOvershootDetector::~EncoderOvershootDetector() = default;

void EncoderOvershootDetector::SetTargetRate(DataRate target_bitrate,
                                             double target_framerate_fps,
                                             int64_t time_ms) {

  if (target_bitrate_ != DataRate::Zero()) {
    LeakBits(time_ms);
  } else if (target_bitrate != DataRate::Zero()) {

    time_last_update_ms_ = time_ms;
    utilization_factors_.clear();
    sum_network_utilization_factors_ = 0.0;
    sum_media_utilization_factors_ = 0.0;
    network_buffer_level_bits_ = 0;
    media_buffer_level_bits_ = 0;
  }

  target_bitrate_ = target_bitrate;
  target_framerate_fps_ = target_framerate_fps;
}

void EncoderOvershootDetector::OnEncodedFrame(size_t bytes, int64_t time_ms) {


  LeakBits(time_ms);

  const int64_t ideal_frame_size_bits = IdealFrameSizeBits();
  if (ideal_frame_size_bits == 0) {

    return;
  }

  const double network_utilization_factor = HandleEncodedFrame(
      bytes * 8, ideal_frame_size_bits, time_ms, &network_buffer_level_bits_);
  const double media_utilization_factor = HandleEncodedFrame(
      bytes * 8, ideal_frame_size_bits, time_ms, &media_buffer_level_bits_);

  sum_network_utilization_factors_ += network_utilization_factor;
  sum_media_utilization_factors_ += media_utilization_factor;

  utilization_factors_.emplace_back(network_utilization_factor,
                                    media_utilization_factor, time_ms);
}

double EncoderOvershootDetector::HandleEncodedFrame(
    size_t frame_size_bits,
    int64_t ideal_frame_size_bits,
    int64_t time_ms,
    int64_t* buffer_level_bits) const {






  const int64_t bitsum = frame_size_bits + *buffer_level_bits;
  int64_t overshoot_bits = 0;
  if (bitsum > ideal_frame_size_bits) {
    overshoot_bits =
        std::min(*buffer_level_bits, bitsum - ideal_frame_size_bits);
  }


  double utilization_factor;
  if (utilization_factors_.empty()) {


    utilization_factor = std::max(
        1.0, static_cast<double>(frame_size_bits) / ideal_frame_size_bits);
  } else {
    utilization_factor =
        1.0 + (static_cast<double>(overshoot_bits) / ideal_frame_size_bits);
  }


  *buffer_level_bits -= overshoot_bits;
  *buffer_level_bits += frame_size_bits;

  return utilization_factor;
}

absl::optional<double>
EncoderOvershootDetector::GetNetworkRateUtilizationFactor(int64_t time_ms) {
  CullOldUpdates(time_ms);

  if (utilization_factors_.empty()) {
    return absl::nullopt;
  }


  return sum_network_utilization_factors_ / utilization_factors_.size();
}

absl::optional<double> EncoderOvershootDetector::GetMediaRateUtilizationFactor(
    int64_t time_ms) {
  CullOldUpdates(time_ms);

  if (utilization_factors_.empty()) {
    return absl::nullopt;
  }

  return sum_media_utilization_factors_ / utilization_factors_.size();
}

void EncoderOvershootDetector::Reset() {
  time_last_update_ms_ = -1;
  utilization_factors_.clear();
  target_bitrate_ = DataRate::Zero();
  sum_network_utilization_factors_ = 0.0;
  sum_media_utilization_factors_ = 0.0;
  target_framerate_fps_ = 0.0;
  network_buffer_level_bits_ = 0;
  media_buffer_level_bits_ = 0;
}

int64_t EncoderOvershootDetector::IdealFrameSizeBits() const {
  if (target_framerate_fps_ <= 0 || target_bitrate_ == DataRate::Zero()) {
    return 0;
  }

  return static_cast<int64_t>(
      (target_bitrate_.bps() + target_framerate_fps_ / 2) /
      target_framerate_fps_);
}

void EncoderOvershootDetector::LeakBits(int64_t time_ms) {
  if (time_last_update_ms_ != -1 && target_bitrate_ > DataRate::Zero()) {
    int64_t time_delta_ms = time_ms - time_last_update_ms_;

    const int64_t leaked_bits = (target_bitrate_.bps() * time_delta_ms) / 1000;

    network_buffer_level_bits_ =
        std::max<int64_t>(0, network_buffer_level_bits_ - leaked_bits);


    const double max_underrun_seconds =
        std::min(kMaxMediaUnderrunFrames, target_framerate_fps_) /
        target_framerate_fps_;
    media_buffer_level_bits_ = std::max<int64_t>(
        -max_underrun_seconds * target_bitrate_.bps<int64_t>(),
        media_buffer_level_bits_ - leaked_bits);
  }
  time_last_update_ms_ = time_ms;
}

void EncoderOvershootDetector::CullOldUpdates(int64_t time_ms) {

  const int64_t cutoff_time_ms = time_ms - window_size_ms_;
  while (!utilization_factors_.empty() &&
         utilization_factors_.front().update_time_ms < cutoff_time_ms) {

    sum_network_utilization_factors_ = std::max(
        0.0, sum_network_utilization_factors_ -
                 utilization_factors_.front().network_utilization_factor);
    sum_media_utilization_factors_ = std::max(
        0.0, sum_media_utilization_factors_ -
                 utilization_factors_.front().media_utilization_factor);
    utilization_factors_.pop_front();
  }
}

}  // namespace webrtc
