/*
 *  Copyright (c) 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_TIMESTAMP_ALIGNER_H_
#define RTC_BASE_TIMESTAMP_ALIGNER_H_

#include <stdint.h>

#include "rtc_base/system/rtc_export.h"

namespace rtc {

// into the same timescale as is used by rtc::TimeMicros(). Some capture systems
// provide timestamps, which comes from the capturing hardware (camera or sound
// card) or stamped close to the capturing hardware. Such timestamps are more
// accurate (less jittery) than reading the system clock, but may have a
// different epoch and unknown clock drift. Frame timestamps in webrtc should
// use rtc::TimeMicros (system monotonic time), and this class provides a filter
// which lets us use the rtc::TimeMicros timescale, and at the same time take
// advantage of higher accuracy of the capturer's clock.

// externally.
class RTC_EXPORT TimestampAligner {
 public:
  TimestampAligner();
  ~TimestampAligner();

  TimestampAligner(const TimestampAligner&) = delete;
  TimestampAligner& operator=(const TimestampAligner&) = delete;

 public:







  int64_t TranslateTimestamp(int64_t capturer_time_us, int64_t system_time_us);



  int64_t TranslateTimestamp(int64_t capturer_time_us) const;

 protected:


  int64_t UpdateOffset(int64_t capturer_time_us, int64_t system_time_us);




  int64_t ClipTimestamp(int64_t filtered_time_us, int64_t system_time_us);

 private:

  int frames_seen_;

  int64_t offset_us_;




  int64_t clip_bias_us_;

  int64_t prev_translated_time_us_;


  int64_t prev_time_offset_us_;
};

}  // namespace rtc

#endif  // RTC_BASE_TIMESTAMP_ALIGNER_H_
