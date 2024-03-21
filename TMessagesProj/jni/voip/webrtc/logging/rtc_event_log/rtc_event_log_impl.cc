/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "logging/rtc_event_log/rtc_event_log_impl.h"

#include <functional>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/task_queue/task_queue_base.h"
#include "api/units/time_delta.h"
#include "logging/rtc_event_log/encoder/rtc_event_log_encoder_legacy.h"
#include "logging/rtc_event_log/encoder/rtc_event_log_encoder_new_format.h"
#include "rtc_base/checks.h"
#include "rtc_base/event.h"
#include "rtc_base/logging.h"
#include "rtc_base/numerics/safe_conversions.h"
#include "rtc_base/numerics/safe_minmax.h"
#include "rtc_base/time_utils.h"

namespace webrtc {
namespace {
constexpr size_t kMaxEventsInHistory = 10000;
// The config-history is supposed to be unbounded, but needs to have some bound
// to prevent an attack via unreasonable memory use.
constexpr size_t kMaxEventsInConfigHistory = 1000;

std::unique_ptr<RtcEventLogEncoder> CreateEncoder(
    RtcEventLog::EncodingType type) {
  switch (type) {
    case RtcEventLog::EncodingType::Legacy:
      RTC_DLOG(LS_INFO) << "Creating legacy encoder for RTC event log.";
      return std::make_unique<RtcEventLogEncoderLegacy>();
    case RtcEventLog::EncodingType::NewFormat:
      RTC_DLOG(LS_INFO) << "Creating new format encoder for RTC event log.";
      return std::make_unique<RtcEventLogEncoderNewFormat>();
    default:
      RTC_LOG(LS_ERROR) << "Unknown RtcEventLog encoder type (" << int(type)
                        << ")";
      RTC_DCHECK_NOTREACHED();
      return std::unique_ptr<RtcEventLogEncoder>(nullptr);
  }
}
}  // namespace

RtcEventLogImpl::RtcEventLogImpl(RtcEventLog::EncodingType encoding_type,
                                 TaskQueueFactory* task_queue_factory)
    : event_encoder_(CreateEncoder(encoding_type)),
      num_config_events_written_(0),
      last_output_ms_(rtc::TimeMillis()),
      output_scheduled_(false),
      logging_state_started_(false),
      task_queue_(
          std::make_unique<rtc::TaskQueue>(task_queue_factory->CreateTaskQueue(
              "rtc_event_log",
              TaskQueueFactory::Priority::NORMAL))) {}

RtcEventLogImpl::~RtcEventLogImpl() {

  if (logging_state_started_) {
    logging_state_checker_.Detach();
    StopLogging();
  }


  rtc::TaskQueue* tq = task_queue_.get();
  delete tq;
  task_queue_.release();
}

bool RtcEventLogImpl::StartLogging(std::unique_ptr<RtcEventLogOutput> output,
                                   int64_t output_period_ms) {
  RTC_DCHECK(output_period_ms == kImmediateOutput || output_period_ms > 0);

  if (!output->IsActive()) {


    return false;
  }

  const int64_t timestamp_us = rtc::TimeMillis() * 1000;
  const int64_t utc_time_us = rtc::TimeUTCMillis() * 1000;
  RTC_LOG(LS_INFO) << "Starting WebRTC event log. (Timestamp, UTC) = ("
                   << timestamp_us << ", " << utc_time_us << ").";

  RTC_DCHECK_RUN_ON(&logging_state_checker_);
  logging_state_started_ = true;

  task_queue_->PostTask([this, output_period_ms, timestamp_us, utc_time_us,
                         output = std::move(output)]() mutable {
    RTC_DCHECK_RUN_ON(task_queue_.get());
    RTC_DCHECK(output->IsActive());
    output_period_ms_ = output_period_ms;
    event_output_ = std::move(output);
    num_config_events_written_ = 0;
    WriteToOutput(event_encoder_->EncodeLogStart(timestamp_us, utc_time_us));
    LogEventsFromMemoryToOutput();
  });

  return true;
}

void RtcEventLogImpl::StopLogging() {
  RTC_DLOG(LS_INFO) << "Stopping WebRTC event log.";



  rtc::Event output_stopped;
  StopLogging([&output_stopped]() { output_stopped.Set(); });
  output_stopped.Wait(rtc::Event::kForever);

  RTC_DLOG(LS_INFO) << "WebRTC event log successfully stopped.";
}

void RtcEventLogImpl::StopLogging(std::function<void()> callback) {
  RTC_DCHECK_RUN_ON(&logging_state_checker_);
  logging_state_started_ = false;
  task_queue_->PostTask([this, callback] {
    RTC_DCHECK_RUN_ON(task_queue_.get());
    if (event_output_) {
      RTC_DCHECK(event_output_->IsActive());
      LogEventsFromMemoryToOutput();
    }
    StopLoggingInternal();
    callback();
  });
}

void RtcEventLogImpl::Log(std::unique_ptr<RtcEvent> event) {
  RTC_CHECK(event);

  task_queue_->PostTask([this, event = std::move(event)]() mutable {
    RTC_DCHECK_RUN_ON(task_queue_.get());
    LogToMemory(std::move(event));
    if (event_output_)
      ScheduleOutput();
  });
}

void RtcEventLogImpl::ScheduleOutput() {
  RTC_DCHECK(event_output_ && event_output_->IsActive());
  if (history_.size() >= kMaxEventsInHistory) {


    LogEventsFromMemoryToOutput();
    return;
  }

  RTC_DCHECK(output_period_ms_.has_value());
  if (*output_period_ms_ == kImmediateOutput) {


    LogEventsFromMemoryToOutput();
    return;
  }

  if (!output_scheduled_) {
    output_scheduled_ = true;

    auto output_task = [this]() {
      RTC_DCHECK_RUN_ON(task_queue_.get());
      if (event_output_) {
        RTC_DCHECK(event_output_->IsActive());
        LogEventsFromMemoryToOutput();
      }
      output_scheduled_ = false;
    };
    const int64_t now_ms = rtc::TimeMillis();
    const int64_t time_since_output_ms = now_ms - last_output_ms_;
    const uint32_t delay = rtc::SafeClamp(
        *output_period_ms_ - time_since_output_ms, 0, *output_period_ms_);
    task_queue_->PostDelayedTask(std::move(output_task),
                                 TimeDelta::Millis(delay));
  }
}

void RtcEventLogImpl::LogToMemory(std::unique_ptr<RtcEvent> event) {
  std::deque<std::unique_ptr<RtcEvent>>& container =
      event->IsConfigEvent() ? config_history_ : history_;
  const size_t container_max_size =
      event->IsConfigEvent() ? kMaxEventsInConfigHistory : kMaxEventsInHistory;

  if (container.size() >= container_max_size) {
    RTC_DCHECK(!event_output_);  // Shouldn't lose events if we have an output.
    container.pop_front();
  }
  container.push_back(std::move(event));
}

void RtcEventLogImpl::LogEventsFromMemoryToOutput() {
  RTC_DCHECK(event_output_ && event_output_->IsActive());
  last_output_ms_ = rtc::TimeMillis();




  std::string encoded_configs;
  RTC_DCHECK_LE(num_config_events_written_, config_history_.size());
  if (num_config_events_written_ < config_history_.size()) {
    const auto begin = config_history_.begin() + num_config_events_written_;
    const auto end = config_history_.end();
    encoded_configs = event_encoder_->EncodeBatch(begin, end);
    num_config_events_written_ = config_history_.size();
  }







  std::string encoded_history =
      event_encoder_->EncodeBatch(history_.begin(), history_.end());
  history_.clear();

  WriteConfigsAndHistoryToOutput(encoded_configs, encoded_history);
}

void RtcEventLogImpl::WriteConfigsAndHistoryToOutput(
    absl::string_view encoded_configs,
    absl::string_view encoded_history) {



  if (encoded_configs.empty()) {
    WriteToOutput(encoded_history);  // Typical case.
  } else if (encoded_history.empty()) {
    WriteToOutput(encoded_configs);  // Very unusual case.
  } else {
    std::string s;
    s.reserve(encoded_configs.size() + encoded_history.size());
    s.append(encoded_configs.data(), encoded_configs.size());
    s.append(encoded_history.data(), encoded_history.size());
    WriteToOutput(s);
  }
}

void RtcEventLogImpl::StopOutput() {
  event_output_.reset();
}

void RtcEventLogImpl::StopLoggingInternal() {
  if (event_output_) {
    RTC_DCHECK(event_output_->IsActive());
    const int64_t timestamp_us = rtc::TimeMillis() * 1000;
    event_output_->Write(event_encoder_->EncodeLogEnd(timestamp_us));
  }
  StopOutput();
}

void RtcEventLogImpl::WriteToOutput(absl::string_view output_string) {
  RTC_DCHECK(event_output_ && event_output_->IsActive());
  if (!event_output_->Write(output_string)) {
    RTC_LOG(LS_ERROR) << "Failed to write RTC event to output.";

    RTC_DCHECK(!event_output_->IsActive());
    StopOutput();  // Clean-up.
    return;
  }
}

}  // namespace webrtc
