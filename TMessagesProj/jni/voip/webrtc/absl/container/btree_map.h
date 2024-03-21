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
// File: btree_map.h
// -----------------------------------------------------------------------------
//
// This header file defines B-tree maps: sorted associative containers mapping
// keys to values.
//
//     * `absl::btree_map<>`
//     * `absl::btree_multimap<>`
//
// These B-tree types are similar to the corresponding types in the STL
// (`std::map` and `std::multimap`) and generally conform to the STL interfaces
// of those types. However, because they are implemented using B-trees, they
// are more efficient in most situations.
//
// Unlike `std::map` and `std::multimap`, which are commonly implemented using
// red-black tree nodes, B-tree maps use more generic B-tree nodes able to hold
// multiple values per node. Holding multiple values per node often makes
// B-tree maps perform better than their `std::map` counterparts, because
// multiple entries can be checked within the same cache hit.
//
// However, these types should not be considered drop-in replacements for
// `std::map` and `std::multimap` as there are some API differences, which are
// noted in this header file. The most consequential differences with respect to
// migrating to b-tree from the STL types are listed in the next paragraph.
// Other API differences are minor.
//
// Importantly, insertions and deletions may invalidate outstanding iterators,
// pointers, and references to elements. Such invalidations are typically only
// an issue if insertion and deletion operations are interleaved with the use of
// more than one iterator, pointer, or reference simultaneously.  For this
// reason, `insert()`, `erase()`, and `extract_and_get_next()` return a valid
// iterator at the current position. Another important difference is that
// key-types must be copy-constructible.
//
// Another API difference is that btree iterators can be subtracted, and this
// is faster than using std::distance.

#ifndef ABSL_CONTAINER_BTREE_MAP_H_
#define ABSL_CONTAINER_BTREE_MAP_H_

#include "absl/container/internal/btree.h"  // IWYU pragma: export
#include "absl/container/internal/btree_container.h"  // IWYU pragma: export

