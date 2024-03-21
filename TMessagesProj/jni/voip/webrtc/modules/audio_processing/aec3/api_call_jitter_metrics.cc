/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/audio_processing/aec3/api_call_jitter_metrics.h"

#include <algorithm>
#include <limits>

#include "modules/audio_processing/aec3/aec3_common.h"
#include "system_wrappers/include/metrics.h"

namespace webrtc {
namespace {

bool TimeToReportMetrics(int frames_since_last_report) {
  constexpr int kNumFramesPerSecond = 100;
  constexpr int kReportingIntervalFrames = 10 * kNumFramesPerSecond;
  return frames_since_last_report == kReportingIntervalFrames;
}

}  // namespace

ApiCallJitterMetrics::Jitter::Jitter()
    : max_(0), min_(std::numeric_limits<int>::max()) {}

void ApiCallJitterMetrics::Jitter::Update(int num_api_calls_in_a_row) {
  min_ = std::min(min_, num_api_calls_in_a_row);
  max_ = std::max(max_, num_api_calls_in_a_row);
}

void ApiCallJitterMetrics::Jitter::Reset() {
  min_ = std::numeric_limits<int>::max();
  max_ = 0;
}

void ApiCallJitterMetrics::Reset() {
  render_jitter_.Reset();
  capture_jitter_.Reset();
  num_api_calls_in_a_row_ = 0;
  frames_since_last_report_ = 0;
  last_call_was_render_ = false;
  proper_call_observed_ = false;
}

void ApiCallJitterMetrics::ReportRenderCall() {
  if (!last_call_was_render_) {



    if (proper_call_observed_) {
      capture_jitter_.Update(num_api_calls_in_a_row_);
    }

    num_api_calls_in_a_row_ = 0;
  }
  ++num_api_calls_in_a_row_;
  last_call_was_render_ = true;
}

void ApiCallJitterMetrics::ReportCaptureCall() {
  if (last_call_was_render_) {



    if (proper_call_observed_) {
      render_jitter_.Update(num_api_calls_in_a_row_);
    }

    num_api_calls_in_a_row_ = 0;


    proper_call_observed_ = true;
  }
  ++num_api_calls_in_a_row_;
  last_call_was_render_ = false;


  if (proper_call_observed_ &&
      TimeToReportMetrics(++frames_since_last_report_)) {

    constexpr int kMaxJitterToReport = 50;

    RTC_HISTOGRAM_COUNTS_LINEAR(
        "WebRTC.Audio.EchoCanceller.MaxRenderJitter",
        std::min(kMaxJitterToReport, render_jitter().max()), 1,
        kMaxJitterToReport, kMaxJitterToReport);
    RTC_HISTOGRAM_COUNTS_LINEAR(
        "WebRTC.Audio.EchoCanceller.MinRenderJitter",
        std::min(kMaxJitterToReport, render_jitter().min()), 1,
        kMaxJitterToReport, kMaxJitterToReport);

    RTC_HISTOGRAM_COUNTS_LINEAR(
        "WebRTC.Audio.EchoCanceller.MaxCaptureJitter",
        std::min(kMaxJitterToReport, capture_jitter().max()), 1,
        kMaxJitterToReport, kMaxJitterToReport);
    RTC_HISTOGRAM_COUNTS_LINEAR(
        "WebRTC.Audio.EchoCanceller.MinCaptureJitter",
        std::min(kMaxJitterToReport, capture_jitter().min()), 1,
        kMaxJitterToReport, kMaxJitterToReport);

    frames_since_last_report_ = 0;
    Reset();
  }
}

bool ApiCallJitterMetrics::WillReportMetricsAtNextCapture() const {
  return TimeToReportMetrics(frames_since_last_report_ + 1);
}

}  // namespace webrtc
