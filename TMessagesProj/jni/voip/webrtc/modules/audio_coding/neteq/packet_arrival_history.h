/*
 *  Copyright (c) 2022 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_NETEQ_PACKET_ARRIVAL_HISTORY_H_
#define MODULES_AUDIO_CODING_NETEQ_PACKET_ARRIVAL_HISTORY_H_

#include <cstdint>
#include <deque>

#include "absl/types/optional.h"
#include "api/neteq/tick_timer.h"
#include "modules/include/module_common_types_public.h"

namespace webrtc {

// The history has a fixed window size beyond which old data is automatically
// pruned.
class PacketArrivalHistory {
 public:
  explicit PacketArrivalHistory(int window_size_ms);

  void Insert(uint32_t rtp_timestamp, int64_t arrival_time_ms);




  int GetDelayMs(uint32_t rtp_timestamp, int64_t time_ms) const;

  int GetMaxDelayMs() const;

  bool IsNewestRtpTimestamp(uint32_t rtp_timestamp) const;

  void Reset();

  void set_sample_rate(int sample_rate) {
    sample_rate_khz_ = sample_rate / 1000;
  }

  size_t size() const { return history_.size(); }

 private:
  struct PacketArrival {
    PacketArrival(int64_t rtp_timestamp_ms, int64_t arrival_time_ms)
        : rtp_timestamp_ms(rtp_timestamp_ms),
          arrival_time_ms(arrival_time_ms) {}
    int64_t rtp_timestamp_ms;
    int64_t arrival_time_ms;
    bool operator<=(const PacketArrival& other) const {
      return arrival_time_ms - rtp_timestamp_ms <=
             other.arrival_time_ms - other.rtp_timestamp_ms;
    }
    bool operator>=(const PacketArrival& other) const {
      return arrival_time_ms - rtp_timestamp_ms >=
             other.arrival_time_ms - other.rtp_timestamp_ms;
    }
  };
  std::deque<PacketArrival> history_;
  int GetPacketArrivalDelayMs(const PacketArrival& packet_arrival) const;

  void MaybeUpdateCachedArrivals(const PacketArrival& packet);
  const PacketArrival* min_packet_arrival_ = nullptr;
  const PacketArrival* max_packet_arrival_ = nullptr;
  const int window_size_ms_;
  TimestampUnwrapper timestamp_unwrapper_;
  absl::optional<int64_t> newest_rtp_timestamp_;
  int sample_rate_khz_ = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_CODING_NETEQ_PACKET_ARRIVAL_HISTORY_H_
