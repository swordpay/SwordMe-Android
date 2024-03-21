// Copyright 2022 The Abseil Authors.
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

#ifndef ABSL_CONTAINER_INTERNAL_COMMON_POLICY_TRAITS_H_
#define ABSL_CONTAINER_INTERNAL_COMMON_POLICY_TRAITS_H_

#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "absl/meta/type_traits.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {

template <class Policy, class = void>
struct common_policy_traits {

  using slot_type = typename Policy::slot_type;
  using reference = decltype(Policy::element(std::declval<slot_type*>()));
  using value_type = typename std::remove_reference<reference>::type;


  template <class Alloc, class... Args>
  static void construct(Alloc* alloc, slot_type* slot, Args&&... args) {
    Policy::construct(alloc, slot, std::forward<Args>(args)...);
  }


  template <class Alloc>
  static void destroy(Alloc* alloc, slot_type* slot) {
    Policy::destroy(alloc, slot);
  }











  template <class Alloc>
  static void transfer(Alloc* alloc, slot_type* new_slot, slot_type* old_slot) {
    transfer_impl(alloc, new_slot, old_slot, 0);
  }




  template <class P = Policy>
  static auto element(absl::remove_const_t<slot_type>* slot)
      -> decltype(P::element(slot)) {
    return P::element(slot);
  }
  template <class P = Policy>
  static auto element(const slot_type* slot) -> decltype(P::element(slot)) {
    return P::element(slot);
  }

 private:

  template <class Alloc, class P = Policy>
  static auto transfer_impl(Alloc* alloc, slot_type* new_slot,
                            slot_type* old_slot, int)
      -> decltype((void)P::transfer(alloc, new_slot, old_slot)) {
    P::transfer(alloc, new_slot, old_slot);
  }
  template <class Alloc>
  static void transfer_impl(Alloc* alloc, slot_type* new_slot,
                            slot_type* old_slot, char) {
#if defined(__cpp_lib_launder) && __cpp_lib_launder >= 201606
    if (absl::is_trivially_relocatable<value_type>()) {

      std::memcpy(static_cast<void*>(
                      std::launder(const_cast<std::remove_const_t<value_type>*>(
                          &element(new_slot)))),
                  static_cast<const void*>(&element(old_slot)),
                  sizeof(value_type));
      return;
    }
#endif

    construct(alloc, new_slot, std::move(element(old_slot)));
    destroy(alloc, old_slot);
  }
};

}  // namespace container_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_INTERNAL_COMMON_POLICY_TRAITS_H_