namespace absl {
ABSL_NAMESPACE_BEGIN

namespace container_internal {

template <typename Key, typename Data, typename Compare, typename Alloc,
          int TargetNodeSize, bool IsMulti>
struct map_params;

}  // namespace container_internal

//
// An `absl::btree_map<K, V>` is an ordered associative container of
// unique keys and associated values designed to be a more efficient replacement
// for `std::map` (in most cases).
//
// Keys are sorted using an (optional) comparison function, which defaults to
// `std::less<K>`.
//
// An `absl::btree_map<K, V>` uses a default allocator of
// `std::allocator<std::pair<const K, V>>` to allocate (and deallocate)
// nodes, and construct and destruct values within those nodes. You may
// instead specify a custom allocator `A` (which in turn requires specifying a
// custom comparator `C`) as in `absl::btree_map<K, V, C, A>`.
//
template <typename Key, typename Value, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class btree_map
    : public container_internal::btree_map_container<
          container_internal::btree<container_internal::map_params<
              Key, Value, Compare, Alloc, /*TargetNodeSize=*/256,
              /*IsMulti=*/false>>> {
  using Base = typename btree_map::btree_map_container;

 public:






































  btree_map() {}
  using Base::Base;



  using Base::begin;



  using Base::cbegin;



  using Base::end;



  using Base::cend;



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













  using Base::extract_and_get_next;





  using Base::merge;








  using Base::swap;




  using Base::at;









  using Base::contains;










  using Base::count;




  using Base::equal_range;









  using Base::find;










  using Base::lower_bound;










  using Base::upper_bound;















  using Base::operator[];



  using Base::get_allocator;



  using Base::key_comp;



  using Base::value_comp;
};

//
// Swaps the contents of two `absl::btree_map` containers.
template <typename K, typename V, typename C, typename A>
void swap(btree_map<K, V, C, A> &x, btree_map<K, V, C, A> &y) {
  return x.swap(y);
}

//
// Erases all elements that satisfy the predicate pred from the container.
// Returns the number of erased elements.
template <typename K, typename V, typename C, typename A, typename Pred>
typename btree_map<K, V, C, A>::size_type erase_if(
    btree_map<K, V, C, A> &map, Pred pred) {
  return container_internal::btree_access::erase_if(map, std::move(pred));
}

//
// An `absl::btree_multimap<K, V>` is an ordered associative container of
// keys and associated values designed to be a more efficient replacement for
// `std::multimap` (in most cases). Unlike `absl::btree_map`, a B-tree multimap
// allows multiple elements with equivalent keys.
//
// Keys are sorted using an (optional) comparison function, which defaults to
// `std::less<K>`.
//
// An `absl::btree_multimap<K, V>` uses a default allocator of
// `std::allocator<std::pair<const K, V>>` to allocate (and deallocate)
// nodes, and construct and destruct values within those nodes. You may
// instead specify a custom allocator `A` (which in turn requires specifying a
// custom comparator `C`) as in `absl::btree_multimap<K, V, C, A>`.
//
template <typename Key, typename Value, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class btree_multimap
    : public container_internal::btree_multimap_container<
          container_internal::btree<container_internal::map_params<
              Key, Value, Compare, Alloc, /*TargetNodeSize=*/256,
              /*IsMulti=*/true>>> {
  using Base = typename btree_multimap::btree_multimap_container;

 public:






































  btree_multimap() {}
  using Base::Base;



  using Base::begin;



  using Base::cbegin;



  using Base::end;



  using Base::cend;



  using Base::empty;






  using Base::max_size;



  using Base::size;




  using Base::clear;























  using Base::erase;































  using Base::insert;





  using Base::emplace;







  using Base::emplace_hint;


























  using Base::extract;













  using Base::extract_and_get_next;




  using Base::merge;








  using Base::swap;









  using Base::contains;









  using Base::count;





  using Base::equal_range;









  using Base::find;










  using Base::lower_bound;










  using Base::upper_bound;



  using Base::get_allocator;



  using Base::key_comp;



  using Base::value_comp;
};

//
// Swaps the contents of two `absl::btree_multimap` containers.
template <typename K, typename V, typename C, typename A>
void swap(btree_multimap<K, V, C, A> &x, btree_multimap<K, V, C, A> &y) {
  return x.swap(y);
}

//
// Erases all elements that satisfy the predicate pred from the container.
// Returns the number of erased elements.
template <typename K, typename V, typename C, typename A, typename Pred>
typename btree_multimap<K, V, C, A>::size_type erase_if(
    btree_multimap<K, V, C, A> &map, Pred pred) {
  return container_internal::btree_access::erase_if(map, std::move(pred));
}

namespace container_internal {

// Compare and Alloc should be nothrow copy-constructible.
template <typename Key, typename Data, typename Compare, typename Alloc,
          int TargetNodeSize, bool IsMulti>
struct map_params : common_params<Key, Compare, Alloc, TargetNodeSize, IsMulti,
                                  /*IsMap=*/true, map_slot_policy<Key, Data>> {
  using super_type = typename map_params::common_params;
  using mapped_type = Data;


  using slot_policy = typename super_type::slot_policy;
  using slot_type = typename super_type::slot_type;
  using value_type = typename super_type::value_type;
  using init_type = typename super_type::init_type;

  template <typename V>
  static auto key(const V &value) -> decltype(value.first) {
    return value.first;
  }
  static const Key &key(const slot_type *s) { return slot_policy::key(s); }
  static const Key &key(slot_type *s) { return slot_policy::key(s); }

  static auto mutable_key(slot_type *s)
      -> decltype(slot_policy::mutable_key(s)) {
    return slot_policy::mutable_key(s);
  }
  static mapped_type &value(value_type *value) { return value->second; }
};

}  // namespace container_internal

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_BTREE_MAP_H_
