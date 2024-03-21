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

#ifndef DOUBLE_CONVERSION_DIY_FP_H_
#define DOUBLE_CONVERSION_DIY_FP_H_

#include "utils.h"

namespace double_conversion {

// with a uint64 significand and an int exponent. Normalized DiyFp numbers will
// have the most significant bit of the significand set.
// Multiplication and Subtraction do not normalize their results.
// DiyFp store only non-negative numbers and are not designed to contain special
// doubles (NaN and Infinity).
class DiyFp {
 public:
  static const int kSignificandSize = 64;

  DiyFp() : f_(0), e_(0) {}
  DiyFp(const uint64_t significand, const int32_t exponent) : f_(significand), e_(exponent) {}




  void Subtract(const DiyFp& other) {
    DOUBLE_CONVERSION_ASSERT(e_ == other.e_);
    DOUBLE_CONVERSION_ASSERT(f_ >= other.f_);
    f_ -= other.f_;
  }



  static DiyFp Minus(const DiyFp& a, const DiyFp& b) {
    DiyFp result = a;
    result.Subtract(b);
    return result;
  }

  void Multiply(const DiyFp& other) {




    const uint64_t kM32 = 0xFFFFFFFFU;
    const uint64_t a = f_ >> 32;
    const uint64_t b = f_ & kM32;
    const uint64_t c = other.f_ >> 32;
    const uint64_t d = other.f_ & kM32;
    const uint64_t ac = a * c;
    const uint64_t bc = b * c;
    const uint64_t ad = a * d;
    const uint64_t bd = b * d;


    const uint64_t tmp = (bd >> 32) + (ad & kM32) + (bc & kM32) + (1U << 31);
    e_ += other.e_ + 64;
    f_ = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
  }

  static DiyFp Times(const DiyFp& a, const DiyFp& b) {
    DiyFp result = a;
    result.Multiply(b);
    return result;
  }

  void Normalize() {
    DOUBLE_CONVERSION_ASSERT(f_ != 0);
    uint64_t significand = f_;
    int32_t exponent = e_;


    const uint64_t k10MSBits = DOUBLE_CONVERSION_UINT64_2PART_C(0xFFC00000, 00000000);
    while ((significand & k10MSBits) == 0) {
      significand <<= 10;
      exponent -= 10;
    }
    while ((significand & kUint64MSB) == 0) {
      significand <<= 1;
      exponent--;
    }
    f_ = significand;
    e_ = exponent;
  }

  static DiyFp Normalize(const DiyFp& a) {
    DiyFp result = a;
    result.Normalize();
    return result;
  }

  uint64_t f() const { return f_; }
  int32_t e() const { return e_; }

  void set_f(uint64_t new_value) { f_ = new_value; }
  void set_e(int32_t new_value) { e_ = new_value; }

 private:
  static const uint64_t kUint64MSB = DOUBLE_CONVERSION_UINT64_2PART_C(0x80000000, 00000000);

  uint64_t f_;
  int32_t e_;
};

}  // namespace double_conversion

#endif  // DOUBLE_CONVERSION_DIY_FP_H_
