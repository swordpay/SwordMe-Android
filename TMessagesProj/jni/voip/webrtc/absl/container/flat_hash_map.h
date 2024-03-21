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
// File: flat_hash_map.h
// -----------------------------------------------------------------------------
//
// An `absl::flat_hash_map<K, V>` is an unordered associative container of
// unique keys and associated values designed to be a more efficient replacement
// for `std::unordered_map`. Like `unordered_map`, search, insertion, and
// deletion of map elements can be done as an `O(1)` operation. However,
// `flat_hash_map` (and other unordered associative containers known as the
// collection of Abseil "Swiss tables") contain other optimizations that result
// in both memory and computation advantages.
//
// In most cases, your default choice for a hash map should be a map of type
// `flat_hash_map`.

#ifndef ABSL_CONTAINER_FLAT_HASH_MAP_H_
#define ABSL_CONTAINER_FLAT_HASH_MAP_H_

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/base/macros.h"
#include "absl/container/internal/container_memory.h"
#include "absl/container/internal/hash_function_defaults.h"  // IWYU pragma: export
#include "absl/container/internal/raw_hash_map.h"  // IWYU pragma: export
#include "absl/memory/memory.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {
template <class K, class V>
struct FlatHashMapPolicy;
}  // namespace container_internal

// absl::flat_hash_map
// -----------------------------------------------------------------------------
//
// An `absl::flat_hash_map<K, V>` is an unordered associative container which
// has been optimized for both speed and memory footprint in most common use
// cases. Its interface is similar to that of `std::unordered_map<K, V>` with
// the following notable differences:
//
// * Requires keys that are CopyConstructible
// * Requires values that are MoveConstructible
// * Supports heterogeneous lookup, through `find()`, `operator[]()` and
//   `insert()`, provided that the map is provided a compatible heterogeneous
//   hashing function and equality operator.
// * Invalidates any references and pointers to elements within the table after
//   `rehash()`.
// * Contains a `capacity()` member function indicating the number of element
//   slots (open, deleted, and empty) within the hash map.
// * Returns `void` from the `erase(iterator)` overload.
//
// By default, `flat_hash_map` uses the `absl::Hash` hashing framework.
// All fundamental and Abseil types that support the `absl::Hash` framework have
// a compatible equality operator for comparing insertions into `flat_hash_map`.
// If your type is not yet supported by the `absl::Hash` framework, see
// absl/hash/hash.h for information on extending Abseil hashing to user-defined
// types.
//
// Using `absl::flat_hash_map` at interface boundaries in dynamically loaded
// libraries (e.g. .dll, .so) is unsupported due to way `absl::Hash` values may
// be randomized across dynamically loaded libraries.
//
// NOTE: A `flat_hash_map` stores its value types directly inside its
// implementation array to avoid memory indirection. Because a `flat_hash_map`
// is designed to move data when rehashed, map values will not retain pointer
// stability. If you require pointer stability, or if your values are large,
// consider using `absl::flat_hash_map<Key, std::unique_ptr<Value>>` instead.
// If your types are not moveable or you require pointer stability for keys,
// consider `absl::node_hash_map`.
//
// Example:
//
//   // Create a flat hash map of three strings (that map to strings)
//   absl::flat_hash_map<std::string, std::string> ducks =
//     {{"a", "huey"}, {"b", "dewey"}, {"c", "louie"}};
//
//  // Insert a new element into the flat hash map
//  ducks.insert({"d", "donald"});
//
//  // Force a rehash of the flat hash map
//  ducks.rehash(0);
//
//  // Find the element with the key "b"
//  std::string search_key = "b";
//  auto result = ducks.find(search_key);
//  if (result != ducks.end()) {
//    std::cout << "Result: " << result->second << std::endl;
//  }
template <class K, class V,
          class Hash = absl::container_internal::hash_default_hash<K>,
          class Eq = absl::container_internal::hash_default_eq<K>,
          class Allocator = std::allocator<std::pair<const K, V>>>
class flat_hash_map : public absl::container_internal::raw_hash_map<
                          absl::container_internal::FlatHashMapPolicy<K, V>,
                          Hash, Eq, Allocator> {
  using Base = typename flat_hash_map::raw_hash_map;

 public:








































  flat_hash_map() {}
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






















  using Base::insert_or_assign;












  using Base::emplace;













  using Base::emplace_hint;



























  using Base::try_emplace;





















  using Base::extract;





  using Base::merge;















  using Base::swap;












  using Base::rehash;





  using Base::reserve;




  using Base::at;





  using Base::contains;






  using Base::count;





  using Base::equal_range;



  using Base::find;



















  using Base::operator[];





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
template <typename K, typename V, typename H, typename E, typename A,
          typename Predicate>
typename flat_hash_map<K, V, H, E, A>::size_type erase_if(
    flat_hash_map<K, V, H, E, A>& c, Predicate pred) {
  return container_internal::EraseIf(pred, &c);
}

namespace container_internal {

template <class K, class V>
struct FlatHashMapPolicy {
  using slot_policy = container_internal::map_slot_policy<K, V>;
  using slot_type = typename slot_policy::slot_type;
  using key_type = K;
  using mapped_type = V;
  using init_type = std::pair</*non const*/ key_type, mapped_type>;

  template <class Allocator, class... Args>
  static void construct(Allocator* alloc, slot_type* slot, Args&&... args) {
    slot_policy::construct(alloc, slot, std::forward<Args>(args)...);
  }

  template <class Allocator>
  static void destroy(Allocator* alloc, slot_type* slot) {
    slot_policy::destroy(alloc, slot);
  }

  template <class Allocator>
  static void transfer(Allocator* alloc, slot_type* new_slot,
                       slot_type* old_slot) {
    slot_policy::transfer(alloc, new_slot, old_slot);
  }

  template <class F, class... Args>
  static decltype(absl::container_internal::DecomposePair(
      std::declval<F>(), std::declval<Args>()...))
  apply(F&& f, Args&&... args) {
    return absl::container_internal::DecomposePair(std::forward<F>(f),
                                                   std::forward<Args>(args)...);
  }

  static size_t space_used(const slot_type*) { return 0; }

  static std::pair<const K, V>& element(slot_type* slot) { return slot->value; }

  static V& value(std::pair<const K, V>* kv) { return kv->second; }
  static const V& value(const std::pair<const K, V>* kv) { return kv->second; }
};

}  // namespace container_internal

namespace container_algorithm_internal {

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
struct IsUnorderedContainer<
    absl::flat_hash_map<Key, T, Hash, KeyEqual, Allocator>> : std::true_type {};

}  // namespace container_algorithm_internal

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_FLAT_HASH_MAP_H_
