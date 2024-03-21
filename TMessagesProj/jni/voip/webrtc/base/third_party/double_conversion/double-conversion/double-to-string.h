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

#ifndef DOUBLE_CONVERSION_DOUBLE_TO_STRING_H_
#define DOUBLE_CONVERSION_DOUBLE_TO_STRING_H_

#include "utils.h"

namespace double_conversion {

class DoubleToStringConverter {
 public:



  static const int kMaxFixedDigitsBeforePoint = 60;
  static const int kMaxFixedDigitsAfterPoint = 60;


  static const int kMaxExponentialDigits = 120;



  static const int kMinPrecisionDigits = 1;
  static const int kMaxPrecisionDigits = 120;

  enum Flags {
    NO_FLAGS = 0,
    EMIT_POSITIVE_EXPONENT_SIGN = 1,
    EMIT_TRAILING_DECIMAL_POINT = 2,
    EMIT_TRAILING_ZERO_AFTER_POINT = 4,
    UNIQUE_ZERO = 8
  };




















































  DoubleToStringConverter(int flags,
                          const char* infinity_symbol,
                          const char* nan_symbol,
                          char exponent_character,
                          int decimal_in_shortest_low,
                          int decimal_in_shortest_high,
                          int max_leading_padding_zeroes_in_precision_mode,
                          int max_trailing_padding_zeroes_in_precision_mode,
                          int min_exponent_width = 0)
      : flags_(flags),
        infinity_symbol_(infinity_symbol),
        nan_symbol_(nan_symbol),
        exponent_character_(exponent_character),
        decimal_in_shortest_low_(decimal_in_shortest_low),
        decimal_in_shortest_high_(decimal_in_shortest_high),
        max_leading_padding_zeroes_in_precision_mode_(
            max_leading_padding_zeroes_in_precision_mode),
        max_trailing_padding_zeroes_in_precision_mode_(
            max_trailing_padding_zeroes_in_precision_mode),
        min_exponent_width_(min_exponent_width) {


    DOUBLE_CONVERSION_ASSERT(((flags & EMIT_TRAILING_DECIMAL_POINT) != 0) ||
        !((flags & EMIT_TRAILING_ZERO_AFTER_POINT) != 0));
  }

  static const DoubleToStringConverter& EcmaScriptConverter();























  bool ToShortest(double value, StringBuilder* result_builder) const {
    return ToShortestIeeeNumber(value, result_builder, SHORTEST);
  }

  bool ToShortestSingle(float value, StringBuilder* result_builder) const {
    return ToShortestIeeeNumber(value, result_builder, SHORTEST_SINGLE);
  }

































  bool ToFixed(double value,
               int requested_digits,
               StringBuilder* result_builder) const;




























  bool ToExponential(double value,
                     int requested_digits,
                     StringBuilder* result_builder) const;


































  bool ToPrecision(double value,
                   int precision,
                   StringBuilder* result_builder) const;

  enum DtoaMode {



    SHORTEST,

    SHORTEST_SINGLE,



    FIXED,

    PRECISION
  };






  static const int kBase10MaximalLength = 17;











































  static void DoubleToAscii(double v,
                            DtoaMode mode,
                            int requested_digits,
                            char* buffer,
                            int buffer_length,
                            bool* sign,
                            int* length,
                            int* point);

 private:

  bool ToShortestIeeeNumber(double value,
                            StringBuilder* result_builder,
                            DtoaMode mode) const;




  bool HandleSpecialValues(double value, StringBuilder* result_builder) const;


  void CreateExponentialRepresentation(const char* decimal_digits,
                                       int length,
                                       int exponent,
                                       StringBuilder* result_builder) const;

  void CreateDecimalRepresentation(const char* decimal_digits,
                                   int length,
                                   int decimal_point,
                                   int digits_after_point,
                                   StringBuilder* result_builder) const;

  const int flags_;
  const char* const infinity_symbol_;
  const char* const nan_symbol_;
  const char exponent_character_;
  const int decimal_in_shortest_low_;
  const int decimal_in_shortest_high_;
  const int max_leading_padding_zeroes_in_precision_mode_;
  const int max_trailing_padding_zeroes_in_precision_mode_;
  const int min_exponent_width_;

  DOUBLE_CONVERSION_DISALLOW_IMPLICIT_CONSTRUCTORS(DoubleToStringConverter);
};

}  // namespace double_conversion

#endif  // DOUBLE_CONVERSION_DOUBLE_TO_STRING_H_
