// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINERS_FLAT_TREE_H_
#define BASE_CONTAINERS_FLAT_TREE_H_

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/stl_util.h"
#include "base/template_util.h"

namespace base {

namespace internal {

// ForwardIterator and thus supports multiple passes over a range.
template <class Iterator>
constexpr bool is_multipass() {
  return std::is_base_of<
      std::forward_iterator_tag,
      typename std::iterator_traits<Iterator>::iterator_category>::value;
}

template <typename T, typename = void>
struct IsTransparentCompare : std::false_type {};
template <typename T>
struct IsTransparentCompare<T, void_t<typename T::is_transparent>>
    : std::true_type {};


// use directly.
//
// The use of "value" in this is like std::map uses, meaning it's the thing
// contained (in the case of map it's a <Kay, Mapped> pair). The Key is how
// things are looked up. In the case of a set, Key == Value. In the case of
// a map, the Key is a component of a Value.
//
// The helper class GetKeyFromValue provides the means to extract a key from a
// value for comparison purposes. It should implement:
//   const Key& operator()(const Value&).
template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
class flat_tree {
 protected:
  using underlying_type = std::vector<Value>;

 public:



  using key_type = Key;
  using key_compare = KeyCompare;
  using value_type = Value;

  class value_compare : public key_compare {
   public:
    value_compare() = default;

    template <class Cmp>
    explicit value_compare(Cmp&& compare_arg)
        : KeyCompare(std::forward<Cmp>(compare_arg)) {}

    bool operator()(const value_type& left, const value_type& right) const {
      GetKeyFromValue extractor;
      return key_compare::operator()(extractor(left), extractor(right));
    }
  };

  using pointer = typename underlying_type::pointer;
  using const_pointer = typename underlying_type::const_pointer;
  using reference = typename underlying_type::reference;
  using const_reference = typename underlying_type::const_reference;
  using size_type = typename underlying_type::size_type;
  using difference_type = typename underlying_type::difference_type;
  using iterator = typename underlying_type::iterator;
  using const_iterator = typename underlying_type::const_iterator;
  using reverse_iterator = typename underlying_type::reverse_iterator;
  using const_reverse_iterator =
      typename underlying_type::const_reverse_iterator;












  flat_tree();
  explicit flat_tree(const key_compare& comp);

  template <class InputIterator>
  flat_tree(InputIterator first,
            InputIterator last,
            const key_compare& comp = key_compare());

  flat_tree(const flat_tree&);
  flat_tree(flat_tree&&) noexcept = default;

  flat_tree(const underlying_type& items,
            const key_compare& comp = key_compare());
  flat_tree(underlying_type&& items, const key_compare& comp = key_compare());

  flat_tree(std::initializer_list<value_type> ilist,
            const key_compare& comp = key_compare());

  ~flat_tree();





  flat_tree& operator=(const flat_tree&);
  flat_tree& operator=(flat_tree&&);

  flat_tree& operator=(std::initializer_list<value_type> ilist);









  void reserve(size_type new_capacity);
  size_type capacity() const;
  void shrink_to_fit();





  void clear();

  size_type size() const;
  size_type max_size() const;
  bool empty() const;



  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;

  iterator end();
  const_iterator end() const;
  const_iterator cend() const;

  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  const_reverse_iterator crbegin() const;

  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crend() const;










  std::pair<iterator, bool> insert(const value_type& val);
  std::pair<iterator, bool> insert(value_type&& val);

  iterator insert(const_iterator position_hint, const value_type& x);
  iterator insert(const_iterator position_hint, value_type&& x);


  template <class InputIterator>
  void insert(InputIterator first, InputIterator last);

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args);

  template <class... Args>
  iterator emplace_hint(const_iterator position_hint, Args&&... args);






  underlying_type extract() &&;


  void replace(underlying_type&& body);











  iterator erase(iterator position);
  iterator erase(const_iterator position);
  iterator erase(const_iterator first, const_iterator last);
  template <typename K>
  size_type erase(const K& key);



  key_compare key_comp() const;
  value_compare value_comp() const;





  template <typename K>
  size_type count(const K& key) const;

  template <typename K>
  iterator find(const K& key);

  template <typename K>
  const_iterator find(const K& key) const;

  template <typename K>
  bool contains(const K& key) const;

  template <typename K>
  std::pair<iterator, iterator> equal_range(const K& key);

  template <typename K>
  std::pair<const_iterator, const_iterator> equal_range(const K& key) const;

  template <typename K>
  iterator lower_bound(const K& key);

  template <typename K>
  const_iterator lower_bound(const K& key) const;

  template <typename K>
  iterator upper_bound(const K& key);

  template <typename K>
  const_iterator upper_bound(const K& key) const;











  void swap(flat_tree& other) noexcept;

