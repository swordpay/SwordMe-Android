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

#ifndef DOUBLE_CONVERSION_STRING_TO_DOUBLE_H_
#define DOUBLE_CONVERSION_STRING_TO_DOUBLE_H_

#include "utils.h"

namespace double_conversion {

class StringToDoubleConverter {
 public:


  enum Flags {
    NO_FLAGS = 0,
    ALLOW_HEX = 1,
    ALLOW_OCTALS = 2,
    ALLOW_TRAILING_JUNK = 4,
    ALLOW_LEADING_SPACES = 8,
    ALLOW_TRAILING_SPACES = 16,
    ALLOW_SPACES_AFTER_SIGN = 32,
    ALLOW_CASE_INSENSITIVITY = 64,
    ALLOW_CASE_INSENSIBILITY = 64,  // Deprecated
    ALLOW_HEX_FLOATS = 128,
  };

  static const uc16 kNoSeparator = '\0';

















































































































  StringToDoubleConverter(int flags,
                          double empty_string_value,
                          double junk_string_value,
                          const char* infinity_symbol,
                          const char* nan_symbol,
                          uc16 separator = kNoSeparator)
      : flags_(flags),
        empty_string_value_(empty_string_value),
        junk_string_value_(junk_string_value),
        infinity_symbol_(infinity_symbol),
        nan_symbol_(nan_symbol),
        separator_(separator) {
  }





  double StringToDouble(const char* buffer,
                        int length,
                        int* processed_characters_count) const;

  double StringToDouble(const uc16* buffer,
                        int length,
                        int* processed_characters_count) const;



  float StringToFloat(const char* buffer,
                      int length,
                      int* processed_characters_count) const;

  float StringToFloat(const uc16* buffer,
                      int length,
                      int* processed_characters_count) const;

 private:
  const int flags_;
  const double empty_string_value_;
  const double junk_string_value_;
  const char* const infinity_symbol_;
  const char* const nan_symbol_;
  const uc16 separator_;

  template <class Iterator>
  double StringToIeee(Iterator start_pointer,
                      int length,
                      bool read_as_double,
                      int* processed_characters_count) const;

  DOUBLE_CONVERSION_DISALLOW_IMPLICIT_CONSTRUCTORS(StringToDoubleConverter);
};

}  // namespace double_conversion

#endif  // DOUBLE_CONVERSION_STRING_TO_DOUBLE_H_
