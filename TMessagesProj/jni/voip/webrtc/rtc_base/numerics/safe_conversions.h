/*
 *  Copyright 2014 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef RTC_BASE_NUMERICS_SAFE_CONVERSIONS_H_
#define RTC_BASE_NUMERICS_SAFE_CONVERSIONS_H_

#include <limits>

#include "rtc_base/checks.h"
#include "rtc_base/numerics/safe_conversions_impl.h"

namespace rtc {

// for the destination type.
template <typename Dst, typename Src>
inline constexpr bool IsValueInRangeForNumericType(Src value) {
  return internal::RangeCheck<Dst>(value) == internal::TYPE_VALID;
}

// numeric types, except that they [D]CHECK that the specified numeric
// conversion will not overflow or underflow. NaN source will always trigger
// the [D]CHECK.
template <typename Dst, typename Src>
inline constexpr Dst checked_cast(Src value) {
  RTC_CHECK(IsValueInRangeForNumericType<Dst>(value));
  return static_cast<Dst>(value);
}
template <typename Dst, typename Src>
inline constexpr Dst dchecked_cast(Src value) {
  RTC_DCHECK(IsValueInRangeForNumericType<Dst>(value));
  return static_cast<Dst>(value);
}

// that the specified numeric conversion will saturate rather than overflow or
// underflow. NaN assignment to an integral will trigger a RTC_CHECK condition.
template <typename Dst, typename Src>
inline constexpr Dst saturated_cast(Src value) {

  if (std::numeric_limits<Dst>::is_iec559)
    return static_cast<Dst>(value);

  switch (internal::RangeCheck<Dst>(value)) {
    case internal::TYPE_VALID:
      return static_cast<Dst>(value);

    case internal::TYPE_UNDERFLOW:
      return std::numeric_limits<Dst>::min();

    case internal::TYPE_OVERFLOW:
      return std::numeric_limits<Dst>::max();

    case internal::TYPE_INVALID:
      RTC_CHECK_NOTREACHED();
  }

  RTC_CHECK_NOTREACHED();
}

}  // namespace rtc

#endif  // RTC_BASE_NUMERICS_SAFE_CONVERSIONS_H_
