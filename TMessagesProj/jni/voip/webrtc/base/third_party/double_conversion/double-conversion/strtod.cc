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

#include <climits>
#include <cstdarg>

#include "bignum.h"
#include "cached-powers.h"
#include "ieee.h"
#include "strtod.h"

namespace double_conversion {

#if defined(DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS)
// 2^53 = 9007199254740992.
// Any integer with at most 15 decimal digits will hence fit into a double
// (which has a 53bit significand) without loss of precision.
static const int kMaxExactDoubleIntegerDecimalDigits = 15;
#endif // #if defined(DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS)
// 2^64 = 18446744073709551616 > 10^19
static const int kMaxUint64DecimalDigits = 19;

// Min non-zero double: 4.9406564584124654 x 10^-324
// Any x >= 10^309 is interpreted as +infinity.
// Any x <= 10^-324 is interpreted as 0.
// Note that 2.5e-324 (despite being smaller than the min double) will be read
// as non-zero (equal to the min non-zero double).
static const int kMaxDecimalPower = 309;
static const int kMinDecimalPower = -324;

static const uint64_t kMaxUint64 = DOUBLE_CONVERSION_UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF);


#if defined(DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS)
static const double exact_powers_of_ten[] = {
  1.0,  // 10^0
  10.0,
  100.0,
  1000.0,
  10000.0,
  100000.0,
  1000000.0,
  10000000.0,
  100000000.0,
  1000000000.0,
  10000000000.0,  // 10^10
  100000000000.0,
  1000000000000.0,
  10000000000000.0,
  100000000000000.0,
  1000000000000000.0,
  10000000000000000.0,
  100000000000000000.0,
  1000000000000000000.0,
  10000000000000000000.0,
  100000000000000000000.0,  // 10^20
  1000000000000000000000.0,

  10000000000000000000000.0
};
static const int kExactPowersOfTenSize = DOUBLE_CONVERSION_ARRAY_SIZE(exact_powers_of_ten);
#endif // #if defined(DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS)

// In fact the value is 772 (see conversions.cc), but to give us some margin
// we round up to 780.
static const int kMaxSignificantDecimalDigits = 780;

static Vector<const char> TrimLeadingZeros(Vector<const char> buffer) {
  for (int i = 0; i < buffer.length(); i++) {
    if (buffer[i] != '0') {
      return buffer.SubVector(i, buffer.length());
    }
  }
  return Vector<const char>(buffer.start(), 0);
}


static Vector<const char> TrimTrailingZeros(Vector<const char> buffer) {
  for (int i = buffer.length() - 1; i >= 0; --i) {
    if (buffer[i] != '0') {
      return buffer.SubVector(0, i + 1);
    }
  }
  return Vector<const char>(buffer.start(), 0);
}


static void CutToMaxSignificantDigits(Vector<const char> buffer,
                                       int exponent,
                                       char* significant_buffer,
                                       int* significant_exponent) {
  for (int i = 0; i < kMaxSignificantDecimalDigits - 1; ++i) {
    significant_buffer[i] = buffer[i];
  }


  DOUBLE_CONVERSION_ASSERT(buffer[buffer.length() - 1] != '0');


  significant_buffer[kMaxSignificantDecimalDigits - 1] = '1';
  *significant_exponent =
      exponent + (buffer.length() - kMaxSignificantDecimalDigits);
}

// If possible the input-buffer is reused, but if the buffer needs to be
// modified (due to cutting), then the input needs to be copied into the
// buffer_copy_space.
static void TrimAndCut(Vector<const char> buffer, int exponent,
                       char* buffer_copy_space, int space_size,
                       Vector<const char>* trimmed, int* updated_exponent) {
  Vector<const char> left_trimmed = TrimLeadingZeros(buffer);
  Vector<const char> right_trimmed = TrimTrailingZeros(left_trimmed);
  exponent += left_trimmed.length() - right_trimmed.length();
  if (right_trimmed.length() > kMaxSignificantDecimalDigits) {
    (void) space_size;  // Mark variable as used.
    DOUBLE_CONVERSION_ASSERT(space_size >= kMaxSignificantDecimalDigits);
    CutToMaxSignificantDigits(right_trimmed, exponent,
                              buffer_copy_space, updated_exponent);
    *trimmed = Vector<const char>(buffer_copy_space,
                                 kMaxSignificantDecimalDigits);
  } else {
    *trimmed = right_trimmed;
    *updated_exponent = exponent;
  }
}

