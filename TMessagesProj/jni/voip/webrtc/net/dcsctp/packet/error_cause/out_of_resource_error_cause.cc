/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "net/dcsctp/packet/error_cause/out_of_resource_error_cause.h"

#include <stdint.h>

#include <vector>

#include "absl/types/optional.h"
#include "api/array_view.h"

namespace dcsctp {


//  |     Cause Code=4              |      Cause Length=4           |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
constexpr int OutOfResourceErrorCause::kType;

absl::optional<OutOfResourceErrorCause> OutOfResourceErrorCause::Parse(
    rtc::ArrayView<const uint8_t> data) {
  if (!ParseTLV(data).has_value()) {
    return absl::nullopt;
  }
  return OutOfResourceErrorCause();
}

void OutOfResourceErrorCause::SerializeTo(std::vector<uint8_t>& out) const {
  AllocateTLV(out);
}

std::string OutOfResourceErrorCause::ToString() const {
  return "Out Of Resource";
}

}  // namespace dcsctp
