// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PARAMETER_PACK_H_
#define BASE_PARAMETER_PACK_H_

#include <stddef.h>

#include <initializer_list>
#include <tuple>
#include <type_traits>

#include "base/template_util.h"
#include "build/build_config.h"

namespace base {

// Similar to std::any_of for the case of constexpr initializer_list.
inline constexpr bool any_of(std::initializer_list<bool> ilist) {
  for (auto c : ilist) {
    if (c)
      return true;
  }
  return false;
}

// Similar to std::all_of for the case of constexpr initializer_list.
inline constexpr bool all_of(std::initializer_list<bool> ilist) {
  for (auto c : ilist) {
    if (!c)
      return false;
  }
  return true;
}

// Similar to std::count for the case of constexpr initializer_list.
template <class T>
inline constexpr size_t count(std::initializer_list<T> ilist, T value) {
  size_t c = 0;
  for (const auto& v : ilist) {
    c += (v == value);
  }
  return c;
}

constexpr size_t pack_npos = -1;

template <typename... Ts>
struct ParameterPack {

  template <typename Type>
  using HasType = bool_constant<any_of({std::is_same<Type, Ts>::value...})>;

  template <typename Type>
  using OnlyHasType = bool_constant<all_of({std::is_same<Type, Ts>::value...})>;

  template <typename Type>
  using IsUniqueInPack =
      bool_constant<count({std::is_same<Type, Ts>::value...}, true) == 1>;


  template <typename Type>
  static constexpr size_t IndexInPack() {
    size_t index = 0;
    for (bool value : {std::is_same<Type, Ts>::value...}) {
      if (value)
        return index;
      index++;
    }
    return pack_npos;
  }

  template <size_t N>
  using NthType = std::tuple_element_t<N, std::tuple<Ts...>>;

  using IsAllSameType =
      bool_constant<all_of({std::is_same<NthType<0>, Ts>::value...})>;
};

}  // namespace base

#endif  // BASE_PARAMETER_PACK_H_
