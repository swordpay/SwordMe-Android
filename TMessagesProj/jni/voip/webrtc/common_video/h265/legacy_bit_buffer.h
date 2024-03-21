/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef COMMON_VIDEO_H265_LEGACY_BIT_BUFFER_H_
#define COMMON_VIDEO_H265_LEGACY_BIT_BUFFER_H_

#include <stddef.h>  // For size_t.
#include <stdint.h>  // For integer types.

#include "rtc_base/constructor_magic.h"

namespace rtc {

// bytes. Has a similar API to ByteBuffer, plus methods for reading bit-sized
// and exponential golomb encoded data. For a writable version, use
// BitBufferWriter. Unlike ByteBuffer, this class doesn't make a copy of the
// source bytes, so it can be used on read-only data.
// Sizes/counts specify bits/bytes, for clarity.
// Byte order is assumed big-endian/network.
class BitBuffer {
 public:
  BitBuffer(const uint8_t* bytes, size_t byte_count);


  void GetCurrentOffset(size_t* out_byte_offset, size_t* out_bit_offset);

  uint64_t RemainingBitCount() const;


  bool ReadUInt8(uint8_t* val);
  bool ReadUInt16(uint16_t* val);
  bool ReadUInt32(uint32_t* val);


  bool ReadBits(uint32_t* val, size_t bit_count);



  bool PeekBits(uint32_t* val, size_t bit_count);










  bool ReadNonSymmetric(uint32_t* val, uint32_t num_values);








  bool ReadExponentialGolomb(uint32_t* val);



  bool ReadSignedExponentialGolomb(int32_t* val);


  bool ConsumeBytes(size_t byte_count);


  bool ConsumeBits(size_t bit_count);


  bool Seek(size_t byte_offset, size_t bit_offset);

 protected:
  const uint8_t* const bytes_;

  size_t byte_count_;

  size_t byte_offset_;

  size_t bit_offset_;

  RTC_DISALLOW_COPY_AND_ASSIGN(BitBuffer);
};

}  // namespace rtc

#endif  // COMMON_VIDEO_H265_LEGACY_BIT_BUFFER_H_
