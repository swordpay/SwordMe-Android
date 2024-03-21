// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_STRINGS_INTERNAL_CHARCONV_PARSE_H_
#define ABSL_STRINGS_INTERNAL_CHARCONV_PARSE_H_

#include <cstdint>

#include "absl/base/config.h"
#include "absl/strings/charconv.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace strings_internal {

enum class FloatType { kNumber, kInfinity, kNan };

struct ParsedFloat {














  uint64_t mantissa = 0;




  int exponent = 0;


  int literal_exponent = 0;

  FloatType type = FloatType::kNumber;








  const char* subrange_begin = nullptr;
  const char* subrange_end = nullptr;


  const char* end = nullptr;
};

// ParsedFloat accordingly.
//
// format_flags is a bitmask value specifying what patterns this API will match.
// `scientific` and `fixed`  are honored per std::from_chars rules
// ([utility.from.chars], C++17): if exactly one of these bits is set, then an
// exponent is required, or dislallowed, respectively.
//
// Template parameter `base` must be either 10 or 16.  For base 16, a "0x" is
// *not* consumed.  The `hex` bit from format_flags is ignored by ParseFloat.
template <int base>
ParsedFloat ParseFloat(const char* begin, const char* end,
                       absl::chars_format format_flags);

extern template ParsedFloat ParseFloat<10>(const char* begin, const char* end,
                                           absl::chars_format format_flags);
extern template ParsedFloat ParseFloat<16>(const char* begin, const char* end,
                                           absl::chars_format format_flags);

}  // namespace strings_internal
ABSL_NAMESPACE_END
}  // namespace absl
#endif  // ABSL_STRINGS_INTERNAL_CHARCONV_PARSE_H_
