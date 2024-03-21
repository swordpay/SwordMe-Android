/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_NETEQ_DECISION_LOGIC_H_
#define MODULES_AUDIO_CODING_NETEQ_DECISION_LOGIC_H_

#include <memory>

#include "api/neteq/neteq.h"
#include "api/neteq/neteq_controller.h"
#include "api/neteq/tick_timer.h"
#include "modules/audio_coding/neteq/buffer_level_filter.h"
#include "modules/audio_coding/neteq/delay_manager.h"
#include "modules/audio_coding/neteq/packet_arrival_history.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace webrtc {

class DecisionLogic : public NetEqController {
 public:
  DecisionLogic(NetEqController::Config config);
  DecisionLogic(NetEqController::Config config,
                std::unique_ptr<DelayManager> delay_manager,
                std::unique_ptr<BufferLevelFilter> buffer_level_filter);

  ~DecisionLogic() override;

  DecisionLogic(const DecisionLogic&) = delete;
  DecisionLogic& operator=(const DecisionLogic&) = delete;

  void Reset() override {}

  void SoftReset() override;

  void SetSampleRate(int fs_hz, size_t output_size_samples) override;










  NetEq::Operation GetDecision(const NetEqController::NetEqStatus& status,
                               bool* reset_decoder) override;

  bool CngRfc3389On() const override { return cng_state_ == kCngRfc3389On; }
  bool CngOff() const override { return cng_state_ == kCngOff; }

  void SetCngOff() override { cng_state_ = kCngOff; }

  void ExpandDecision(NetEq::Operation operation) override {}

  void AddSampleMemory(int32_t value) override { sample_memory_ += value; }

  int TargetLevelMs() const override;

  int UnlimitedTargetLevelMs() const override;

  absl::optional<int> PacketArrived(int fs_hz,
                                    bool should_update_stats,
                                    const PacketArrivedInfo& info) override;

  void RegisterEmptyPacket() override {}

  void NotifyMutedState() override;

  bool SetMaximumDelay(int delay_ms) override {
    return delay_manager_->SetMaximumDelay(delay_ms);
  }
  bool SetMinimumDelay(int delay_ms) override {
    return delay_manager_->SetMinimumDelay(delay_ms);
  }
  bool SetBaseMinimumDelay(int delay_ms) override {
    return delay_manager_->SetBaseMinimumDelay(delay_ms);
  }
  int GetBaseMinimumDelay() const override {
    return delay_manager_->GetBaseMinimumDelay();
  }
  bool PeakFound() const override { return false; }

  int GetFilteredBufferLevel() const override;

  void set_sample_memory(int32_t value) override { sample_memory_ = value; }
  size_t noise_fast_forward() const override { return noise_fast_forward_; }
  size_t packet_length_samples() const override {
    return packet_length_samples_;
  }
  void set_packet_length_samples(size_t value) override {
    packet_length_samples_ = value;
  }
  void set_prev_time_scale(bool value) override { prev_time_scale_ = value; }

 private:

  static const int kMinTimescaleInterval = 5;

  enum CngState { kCngOff, kCngRfc3389On, kCngInternalOn };


  void FilterBufferLevel(size_t buffer_size_samples);


  virtual NetEq::Operation CngOperation(NetEqController::NetEqStatus status);


  virtual NetEq::Operation NoPacket(NetEqController::NetEqStatus status);

  virtual NetEq::Operation ExpectedPacketAvailable(
      NetEqController::NetEqStatus status);


  virtual NetEq::Operation FuturePacketAvailable(
      NetEqController::NetEqStatus status);


  bool TimescaleAllowed() const {
    return !timescale_countdown_ || timescale_countdown_->Finished();
  }

  bool UnderTargetLevel() const;


  bool ReinitAfterExpands(uint32_t timestamp_leap) const;



  bool PacketTooEarly(uint32_t timestamp_leap) const;

  bool MaxWaitForPacket() const;

  bool ShouldContinueExpand(NetEqController::NetEqStatus status) const;

  int GetNextPacketDelayMs(NetEqController::NetEqStatus status) const;
  int GetPlayoutDelayMs(NetEqController::NetEqStatus status) const;

  int LowThreshold() const;
  int HighThreshold() const;
  int LowThresholdCng() const;
  int HighThresholdCng() const;


  struct Config {
    Config();

    bool enable_stable_playout_delay = false;
    int reinit_after_expands = 100;
    int deceleration_target_level_offset_ms = 85;
    int packet_history_size_ms = 2000;
  };
  Config config_;
  std::unique_ptr<DelayManager> delay_manager_;
  std::unique_ptr<BufferLevelFilter> buffer_level_filter_;
  PacketArrivalHistory packet_arrival_history_;
  const TickTimer* tick_timer_;
  int sample_rate_khz_;
  size_t output_size_samples_;
  CngState cng_state_ = kCngOff;  // Remember if comfort noise is interrupted by

  size_t noise_fast_forward_ = 0;
  size_t packet_length_samples_ = 0;
  int sample_memory_ = 0;
  bool prev_time_scale_ = false;
  bool disallow_time_stretching_;
  std::unique_ptr<TickTimer::Countdown> timescale_countdown_;
  int num_consecutive_expands_ = 0;
  int time_stretched_cn_samples_ = 0;
  bool buffer_flush_ = false;
  int last_playout_delay_ms_ = 0;
};

}  // namespace webrtc
#endif  // MODULES_AUDIO_CODING_NETEQ_DECISION_LOGIC_H_