// Reads in as many digits as fit into a uint64.
// When the string starts with "1844674407370955161" no further digit is read.
// Since 2^64 = 18446744073709551616 it would still be possible read another
// digit if it was less or equal than 6, but this would complicate the code.
static uint64_t ReadUint64(Vector<const char> buffer,
                           int* number_of_read_digits) {
  uint64_t result = 0;
  int i = 0;
  while (i < buffer.length() && result <= (kMaxUint64 / 10 - 1)) {
    int digit = buffer[i++] - '0';
    DOUBLE_CONVERSION_ASSERT(0 <= digit && digit <= 9);
    result = 10 * result + digit;
  }
  *number_of_read_digits = i;
  return result;
}

// The returned DiyFp is not necessarily normalized.
// If remaining_decimals is zero then the returned DiyFp is accurate.
// Otherwise it has been rounded and has error of at most 1/2 ulp.
static void ReadDiyFp(Vector<const char> buffer,
                      DiyFp* result,
                      int* remaining_decimals) {
  int read_digits;
  uint64_t significand = ReadUint64(buffer, &read_digits);
  if (buffer.length() == read_digits) {
    *result = DiyFp(significand, 0);
    *remaining_decimals = 0;
  } else {

    if (buffer[read_digits] >= '5') {
      significand++;
    }

    int exponent = 0;
    *result = DiyFp(significand, exponent);
    *remaining_decimals = buffer.length() - read_digits;
  }
}


static bool DoubleStrtod(Vector<const char> trimmed,
                         int exponent,
                         double* result) {
#if !defined(DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS)






  return false;
#else
  if (trimmed.length() <= kMaxExactDoubleIntegerDecimalDigits) {
    int read_digits;






    if (exponent < 0 && -exponent < kExactPowersOfTenSize) {

      *result = static_cast<double>(ReadUint64(trimmed, &read_digits));
      DOUBLE_CONVERSION_ASSERT(read_digits == trimmed.length());
      *result /= exact_powers_of_ten[-exponent];
      return true;
    }
    if (0 <= exponent && exponent < kExactPowersOfTenSize) {

      *result = static_cast<double>(ReadUint64(trimmed, &read_digits));
      DOUBLE_CONVERSION_ASSERT(read_digits == trimmed.length());
      *result *= exact_powers_of_ten[exponent];
      return true;
    }
    int remaining_digits =
        kMaxExactDoubleIntegerDecimalDigits - trimmed.length();
    if ((0 <= exponent) &&
        (exponent - remaining_digits < kExactPowersOfTenSize)) {



      *result = static_cast<double>(ReadUint64(trimmed, &read_digits));
      DOUBLE_CONVERSION_ASSERT(read_digits == trimmed.length());
      *result *= exact_powers_of_ten[remaining_digits];
      *result *= exact_powers_of_ten[exponent - remaining_digits];
      return true;
    }
  }
  return false;
#endif
}

// The given exponent must be in the range [1; kDecimalExponentDistance[.
static DiyFp AdjustmentPowerOfTen(int exponent) {
  DOUBLE_CONVERSION_ASSERT(0 < exponent);
  DOUBLE_CONVERSION_ASSERT(exponent < PowersOfTenCache::kDecimalExponentDistance);


  DOUBLE_CONVERSION_ASSERT(PowersOfTenCache::kDecimalExponentDistance == 8);
  switch (exponent) {
    case 1: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0xa0000000, 00000000), -60);
    case 2: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0xc8000000, 00000000), -57);
    case 3: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0xfa000000, 00000000), -54);
    case 4: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0x9c400000, 00000000), -50);
    case 5: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0xc3500000, 00000000), -47);
    case 6: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0xf4240000, 00000000), -44);
    case 7: return DiyFp(DOUBLE_CONVERSION_UINT64_2PART_C(0x98968000, 00000000), -40);
    default:
      DOUBLE_CONVERSION_UNREACHABLE();
  }
}

