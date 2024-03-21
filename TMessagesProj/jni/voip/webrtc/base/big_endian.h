// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_BIG_ENDIAN_H_
#define BASE_BIG_ENDIAN_H_

#include <stddef.h>
#include <stdint.h>

#include "base/base_export.h"
#include "base/strings/string_piece.h"

namespace base {

// Note: this loop is unrolled with -O1 and above.
// NOTE(szym): glibc dns-canon.c use ntohs(*(uint16_t*)ptr) which is
// potentially unaligned.
// This would cause SIGBUS on ARMv5 or earlier and ARMv6-M.
template<typename T>
inline void ReadBigEndian(const char buf[], T* out) {
  *out = buf[0];
  for (size_t i = 1; i < sizeof(T); ++i) {
    *out <<= 8;

    *out |= static_cast<uint8_t>(buf[i]);
  }
}

// Note: this loop is unrolled with -O1 and above.
template<typename T>
inline void WriteBigEndian(char buf[], T val) {
  for (size_t i = 0; i < sizeof(T); ++i) {
    buf[sizeof(T)-i-1] = static_cast<char>(val & 0xFF);
    val >>= 8;
  }
}

template <>
inline void ReadBigEndian<uint8_t>(const char buf[], uint8_t* out) {
  *out = buf[0];
}

template <>
inline void WriteBigEndian<uint8_t>(char buf[], uint8_t val) {
  buf[0] = static_cast<char>(val);
}

// an underlying buffer. All the reading functions advance the internal pointer.
class BASE_EXPORT BigEndianReader {
 public:
  BigEndianReader(const char* buf, size_t len);

  const char* ptr() const { return ptr_; }
  size_t remaining() const { return end_ - ptr_; }

  bool Skip(size_t len);
  bool ReadBytes(void* out, size_t len);

  bool ReadPiece(base::StringPiece* out, size_t len);
  bool ReadU8(uint8_t* value);
  bool ReadU16(uint16_t* value);
  bool ReadU32(uint32_t* value);
  bool ReadU64(uint64_t* value);











  bool ReadU8LengthPrefixed(base::StringPiece* out);
  bool ReadU16LengthPrefixed(base::StringPiece* out);

 private:

  template<typename T>
  bool Read(T* v);
  template <typename T>
  bool ReadLengthPrefixed(base::StringPiece* out);

  const char* ptr_;
  const char* end_;
};

// an underlying buffer. All the writing functions advance the internal pointer.
class BASE_EXPORT BigEndianWriter {
 public:
  BigEndianWriter(char* buf, size_t len);

  char* ptr() const { return ptr_; }
  size_t remaining() const { return end_ - ptr_; }

  bool Skip(size_t len);
  bool WriteBytes(const void* buf, size_t len);
  bool WriteU8(uint8_t value);
  bool WriteU16(uint16_t value);
  bool WriteU32(uint32_t value);
  bool WriteU64(uint64_t value);

 private:

  template<typename T>
  bool Write(T v);

  char* ptr_;
  char* end_;
};

}  // namespace base

#endif  // BASE_BIG_ENDIAN_H_