  friend bool operator==(const flat_tree& lhs, const flat_tree& rhs) {
    return lhs.impl_.body_ == rhs.impl_.body_;
  }

  friend bool operator!=(const flat_tree& lhs, const flat_tree& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const flat_tree& lhs, const flat_tree& rhs) {
    return lhs.impl_.body_ < rhs.impl_.body_;
  }

  friend bool operator>(const flat_tree& lhs, const flat_tree& rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const flat_tree& lhs, const flat_tree& rhs) {
    return !(lhs < rhs);
  }

  friend bool operator<=(const flat_tree& lhs, const flat_tree& rhs) {
    return !(lhs > rhs);
  }

  friend void swap(flat_tree& lhs, flat_tree& rhs) noexcept { lhs.swap(rhs); }

 protected:


  template <class... Args>
  iterator unsafe_emplace(const_iterator position, Args&&... args);




  template <class K, class... Args>
  std::pair<iterator, bool> emplace_key_args(const K& key, Args&&... args);


  template <class K, class... Args>
  std::pair<iterator, bool> emplace_hint_key_args(const_iterator hint,
                                                  const K& key,
                                                  Args&&... args);

 private:


  struct KeyValueCompare {

    explicit KeyValueCompare(const key_compare& key_comp)
        : key_comp_(key_comp) {}

    template <typename T, typename U>
    bool operator()(const T& lhs, const U& rhs) const {
      return key_comp_(extract_if_value_type(lhs), extract_if_value_type(rhs));
    }

   private:
    const key_type& extract_if_value_type(const value_type& v) const {
      GetKeyFromValue extractor;
      return extractor(v);
    }

    template <typename K>
    const K& extract_if_value_type(const K& k) const {
      return k;
    }

    const key_compare& key_comp_;
  };

  iterator const_cast_it(const_iterator c_it) {
    auto distance = std::distance(cbegin(), c_it);
    return std::next(begin(), distance);
  }





  template <class V>
  std::pair<iterator, bool> insert_or_assign(V&& val) {
    auto position = lower_bound(GetKeyFromValue()(val));

    if (position == end() || value_comp()(val, *position))
      return {impl_.body_.emplace(position, std::forward<V>(val)), true};

    *position = std::forward<V>(val);
    return {position, false};
  }





  template <class V>
  std::pair<iterator, bool> append_or_assign(iterator first,
                                             iterator last,
                                             V&& val) {
    auto position = std::lower_bound(first, last, val, value_comp());

    if (position == last || value_comp()(val, *position)) {


      const difference_type distance = std::distance(begin(), position);
      impl_.body_.emplace_back(std::forward<V>(val));
      return {std::next(begin(), distance), true};
    }

    *position = std::forward<V>(val);
    return {position, false};
  }





  template <class V>
  std::pair<iterator, bool> append_unique(iterator first,
                                          iterator last,
                                          V&& val) {
    auto position = std::lower_bound(first, last, val, value_comp());

    if (position == last || value_comp()(val, *position)) {


      const difference_type distance = std::distance(begin(), position);
      impl_.body_.emplace_back(std::forward<V>(val));
      return {std::next(begin(), distance), true};
    }

    return {position, false};
  }

  void sort_and_unique(iterator first, iterator last) {

    std::stable_sort(first, last, value_comp());

    auto equal_comp = [this](const value_type& lhs, const value_type& rhs) {


      return !value_comp()(lhs, rhs);
    };

    erase(std::unique(first, last, equal_comp), last);
  }





  struct Impl : private value_compare {
    Impl() = default;

    template <class Cmp, class... Body>
    explicit Impl(Cmp&& compare_arg, Body&&... underlying_type_args)
        : value_compare(std::forward<Cmp>(compare_arg)),
          body_(std::forward<Body>(underlying_type_args)...) {}

    const value_compare& get_value_comp() const { return *this; }
    const key_compare& get_key_comp() const { return *this; }

    underlying_type body_;
  } impl_;

