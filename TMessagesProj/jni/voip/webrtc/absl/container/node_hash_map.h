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
// File: node_hash_map.h
// -----------------------------------------------------------------------------
//
// An `absl::node_hash_map<K, V>` is an unordered associative container of
// unique keys and associated values designed to be a more efficient replacement
// for `std::unordered_map`. Like `unordered_map`, search, insertion, and
// deletion of map elements can be done as an `O(1)` operation. However,
// `node_hash_map` (and other unordered associative containers known as the
// collection of Abseil "Swiss tables") contain other optimizations that result
// in both memory and computation advantages.
//
// In most cases, your default choice for a hash map should be a map of type
// `flat_hash_map`. However, if you need pointer stability and cannot store
// a `flat_hash_map` with `unique_ptr` elements, a `node_hash_map` may be a
// valid alternative. As well, if you are migrating your code from using
// `std::unordered_map`, a `node_hash_map` provides a more straightforward
// migration, because it guarantees pointer stability. Consider migrating to
// `node_hash_map` and perhaps converting to a more efficient `flat_hash_map`
// upon further review.

#ifndef ABSL_CONTAINER_NODE_HASH_MAP_H_
#define ABSL_CONTAINER_NODE_HASH_MAP_H_

#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/base/macros.h"
#include "absl/container/internal/container_memory.h"
#include "absl/container/internal/hash_function_defaults.h"  // IWYU pragma: export
#include "absl/container/internal/node_slot_policy.h"
#include "absl/container/internal/raw_hash_map.h"  // IWYU pragma: export
#include "absl/memory/memory.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {
template <class Key, class Value>
class NodeHashMapPolicy;
}  // namespace container_internal

// absl::node_hash_map
// -----------------------------------------------------------------------------
//
// An `absl::node_hash_map<K, V>` is an unordered associative container which
// has been optimized for both speed and memory footprint in most common use
// cases. Its interface is similar to that of `std::unordered_map<K, V>` with
// the following notable differences:
//
// * Supports heterogeneous lookup, through `find()`, `operator[]()` and
//   `insert()`, provided that the map is provided a compatible heterogeneous
//   hashing function and equality operator.
// * Contains a `capacity()` member function indicating the number of element
//   slots (open, deleted, and empty) within the hash map.
// * Returns `void` from the `erase(iterator)` overload.
//
// By default, `node_hash_map` uses the `absl::Hash` hashing framework.
// All fundamental and Abseil types that support the `absl::Hash` framework have
// a compatible equality operator for comparing insertions into `node_hash_map`.
// If your type is not yet supported by the `absl::Hash` framework, see
// absl/hash/hash.h for information on extending Abseil hashing to user-defined
// types.
//
// Using `absl::node_hash_map` at interface boundaries in dynamically loaded
// libraries (e.g. .dll, .so) is unsupported due to way `absl::Hash` values may
// be randomized across dynamically loaded libraries.
//
// Example:
//
//   // Create a node hash map of three strings (that map to strings)
//   absl::node_hash_map<std::string, std::string> ducks =
//     {{"a", "huey"}, {"b", "dewey"}, {"c", "louie"}};
//
//  // Insert a new element into the node hash map
//  ducks.insert({"d", "donald"}};
//
//  // Force a rehash of the node hash map
//  ducks.rehash(0);
//
//  // Find the element with the key "b"
//  std::string search_key = "b";
//  auto result = ducks.find(search_key);
//  if (result != ducks.end()) {
//    std::cout << "Result: " << result->second << std::endl;
//  }
template <class Key, class Value,
          class Hash = absl::container_internal::hash_default_hash<Key>,
          class Eq = absl::container_internal::hash_default_eq<Key>,
          class Alloc = std::allocator<std::pair<const Key, Value>>>
class node_hash_map
    : public absl::container_internal::raw_hash_map<
          absl::container_internal::NodeHashMapPolicy<Key, Value>, Hash, Eq,
          Alloc> {
  using Base = typename node_hash_map::raw_hash_map;

 public:








































  node_hash_map() {}
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
typename node_hash_map<K, V, H, E, A>::size_type erase_if(
    node_hash_map<K, V, H, E, A>& c, Predicate pred) {
  return container_internal::EraseIf(pred, &c);
}

namespace container_internal {

template <class Key, class Value>
class NodeHashMapPolicy
    : public absl::container_internal::node_slot_policy<
          std::pair<const Key, Value>&, NodeHashMapPolicy<Key, Value>> {
  using value_type = std::pair<const Key, Value>;

 public:
  using key_type = Key;
  using mapped_type = Value;
  using init_type = std::pair</*non const*/ key_type, mapped_type>;

  template <class Allocator, class... Args>
  static value_type* new_element(Allocator* alloc, Args&&... args) {
    using PairAlloc = typename absl::allocator_traits<
        Allocator>::template rebind_alloc<value_type>;
    PairAlloc pair_alloc(*alloc);
    value_type* res =
        absl::allocator_traits<PairAlloc>::allocate(pair_alloc, 1);
    absl::allocator_traits<PairAlloc>::construct(pair_alloc, res,
                                                 std::forward<Args>(args)...);
    return res;
  }

  template <class Allocator>
  static void delete_element(Allocator* alloc, value_type* pair) {
    using PairAlloc = typename absl::allocator_traits<
        Allocator>::template rebind_alloc<value_type>;
    PairAlloc pair_alloc(*alloc);
    absl::allocator_traits<PairAlloc>::destroy(pair_alloc, pair);
    absl::allocator_traits<PairAlloc>::deallocate(pair_alloc, pair, 1);
  }

  template <class F, class... Args>
  static decltype(absl::container_internal::DecomposePair(
      std::declval<F>(), std::declval<Args>()...))
  apply(F&& f, Args&&... args) {
    return absl::container_internal::DecomposePair(std::forward<F>(f),
                                                   std::forward<Args>(args)...);
  }

  static size_t element_space_used(const value_type*) {
    return sizeof(value_type);
  }

  static Value& value(value_type* elem) { return elem->second; }
  static const Value& value(const value_type* elem) { return elem->second; }
};
}  // namespace container_internal

namespace container_algorithm_internal {

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
struct IsUnorderedContainer<
    absl::node_hash_map<Key, T, Hash, KeyEqual, Allocator>> : std::true_type {};

}  // namespace container_algorithm_internal

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_NODE_HASH_MAP_H_
