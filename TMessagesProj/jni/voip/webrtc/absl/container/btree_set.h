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
// File: btree_set.h
// -----------------------------------------------------------------------------
//
// This header file defines B-tree sets: sorted associative containers of
// values.
//
//     * `absl::btree_set<>`
//     * `absl::btree_multiset<>`
//
// These B-tree types are similar to the corresponding types in the STL
// (`std::set` and `std::multiset`) and generally conform to the STL interfaces
// of those types. However, because they are implemented using B-trees, they
// are more efficient in most situations.
//
// Unlike `std::set` and `std::multiset`, which are commonly implemented using
// red-black tree nodes, B-tree sets use more generic B-tree nodes able to hold
// multiple values per node. Holding multiple values per node often makes
// B-tree sets perform better than their `std::set` counterparts, because
// multiple entries can be checked within the same cache hit.
//
// However, these types should not be considered drop-in replacements for
// `std::set` and `std::multiset` as there are some API differences, which are
// noted in this header file. The most consequential differences with respect to
// migrating to b-tree from the STL types are listed in the next paragraph.
// Other API differences are minor.
//
// Importantly, insertions and deletions may invalidate outstanding iterators,
// pointers, and references to elements. Such invalidations are typically only
// an issue if insertion and deletion operations are interleaved with the use of
// more than one iterator, pointer, or reference simultaneously. For this
// reason, `insert()`, `erase()`, and `extract_and_get_next()` return a valid
// iterator at the current position.
//
// Another API difference is that btree iterators can be subtracted, and this
// is faster than using std::distance.

#ifndef ABSL_CONTAINER_BTREE_SET_H_
#define ABSL_CONTAINER_BTREE_SET_H_

#include "absl/container/internal/btree.h"  // IWYU pragma: export
#include "absl/container/internal/btree_container.h"  // IWYU pragma: export

namespace absl {
ABSL_NAMESPACE_BEGIN

namespace container_internal {

template <typename Key>
struct set_slot_policy;

template <typename Key, typename Compare, typename Alloc, int TargetNodeSize,
          bool IsMulti>
struct set_params;

}  // namespace container_internal

//
// An `absl::btree_set<K>` is an ordered associative container of unique key
// values designed to be a more efficient replacement for `std::set` (in most
// cases).
//
// Keys are sorted using an (optional) comparison function, which defaults to
// `std::less<K>`.
//
// An `absl::btree_set<K>` uses a default allocator of `std::allocator<K>` to
// allocate (and deallocate) nodes, and construct and destruct values within
// those nodes. You may instead specify a custom allocator `A` (which in turn
// requires specifying a custom comparator `C`) as in
// `absl::btree_set<K, C, A>`.
//
template <typename Key, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<Key>>
class btree_set
    : public container_internal::btree_set_container<
          container_internal::btree<container_internal::set_params<
              Key, Compare, Alloc, /*TargetNodeSize=*/256,
              /*IsMulti=*/false>>> {
  using Base = typename btree_set::btree_set_container;

 public:






































  btree_set() {}
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
// Swaps the contents of two `absl::btree_set` containers.
template <typename K, typename C, typename A>
void swap(btree_set<K, C, A> &x, btree_set<K, C, A> &y) {
  return x.swap(y);
}

//
// Erases all elements that satisfy the predicate pred from the container.
// Returns the number of erased elements.
template <typename K, typename C, typename A, typename Pred>
typename btree_set<K, C, A>::size_type erase_if(btree_set<K, C, A> &set,
                                                Pred pred) {
  return container_internal::btree_access::erase_if(set, std::move(pred));
}

//
// An `absl::btree_multiset<K>` is an ordered associative container of
// keys and associated values designed to be a more efficient replacement
// for `std::multiset` (in most cases). Unlike `absl::btree_set`, a B-tree
// multiset allows equivalent elements.
//
// Keys are sorted using an (optional) comparison function, which defaults to
// `std::less<K>`.
//
// An `absl::btree_multiset<K>` uses a default allocator of `std::allocator<K>`
// to allocate (and deallocate) nodes, and construct and destruct values within
// those nodes. You may instead specify a custom allocator `A` (which in turn
// requires specifying a custom comparator `C`) as in
// `absl::btree_multiset<K, C, A>`.
//
template <typename Key, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<Key>>
class btree_multiset
    : public container_internal::btree_multiset_container<
          container_internal::btree<container_internal::set_params<
              Key, Compare, Alloc, /*TargetNodeSize=*/256,
              /*IsMulti=*/true>>> {
  using Base = typename btree_multiset::btree_multiset_container;

 public:






































  btree_multiset() {}
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
// Swaps the contents of two `absl::btree_multiset` containers.
template <typename K, typename C, typename A>
void swap(btree_multiset<K, C, A> &x, btree_multiset<K, C, A> &y) {
  return x.swap(y);
}

//
// Erases all elements that satisfy the predicate pred from the container.
// Returns the number of erased elements.
template <typename K, typename C, typename A, typename Pred>
typename btree_multiset<K, C, A>::size_type erase_if(
   btree_multiset<K, C, A> & set, Pred pred) {
  return container_internal::btree_access::erase_if(set, std::move(pred));
}

namespace container_internal {

// absl::container_internal::slot_type interface for btree_(multi)set.
template <typename Key>
struct set_slot_policy {
  using slot_type = Key;
  using value_type = Key;
  using mutable_value_type = Key;

  static value_type &element(slot_type *slot) { return *slot; }
  static const value_type &element(const slot_type *slot) { return *slot; }

  template <typename Alloc, class... Args>
  static void construct(Alloc *alloc, slot_type *slot, Args &&...args) {
    absl::allocator_traits<Alloc>::construct(*alloc, slot,
                                             std::forward<Args>(args)...);
  }

  template <typename Alloc>
  static void construct(Alloc *alloc, slot_type *slot, slot_type *other) {
    absl::allocator_traits<Alloc>::construct(*alloc, slot, std::move(*other));
  }

  template <typename Alloc>
  static void construct(Alloc *alloc, slot_type *slot, const slot_type *other) {
    absl::allocator_traits<Alloc>::construct(*alloc, slot, *other);
  }

  template <typename Alloc>
  static void destroy(Alloc *alloc, slot_type *slot) {
    absl::allocator_traits<Alloc>::destroy(*alloc, slot);
  }
};

// Compare and Alloc should be nothrow copy-constructible.
template <typename Key, typename Compare, typename Alloc, int TargetNodeSize,
          bool IsMulti>
struct set_params : common_params<Key, Compare, Alloc, TargetNodeSize, IsMulti,
                                  /*IsMap=*/false, set_slot_policy<Key>> {
  using value_type = Key;
  using slot_type = typename set_params::common_params::slot_type;

  template <typename V>
  static const V &key(const V &value) {
    return value;
  }
  static const Key &key(const slot_type *slot) { return *slot; }
  static const Key &key(slot_type *slot) { return *slot; }
};

}  // namespace container_internal

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_BTREE_SET_H_
