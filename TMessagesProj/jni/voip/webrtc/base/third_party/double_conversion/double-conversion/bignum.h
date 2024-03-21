// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DOUBLE_CONVERSION_BIGNUM_H_
#define DOUBLE_CONVERSION_BIGNUM_H_

#include "utils.h"

namespace double_conversion {

class Bignum {
 public:



  static const int kMaxSignificantBits = 3584;

  Bignum() : used_bigits_(0), exponent_(0) {}

  void AssignUInt16(const uint16_t value);
  void AssignUInt64(uint64_t value);
  void AssignBignum(const Bignum& other);

  void AssignDecimalString(const Vector<const char> value);
  void AssignHexString(const Vector<const char> value);

  void AssignPowerUInt16(uint16_t base, const int exponent);

  void AddUInt64(const uint64_t operand);
  void AddBignum(const Bignum& other);

  void SubtractBignum(const Bignum& other);

  void Square();
  void ShiftLeft(const int shift_amount);
  void MultiplyByUInt32(const uint32_t factor);
  void MultiplyByUInt64(const uint64_t factor);
  void MultiplyByPowerOfTen(const int exponent);
  void Times10() { return MultiplyByUInt32(10); }




  uint16_t DivideModuloIntBignum(const Bignum& other);

  bool ToHexString(char* buffer, const int buffer_size) const;




  static int Compare(const Bignum& a, const Bignum& b);
  static bool Equal(const Bignum& a, const Bignum& b) {
    return Compare(a, b) == 0;
  }
  static bool LessEqual(const Bignum& a, const Bignum& b) {
    return Compare(a, b) <= 0;
  }
  static bool Less(const Bignum& a, const Bignum& b) {
    return Compare(a, b) < 0;
  }

  static int PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c);

  static bool PlusEqual(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) == 0;
  }

  static bool PlusLessEqual(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) <= 0;
  }

  static bool PlusLess(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) < 0;
  }
 private:
  typedef uint32_t Chunk;
  typedef uint64_t DoubleChunk;

  static const int kChunkSize = sizeof(Chunk) * 8;
  static const int kDoubleChunkSize = sizeof(DoubleChunk) * 8;


  static const int kBigitSize = 28;
  static const Chunk kBigitMask = (1 << kBigitSize) - 1;


  static const int kBigitCapacity = kMaxSignificantBits / kBigitSize;

  static void EnsureCapacity(const int size) {
    if (size > kBigitCapacity) {
      DOUBLE_CONVERSION_UNREACHABLE();
    }
  }
  void Align(const Bignum& other);
  void Clamp();
  bool IsClamped() const {
    return used_bigits_ == 0 || RawBigit(used_bigits_ - 1) != 0;
  }
  void Zero() {
    used_bigits_ = 0;
    exponent_ = 0;
  }



  void BigitsShiftLeft(const int shift_amount);

  int BigitLength() const { return used_bigits_ + exponent_; }
  Chunk& RawBigit(const int index);
  const Chunk& RawBigit(const int index) const;
  Chunk BigitOrZero(const int index) const;
  void SubtractTimes(const Bignum& other, const int factor);




  int16_t used_bigits_;
  int16_t exponent_;
  Chunk bigits_buffer_[kBigitCapacity];

  DOUBLE_CONVERSION_DISALLOW_COPY_AND_ASSIGN(Bignum);
};

}  // namespace double_conversion

#endif  // DOUBLE_CONVERSION_BIGNUM_H_
