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

#ifndef ABSL_CONTAINER_INTERNAL_HASH_POLICY_TRAITS_H_
#define ABSL_CONTAINER_INTERNAL_HASH_POLICY_TRAITS_H_

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "absl/container/internal/common_policy_traits.h"
#include "absl/meta/type_traits.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {

template <class Policy, class = void>
struct hash_policy_traits : common_policy_traits<Policy> {

  using key_type = typename Policy::key_type;

 private:
  struct ReturnKey {


#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606
    template <class Key,
              absl::enable_if_t<std::is_lvalue_reference<Key>::value, int> = 0>
    static key_type& Impl(Key&& k, int) {
      return *std::launder(
          const_cast<key_type*>(std::addressof(std::forward<Key>(k))));
    }
#endif

    template <class Key>
    static Key Impl(Key&& k, char) {
      return std::forward<Key>(k);
    }



    template <class Key, class... Args>
    auto operator()(Key&& k, const Args&...) const
        -> decltype(Impl(std::forward<Key>(k), 0)) {
      return Impl(std::forward<Key>(k), 0);
    }
  };

  template <class P = Policy, class = void>
  struct ConstantIteratorsImpl : std::false_type {};

  template <class P>
  struct ConstantIteratorsImpl<P, absl::void_t<typename P::constant_iterators>>
      : P::constant_iterators {};

 public:

  using slot_type = typename Policy::slot_type;



  using init_type = typename Policy::init_type;

  using reference = decltype(Policy::element(std::declval<slot_type*>()));
  using pointer = typename std::remove_reference<reference>::type*;
  using value_type = typename std::remove_reference<reference>::type;




  using constant_iterators = ConstantIteratorsImpl<>;






  template <class P = Policy>
  static size_t space_used(const slot_type* slot) {
    return P::space_used(slot);
  }





























  template <class F, class... Ts, class P = Policy>
  static auto apply(F&& f, Ts&&... ts)
      -> decltype(P::apply(std::forward<F>(f), std::forward<Ts>(ts)...)) {
    return P::apply(std::forward<F>(f), std::forward<Ts>(ts)...);
  }


  template <class P = Policy>
  static auto mutable_key(slot_type* slot)
      -> decltype(P::apply(ReturnKey(), hash_policy_traits::element(slot))) {
    return P::apply(ReturnKey(), hash_policy_traits::element(slot));
  }


  template <class T, class P = Policy>
  static auto value(T* elem) -> decltype(P::value(elem)) {
    return P::value(elem);
  }
};

}  // namespace container_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_INTERNAL_HASH_POLICY_TRAITS_H_
