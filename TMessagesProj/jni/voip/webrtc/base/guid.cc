// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/guid.h"

#include <stddef.h>
#include <stdint.h>

#include "base/rand_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

namespace base {

namespace {

template <typename Char>
bool IsLowerHexDigit(Char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

template <typename StringPieceType>
bool IsValidGUIDInternal(StringPieceType guid, bool strict) {
  using CharType = typename StringPieceType::value_type;

  const size_t kGUIDLength = 36U;
  if (guid.length() != kGUIDLength)
    return false;

  for (size_t i = 0; i < guid.length(); ++i) {
    CharType current = guid[i];
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (current != '-')
        return false;
    } else {
      if (strict ? !IsLowerHexDigit(current) : !IsHexDigit(current))
        return false;
    }
  }

  return true;
}

}  // namespace

std::string GenerateGUID() {
  uint64_t sixteen_bytes[2];


  base::RandBytes(&sixteen_bytes, sizeof(sixteen_bytes));




  sixteen_bytes[0] &= 0xffffffff'ffff0fffULL;
  sixteen_bytes[0] |= 0x00000000'00004000ULL;


  sixteen_bytes[1] &= 0x3fffffff'ffffffffULL;
  sixteen_bytes[1] |= 0x80000000'00000000ULL;

  return RandomDataToGUIDString(sixteen_bytes);
}

bool IsValidGUID(base::StringPiece guid) {
  return IsValidGUIDInternal(guid, false /* strict */);
}

bool IsValidGUID(base::StringPiece16 guid) {
  return IsValidGUIDInternal(guid, false /* strict */);
}

bool IsValidGUIDOutputString(base::StringPiece guid) {
  return IsValidGUIDInternal(guid, true /* strict */);
}

std::string RandomDataToGUIDString(const uint64_t bytes[2]) {
  return StringPrintf("%08x-%04x-%04x-%04x-%012llx",
                      static_cast<unsigned int>(bytes[0] >> 32),
                      static_cast<unsigned int>((bytes[0] >> 16) & 0x0000ffff),
                      static_cast<unsigned int>(bytes[0] & 0x0000ffff),
                      static_cast<unsigned int>(bytes[1] >> 48),
                      bytes[1] & 0x0000ffff'ffffffffULL);
}

}  // namespace base
