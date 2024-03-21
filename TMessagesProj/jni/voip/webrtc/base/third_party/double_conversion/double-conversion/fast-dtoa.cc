// Copyright 2012 the V8 project authors. All rights reserved.
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

#include "fast-dtoa.h"

#include "cached-powers.h"
#include "diy-fp.h"
#include "ieee.h"

namespace double_conversion {

// exponent, where 'w' is the result of multiplying the input by a cached power
// of ten.
//
// A different range might be chosen on a different platform, to optimize digit
// generation, but a smaller range requires more powers of ten to be cached.
static const int kMinimalTargetExponent = -60;
static const int kMaximalTargetExponent = -32;

// solutions that may be inaccurate. A solution may be inaccurate if it is
// outside the safe interval, or if we cannot prove that it is closer to the
// input than a neighboring representation of the same length.
//
// Input: * buffer containing the digits of too_high / 10^kappa
//        * the buffer's length
//        * distance_too_high_w == (too_high - w).f() * unit
//        * unsafe_interval == (too_high - too_low).f() * unit
//        * rest = (too_high - buffer * 10^kappa).f() * unit
//        * ten_kappa = 10^kappa * unit
//        * unit = the common multiplier
// Output: returns true if the buffer is guaranteed to contain the closest
//    representable number to the input.
//  Modifies the generated digits in the buffer to approach (round towards) w.
static bool RoundWeed(Vector<char> buffer,
                      int length,
                      uint64_t distance_too_high_w,
                      uint64_t unsafe_interval,
                      uint64_t rest,
                      uint64_t ten_kappa,
                      uint64_t unit) {
  uint64_t small_distance = distance_too_high_w - unit;
  uint64_t big_distance = distance_too_high_w + unit;






































































  DOUBLE_CONVERSION_ASSERT(rest <= unsafe_interval);
  while (rest < small_distance &&  // Negated condition 1
         unsafe_interval - rest >= ten_kappa &&  // Negated condition 2
         (rest + ten_kappa < small_distance ||  // buffer{-1} > w_high
          small_distance - rest >= rest + ten_kappa - small_distance)) {
    buffer[length - 1]--;
    rest += ten_kappa;
  }



  if (rest < big_distance &&
      unsafe_interval - rest >= ten_kappa &&
      (rest + ten_kappa < big_distance ||
       big_distance - rest > rest + ten_kappa - big_distance)) {
    return false;
  }





  return (2 * unit <= rest) && (rest <= unsafe_interval - 4 * unit);
}

// 1 to the buffer. If the precision of the calculation is not sufficient to
// round correctly, return false.
// The rounding might shift the whole buffer in which case the kappa is
// adjusted. For example "99", kappa = 3 might become "10", kappa = 4.
//
// If 2*rest > ten_kappa then the buffer needs to be round up.
// rest can have an error of +/- 1 unit. This function accounts for the
// imprecision and returns false, if the rounding direction cannot be
// unambiguously determined.
//
// Precondition: rest < ten_kappa.
static bool RoundWeedCounted(Vector<char> buffer,
                             int length,
                             uint64_t rest,
                             uint64_t ten_kappa,
                             uint64_t unit,
                             int* kappa) {
  DOUBLE_CONVERSION_ASSERT(rest < ten_kappa);






  if (unit >= ten_kappa) return false;



  if (ten_kappa - unit <= unit) return false;

  if ((ten_kappa - rest > rest) && (ten_kappa - 2 * rest >= 2 * unit)) {
    return true;
  }

  if ((rest > unit) && (ten_kappa - (rest - unit) <= (rest - unit))) {

    buffer[length - 1]++;
    for (int i = length - 1; i > 0; --i) {
      if (buffer[i] != '0' + 10) break;
      buffer[i] = '0';
      buffer[i - 1]++;
    }




    if (buffer[0] == '0' + 10) {
      buffer[0] = '1';
      (*kappa) += 1;
    }
    return true;
  }
  return false;
}

// number. We furthermore receive the maximum number of bits 'number' has.
//
// Returns power == 10^(exponent_plus_one-1) such that
//    power <= number < power * 10.
// If number_bits == 0 then 0^(0-1) is returned.
// The number of bits must be <= 32.
// Precondition: number < (1 << (number_bits + 1)).

// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
static unsigned int const kSmallPowersOfTen[] =
    {0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
     1000000000};

static void BiggestPowerTen(uint32_t number,
                            int number_bits,
                            uint32_t* power,
                            int* exponent_plus_one) {
  DOUBLE_CONVERSION_ASSERT(number < (1u << (number_bits + 1)));

  int exponent_plus_one_guess = ((number_bits + 1) * 1233 >> 12);


  exponent_plus_one_guess++;

  if (number < kSmallPowersOfTen[exponent_plus_one_guess]) {
    exponent_plus_one_guess--;
  }
  *power = kSmallPowersOfTen[exponent_plus_one_guess];
  *exponent_plus_one = exponent_plus_one_guess;
}

// w is a floating-point number (DiyFp), consisting of a significand and an
// exponent. Its exponent is bounded by kMinimalTargetExponent and
// kMaximalTargetExponent.
//       Hence -60 <= w.e() <= -32.
//
// Returns false if it fails, in which case the generated digits in the buffer
// should not be used.
// Preconditions:
//  * low, w and high are correct up to 1 ulp (unit in the last place). That
//    is, their error must be less than a unit of their last digits.
//  * low.e() == w.e() == high.e()
//  * low < w < high, and taking into account their error: low~ <= high~
//  * kMinimalTargetExponent <= w.e() <= kMaximalTargetExponent
// Postconditions: returns false if procedure fails.
//   otherwise:
//     * buffer is not null-terminated, but len contains the number of digits.
//     * buffer contains the shortest possible decimal digit-sequence
//       such that LOW < buffer * 10^kappa < HIGH, where LOW and HIGH are the
//       correct values of low and high (without their error).
//     * if more than one decimal representation gives the minimal number of
//       decimal digits then the one closest to W (where W is the correct value
//       of w) is chosen.
// Remark: this procedure takes into account the imprecision of its input
//   numbers. If the precision is not enough to guarantee all the postconditions
//   then false is returned. This usually happens rarely (~0.5%).
//
// Say, for the sake of example, that
//   w.e() == -48, and w.f() == 0x1234567890abcdef
// w's value can be computed by w.f() * 2^w.e()
// We can obtain w's integral digits by simply shifting w.f() by -w.e().
//  -> w's integral part is 0x1234
//  w's fractional part is therefore 0x567890abcdef.
// Printing w's integral part is easy (simply print 0x1234 in decimal).
// In order to print its fraction we repeatedly multiply the fraction by 10 and
// get each digit. Example the first digit after the point would be computed by
//   (0x567890abcdef * 10) >> 48. -> 3
// The whole thing becomes slightly more complicated because we want to stop
// once we have enough digits. That is, once the digits inside the buffer
// represent 'w' we can stop. Everything inside the interval low - high
// represents w. However we have to pay attention to low, high and w's
// imprecision.
static bool DigitGen(DiyFp low,
                     DiyFp w,
                     DiyFp high,
                     Vector<char> buffer,
                     int* length,
                     int* kappa) {
  DOUBLE_CONVERSION_ASSERT(low.e() == w.e() && w.e() == high.e());
  DOUBLE_CONVERSION_ASSERT(low.f() + 1 <= high.f() - 1);
  DOUBLE_CONVERSION_ASSERT(kMinimalTargetExponent <= w.e() && w.e() <= kMaximalTargetExponent);











  uint64_t unit = 1;
  DiyFp too_low = DiyFp(low.f() - unit, low.e());
  DiyFp too_high = DiyFp(high.f() + unit, high.e());


  DiyFp unsafe_interval = DiyFp::Minus(too_high, too_low);







  DiyFp one = DiyFp(static_cast<uint64_t>(1) << -w.e(), w.e());

  uint32_t integrals = static_cast<uint32_t>(too_high.f() >> -one.e());

  uint64_t fractionals = too_high.f() & (one.f() - 1);
  uint32_t divisor;
  int divisor_exponent_plus_one;
  BiggestPowerTen(integrals, DiyFp::kSignificandSize - (-one.e()),
                  &divisor, &divisor_exponent_plus_one);
  *kappa = divisor_exponent_plus_one;
  *length = 0;




  while (*kappa > 0) {
    int digit = integrals / divisor;
    DOUBLE_CONVERSION_ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    integrals %= divisor;
    (*kappa)--;


    uint64_t rest =
        (static_cast<uint64_t>(integrals) << -one.e()) + fractionals;


    if (rest < unsafe_interval.f()) {


      return RoundWeed(buffer, *length, DiyFp::Minus(too_high, w).f(),
                       unsafe_interval.f(), rest,
                       static_cast<uint64_t>(divisor) << -one.e(), unit);
    }
    divisor /= 10;
  }






  DOUBLE_CONVERSION_ASSERT(one.e() >= -60);
  DOUBLE_CONVERSION_ASSERT(fractionals < one.f());
  DOUBLE_CONVERSION_ASSERT(DOUBLE_CONVERSION_UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF) / 10 >= one.f());
  for (;;) {
    fractionals *= 10;
    unit *= 10;
    unsafe_interval.set_f(unsafe_interval.f() * 10);

    int digit = static_cast<int>(fractionals >> -one.e());
    DOUBLE_CONVERSION_ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    fractionals &= one.f() - 1;  // Modulo by one.
    (*kappa)--;
    if (fractionals < unsafe_interval.f()) {
      return RoundWeed(buffer, *length, DiyFp::Minus(too_high, w).f() * unit,
                       unsafe_interval.f(), fractionals, one.f(), unit);
    }
  }
}

