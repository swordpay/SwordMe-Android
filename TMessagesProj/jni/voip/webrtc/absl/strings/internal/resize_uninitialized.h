//
// Copyright 2017 The Abseil Authors.
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
//

#ifndef ABSL_STRINGS_INTERNAL_RESIZE_UNINITIALIZED_H_
#define ABSL_STRINGS_INTERNAL_RESIZE_UNINITIALIZED_H_

#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/base/port.h"
#include "absl/meta/type_traits.h"  //  for void_t

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace strings_internal {

// we use it if available, otherwise, we use resize. We provide HasMember to
// indicate whether __resize_default_init is present.
template <typename string_type, typename = void>
struct ResizeUninitializedTraits {
  using HasMember = std::false_type;
  static void Resize(string_type* s, size_t new_size) { s->resize(new_size); }
};

template <typename string_type>
struct ResizeUninitializedTraits<
    string_type, absl::void_t<decltype(std::declval<string_type&>()
                                           .__resize_default_init(237))> > {
  using HasMember = std::true_type;
  static void Resize(string_type* s, size_t new_size) {
    s->__resize_default_init(new_size);
  }
};

// the new characters added to the std::string are left untouched.
//
// (A better name might be "STLStringSupportsUninitializedResize", alluding to
// the previous function.)
template <typename string_type>
inline constexpr bool STLStringSupportsNontrashingResize(string_type*) {
  return ResizeUninitializedTraits<string_type>::HasMember::value;
}

// result of resizing may be left uninitialized, rather than being filled with
// '0' bytes. Typically used when code is then going to overwrite the backing
// store of the std::string with known data.
template <typename string_type, typename = void>
inline void STLStringResizeUninitialized(string_type* s, size_t new_size) {
  ResizeUninitializedTraits<string_type>::Resize(s, new_size);
}

// increasing the string size by a small amount is O(1), in contrast to
// O(str->size()) in the case of precise growth.
template <typename string_type>
void STLStringReserveAmortized(string_type* s, size_t new_size) {
  const size_t cap = s->capacity();
  if (new_size > cap) {

    s->reserve((std::max)(new_size, 2 * cap));
  }
}

// we use it if available, otherwise, we use append.
template <typename string_type, typename = void>
struct AppendUninitializedTraits {
  static void Append(string_type* s, size_t n) {
    s->append(n, typename string_type::value_type());
  }
};

template <typename string_type>
struct AppendUninitializedTraits<
    string_type, absl::void_t<decltype(std::declval<string_type&>()
                                           .__append_default_init(237))> > {
  static void Append(string_type* s, size_t n) {
    s->__append_default_init(n);
  }
};

// exponential growth so that the amortized complexity of increasing the string
// size by a small amount is O(1), in contrast to O(str->size()) in the case of
// precise growth.
template <typename string_type>
void STLStringResizeUninitializedAmortized(string_type* s, size_t new_size) {
  const size_t size = s->size();
  if (new_size > size) {
    AppendUninitializedTraits<string_type>::Append(s, new_size - size);
  } else {
    s->erase(new_size);
  }
}

}  // namespace strings_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_RESIZE_UNINITIALIZED_H_
