/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_REPORT_BLOCK_STATS_H_
#define VIDEO_REPORT_BLOCK_STATS_H_

#include <stdint.h>

#include <map>

namespace webrtc {

// cumulative counters, from which we compute deltas, and then accumulate the
// deltas. May be needed on the send side, to handle wraparound in the short
// counters received over RTCP, but should not be needed on the receive side
// where we can use large enough types for all counters we need.

class ReportBlockStats {
 public:
  ReportBlockStats();
  ~ReportBlockStats();

  void Store(uint32_t ssrc,
             int packets_lost,
             uint32_t extended_highest_sequence_number);


  int FractionLostInPercent() const;

 private:

  struct Report {
    uint32_t extended_highest_sequence_number;
    int32_t packets_lost;
  };

  uint32_t num_sequence_numbers_;
  uint32_t num_lost_sequence_numbers_;

  std::map<uint32_t, Report> prev_reports_;
};

}  // namespace webrtc

#endif  // VIDEO_REPORT_BLOCK_STATS_H_