// w is a floating-point number (DiyFp), consisting of a significand and an
// exponent. Its exponent is bounded by kMinimalTargetExponent and
// kMaximalTargetExponent.
//       Hence -60 <= w.e() <= -32.
//
// Returns false if it fails, in which case the generated digits in the buffer
// should not be used.
// Preconditions:
//  * w is correct up to 1 ulp (unit in the last place). That
//    is, its error must be strictly less than a unit of its last digit.
//  * kMinimalTargetExponent <= w.e() <= kMaximalTargetExponent
//
// Postconditions: returns false if procedure fails.
//   otherwise:
//     * buffer is not null-terminated, but length contains the number of
//       digits.
//     * the representation in buffer is the most precise representation of
//       requested_digits digits.
//     * buffer contains at most requested_digits digits of w. If there are less
//       than requested_digits digits then some trailing '0's have been removed.
//     * kappa is such that
//            w = buffer * 10^kappa + eps with |eps| < 10^kappa / 2.
//
// Remark: This procedure takes into account the imprecision of its input
//   numbers. If the precision is not enough to guarantee all the postconditions
//   then false is returned. This usually happens rarely, but the failure-rate
//   increases with higher requested_digits.
static bool DigitGenCounted(DiyFp w,
                            int requested_digits,
                            Vector<char> buffer,
                            int* length,
                            int* kappa) {
  DOUBLE_CONVERSION_ASSERT(kMinimalTargetExponent <= w.e() && w.e() <= kMaximalTargetExponent);
  DOUBLE_CONVERSION_ASSERT(kMinimalTargetExponent >= -60);
  DOUBLE_CONVERSION_ASSERT(kMaximalTargetExponent <= -32);


  uint64_t w_error = 1;




  DiyFp one = DiyFp(static_cast<uint64_t>(1) << -w.e(), w.e());

  uint32_t integrals = static_cast<uint32_t>(w.f() >> -one.e());

  uint64_t fractionals = w.f() & (one.f() - 1);
  uint32_t divisor;
  int divisor_exponent_plus_one;
  BiggestPowerTen(integrals, DiyFp::kSignificandSize - (-one.e()),
                  &divisor, &divisor_exponent_plus_one);
  *kappa = divisor_exponent_plus_one;
  *length = 0;




  while (*kappa > 0) {
    int digit = integrals / divisor;
    DOUBLE_CONVERSION_ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    requested_digits--;
    integrals %= divisor;
    (*kappa)--;


    if (requested_digits == 0) break;
    divisor /= 10;
  }

  if (requested_digits == 0) {
    uint64_t rest =
        (static_cast<uint64_t>(integrals) << -one.e()) + fractionals;
    return RoundWeedCounted(buffer, *length, rest,
                            static_cast<uint64_t>(divisor) << -one.e(), w_error,
                            kappa);
  }






  DOUBLE_CONVERSION_ASSERT(one.e() >= -60);
  DOUBLE_CONVERSION_ASSERT(fractionals < one.f());
  DOUBLE_CONVERSION_ASSERT(DOUBLE_CONVERSION_UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF) / 10 >= one.f());
  while (requested_digits > 0 && fractionals > w_error) {
    fractionals *= 10;
    w_error *= 10;

    int digit = static_cast<int>(fractionals >> -one.e());
    DOUBLE_CONVERSION_ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    requested_digits--;
    fractionals &= one.f() - 1;  // Modulo by one.
    (*kappa)--;
  }
  if (requested_digits != 0) return false;
  return RoundWeedCounted(buffer, *length, fractionals, one.f(), w_error,
                          kappa);
}

