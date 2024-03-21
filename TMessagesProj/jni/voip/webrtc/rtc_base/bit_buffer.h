/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_BIT_BUFFER_H_
#define RTC_BASE_BIT_BUFFER_H_

#include <stddef.h>  // For size_t.
#include <stdint.h>  // For integer types.

namespace rtc {

// reading APIs of BitstreamReader.
// Sizes/counts specify bits/bytes, for clarity.
// Byte order is assumed big-endian/network.
class BitBufferWriter {
 public:

  BitBufferWriter(uint8_t* bytes, size_t byte_count);

  BitBufferWriter(const BitBufferWriter&) = delete;
  BitBufferWriter& operator=(const BitBufferWriter&) = delete;


  void GetCurrentOffset(size_t* out_byte_offset, size_t* out_bit_offset);

  uint64_t RemainingBitCount() const;


  bool ReadUInt8(uint8_t& val);
  bool ReadUInt16(uint16_t& val);
  bool ReadUInt32(uint32_t& val);
  bool ReadUInt8(uint8_t* val) {
    return val ? ReadUInt8(*val) : false;
  }
  bool ReadUInt16(uint16_t* val) {
    return val ? ReadUInt16(*val) : false;
  }
  bool ReadUInt32(uint32_t* val) {
    return val ? ReadUInt32(*val) : false;
  }


  bool ReadBits(size_t bit_count, uint32_t& val);
  bool ReadBits(size_t bit_count, uint64_t& val);
  bool ReadBits(uint32_t* val, size_t bit_count) {
    return val ? ReadBits(bit_count, *val) : false;
  }



  bool PeekBits(size_t bit_count, uint32_t& val);
  bool PeekBits(size_t bit_count, uint64_t& val);
  bool PeekBits(uint32_t* val, size_t bit_count) {
    return val ? PeekBits(bit_count, *val) : false;
  }










  bool ReadNonSymmetric(uint32_t num_values, uint32_t& val);
  bool ReadNonSymmetric(uint32_t* val, uint32_t num_values) {
    return val ? ReadNonSymmetric(num_values, *val) : false;
  }








  bool ReadExponentialGolomb(uint32_t& val);
  bool ReadExponentialGolomb(uint32_t* val) {
    return val ? ReadExponentialGolomb(*val) : false;
  }



  bool ReadSignedExponentialGolomb(int32_t& val);
  bool ReadSignedExponentialGolomb(int32_t* val) {
    return val ? ReadSignedExponentialGolomb(*val) : false;
  }


  bool ConsumeBytes(size_t byte_count);


  bool ConsumeBits(size_t bit_count);


  bool Seek(size_t byte_offset, size_t bit_offset);


  bool WriteUInt8(uint8_t val);
  bool WriteUInt16(uint16_t val);
  bool WriteUInt32(uint32_t val);


  bool WriteBits(uint64_t val, size_t bit_count);




  bool WriteNonSymmetric(uint32_t val, uint32_t num_values);

  static size_t SizeNonSymmetricBits(uint32_t val, uint32_t num_values);


  bool WriteExponentialGolomb(uint32_t val);



  bool WriteSignedExponentialGolomb(int32_t val);

 private:

  uint8_t* const writable_bytes_;

  const size_t byte_count_;

  size_t byte_offset_;

  size_t bit_offset_;
};

}  // namespace rtc

#endif  // RTC_BASE_BIT_BUFFER_H_
