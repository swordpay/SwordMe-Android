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
//
// -----------------------------------------------------------------------------
// File: flat_hash_set.h
// -----------------------------------------------------------------------------
//
// An `absl::flat_hash_set<T>` is an unordered associative container designed to
// be a more efficient replacement for `std::unordered_set`. Like
// `unordered_set`, search, insertion, and deletion of set elements can be done
// as an `O(1)` operation. However, `flat_hash_set` (and other unordered
// associative containers known as the collection of Abseil "Swiss tables")
// contain other optimizations that result in both memory and computation
// advantages.
//
// In most cases, your default choice for a hash set should be a set of type
// `flat_hash_set`.
#ifndef ABSL_CONTAINER_FLAT_HASH_SET_H_
#define ABSL_CONTAINER_FLAT_HASH_SET_H_

#include <type_traits>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/base/macros.h"
#include "absl/container/internal/container_memory.h"
#include "absl/container/internal/hash_function_defaults.h"  // IWYU pragma: export
#include "absl/container/internal/raw_hash_set.h"  // IWYU pragma: export
#include "absl/memory/memory.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {
template <typename T>
struct FlatHashSetPolicy;
}  // namespace container_internal

// absl::flat_hash_set
// -----------------------------------------------------------------------------
//
// An `absl::flat_hash_set<T>` is an unordered associative container which has
// been optimized for both speed and memory footprint in most common use cases.
// Its interface is similar to that of `std::unordered_set<T>` with the
// following notable differences:
//
// * Requires keys that are CopyConstructible
// * Supports heterogeneous lookup, through `find()` and `insert()`, provided
//   that the set is provided a compatible heterogeneous hashing function and
//   equality operator.
// * Invalidates any references and pointers to elements within the table after
//   `rehash()`.
// * Contains a `capacity()` member function indicating the number of element
//   slots (open, deleted, and empty) within the hash set.
// * Returns `void` from the `erase(iterator)` overload.
//
// By default, `flat_hash_set` uses the `absl::Hash` hashing framework. All
// fundamental and Abseil types that support the `absl::Hash` framework have a
// compatible equality operator for comparing insertions into `flat_hash_set`.
// If your type is not yet supported by the `absl::Hash` framework, see
// absl/hash/hash.h for information on extending Abseil hashing to user-defined
// types.
//
// Using `absl::flat_hash_set` at interface boundaries in dynamically loaded
// libraries (e.g. .dll, .so) is unsupported due to way `absl::Hash` values may
// be randomized across dynamically loaded libraries.
//
// NOTE: A `flat_hash_set` stores its keys directly inside its implementation
// array to avoid memory indirection. Because a `flat_hash_set` is designed to
// move data when rehashed, set keys will not retain pointer stability. If you
// require pointer stability, consider using
// `absl::flat_hash_set<std::unique_ptr<T>>`. If your type is not moveable and
// you require pointer stability, consider `absl::node_hash_set` instead.
//
// Example:
//
//   // Create a flat hash set of three strings
//   absl::flat_hash_set<std::string> ducks =
//     {"huey", "dewey", "louie"};
//
//  // Insert a new element into the flat hash set
//  ducks.insert("donald");
//
//  // Force a rehash of the flat hash set
//  ducks.rehash(0);
//
//  // See if "dewey" is present
//  if (ducks.contains("dewey")) {
//    std::cout << "We found dewey!" << std::endl;
//  }
template <class T, class Hash = absl::container_internal::hash_default_hash<T>,
          class Eq = absl::container_internal::hash_default_eq<T>,
          class Allocator = std::allocator<T>>
class flat_hash_set
    : public absl::container_internal::raw_hash_set<
          absl::container_internal::FlatHashSetPolicy<T>, Hash, Eq, Allocator> {
  using Base = typename flat_hash_set::raw_hash_set;

 public:








































  flat_hash_set() {}
  using Base::Base;



  using Base::begin;



  using Base::cbegin;



  using Base::cend;



  using Base::end;







  using Base::capacity;



  using Base::empty;






  using Base::max_size;



  using Base::size;







  using Base::clear;


























  using Base::erase;












































  using Base::insert;











  using Base::emplace;












  using Base::emplace_hint;
















  using Base::extract;





  using Base::merge;















  using Base::swap;












  using Base::rehash;





  using Base::reserve;




  using Base::contains;





  using Base::count;





  using Base::equal_range;



  using Base::find;





  using Base::bucket_count;




  using Base::load_factor;
















  using Base::max_load_factor;



  using Base::get_allocator;




  using Base::hash_function;



  using Base::key_eq;
};

//
// Erases all elements that satisfy the predicate `pred` from the container `c`.
// Returns the number of erased elements.
template <typename T, typename H, typename E, typename A, typename Predicate>
typename flat_hash_set<T, H, E, A>::size_type erase_if(
    flat_hash_set<T, H, E, A>& c, Predicate pred) {
  return container_internal::EraseIf(pred, &c);
}

namespace container_internal {

template <class T>
struct FlatHashSetPolicy {
  using slot_type = T;
  using key_type = T;
  using init_type = T;
  using constant_iterators = std::true_type;

  template <class Allocator, class... Args>
  static void construct(Allocator* alloc, slot_type* slot, Args&&... args) {
    absl::allocator_traits<Allocator>::construct(*alloc, slot,
                                                 std::forward<Args>(args)...);
  }

  template <class Allocator>
  static void destroy(Allocator* alloc, slot_type* slot) {
    absl::allocator_traits<Allocator>::destroy(*alloc, slot);
  }

  static T& element(slot_type* slot) { return *slot; }

  template <class F, class... Args>
  static decltype(absl::container_internal::DecomposeValue(
      std::declval<F>(), std::declval<Args>()...))
  apply(F&& f, Args&&... args) {
    return absl::container_internal::DecomposeValue(
        std::forward<F>(f), std::forward<Args>(args)...);
  }

  static size_t space_used(const T*) { return 0; }
};
}  // namespace container_internal

namespace container_algorithm_internal {

template <class Key, class Hash, class KeyEqual, class Allocator>
struct IsUnorderedContainer<absl::flat_hash_set<Key, Hash, KeyEqual, Allocator>>
    : std::true_type {};

}  // namespace container_algorithm_internal

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_FLAT_HASH_SET_H_