// Otherwise it is either the correct double or the double that is just below
// the correct double.
static bool DiyFpStrtod(Vector<const char> buffer,
                        int exponent,
                        double* result) {
  DiyFp input;
  int remaining_decimals;
  ReadDiyFp(buffer, &input, &remaining_decimals);





  const int kDenominatorLog = 3;
  const int kDenominator = 1 << kDenominatorLog;

  exponent += remaining_decimals;
  uint64_t error = (remaining_decimals == 0 ? 0 : kDenominator / 2);

  int old_e = input.e();
  input.Normalize();
  error <<= old_e - input.e();

  DOUBLE_CONVERSION_ASSERT(exponent <= PowersOfTenCache::kMaxDecimalExponent);
  if (exponent < PowersOfTenCache::kMinDecimalExponent) {
    *result = 0.0;
    return true;
  }
  DiyFp cached_power;
  int cached_decimal_exponent;
  PowersOfTenCache::GetCachedPowerForDecimalExponent(exponent,
                                                     &cached_power,
                                                     &cached_decimal_exponent);

  if (cached_decimal_exponent != exponent) {
    int adjustment_exponent = exponent - cached_decimal_exponent;
    DiyFp adjustment_power = AdjustmentPowerOfTen(adjustment_exponent);
    input.Multiply(adjustment_power);
    if (kMaxUint64DecimalDigits - buffer.length() >= adjustment_exponent) {


      DOUBLE_CONVERSION_ASSERT(DiyFp::kSignificandSize == 64);
    } else {

      error += kDenominator / 2;
    }
  }

  input.Multiply(cached_power);





  int error_b = kDenominator / 2;
  int error_ab = (error == 0 ? 0 : 1);  // We round up to 1.
  int fixed_error = kDenominator / 2;
  error += error_b + error_ab + fixed_error;

  old_e = input.e();
  input.Normalize();
  error <<= old_e - input.e();

  int order_of_magnitude = DiyFp::kSignificandSize + input.e();
  int effective_significand_size =
      Double::SignificandSizeForOrderOfMagnitude(order_of_magnitude);
  int precision_digits_count =
      DiyFp::kSignificandSize - effective_significand_size;
  if (precision_digits_count + kDenominatorLog >= DiyFp::kSignificandSize) {



    int shift_amount = (precision_digits_count + kDenominatorLog) -
        DiyFp::kSignificandSize + 1;
    input.set_f(input.f() >> shift_amount);
    input.set_e(input.e() + shift_amount);


    error = (error >> shift_amount) + 1 + kDenominator;
    precision_digits_count -= shift_amount;
  }

  DOUBLE_CONVERSION_ASSERT(DiyFp::kSignificandSize == 64);
  DOUBLE_CONVERSION_ASSERT(precision_digits_count < 64);
  uint64_t one64 = 1;
  uint64_t precision_bits_mask = (one64 << precision_digits_count) - 1;
  uint64_t precision_bits = input.f() & precision_bits_mask;
  uint64_t half_way = one64 << (precision_digits_count - 1);
  precision_bits *= kDenominator;
  half_way *= kDenominator;
  DiyFp rounded_input(input.f() >> precision_digits_count,
                      input.e() + precision_digits_count);
  if (precision_bits >= half_way + error) {
    rounded_input.set_f(rounded_input.f() + 1);
  }




  *result = Double(rounded_input).value();
  if (half_way - error < precision_bits && precision_bits < half_way + error) {



    return false;
  } else {
    return true;
  }
}

//   - -1 if buffer*10^exponent < diy_fp.
//   -  0 if buffer*10^exponent == diy_fp.
//   - +1 if buffer*10^exponent > diy_fp.
// Preconditions:
//   buffer.length() + exponent <= kMaxDecimalPower + 1
//   buffer.length() + exponent > kMinDecimalPower
//   buffer.length() <= kMaxDecimalSignificantDigits
static int CompareBufferWithDiyFp(Vector<const char> buffer,
                                  int exponent,
                                  DiyFp diy_fp) {
  DOUBLE_CONVERSION_ASSERT(buffer.length() + exponent <= kMaxDecimalPower + 1);
  DOUBLE_CONVERSION_ASSERT(buffer.length() + exponent > kMinDecimalPower);
  DOUBLE_CONVERSION_ASSERT(buffer.length() <= kMaxSignificantDecimalDigits);




  DOUBLE_CONVERSION_ASSERT(((kMaxDecimalPower + 1) * 333 / 100) < Bignum::kMaxSignificantBits);
  Bignum buffer_bignum;
  Bignum diy_fp_bignum;
  buffer_bignum.AssignDecimalString(buffer);
  diy_fp_bignum.AssignUInt64(diy_fp.f());
  if (exponent >= 0) {
    buffer_bignum.MultiplyByPowerOfTen(exponent);
  } else {
    diy_fp_bignum.MultiplyByPowerOfTen(-exponent);
  }
  if (diy_fp.e() > 0) {
    diy_fp_bignum.ShiftLeft(diy_fp.e());
  } else {
    buffer_bignum.ShiftLeft(-diy_fp.e());
  }
  return Bignum::Compare(buffer_bignum, diy_fp_bignum);
}

// Returns false, when guess is either correct or the next-lower double.
static bool ComputeGuess(Vector<const char> trimmed, int exponent,
                         double* guess) {
  if (trimmed.length() == 0) {
    *guess = 0.0;
    return true;
  }
  if (exponent + trimmed.length() - 1 >= kMaxDecimalPower) {
    *guess = Double::Infinity();
    return true;
  }
  if (exponent + trimmed.length() <= kMinDecimalPower) {
    *guess = 0.0;
    return true;
  }

  if (DoubleStrtod(trimmed, exponent, guess) ||
      DiyFpStrtod(trimmed, exponent, guess)) {
    return true;
  }
  if (*guess == Double::Infinity()) {
    return true;
  }
  return false;
}