  template <typename K>
  using KeyTypeOrK = typename std::
      conditional<IsTransparentCompare<key_compare>::value, K, key_type>::type;
};

// Lifetime.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree() = default;

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree(
    const KeyCompare& comp)
    : impl_(comp) {}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class InputIterator>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree(
    InputIterator first,
    InputIterator last,
    const KeyCompare& comp)
    : impl_(comp, first, last) {
  sort_and_unique(begin(), end());
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree(
    const flat_tree&) = default;

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree(
    const underlying_type& items,
    const KeyCompare& comp)
    : impl_(comp, items) {
  sort_and_unique(begin(), end());
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree(
    underlying_type&& items,
    const KeyCompare& comp)
    : impl_(comp, std::move(items)) {
  sort_and_unique(begin(), end());
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::flat_tree(
    std::initializer_list<value_type> ilist,
    const KeyCompare& comp)
    : flat_tree(std::begin(ilist), std::end(ilist), comp) {}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::~flat_tree() = default;

// Assignments.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::operator=(
    const flat_tree&) -> flat_tree& = default;

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::operator=(flat_tree &&)
    -> flat_tree& = default;

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::operator=(
    std::initializer_list<value_type> ilist) -> flat_tree& {
  impl_.body_ = ilist;
  sort_and_unique(begin(), end());
  return *this;
}

// Memory management.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
void flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::reserve(
    size_type new_capacity) {
  impl_.body_.reserve(new_capacity);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::capacity() const
    -> size_type {
  return impl_.body_.capacity();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
void flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::shrink_to_fit() {
  impl_.body_.shrink_to_fit();
}

// Size management.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
void flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::clear() {
  impl_.body_.clear();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::size() const
    -> size_type {
  return impl_.body_.size();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::max_size() const
    -> size_type {
  return impl_.body_.max_size();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
bool flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::empty() const {
  return impl_.body_.empty();
}

// Iterators.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::begin() -> iterator {
  return impl_.body_.begin();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::begin() const
    -> const_iterator {
  return impl_.body_.begin();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::cbegin() const
    -> const_iterator {
  return impl_.body_.cbegin();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::end() -> iterator {
  return impl_.body_.end();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::end() const
    -> const_iterator {
  return impl_.body_.end();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::cend() const
    -> const_iterator {
  return impl_.body_.cend();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::rbegin()
    -> reverse_iterator {
  return impl_.body_.rbegin();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::rbegin() const
    -> const_reverse_iterator {
  return impl_.body_.rbegin();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::crbegin() const
    -> const_reverse_iterator {
  return impl_.body_.crbegin();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::rend()
    -> reverse_iterator {
  return impl_.body_.rend();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::rend() const
    -> const_reverse_iterator {
  return impl_.body_.rend();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::crend() const
    -> const_reverse_iterator {
  return impl_.body_.crend();
}

// Insert operations.
//
// Currently we use position_hint the same way as eastl or boost:
// https://github.com/electronicarts/EASTL/blob/master/include/EASTL/vector_set.h#L493

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::insert(
    const value_type& val) -> std::pair<iterator, bool> {
  return emplace_key_args(GetKeyFromValue()(val), val);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::insert(
    value_type&& val) -> std::pair<iterator, bool> {
  return emplace_key_args(GetKeyFromValue()(val), std::move(val));
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::insert(
    const_iterator position_hint,
    const value_type& val) -> iterator {
  return emplace_hint_key_args(position_hint, GetKeyFromValue()(val), val)
      .first;
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::insert(
    const_iterator position_hint,
    value_type&& val) -> iterator {
  return emplace_hint_key_args(position_hint, GetKeyFromValue()(val),
                               std::move(val))
      .first;
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class InputIterator>
void flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::insert(
    InputIterator first,
    InputIterator last) {
  if (first == last)
    return;


  if (is_multipass<InputIterator>() && std::next(first) == last) {
    insert(end(), *first);
    return;
  }


  auto middle = [this, size = size()] { return std::next(begin(), size); };

  difference_type pos_first_new = size();


  for (; first != last; ++first) {
    std::pair<iterator, bool> result = append_unique(begin(), middle(), *first);
    if (result.second) {
      pos_first_new =
          std::min(pos_first_new, std::distance(begin(), result.first));
    }
  }



  sort_and_unique(middle(), end());
  std::inplace_merge(std::next(begin(), pos_first_new), middle(), end(),
                     value_comp());
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class... Args>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::emplace(Args&&... args)
    -> std::pair<iterator, bool> {
  return insert(value_type(std::forward<Args>(args)...));
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class... Args>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::emplace_hint(
    const_iterator position_hint,
    Args&&... args) -> iterator {
  return insert(position_hint, value_type(std::forward<Args>(args)...));
}

// Underlying type operations.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::
    extract() && -> underlying_type {
  return std::exchange(impl_.body_, underlying_type());
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
void flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::replace(
    underlying_type&& body) {

  DCHECK(std::is_sorted(body.begin(), body.end(), value_comp()));
  DCHECK(std::adjacent_find(body.begin(), body.end(),
                            [this](const auto& lhs, const auto& rhs) {
                              return !value_comp()(lhs, rhs);
                            }) == body.end());
  impl_.body_ = std::move(body);
}

// Erase operations.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::erase(
    iterator position) -> iterator {
  CHECK(position != impl_.body_.end());
  return impl_.body_.erase(position);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::erase(
    const_iterator position) -> iterator {
  CHECK(position != impl_.body_.end());
  return impl_.body_.erase(position);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::erase(const K& val)
    -> size_type {
  auto eq_range = equal_range(val);
  auto res = std::distance(eq_range.first, eq_range.second);
  erase(eq_range.first, eq_range.second);
  return res;
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::erase(
    const_iterator first,
    const_iterator last) -> iterator {
  return impl_.body_.erase(first, last);
}

// Comparators.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::key_comp() const
    -> key_compare {
  return impl_.get_key_comp();
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::value_comp() const
    -> value_compare {
  return impl_.get_value_comp();
}

// Search operations.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::count(
    const K& key) const -> size_type {
  auto eq_range = equal_range(key);
  return std::distance(eq_range.first, eq_range.second);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::find(const K& key)
    -> iterator {
  return const_cast_it(base::as_const(*this).find(key));
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::find(
    const K& key) const -> const_iterator {
  auto eq_range = equal_range(key);
  return (eq_range.first == eq_range.second) ? end() : eq_range.first;
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
bool flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::contains(
    const K& key) const {
  auto lower = lower_bound(key);
  return lower != end() && !key_comp()(key, GetKeyFromValue()(*lower));
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::equal_range(
    const K& key) -> std::pair<iterator, iterator> {
  auto res = base::as_const(*this).equal_range(key);
  return {const_cast_it(res.first), const_cast_it(res.second)};
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::equal_range(
    const K& key) const -> std::pair<const_iterator, const_iterator> {
  auto lower = lower_bound(key);

  GetKeyFromValue extractor;
  if (lower == end() || impl_.get_key_comp()(key, extractor(*lower)))
    return {lower, lower};

  return {lower, std::next(lower)};
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::lower_bound(
    const K& key) -> iterator {
  return const_cast_it(base::as_const(*this).lower_bound(key));
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::lower_bound(
    const K& key) const -> const_iterator {
  static_assert(std::is_convertible<const KeyTypeOrK<K>&, const K&>::value,
                "Requested type cannot be bound to the container's key_type "
                "which is required for a non-transparent compare.");

  const KeyTypeOrK<K>& key_ref = key;

  KeyValueCompare key_value(impl_.get_key_comp());
  return std::lower_bound(begin(), end(), key_ref, key_value);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::upper_bound(
    const K& key) -> iterator {
  return const_cast_it(base::as_const(*this).upper_bound(key));
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <typename K>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::upper_bound(
    const K& key) const -> const_iterator {
  static_assert(std::is_convertible<const KeyTypeOrK<K>&, const K&>::value,
                "Requested type cannot be bound to the container's key_type "
                "which is required for a non-transparent compare.");

  const KeyTypeOrK<K>& key_ref = key;

  KeyValueCompare key_value(impl_.get_key_comp());
  return std::upper_bound(begin(), end(), key_ref, key_value);
}

// General operations.

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
void flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::swap(
    flat_tree& other) noexcept {
  std::swap(impl_, other.impl_);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class... Args>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::unsafe_emplace(
    const_iterator position,
    Args&&... args) -> iterator {
  return impl_.body_.emplace(position, std::forward<Args>(args)...);
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class K, class... Args>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::emplace_key_args(
    const K& key,
    Args&&... args) -> std::pair<iterator, bool> {
  auto lower = lower_bound(key);
  if (lower == end() || key_comp()(key, GetKeyFromValue()(*lower)))
    return {unsafe_emplace(lower, std::forward<Args>(args)...), true};
  return {lower, false};
}

template <class Key, class Value, class GetKeyFromValue, class KeyCompare>
template <class K, class... Args>
auto flat_tree<Key, Value, GetKeyFromValue, KeyCompare>::emplace_hint_key_args(
    const_iterator hint,
    const K& key,
    Args&&... args) -> std::pair<iterator, bool> {
  GetKeyFromValue extractor;
  if ((hint == begin() || key_comp()(extractor(*std::prev(hint)), key))) {
    if (hint == end() || key_comp()(key, extractor(*hint))) {

      return {unsafe_emplace(hint, std::forward<Args>(args)...), true};
    }
    if (!key_comp()(extractor(*hint), key)) {

      return {const_cast_it(hint), false};
    }
  }

  return emplace_key_args(key, std::forward<Args>(args)...);
}

// the GetKeyFromValue template parameter to flat_tree for this case.
template <class Key>
struct GetKeyFromValueIdentity {
  const Key& operator()(const Key& k) const { return k; }
};

}  // namespace internal

// Free functions.

template <class Key,
          class Value,
          class GetKeyFromValue,
          class KeyCompare,
          typename Predicate>
size_t EraseIf(
    base::internal::flat_tree<Key, Value, GetKeyFromValue, KeyCompare>&
        container,
    Predicate pred) {
  auto it = std::remove_if(container.begin(), container.end(), pred);
  size_t removed = std::distance(it, container.end());
  container.erase(it, container.end());
  return removed;
}

}  // namespace base

#endif  // BASE_CONTAINERS_FLAT_TREE_H_
