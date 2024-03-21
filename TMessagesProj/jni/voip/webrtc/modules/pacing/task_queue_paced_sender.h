/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_TASK_QUEUE_PACED_SENDER_H_
#define MODULES_PACING_TASK_QUEUE_PACED_SENDER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/sequence_checker.h"
#include "api/task_queue/task_queue_factory.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "modules/pacing/pacing_controller.h"
#include "modules/pacing/rtp_packet_pacer.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"
#include "modules/utility/maybe_worker_thread.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/numerics/exp_filter.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {
class Clock;

class TaskQueuePacedSender : public RtpPacketPacer, public RtpPacketSender {
 public:
  static const int kNoPacketHoldback;




  TaskQueuePacedSender(Clock* clock,
                       PacingController::PacketSender* packet_sender,
                       const FieldTrialsView& field_trials,
                       TaskQueueFactory* task_queue_factory,
                       TimeDelta max_hold_back_window,
                       int max_hold_back_window_in_packets);

  ~TaskQueuePacedSender() override;

  void EnsureStarted();



  void EnqueuePackets(
      std::vector<std::unique_ptr<RtpPacketToSend>> packets) override;


  void CreateProbeClusters(
      std::vector<ProbeClusterConfig> probe_cluster_configs) override;

  void Pause() override;

  void Resume() override;

  void SetCongested(bool congested) override;

  void SetPacingRates(DataRate pacing_rate, DataRate padding_rate) override;




  void SetAccountForAudioPackets(bool account_for_audio) override;

  void SetIncludeOverhead() override;
  void SetTransportOverhead(DataSize overhead_per_packet) override;

  TimeDelta OldestPacketWaitTime() const override;

  DataSize QueueSizeData() const override;

  absl::optional<Timestamp> FirstSentPacketTime() const override;


  TimeDelta ExpectedQueueTime() const override;


  void SetQueueTimeLimit(TimeDelta limit) override;

 protected:

  struct Stats {
    Stats()
        : oldest_packet_enqueue_time(Timestamp::MinusInfinity()),
          queue_size(DataSize::Zero()),
          expected_queue_time(TimeDelta::Zero()) {}
    Timestamp oldest_packet_enqueue_time;
    DataSize queue_size;
    TimeDelta expected_queue_time;
    absl::optional<Timestamp> first_sent_packet_time;
  };
  void OnStatsUpdated(const Stats& stats);

 private:





  void MaybeProcessPackets(Timestamp scheduled_process_time);

  void UpdateStats() RTC_RUN_ON(task_queue_);
  Stats GetStats() const;

  Clock* const clock_;
  struct BurstyPacerFlags {


    explicit BurstyPacerFlags(const FieldTrialsView& field_trials);


    FieldTrialOptional<TimeDelta> burst;
  };
  const BurstyPacerFlags bursty_pacer_flags_;
  struct SlackedPacerFlags {


    explicit SlackedPacerFlags(const FieldTrialsView& field_trials);





    FieldTrialFlag allow_low_precision;



    FieldTrialOptional<TimeDelta> max_low_precision_expected_queue_time;



    FieldTrialOptional<TimeDelta> send_burst_interval;
  };
  const SlackedPacerFlags slacked_pacer_flags_;


  const TimeDelta max_hold_back_window_;
  const int max_hold_back_window_in_packets_;

  PacingController pacing_controller_ RTC_GUARDED_BY(task_queue_);





  Timestamp next_process_time_ RTC_GUARDED_BY(task_queue_);


  bool is_started_ RTC_GUARDED_BY(task_queue_);



  bool is_shutdown_ RTC_GUARDED_BY(task_queue_);

  rtc::ExpFilter packet_size_ RTC_GUARDED_BY(task_queue_);
  bool include_overhead_ RTC_GUARDED_BY(task_queue_);


  mutable Mutex stats_mutex_;
  Stats current_stats_ RTC_GUARDED_BY(stats_mutex_);

  ScopedTaskSafety safety_;
  MaybeWorkerThread task_queue_;
};
}  // namespace webrtc
#endif  // MODULES_PACING_TASK_QUEUE_PACED_SENDER_H_