static bool IsDigit(const char d) {
  return ('0' <= d) && (d <= '9');
}

static bool IsNonZeroDigit(const char d) {
  return ('1' <= d) && (d <= '9');
}

static bool AssertTrimmedDigits(const Vector<const char>& buffer) {
  for(int i = 0; i < buffer.length(); ++i) {
    if(!IsDigit(buffer[i])) {
      return false;
    }
  }
  return (buffer.length() == 0) || (IsNonZeroDigit(buffer[0]) && IsNonZeroDigit(buffer[buffer.length()-1]));
}

double StrtodTrimmed(Vector<const char> trimmed, int exponent) {
  DOUBLE_CONVERSION_ASSERT(trimmed.length() <= kMaxSignificantDecimalDigits);
  DOUBLE_CONVERSION_ASSERT(AssertTrimmedDigits(trimmed));
  double guess;
  const bool is_correct = ComputeGuess(trimmed, exponent, &guess);
  if (is_correct) {
    return guess;
  }
  DiyFp upper_boundary = Double(guess).UpperBoundary();
  int comparison = CompareBufferWithDiyFp(trimmed, exponent, upper_boundary);
  if (comparison < 0) {
    return guess;
  } else if (comparison > 0) {
    return Double(guess).NextDouble();
  } else if ((Double(guess).Significand() & 1) == 0) {

    return guess;
  } else {
    return Double(guess).NextDouble();
  }
}

double Strtod(Vector<const char> buffer, int exponent) {
  char copy_buffer[kMaxSignificantDecimalDigits];
  Vector<const char> trimmed;
  int updated_exponent;
  TrimAndCut(buffer, exponent, copy_buffer, kMaxSignificantDecimalDigits,
             &trimmed, &updated_exponent);
  return StrtodTrimmed(trimmed, updated_exponent);
}

static float SanitizedDoubletof(double d) {
  DOUBLE_CONVERSION_ASSERT(d >= 0.0);





  float max_finite = 3.4028234663852885981170418348451692544e+38;



  double half_max_finite_infinity =
      3.40282356779733661637539395458142568448e+38;
  if (d >= max_finite) {
    if (d >= half_max_finite_infinity) {
      return Single::Infinity();
    } else {
      return max_finite;
    }
  } else {
    return static_cast<float>(d);
  }
}

float Strtof(Vector<const char> buffer, int exponent) {
  char copy_buffer[kMaxSignificantDecimalDigits];
  Vector<const char> trimmed;
  int updated_exponent;
  TrimAndCut(buffer, exponent, copy_buffer, kMaxSignificantDecimalDigits,
             &trimmed, &updated_exponent);
  exponent = updated_exponent;

  double double_guess;
  bool is_correct = ComputeGuess(trimmed, exponent, &double_guess);

  float float_guess = SanitizedDoubletof(double_guess);
  if (float_guess == double_guess) {

    return float_guess;
  }














  double double_next = Double(double_guess).NextDouble();
  double double_previous = Double(double_guess).PreviousDouble();

  float f1 = SanitizedDoubletof(double_previous);
  float f2 = float_guess;
  float f3 = SanitizedDoubletof(double_next);
  float f4;
  if (is_correct) {
    f4 = f3;
  } else {
    double double_next2 = Double(double_next).NextDouble();
    f4 = SanitizedDoubletof(double_next2);
  }
  (void) f2;  // Mark variable as used.
  DOUBLE_CONVERSION_ASSERT(f1 <= f2 && f2 <= f3 && f3 <= f4);


  if (f1 == f4) {
    return float_guess;
  }

  DOUBLE_CONVERSION_ASSERT((f1 != f2 && f2 == f3 && f3 == f4) ||
         (f1 == f2 && f2 != f3 && f3 == f4) ||
         (f1 == f2 && f2 == f3 && f3 != f4));


  float guess = f1;
  float next = f4;
  DiyFp upper_boundary;
  if (guess == 0.0f) {
    float min_float = 1e-45f;
    upper_boundary = Double(static_cast<double>(min_float) / 2).AsDiyFp();
  } else {
    upper_boundary = Single(guess).UpperBoundary();
  }
  int comparison = CompareBufferWithDiyFp(trimmed, exponent, upper_boundary);
  if (comparison < 0) {
    return guess;
  } else if (comparison > 0) {
    return next;
  } else if ((Single(guess).Significand() & 1) == 0) {

    return guess;
  } else {
    return next;
  }
}

}  // namespace double_conversion
