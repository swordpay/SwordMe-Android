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

#include <cmath>

#include "bignum-dtoa.h"

#include "bignum.h"
#include "ieee.h"

namespace double_conversion {

static int NormalizedExponent(uint64_t significand, int exponent) {
  DOUBLE_CONVERSION_ASSERT(significand != 0);
  while ((significand & Double::kHiddenBit) == 0) {
    significand = significand << 1;
    exponent = exponent - 1;
  }
  return exponent;
}

// Returns an estimation of k such that 10^(k-1) <= v < 10^k.
static int EstimatePower(int exponent);
// Computes v / 10^estimated_power exactly, as a ratio of two bignums, numerator
// and denominator.
static void InitialScaledStartValues(uint64_t significand,
                                     int exponent,
                                     bool lower_boundary_is_closer,
                                     int estimated_power,
                                     bool need_boundary_deltas,
                                     Bignum* numerator,
                                     Bignum* denominator,
                                     Bignum* delta_minus,
                                     Bignum* delta_plus);
// Multiplies numerator/denominator so that its values lies in the range 1-10.
// Returns decimal_point s.t.
//  v = numerator'/denominator' * 10^(decimal_point-1)
//     where numerator' and denominator' are the values of numerator and
//     denominator after the call to this function.
static void FixupMultiply10(int estimated_power, bool is_even,
                            int* decimal_point,
                            Bignum* numerator, Bignum* denominator,
                            Bignum* delta_minus, Bignum* delta_plus);
// Generates digits from the left to the right and stops when the generated
// digits yield the shortest decimal representation of v.
static void GenerateShortestDigits(Bignum* numerator, Bignum* denominator,
                                   Bignum* delta_minus, Bignum* delta_plus,
                                   bool is_even,
                                   Vector<char> buffer, int* length);
// Generates 'requested_digits' after the decimal point.
static void BignumToFixed(int requested_digits, int* decimal_point,
                          Bignum* numerator, Bignum* denominator,
                          Vector<char> buffer, int* length);
// Generates 'count' digits of numerator/denominator.
// Once 'count' digits have been produced rounds the result depending on the
// remainder (remainders of exactly .5 round upwards). Might update the
// decimal_point when rounding up (for example for 0.9999).
static void GenerateCountedDigits(int count, int* decimal_point,
                                  Bignum* numerator, Bignum* denominator,
                                  Vector<char> buffer, int* length);


void BignumDtoa(double v, BignumDtoaMode mode, int requested_digits,
                Vector<char> buffer, int* length, int* decimal_point) {
  DOUBLE_CONVERSION_ASSERT(v > 0);
  DOUBLE_CONVERSION_ASSERT(!Double(v).IsSpecial());
  uint64_t significand;
  int exponent;
  bool lower_boundary_is_closer;
  if (mode == BIGNUM_DTOA_SHORTEST_SINGLE) {
    float f = static_cast<float>(v);
    DOUBLE_CONVERSION_ASSERT(f == v);
    significand = Single(f).Significand();
    exponent = Single(f).Exponent();
    lower_boundary_is_closer = Single(f).LowerBoundaryIsCloser();
  } else {
    significand = Double(v).Significand();
    exponent = Double(v).Exponent();
    lower_boundary_is_closer = Double(v).LowerBoundaryIsCloser();
  }
  bool need_boundary_deltas =
      (mode == BIGNUM_DTOA_SHORTEST || mode == BIGNUM_DTOA_SHORTEST_SINGLE);

  bool is_even = (significand & 1) == 0;
  int normalized_exponent = NormalizedExponent(significand, exponent);

  int estimated_power = EstimatePower(normalized_exponent);




  if (mode == BIGNUM_DTOA_FIXED && -estimated_power - 1 > requested_digits) {
    buffer[0] = '\0';
    *length = 0;



    *decimal_point = -requested_digits;
    return;
  }

  Bignum numerator;
  Bignum denominator;
  Bignum delta_minus;
  Bignum delta_plus;




  DOUBLE_CONVERSION_ASSERT(Bignum::kMaxSignificantBits >= 324*4);
  InitialScaledStartValues(significand, exponent, lower_boundary_is_closer,
                           estimated_power, need_boundary_deltas,
                           &numerator, &denominator,
                           &delta_minus, &delta_plus);

  FixupMultiply10(estimated_power, is_even, decimal_point,
                  &numerator, &denominator,
                  &delta_minus, &delta_plus);


  switch (mode) {
    case BIGNUM_DTOA_SHORTEST:
    case BIGNUM_DTOA_SHORTEST_SINGLE:
      GenerateShortestDigits(&numerator, &denominator,
                             &delta_minus, &delta_plus,
                             is_even, buffer, length);
      break;
    case BIGNUM_DTOA_FIXED:
      BignumToFixed(requested_digits, decimal_point,
                    &numerator, &denominator,
                    buffer, length);
      break;
    case BIGNUM_DTOA_PRECISION:
      GenerateCountedDigits(requested_digits, decimal_point,
                            &numerator, &denominator,
                            buffer, length);
      break;
    default:
      DOUBLE_CONVERSION_UNREACHABLE();
  }
  buffer[*length] = '\0';
}

// when the generated digits yield the shortest decimal representation of v. A
// decimal representation of v is a number lying closer to v than to any other
// double, so it converts to v when read.
//
// This is true if d, the decimal representation, is between m- and m+, the
// upper and lower boundaries. d must be strictly between them if !is_even.
//           m- := (numerator - delta_minus) / denominator
//           m+ := (numerator + delta_plus) / denominator
//
// Precondition: 0 <= (numerator+delta_plus) / denominator < 10.
//   If 1 <= (numerator+delta_plus) / denominator < 10 then no leading 0 digit
//   will be produced. This should be the standard precondition.
static void GenerateShortestDigits(Bignum* numerator, Bignum* denominator,
                                   Bignum* delta_minus, Bignum* delta_plus,
                                   bool is_even,
                                   Vector<char> buffer, int* length) {


  if (Bignum::Equal(*delta_minus, *delta_plus)) {
    delta_plus = delta_minus;
  }
  *length = 0;
  for (;;) {
    uint16_t digit;
    digit = numerator->DivideModuloIntBignum(*denominator);
    DOUBLE_CONVERSION_ASSERT(digit <= 9);  // digit is a uint16_t and therefore always positive.


    buffer[(*length)++] = static_cast<char>(digit + '0');





    bool in_delta_room_minus;
    bool in_delta_room_plus;
    if (is_even) {
      in_delta_room_minus = Bignum::LessEqual(*numerator, *delta_minus);
    } else {
      in_delta_room_minus = Bignum::Less(*numerator, *delta_minus);
    }
    if (is_even) {
      in_delta_room_plus =
          Bignum::PlusCompare(*numerator, *delta_plus, *denominator) >= 0;
    } else {
      in_delta_room_plus =
          Bignum::PlusCompare(*numerator, *delta_plus, *denominator) > 0;
    }
    if (!in_delta_room_minus && !in_delta_room_plus) {

      numerator->Times10();
      delta_minus->Times10();



      if (delta_minus != delta_plus) {
        delta_plus->Times10();
      }
    } else if (in_delta_room_minus && in_delta_room_plus) {


      int compare = Bignum::PlusCompare(*numerator, *numerator, *denominator);
      if (compare < 0) {

      } else if (compare > 0) {





        DOUBLE_CONVERSION_ASSERT(buffer[(*length) - 1] != '9');
        buffer[(*length) - 1]++;
      } else {





        if ((buffer[(*length) - 1] - '0') % 2 == 0) {

        } else {
          DOUBLE_CONVERSION_ASSERT(buffer[(*length) - 1] != '9');
          buffer[(*length) - 1]++;
        }
      }
      return;
    } else if (in_delta_room_minus) {

      return;
    } else {  // in_delta_room_plus





      DOUBLE_CONVERSION_ASSERT(buffer[(*length) -1] != '9');
      buffer[(*length) - 1]++;
      return;
    }
  }
}

// Then we generate 'count' digits of d = x.xxxxx... (without the decimal point)
// from left to right. Once 'count' digits have been produced we decide wether
// to round up or down. Remainders of exactly .5 round upwards. Numbers such
// as 9.999999 propagate a carry all the way, and change the
// exponent (decimal_point), when rounding upwards.
static void GenerateCountedDigits(int count, int* decimal_point,
                                  Bignum* numerator, Bignum* denominator,
                                  Vector<char> buffer, int* length) {
  DOUBLE_CONVERSION_ASSERT(count >= 0);
  for (int i = 0; i < count - 1; ++i) {
    uint16_t digit;
    digit = numerator->DivideModuloIntBignum(*denominator);
    DOUBLE_CONVERSION_ASSERT(digit <= 9);  // digit is a uint16_t and therefore always positive.


    buffer[i] = static_cast<char>(digit + '0');

    numerator->Times10();
  }

  uint16_t digit;
  digit = numerator->DivideModuloIntBignum(*denominator);
  if (Bignum::PlusCompare(*numerator, *numerator, *denominator) >= 0) {
    digit++;
  }
  DOUBLE_CONVERSION_ASSERT(digit <= 10);
  buffer[count - 1] = static_cast<char>(digit + '0');


  for (int i = count - 1; i > 0; --i) {
    if (buffer[i] != '0' + 10) break;
    buffer[i] = '0';
    buffer[i - 1]++;
  }
  if (buffer[0] == '0' + 10) {

    buffer[0] = '1';
    (*decimal_point)++;
  }
  *length = count;
}

// trailing '0's. If the input number is too small then no digits at all are
// generated (ex.: 2 fixed digits for 0.00001).
//
// Input verifies:  1 <= (numerator + delta) / denominator < 10.
static void BignumToFixed(int requested_digits, int* decimal_point,
                          Bignum* numerator, Bignum* denominator,
                          Vector<char> buffer, int* length) {



  if (-(*decimal_point) > requested_digits) {





    *decimal_point = -requested_digits;
    *length = 0;
    return;
  } else if (-(*decimal_point) == requested_digits) {


    DOUBLE_CONVERSION_ASSERT(*decimal_point == -requested_digits);


    denominator->Times10();
    if (Bignum::PlusCompare(*numerator, *numerator, *denominator) >= 0) {


      buffer[0] = '1';
      *length = 1;
      (*decimal_point)++;
    } else {

      *length = 0;
    }
    return;
  } else {


    int needed_digits = (*decimal_point) + requested_digits;
    GenerateCountedDigits(needed_digits, decimal_point,
                          numerator, denominator,
                          buffer, length);
  }
}

// v = f * 2^exponent and 2^52 <= f < 2^53.
// v is hence a normalized double with the given exponent. The output is an
// approximation for the exponent of the decimal approimation .digits * 10^k.
//
// The result might undershoot by 1 in which case 10^k <= v < 10^k+1.
// Note: this property holds for v's upper boundary m+ too.
//    10^k <= m+ < 10^k+1.
//   (see explanation below).
//
// Examples:
//  EstimatePower(0)   => 16
//  EstimatePower(-52) => 0
//
// Note: e >= 0 => EstimatedPower(e) > 0. No similar claim can be made for e<0.
static int EstimatePower(int exponent) {





















  const double k1Log10 = 0.30102999566398114;  // 1/lg(10)

  const int kSignificandSize = Double::kSignificandSize;
  double estimate = ceil((exponent + kSignificandSize - 1) * k1Log10 - 1e-10);
  return static_cast<int>(estimate);
}

static void InitialScaledStartValuesPositiveExponent(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus) {

  DOUBLE_CONVERSION_ASSERT(estimated_power >= 0);



  numerator->AssignUInt64(significand);
  numerator->ShiftLeft(exponent);

  denominator->AssignPowerUInt16(10, estimated_power);

  if (need_boundary_deltas) {


    denominator->ShiftLeft(1);
    numerator->ShiftLeft(1);


    delta_plus->AssignUInt16(1);
    delta_plus->ShiftLeft(exponent);

    delta_minus->AssignUInt16(1);
    delta_minus->ShiftLeft(exponent);
  }
}

static void InitialScaledStartValuesNegativeExponentPositivePower(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus) {






  numerator->AssignUInt64(significand);

  denominator->AssignPowerUInt16(10, estimated_power);
  denominator->ShiftLeft(-exponent);

  if (need_boundary_deltas) {


    denominator->ShiftLeft(1);
    numerator->ShiftLeft(1);




    delta_plus->AssignUInt16(1);

    delta_minus->AssignUInt16(1);
  }
}

static void InitialScaledStartValuesNegativeExponentNegativePower(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus) {



  Bignum* power_ten = numerator;
  power_ten->AssignPowerUInt16(10, -estimated_power);

  if (need_boundary_deltas) {



    delta_plus->AssignBignum(*power_ten);
    delta_minus->AssignBignum(*power_ten);
  }





  DOUBLE_CONVERSION_ASSERT(numerator == power_ten);
  numerator->MultiplyByUInt64(significand);

  denominator->AssignUInt16(1);
  denominator->ShiftLeft(-exponent);

  if (need_boundary_deltas) {


    numerator->ShiftLeft(1);
    denominator->ShiftLeft(1);





  }
}

// Computes v / 10^estimated_power exactly, as a ratio of two bignums, numerator
// and denominator. The functions GenerateShortestDigits and
// GenerateCountedDigits will then convert this ratio to its decimal
// representation d, with the required accuracy.
// Then d * 10^estimated_power is the representation of v.
// (Note: the fraction and the estimated_power might get adjusted before
// generating the decimal representation.)
//
// The initial start values consist of:
//  - a scaled numerator: s.t. numerator/denominator == v / 10^estimated_power.
//  - a scaled (common) denominator.
//  optionally (used by GenerateShortestDigits to decide if it has the shortest
//  decimal converting back to v):
//  - v - m-: the distance to the lower boundary.
//  - m+ - v: the distance to the upper boundary.
//
// v, m+, m-, and therefore v - m- and m+ - v all share the same denominator.
//
// Let ep == estimated_power, then the returned values will satisfy:
//  v / 10^ep = numerator / denominator.
//  v's boundarys m- and m+:
//    m- / 10^ep == v / 10^ep - delta_minus / denominator
//    m+ / 10^ep == v / 10^ep + delta_plus / denominator
//  Or in other words:
//    m- == v - delta_minus * 10^ep / denominator;
//    m+ == v + delta_plus * 10^ep / denominator;
//
// Since 10^(k-1) <= v < 10^k    (with k == estimated_power)
//  or       10^k <= v < 10^(k+1)
//  we then have 0.1 <= numerator/denominator < 1
//           or    1 <= numerator/denominator < 10
//
// It is then easy to kickstart the digit-generation routine.
//
// The boundary-deltas are only filled if the mode equals BIGNUM_DTOA_SHORTEST
// or BIGNUM_DTOA_SHORTEST_SINGLE.

static void InitialScaledStartValues(uint64_t significand,
                                     int exponent,
                                     bool lower_boundary_is_closer,
                                     int estimated_power,
                                     bool need_boundary_deltas,
                                     Bignum* numerator,
                                     Bignum* denominator,
                                     Bignum* delta_minus,
                                     Bignum* delta_plus) {
  if (exponent >= 0) {
    InitialScaledStartValuesPositiveExponent(
        significand, exponent, estimated_power, need_boundary_deltas,
        numerator, denominator, delta_minus, delta_plus);
  } else if (estimated_power >= 0) {
    InitialScaledStartValuesNegativeExponentPositivePower(
        significand, exponent, estimated_power, need_boundary_deltas,
        numerator, denominator, delta_minus, delta_plus);
  } else {
    InitialScaledStartValuesNegativeExponentNegativePower(
        significand, exponent, estimated_power, need_boundary_deltas,
        numerator, denominator, delta_minus, delta_plus);
  }

  if (need_boundary_deltas && lower_boundary_is_closer) {


    denominator->ShiftLeft(1);  // *2
    numerator->ShiftLeft(1);    // *2
    delta_plus->ShiftLeft(1);   // *2
  }
}

// range 1-10. That is after a call to this function we have:
//    1 <= (numerator + delta_plus) /denominator < 10.
// Let numerator the input before modification and numerator' the argument
// after modification, then the output-parameter decimal_point is such that
//  numerator / denominator * 10^estimated_power ==
//    numerator' / denominator' * 10^(decimal_point - 1)
// In some cases estimated_power was too low, and this is already the case. We
// then simply adjust the power so that 10^(k-1) <= v < 10^k (with k ==
// estimated_power) but do not touch the numerator or denominator.
// Otherwise the routine multiplies the numerator and the deltas by 10.
static void FixupMultiply10(int estimated_power, bool is_even,
                            int* decimal_point,
                            Bignum* numerator, Bignum* denominator,
                            Bignum* delta_minus, Bignum* delta_plus) {
  bool in_range;
  if (is_even) {


    in_range = Bignum::PlusCompare(*numerator, *delta_plus, *denominator) >= 0;
  } else {
    in_range = Bignum::PlusCompare(*numerator, *delta_plus, *denominator) > 0;
  }
  if (in_range) {


    *decimal_point = estimated_power + 1;
  } else {
    *decimal_point = estimated_power;
    numerator->Times10();
    if (Bignum::Equal(*delta_minus, *delta_plus)) {
      delta_minus->Times10();
      delta_plus->AssignBignum(*delta_minus);
    } else {
      delta_minus->Times10();
      delta_plus->Times10();
    }
  }
}

}  // namespace double_conversion
