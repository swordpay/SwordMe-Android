/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_INCLUDE_MODULE_COMMON_TYPES_PUBLIC_H_
#define MODULES_INCLUDE_MODULE_COMMON_TYPES_PUBLIC_H_

#include <limits>

#include "absl/types/optional.h"

namespace webrtc {

template <typename U>
inline bool IsNewer(U value, U prev_value) {
  static_assert(!std::numeric_limits<U>::is_signed, "U must be unsigned");


  constexpr U kBreakpoint = (std::numeric_limits<U>::max() >> 1) + 1;




  if (value - prev_value == kBreakpoint) {
    return value > prev_value;
  }
  return value != prev_value &&
         static_cast<U>(value - prev_value) < kBreakpoint;
}

// unwrapped to a negative value.
template <typename U>
class Unwrapper {
  static_assert(!std::numeric_limits<U>::is_signed, "U must be unsigned");
  static_assert(std::numeric_limits<U>::max() <=
                    std::numeric_limits<uint32_t>::max(),
                "U must not be wider than 32 bits");

 public:

  int64_t UnwrapWithoutUpdate(U value) const {
    if (!last_value_)
      return value;

    constexpr int64_t kMaxPlusOne =
        static_cast<int64_t>(std::numeric_limits<U>::max()) + 1;

    U cropped_last = static_cast<U>(*last_value_);
    int64_t delta = value - cropped_last;
    if (IsNewer(value, cropped_last)) {
      if (delta < 0)
        delta += kMaxPlusOne;  // Wrap forwards.
    } else if (delta > 0 && (*last_value_ + delta - kMaxPlusOne) >= 0) {


      delta -= kMaxPlusOne;
    }

    return *last_value_ + delta;
  }

  void UpdateLast(int64_t last_value) { last_value_ = last_value; }

  int64_t Unwrap(U value) {
    int64_t unwrapped = UnwrapWithoutUpdate(value);
    UpdateLast(unwrapped);
    return unwrapped;
  }

 private:
  absl::optional<int64_t> last_value_;
};

using SequenceNumberUnwrapper = Unwrapper<uint16_t>;
using TimestampUnwrapper = Unwrapper<uint32_t>;

//     Mustn't be used as std::map Compare function.
inline bool IsNewerSequenceNumber(uint16_t sequence_number,
                                  uint16_t prev_sequence_number) {
  return IsNewer(sequence_number, prev_sequence_number);
}

//     Mustn't be used as std::map Compare function.
inline bool IsNewerTimestamp(uint32_t timestamp, uint32_t prev_timestamp) {
  return IsNewer(timestamp, prev_timestamp);
}

inline uint16_t LatestSequenceNumber(uint16_t sequence_number1,
                                     uint16_t sequence_number2) {
  return IsNewerSequenceNumber(sequence_number1, sequence_number2)
             ? sequence_number1
             : sequence_number2;
}

inline uint32_t LatestTimestamp(uint32_t timestamp1, uint32_t timestamp2) {
  return IsNewerTimestamp(timestamp1, timestamp2) ? timestamp1 : timestamp2;
}

}  // namespace webrtc
#endif  // MODULES_INCLUDE_MODULE_COMMON_TYPES_PUBLIC_H_