// Returns true if it succeeds, otherwise the result cannot be trusted.
// There will be *length digits inside the buffer (not null-terminated).
// If the function returns true then
//        v == (double) (buffer * 10^decimal_exponent).
// The digits in the buffer are the shortest representation possible: no
// 0.09999999999999999 instead of 0.1. The shorter representation will even be
// chosen even if the longer one would be closer to v.
// The last digit will be closest to the actual v. That is, even if several
// digits might correctly yield 'v' when read again, the closest will be
// computed.
static bool Grisu3(double v,
                   FastDtoaMode mode,
                   Vector<char> buffer,
                   int* length,
                   int* decimal_exponent) {
  DiyFp w = Double(v).AsNormalizedDiyFp();




  DiyFp boundary_minus, boundary_plus;
  if (mode == FAST_DTOA_SHORTEST) {
    Double(v).NormalizedBoundaries(&boundary_minus, &boundary_plus);
  } else {
    DOUBLE_CONVERSION_ASSERT(mode == FAST_DTOA_SHORTEST_SINGLE);
    float single_v = static_cast<float>(v);
    Single(single_v).NormalizedBoundaries(&boundary_minus, &boundary_plus);
  }
  DOUBLE_CONVERSION_ASSERT(boundary_plus.e() == w.e());
  DiyFp ten_mk;  // Cached power of ten: 10^-k
  int mk;        // -k
  int ten_mk_minimal_binary_exponent =
     kMinimalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  int ten_mk_maximal_binary_exponent =
     kMaximalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  PowersOfTenCache::GetCachedPowerForBinaryExponentRange(
      ten_mk_minimal_binary_exponent,
      ten_mk_maximal_binary_exponent,
      &ten_mk, &mk);
  DOUBLE_CONVERSION_ASSERT((kMinimalTargetExponent <= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize) &&
         (kMaximalTargetExponent >= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize));








  DiyFp scaled_w = DiyFp::Times(w, ten_mk);
  DOUBLE_CONVERSION_ASSERT(scaled_w.e() ==
         boundary_plus.e() + ten_mk.e() + DiyFp::kSignificandSize);





  DiyFp scaled_boundary_minus = DiyFp::Times(boundary_minus, ten_mk);
  DiyFp scaled_boundary_plus  = DiyFp::Times(boundary_plus,  ten_mk);






  int kappa;
  bool result = DigitGen(scaled_boundary_minus, scaled_w, scaled_boundary_plus,
                         buffer, length, &kappa);
  *decimal_exponent = -mk + kappa;
  return result;
}

