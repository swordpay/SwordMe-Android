/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_CONGESTION_CONTROLLER_INCLUDE_RECEIVE_SIDE_CONGESTION_CONTROLLER_H_
#define MODULES_CONGESTION_CONTROLLER_INCLUDE_RECEIVE_SIDE_CONGESTION_CONTROLLER_H_

#include <memory>
#include <vector>

#include "api/transport/field_trial_based_config.h"
#include "api/transport/network_control.h"
#include "api/units/data_rate.h"
#include "api/units/time_delta.h"
#include "modules/congestion_controller/remb_throttler.h"
#include "modules/pacing/packet_router.h"
#include "modules/remote_bitrate_estimator/remote_estimator_proxy.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {
class RemoteBitrateEstimator;

// streams. For send side bandwidth estimation, this is simply
// relaying for each received RTP packet back to the sender. While for
// receive side bandwidth estimation, we do the estimation locally and
// send our results back to the sender.
class ReceiveSideCongestionController : public CallStatsObserver {
 public:
  ReceiveSideCongestionController(
      Clock* clock,
      RemoteEstimatorProxy::TransportFeedbackSender feedback_sender,
      RembThrottler::RembSender remb_sender,
      NetworkStateEstimator* network_state_estimator);

  ~ReceiveSideCongestionController() override {}

  virtual void OnReceivedPacket(int64_t arrival_time_ms,
                                size_t payload_size,
                                const RTPHeader& header);

  void SetSendPeriodicFeedback(bool send_periodic_feedback);

  void OnRttUpdate(int64_t avg_rtt_ms, int64_t max_rtt_ms) override;

  void OnBitrateChanged(int bitrate_bps);


  void SetMaxDesiredReceiveBitrate(DataRate bitrate);

  void SetTransportOverhead(DataSize overhead_per_packet);


  DataRate LatestReceiveSideEstimate() const;


  void RemoveStream(uint32_t ssrc);


  TimeDelta MaybeProcess();

 private:
  void PickEstimatorFromHeader(const RTPHeader& header)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  void PickEstimator() RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Clock& clock_;
  const FieldTrialBasedConfig field_trial_config_;
  RembThrottler remb_throttler_;
  RemoteEstimatorProxy remote_estimator_proxy_;

  mutable Mutex mutex_;
  std::unique_ptr<RemoteBitrateEstimator> rbe_ RTC_GUARDED_BY(mutex_);
  bool using_absolute_send_time_ RTC_GUARDED_BY(mutex_);
  uint32_t packets_since_absolute_send_time_ RTC_GUARDED_BY(mutex_);
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_INCLUDE_RECEIVE_SIDE_CONGESTION_CONTROLLER_H_
