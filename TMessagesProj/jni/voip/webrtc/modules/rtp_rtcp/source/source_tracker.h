/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_RTP_RTCP_SOURCE_SOURCE_TRACKER_H_
#define MODULES_RTP_RTCP_SOURCE_SOURCE_TRACKER_H_

#include <cstdint>
#include <list>
#include <unordered_map>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "api/rtp_packet_infos.h"
#include "api/transport/rtp/rtp_source.h"
#include "api/units/time_delta.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {

// Tracker for `RTCRtpContributingSource` and `RTCRtpSynchronizationSource`:
//   - https://w3c.github.io/webrtc-pc/#dom-rtcrtpcontributingsource
//   - https://w3c.github.io/webrtc-pc/#dom-rtcrtpsynchronizationsource
//
class SourceTracker {
 public:


  static constexpr int64_t kTimeoutMs = 10000;  // 10 seconds

  explicit SourceTracker(Clock* clock);

  SourceTracker(const SourceTracker& other) = delete;
  SourceTracker(SourceTracker&& other) = delete;
  SourceTracker& operator=(const SourceTracker& other) = delete;
  SourceTracker& operator=(SourceTracker&& other) = delete;


  void OnFrameDelivered(const RtpPacketInfos& packet_infos);



  std::vector<RtpSource> GetSources() const;

 private:
  struct SourceKey {
    SourceKey(RtpSourceType source_type, uint32_t source)
        : source_type(source_type), source(source) {}

    RtpSourceType source_type;

    uint32_t source;
  };

  struct SourceKeyComparator {
    bool operator()(const SourceKey& lhs, const SourceKey& rhs) const {
      return (lhs.source_type == rhs.source_type) && (lhs.source == rhs.source);
    }
  };

  struct SourceKeyHasher {
    size_t operator()(const SourceKey& value) const {
      return static_cast<size_t>(value.source_type) +
             static_cast<size_t>(value.source) * 11076425802534262905ULL;
    }
  };

  struct SourceEntry {



    int64_t timestamp_ms;




    absl::optional<uint8_t> audio_level;



    absl::optional<AbsoluteCaptureTime> absolute_capture_time;





    absl::optional<TimeDelta> local_capture_clock_offset;


    uint32_t rtp_timestamp;
  };

  using SourceList = std::list<std::pair<const SourceKey, SourceEntry>>;
  using SourceMap = std::unordered_map<SourceKey,
                                       SourceList::iterator,
                                       SourceKeyHasher,
                                       SourceKeyComparator>;


  SourceEntry& UpdateEntry(const SourceKey& key)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(lock_);


  void PruneEntries(int64_t now_ms) const RTC_EXCLUSIVE_LOCKS_REQUIRED(lock_);

  Clock* const clock_;
  mutable Mutex lock_;



  mutable SourceList list_ RTC_GUARDED_BY(lock_);
  mutable SourceMap map_ RTC_GUARDED_BY(lock_);
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_SOURCE_TRACKER_H_