// number of digits. This version does not generate the shortest representation,
// and with enough requested digits 0.1 will at some point print as 0.9999999...
// Grisu3 is too imprecise for real halfway cases (1.5 will not work) and
// therefore the rounding strategy for halfway cases is irrelevant.
static bool Grisu3Counted(double v,
                          int requested_digits,
                          Vector<char> buffer,
                          int* length,
                          int* decimal_exponent) {
  DiyFp w = Double(v).AsNormalizedDiyFp();
  DiyFp ten_mk;  // Cached power of ten: 10^-k
  int mk;        // -k
  int ten_mk_minimal_binary_exponent =
     kMinimalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  int ten_mk_maximal_binary_exponent =
     kMaximalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  PowersOfTenCache::GetCachedPowerForBinaryExponentRange(
      ten_mk_minimal_binary_exponent,
      ten_mk_maximal_binary_exponent,
      &ten_mk, &mk);
  DOUBLE_CONVERSION_ASSERT((kMinimalTargetExponent <= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize) &&
         (kMaximalTargetExponent >= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize));








  DiyFp scaled_w = DiyFp::Times(w, ten_mk);





  int kappa;
  bool result = DigitGenCounted(scaled_w, requested_digits,
                                buffer, length, &kappa);
  *decimal_exponent = -mk + kappa;
  return result;
}


bool FastDtoa(double v,
              FastDtoaMode mode,
              int requested_digits,
              Vector<char> buffer,
              int* length,
              int* decimal_point) {
  DOUBLE_CONVERSION_ASSERT(v > 0);
  DOUBLE_CONVERSION_ASSERT(!Double(v).IsSpecial());

  bool result = false;
  int decimal_exponent = 0;
  switch (mode) {
    case FAST_DTOA_SHORTEST:
    case FAST_DTOA_SHORTEST_SINGLE:
      result = Grisu3(v, mode, buffer, length, &decimal_exponent);
      break;
    case FAST_DTOA_PRECISION:
      result = Grisu3Counted(v, requested_digits,
                             buffer, length, &decimal_exponent);
      break;
    default:
      DOUBLE_CONVERSION_UNREACHABLE();
  }
  if (result) {
    *decimal_point = *length + decimal_exponent;
    buffer[*length] = '\0';
  }
  return result;
}

}  // namespace double_conversion
