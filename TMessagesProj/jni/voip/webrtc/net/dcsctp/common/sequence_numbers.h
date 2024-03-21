/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef NET_DCSCTP_COMMON_SEQUENCE_NUMBERS_H_
#define NET_DCSCTP_COMMON_SEQUENCE_NUMBERS_H_

#include <cstdint>
#include <limits>
#include <utility>

#include "net/dcsctp/common/internal_types.h"

namespace dcsctp {

// an int64_t value space, to allow wrapped sequence numbers to be easily
// compared for ordering.
//
// Sequence numbers are expected to be monotonically increasing, but they do not
// need to be unwrapped in order, as long as the difference to the previous one
// is not larger than half the range of the wrapped sequence number.
//
// The WrappedType must be a webrtc::StrongAlias type.
template <typename WrappedType>
class UnwrappedSequenceNumber {
 public:
  static_assert(
      !std::numeric_limits<typename WrappedType::UnderlyingType>::is_signed,
      "The wrapped type must be unsigned");
  static_assert(
      std::numeric_limits<typename WrappedType::UnderlyingType>::max() <
          std::numeric_limits<int64_t>::max(),
      "The wrapped type must be less than the int64_t value space");


  class Unwrapper {
   public:
    Unwrapper() : largest_(kValueLimit) {}
    Unwrapper(const Unwrapper&) = default;
    Unwrapper& operator=(const Unwrapper&) = default;






    UnwrappedSequenceNumber<WrappedType> Unwrap(WrappedType value) {
      WrappedType wrapped_largest =
          static_cast<WrappedType>(largest_ % kValueLimit);
      int64_t result = largest_ + Delta(value, wrapped_largest);
      if (largest_ < result) {
        largest_ = result;
      }
      return UnwrappedSequenceNumber<WrappedType>(result);
    }

    UnwrappedSequenceNumber<WrappedType> PeekUnwrap(WrappedType value) const {
      WrappedType uint32_largest =
          static_cast<WrappedType>(largest_ % kValueLimit);
      int64_t result = largest_ + Delta(value, uint32_largest);
      return UnwrappedSequenceNumber<WrappedType>(result);
    }


    void Reset() { largest_ = kValueLimit; }

   private:
    static int64_t Delta(WrappedType value, WrappedType prev_value) {
      static constexpr typename WrappedType::UnderlyingType kBreakpoint =
          kValueLimit / 2;
      typename WrappedType::UnderlyingType diff = *value - *prev_value;
      diff %= kValueLimit;
      if (diff < kBreakpoint) {
        return static_cast<int64_t>(diff);
      }
      return static_cast<int64_t>(diff) - kValueLimit;
    }

    int64_t largest_;
  };

  WrappedType Wrap() const {
    return static_cast<WrappedType>(value_ % kValueLimit);
  }

  template <typename H>
  friend H AbslHashValue(H state,
                         const UnwrappedSequenceNumber<WrappedType>& hash) {
    return H::combine(std::move(state), hash.value_);
  }

  bool operator==(const UnwrappedSequenceNumber<WrappedType>& other) const {
    return value_ == other.value_;
  }
  bool operator!=(const UnwrappedSequenceNumber<WrappedType>& other) const {
    return value_ != other.value_;
  }
  bool operator<(const UnwrappedSequenceNumber<WrappedType>& other) const {
    return value_ < other.value_;
  }
  bool operator>(const UnwrappedSequenceNumber<WrappedType>& other) const {
    return value_ > other.value_;
  }
  bool operator>=(const UnwrappedSequenceNumber<WrappedType>& other) const {
    return value_ >= other.value_;
  }
  bool operator<=(const UnwrappedSequenceNumber<WrappedType>& other) const {
    return value_ <= other.value_;
  }

  void Increment() { ++value_; }

  UnwrappedSequenceNumber<WrappedType> next_value() const {
    return UnwrappedSequenceNumber<WrappedType>(value_ + 1);
  }


  static UnwrappedSequenceNumber<WrappedType> AddTo(
      UnwrappedSequenceNumber<WrappedType> value,
      int delta) {
    return UnwrappedSequenceNumber<WrappedType>(value.value_ + delta);
  }

  static typename WrappedType::UnderlyingType Difference(
      UnwrappedSequenceNumber<WrappedType> lhs,
      UnwrappedSequenceNumber<WrappedType> rhs) {
    return (lhs.value_ > rhs.value_) ? (lhs.value_ - rhs.value_)
                                     : (rhs.value_ - lhs.value_);
  }

 private:
  explicit UnwrappedSequenceNumber(int64_t value) : value_(value) {}
  static constexpr int64_t kValueLimit =
      static_cast<int64_t>(1)
      << std::numeric_limits<typename WrappedType::UnderlyingType>::digits;

  int64_t value_;
};

using UnwrappedTSN = UnwrappedSequenceNumber<TSN>;

using UnwrappedSSN = UnwrappedSequenceNumber<SSN>;

using UnwrappedMID = UnwrappedSequenceNumber<MID>;

}  // namespace dcsctp

#endif  // NET_DCSCTP_COMMON_SEQUENCE_NUMBERS_H_
