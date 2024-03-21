/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_NETEQ_NACK_TRACKER_H_
#define MODULES_AUDIO_CODING_NETEQ_NACK_TRACKER_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "absl/types/optional.h"
#include "modules/include/module_common_types_public.h"
#include "rtc_base/gtest_prod_util.h"

// The NackTracker class keeps track of the lost packets, an estimate of
// time-to-play for each packet is also given.
//
// Every time a packet is pushed into NetEq, LastReceivedPacket() has to be
// called to update the NACK list.
//
// Every time 10ms audio is pulled from NetEq LastDecodedPacket() should be
// called, and time-to-play is updated at that moment.
//
// If packet N is received, any packet prior to N which has not arrived is
// considered lost, and should be labeled as "missing" (the size of
// the list might be limited and older packet eliminated from the list).
//
// The NackTracker class has to know about the sample rate of the packets to
// compute time-to-play. So sample rate should be set as soon as the first
// packet is received. If there is a change in the receive codec (sender changes
// codec) then NackTracker should be reset. This is because NetEQ would flush
// its buffer and re-transmission is meaning less for old packet. Therefore, in
// that case, after reset the sampling rate has to be updated.
//
// Thread Safety
// =============
// Please note that this class in not thread safe. The class must be protected
// if different APIs are called from different threads.
//
namespace webrtc {

class NackTracker {
 public:

  static const size_t kNackListSizeLimit = 500;  // 10 seconds for 20 ms frame

  NackTracker();
  ~NackTracker();





  void SetMaxNackListSize(size_t max_nack_list_size);






  void UpdateSampleRate(int sample_rate_hz);


  void UpdateLastDecodedPacket(uint16_t sequence_number, uint32_t timestamp);


  void UpdateLastReceivedPacket(uint16_t sequence_number, uint32_t timestamp);





  std::vector<uint16_t> GetNackList(int64_t round_trip_time_ms);


  void Reset();

  uint32_t GetPacketLossRateForTest() { return packet_loss_rate_; }

 private:

  FRIEND_TEST_ALL_PREFIXES(NackTrackerTest, EstimateTimestampAndTimeToPlay);

  struct Config {
    Config();

    double packet_loss_forget_factor = 0.996;


    int ms_per_loss_percent = 20;

    bool never_nack_multiple_times = false;

    bool require_valid_rtt = false;

    int default_rtt_ms = 100;

    double max_loss_rate = 1.0;
  };

  struct NackElement {
    NackElement(int64_t initial_time_to_play_ms, uint32_t initial_timestamp)
        : time_to_play_ms(initial_time_to_play_ms),
          estimated_timestamp(initial_timestamp) {}


    int64_t time_to_play_ms;






    uint32_t estimated_timestamp;
  };

  class NackListCompare {
   public:
    bool operator()(uint16_t sequence_number_old,
                    uint16_t sequence_number_new) const {
      return IsNewerSequenceNumber(sequence_number_new, sequence_number_old);
    }
  };

  typedef std::map<uint16_t, NackElement, NackListCompare> NackList;


  NackList GetNackList() const;


  void UpdateEstimatedPlayoutTimeBy10ms();


  absl::optional<int> GetSamplesPerPacket(
      uint16_t sequence_number_current_received_rtp,
      uint32_t timestamp_current_received_rtp) const;



  void UpdateList(uint16_t sequence_number_current_received_rtp,
                  uint32_t timestamp_current_received_rtp);



  void LimitNackListSize();

  uint32_t EstimateTimestamp(uint16_t sequence_number, int samples_per_packet);

  int64_t TimeToPlay(uint32_t timestamp) const;

  void UpdatePacketLossRate(int packets_lost);

  const Config config_;

  uint16_t sequence_num_last_received_rtp_;
  uint32_t timestamp_last_received_rtp_;
  bool any_rtp_received_;  // If any packet received.

  uint16_t sequence_num_last_decoded_rtp_;
  uint32_t timestamp_last_decoded_rtp_;
  bool any_rtp_decoded_;  // If any packet decoded.

  int sample_rate_khz_;  // Sample rate in kHz.



  NackList nack_list_;


  size_t max_nack_list_size_;

  uint32_t packet_loss_rate_ = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_CODING_NETEQ_NACK_TRACKER_H_
