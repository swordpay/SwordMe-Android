/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "common_video/h264/h264_common.h"

#include <cstdint>

namespace webrtc {
namespace H264 {

const uint8_t kNaluTypeMask = 0x1F;

std::vector<NaluIndex> FindNaluIndices(const uint8_t* buffer,
                                       size_t buffer_size) {




  std::vector<NaluIndex> sequences;
  if (buffer_size < kNaluShortStartSequenceSize)
    return sequences;

  static_assert(kNaluShortStartSequenceSize >= 2,
                "kNaluShortStartSequenceSize must be larger or equals to 2");
  const size_t end = buffer_size - kNaluShortStartSequenceSize;
  for (size_t i = 0; i < end;) {
    if (buffer[i + 2] > 1) {
      i += 3;
    } else if (buffer[i + 2] == 1) {
      if (buffer[i + 1] == 0 && buffer[i] == 0) {

        NaluIndex index = {i, i + 3, 0};
        if (index.start_offset > 0 && buffer[index.start_offset - 1] == 0)
          --index.start_offset;

        auto it = sequences.rbegin();
        if (it != sequences.rend())
          it->payload_size = index.start_offset - it->payload_start_offset;

        sequences.push_back(index);
      }

      i += 3;
    } else {
      ++i;
    }
  }

  auto it = sequences.rbegin();
  if (it != sequences.rend())
    it->payload_size = buffer_size - it->payload_start_offset;

  return sequences;
}

NaluType ParseNaluType(uint8_t data) {
  return static_cast<NaluType>(data & kNaluTypeMask);
}

std::vector<uint8_t> ParseRbsp(const uint8_t* data, size_t length) {
  std::vector<uint8_t> out;
  out.reserve(length);

  for (size_t i = 0; i < length;) {




    if (length - i >= 3 && !data[i] && !data[i + 1] && data[i + 2] == 3) {

      out.push_back(data[i++]);
      out.push_back(data[i++]);

      i++;
    } else {

      out.push_back(data[i++]);
    }
  }
  return out;
}

void WriteRbsp(const uint8_t* bytes, size_t length, rtc::Buffer* destination) {
  static const uint8_t kZerosInStartSequence = 2;
  static const uint8_t kEmulationByte = 0x03u;
  size_t num_consecutive_zeros = 0;
  destination->EnsureCapacity(destination->size() + length);

  for (size_t i = 0; i < length; ++i) {
    uint8_t byte = bytes[i];
    if (byte <= kEmulationByte &&
        num_consecutive_zeros >= kZerosInStartSequence) {

      destination->AppendData(kEmulationByte);
      num_consecutive_zeros = 0;
    }
    destination->AppendData(byte);
    if (byte == 0) {
      ++num_consecutive_zeros;
    } else {
      num_consecutive_zeros = 0;
    }
  }
}

}  // namespace H264
}  // namespace webrtc
