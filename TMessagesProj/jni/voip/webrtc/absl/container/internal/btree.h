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

// and generally also faster than STL set/map (refer to the benchmarks below).
// The red-black tree implementation of STL set/map has an overhead of 3
// pointers (left, right and parent) plus the node color information for each
// stored value. So a set<int32_t> consumes 40 bytes for each value stored in
// 64-bit mode. This btree implementation stores multiple values on fixed
// size nodes (usually 256 bytes) and doesn't store child pointers for leaf
// nodes. The result is that a btree_set<int32_t> may use much less memory per
// stored value. For the random insertion benchmark in btree_bench.cc, a
// btree_set<int32_t> with node-size of 256 uses 5.1 bytes per stored value.
//
// The packing of multiple values on to each node of a btree has another effect
// besides better space utilization: better cache locality due to fewer cache
// lines being accessed. Better cache locality translates into faster
// operations.
//
// CAVEATS
//
// Insertions and deletions on a btree can cause splitting, merging or
// rebalancing of btree nodes. And even without these operations, insertions
// and deletions on a btree will move values around within a node. In both
// cases, the result is that insertions and deletions can invalidate iterators
// pointing to values other than the one being inserted/deleted. Therefore, this
// container does not provide pointer stability. This is notably different from
// STL set/map which takes care to not invalidate iterators on insert/erase
// except, of course, for iterators pointing to the value being erased.  A
// partial workaround when erasing is available: erase() returns an iterator
// pointing to the item just after the one that was erased (or end() if none
// exists).

#ifndef ABSL_CONTAINER_INTERNAL_BTREE_H_
#define ABSL_CONTAINER_INTERNAL_BTREE_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/base/internal/raw_logging.h"
#include "absl/base/macros.h"
#include "absl/container/internal/common.h"
#include "absl/container/internal/common_policy_traits.h"
#include "absl/container/internal/compressed_tuple.h"
#include "absl/container/internal/container_memory.h"
#include "absl/container/internal/layout.h"
#include "absl/memory/memory.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "absl/types/compare.h"
#include "absl/utility/utility.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {

#ifdef ABSL_BTREE_ENABLE_GENERATIONS
#error ABSL_BTREE_ENABLE_GENERATIONS cannot be directly set
#elif defined(ABSL_HAVE_ADDRESS_SANITIZER) || \
    defined(ABSL_HAVE_MEMORY_SANITIZER)
// When compiled in sanitizer mode, we add generation integers to the nodes and
// iterators. When iterators are used, we validate that the container has not
// been mutated since the iterator was constructed.
#define ABSL_BTREE_ENABLE_GENERATIONS
#endif

template <typename Compare, typename T, typename U>
using compare_result_t = absl::result_of_t<const Compare(const T &, const U &)>;

// comparator.
template <typename Compare, typename T>
using btree_is_key_compare_to =
    std::is_convertible<compare_result_t<Compare, T, T>, absl::weak_ordering>;

struct StringBtreeDefaultLess {
  using is_transparent = void;

  StringBtreeDefaultLess() = default;

  StringBtreeDefaultLess(std::less<std::string>) {}        // NOLINT
  StringBtreeDefaultLess(std::less<absl::string_view>) {}  // NOLINT

  explicit operator std::less<std::string>() const { return {}; }
  explicit operator std::less<absl::string_view>() const { return {}; }
  explicit operator std::less<absl::Cord>() const { return {}; }

  absl::weak_ordering operator()(absl::string_view lhs,
                                 absl::string_view rhs) const {
    return compare_internal::compare_result_as_ordering(lhs.compare(rhs));
  }
  StringBtreeDefaultLess(std::less<absl::Cord>) {}  // NOLINT
  absl::weak_ordering operator()(const absl::Cord &lhs,
                                 const absl::Cord &rhs) const {
    return compare_internal::compare_result_as_ordering(lhs.Compare(rhs));
  }
  absl::weak_ordering operator()(const absl::Cord &lhs,
                                 absl::string_view rhs) const {
    return compare_internal::compare_result_as_ordering(lhs.Compare(rhs));
  }
  absl::weak_ordering operator()(absl::string_view lhs,
                                 const absl::Cord &rhs) const {
    return compare_internal::compare_result_as_ordering(-rhs.Compare(lhs));
  }
};

struct StringBtreeDefaultGreater {
  using is_transparent = void;

  StringBtreeDefaultGreater() = default;

  StringBtreeDefaultGreater(std::greater<std::string>) {}        // NOLINT
  StringBtreeDefaultGreater(std::greater<absl::string_view>) {}  // NOLINT

  explicit operator std::greater<std::string>() const { return {}; }
  explicit operator std::greater<absl::string_view>() const { return {}; }
  explicit operator std::greater<absl::Cord>() const { return {}; }

  absl::weak_ordering operator()(absl::string_view lhs,
                                 absl::string_view rhs) const {
    return compare_internal::compare_result_as_ordering(rhs.compare(lhs));
  }
  StringBtreeDefaultGreater(std::greater<absl::Cord>) {}  // NOLINT
  absl::weak_ordering operator()(const absl::Cord &lhs,
                                 const absl::Cord &rhs) const {
    return compare_internal::compare_result_as_ordering(rhs.Compare(lhs));
  }
  absl::weak_ordering operator()(const absl::Cord &lhs,
                                 absl::string_view rhs) const {
    return compare_internal::compare_result_as_ordering(-lhs.Compare(rhs));
  }
  absl::weak_ordering operator()(absl::string_view lhs,
                                 const absl::Cord &rhs) const {
    return compare_internal::compare_result_as_ordering(rhs.Compare(lhs));
  }
};

template <typename Compare, bool is_class = std::is_class<Compare>::value>
struct checked_compare_base : Compare {
  using Compare::Compare;
  explicit checked_compare_base(Compare c) : Compare(std::move(c)) {}
  const Compare &comp() const { return *this; }
};
template <typename Compare>
struct checked_compare_base<Compare, false> {
  explicit checked_compare_base(Compare c) : compare(std::move(c)) {}
  const Compare &comp() const { return compare; }
  Compare compare;
};

struct BtreeTestOnlyCheckedCompareOptOutBase {};

// (1) When using common Abseil string types with common comparison functors,
// convert a boolean comparison into a three-way comparison that returns an
// `absl::weak_ordering`. This helper class is specialized for
// less<std::string>, greater<std::string>, less<string_view>,
// greater<string_view>, less<absl::Cord>, and greater<absl::Cord>.
// (2) Adapt the comparator to diagnose cases of non-strict-weak-ordering (see
// https://en.cppreference.com/w/cpp/named_req/Compare) in debug mode. Whenever
// a comparison is made, we will make assertions to verify that the comparator
// is valid.
template <typename Compare, typename Key>
struct key_compare_adapter {







  struct checked_compare : checked_compare_base<Compare> {
   private:
    using Base = typename checked_compare::checked_compare_base;
    using Base::comp;




    bool is_self_equivalent(const Key &k) const {

      return comp()(k, k) == 0;
    }

    template <typename T>
    bool is_self_equivalent(const T &) const {
      return true;
    }

   public:
    using Base::Base;
    checked_compare(Compare comp) : Base(std::move(comp)) {}  // NOLINT

    explicit operator Compare() const { return comp(); }

    template <typename T, typename U,
              absl::enable_if_t<
                  std::is_same<bool, compare_result_t<Compare, T, U>>::value,
                  int> = 0>
    bool operator()(const T &lhs, const U &rhs) const {



      assert(is_self_equivalent(lhs));
      assert(is_self_equivalent(rhs));
      const bool lhs_comp_rhs = comp()(lhs, rhs);
      assert(!lhs_comp_rhs || !comp()(rhs, lhs));
      return lhs_comp_rhs;
    }

    template <
        typename T, typename U,
        absl::enable_if_t<std::is_convertible<compare_result_t<Compare, T, U>,
                                              absl::weak_ordering>::value,
                          int> = 0>
    absl::weak_ordering operator()(const T &lhs, const U &rhs) const {



      assert(is_self_equivalent(lhs));
      assert(is_self_equivalent(rhs));
      const absl::weak_ordering lhs_comp_rhs = comp()(lhs, rhs);
#ifndef NDEBUG
      const absl::weak_ordering rhs_comp_lhs = comp()(rhs, lhs);
      if (lhs_comp_rhs > 0) {
        assert(rhs_comp_lhs < 0 && "lhs_comp_rhs > 0 -> rhs_comp_lhs < 0");
      } else if (lhs_comp_rhs == 0) {
        assert(rhs_comp_lhs == 0 && "lhs_comp_rhs == 0 -> rhs_comp_lhs == 0");
      } else {
        assert(rhs_comp_lhs > 0 && "lhs_comp_rhs < 0 -> rhs_comp_lhs > 0");
      }
#endif
      return lhs_comp_rhs;
    }
  };
  using type = absl::conditional_t<
      std::is_base_of<BtreeTestOnlyCheckedCompareOptOutBase, Compare>::value,
      Compare, checked_compare>;
};

template <>
struct key_compare_adapter<std::less<std::string>, std::string> {
  using type = StringBtreeDefaultLess;
};

template <>
struct key_compare_adapter<std::greater<std::string>, std::string> {
  using type = StringBtreeDefaultGreater;
};

template <>
struct key_compare_adapter<std::less<absl::string_view>, absl::string_view> {
  using type = StringBtreeDefaultLess;
};

template <>
struct key_compare_adapter<std::greater<absl::string_view>, absl::string_view> {
  using type = StringBtreeDefaultGreater;
};

template <>
struct key_compare_adapter<std::less<absl::Cord>, absl::Cord> {
  using type = StringBtreeDefaultLess;
};

template <>
struct key_compare_adapter<std::greater<absl::Cord>, absl::Cord> {
  using type = StringBtreeDefaultGreater;
};

// a protocol used as an opt-in or opt-out of linear search.
//
//  For example, this would be useful for key types that wrap an integer
//  and define their own cheap operator<(). For example:
//
//   class K {
//    public:
//     using absl_btree_prefer_linear_node_search = std::true_type;
//     ...
//    private:
//     friend bool operator<(K a, K b) { return a.k_ < b.k_; }
//     int k_;
//   };
//
//   btree_map<K, V> m;  // Uses linear search
//
// If T has the preference tag, then it has a preference.
// Btree will use the tag's truth value.
template <typename T, typename = void>
struct has_linear_node_search_preference : std::false_type {};
template <typename T, typename = void>
struct prefers_linear_node_search : std::false_type {};
template <typename T>
struct has_linear_node_search_preference<
    T, absl::void_t<typename T::absl_btree_prefer_linear_node_search>>
    : std::true_type {};
template <typename T>
struct prefers_linear_node_search<
    T, absl::void_t<typename T::absl_btree_prefer_linear_node_search>>
    : T::absl_btree_prefer_linear_node_search {};

template <typename Compare, typename Key>
constexpr bool compare_has_valid_result_type() {
  using compare_result_type = compare_result_t<Compare, Key, Key>;
  return std::is_same<compare_result_type, bool>::value ||
         std::is_convertible<compare_result_type, absl::weak_ordering>::value;
}

template <typename original_key_compare, typename value_type>
class map_value_compare {
  template <typename Params>
  friend class btree;


 protected:
  explicit map_value_compare(original_key_compare c) : comp(std::move(c)) {}

  original_key_compare comp;  // NOLINT

 public:
  auto operator()(const value_type &lhs, const value_type &rhs) const
      -> decltype(comp(lhs.first, rhs.first)) {
    return comp(lhs.first, rhs.first);
  }
};

template <typename Key, typename Compare, typename Alloc, int TargetNodeSize,
          bool IsMulti, bool IsMap, typename SlotPolicy>
struct common_params : common_policy_traits<SlotPolicy> {
  using original_key_compare = Compare;








  using key_compare =
      absl::conditional_t<!compare_has_valid_result_type<Compare, Key>(),
                          Compare,
                          typename key_compare_adapter<Compare, Key>::type>;

  static constexpr bool kIsKeyCompareStringAdapted =
      std::is_same<key_compare, StringBtreeDefaultLess>::value ||
      std::is_same<key_compare, StringBtreeDefaultGreater>::value;
  static constexpr bool kIsKeyCompareTransparent =
      IsTransparent<original_key_compare>::value || kIsKeyCompareStringAdapted;
  static constexpr bool kEnableGenerations =
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
      true;
#else
      false;
#endif


  using is_key_compare_to = btree_is_key_compare_to<key_compare, Key>;

  using allocator_type = Alloc;
  using key_type = Key;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  using slot_policy = SlotPolicy;
  using slot_type = typename slot_policy::slot_type;
  using value_type = typename slot_policy::value_type;
  using init_type = typename slot_policy::mutable_value_type;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  using value_compare =
      absl::conditional_t<IsMap,
                          map_value_compare<original_key_compare, value_type>,
                          original_key_compare>;
  using is_map_container = std::integral_constant<bool, IsMap>;








  template <typename LookupKey>
  constexpr static bool can_have_multiple_equivalent_keys() {
    return IsMulti || (IsTransparent<key_compare>::value &&
                       !std::is_same<LookupKey, Key>::value &&
                       !kIsKeyCompareStringAdapted);
  }

  enum {
    kTargetNodeSize = TargetNodeSize,



    kNodeSlotSpace = TargetNodeSize - /*minimum overhead=*/(sizeof(void *) + 4),
  };


  using node_count_type =
      absl::conditional_t<(kNodeSlotSpace / sizeof(slot_type) >
                           (std::numeric_limits<uint8_t>::max)()),
                          uint16_t, uint8_t>;  // NOLINT
};

// compare. Note: there is no need to make a version of this adapter specialized
// for key-compare-to functors because the upper-bound (the first value greater
// than the input) is never an exact match.
template <typename Compare>
struct upper_bound_adapter {
  explicit upper_bound_adapter(const Compare &c) : comp(c) {}
  template <typename K1, typename K2>
  bool operator()(const K1 &a, const K2 &b) const {

    return !compare_internal::compare_result_as_less_than(comp(b, a));
  }

 private:
  Compare comp;
};

enum class MatchKind : uint8_t { kEq, kNe };

template <typename V, bool IsCompareTo>
struct SearchResult {
  V value;
  MatchKind match;

  static constexpr bool HasMatch() { return true; }
  bool IsEq() const { return match == MatchKind::kEq; }
};

// This ensures that callers can't use it accidentally when it provides no
// useful information.
template <typename V>
struct SearchResult<V, false> {
  SearchResult() {}
  explicit SearchResult(V v) : value(v) {}
  SearchResult(V v, MatchKind /*match*/) : value(v) {}

  V value;

  static constexpr bool HasMatch() { return false; }
  static constexpr bool IsEq() { return false; }
};

// and leaf nodes in the btree, though the nodes are allocated in such a way
// that the children array is only valid in internal nodes.
template <typename Params>
class btree_node {
  using is_key_compare_to = typename Params::is_key_compare_to;
  using field_type = typename Params::node_count_type;
  using allocator_type = typename Params::allocator_type;
  using slot_type = typename Params::slot_type;
  using original_key_compare = typename Params::original_key_compare;

 public:
  using params_type = Params;
  using key_type = typename Params::key_type;
  using value_type = typename Params::value_type;
  using pointer = typename Params::pointer;
  using const_pointer = typename Params::const_pointer;
  using reference = typename Params::reference;
  using const_reference = typename Params::const_reference;
  using key_compare = typename Params::key_compare;
  using size_type = typename Params::size_type;
  using difference_type = typename Params::difference_type;







  using use_linear_search = std::integral_constant<
      bool, has_linear_node_search_preference<original_key_compare>::value
                ? prefers_linear_node_search<original_key_compare>::value
            : has_linear_node_search_preference<key_type>::value
                ? prefers_linear_node_search<key_type>::value
                : std::is_arithmetic<key_type>::value &&
                      (std::is_same<std::less<key_type>,
                                    original_key_compare>::value ||
                       std::is_same<std::greater<key_type>,
                                    original_key_compare>::value)>;











































  ~btree_node() = default;
  btree_node(btree_node const &) = delete;
  btree_node &operator=(btree_node const &) = delete;

  constexpr static size_type Alignment() {
    static_assert(LeafLayout(1).Alignment() == InternalLayout().Alignment(),
                  "Alignment of all nodes must be equal.");
    return InternalLayout().Alignment();
  }

 protected:
  btree_node() = default;

 private:
  using layout_type =
      absl::container_internal::Layout<btree_node *, uint32_t, field_type,
                                       slot_type, btree_node *>;
  constexpr static size_type SizeWithNSlots(size_type n) {
    return layout_type(
               /*parent*/ 1,
               /*generation*/ params_type::kEnableGenerations ? 1 : 0,
               /*position, start, finish, max_count*/ 4,
               /*slots*/ n,
               /*children*/ 0)
        .AllocSize();
  }

  constexpr static size_type MinimumOverhead() {
    return SizeWithNSlots(1) - sizeof(slot_type);
  }


  constexpr static size_type NodeTargetSlots(const size_type begin,
                                             const size_type end) {
    return begin == end ? begin
           : SizeWithNSlots((begin + end) / 2 + 1) >
                   params_type::kTargetNodeSize
               ? NodeTargetSlots(begin, (begin + end) / 2)
               : NodeTargetSlots((begin + end) / 2 + 1, end);
  }

  constexpr static size_type kTargetNodeSize = params_type::kTargetNodeSize;
  constexpr static size_type kNodeTargetSlots =
      NodeTargetSlots(0, kTargetNodeSize);





  constexpr static size_type kMinNodeSlots = 4;

  constexpr static size_type kNodeSlots =
      kNodeTargetSlots >= kMinNodeSlots ? kNodeTargetSlots : kMinNodeSlots;


  constexpr static field_type kInternalNodeMaxCount = 0;

  constexpr static layout_type LeafLayout(
      const size_type slot_count = kNodeSlots) {
    return layout_type(
        /*parent*/ 1,
        /*generation*/ params_type::kEnableGenerations ? 1 : 0,
        /*position, start, finish, max_count*/ 4,
        /*slots*/ slot_count,
        /*children*/ 0);
  }
  constexpr static layout_type InternalLayout() {
    return layout_type(
        /*parent*/ 1,
        /*generation*/ params_type::kEnableGenerations ? 1 : 0,
        /*position, start, finish, max_count*/ 4,
        /*slots*/ kNodeSlots,
        /*children*/ kNodeSlots + 1);
  }
  constexpr static size_type LeafSize(const size_type slot_count = kNodeSlots) {
    return LeafLayout(slot_count).AllocSize();
  }
  constexpr static size_type InternalSize() {
    return InternalLayout().AllocSize();
  }


  template <size_type N>
  inline typename layout_type::template ElementType<N> *GetField() {

    assert(N < 4 || is_internal());
    return InternalLayout().template Pointer<N>(reinterpret_cast<char *>(this));
  }
  template <size_type N>
  inline const typename layout_type::template ElementType<N> *GetField() const {
    assert(N < 4 || is_internal());
    return InternalLayout().template Pointer<N>(
        reinterpret_cast<const char *>(this));
  }
  void set_parent(btree_node *p) { *GetField<0>() = p; }
  field_type &mutable_finish() { return GetField<2>()[2]; }
  slot_type *slot(size_type i) { return &GetField<3>()[i]; }
  slot_type *start_slot() { return slot(start()); }
  slot_type *finish_slot() { return slot(finish()); }
  const slot_type *slot(size_type i) const { return &GetField<3>()[i]; }
  void set_position(field_type v) { GetField<2>()[0] = v; }
  void set_start(field_type v) { GetField<2>()[1] = v; }
  void set_finish(field_type v) { GetField<2>()[2] = v; }

  void set_max_count(field_type v) { GetField<2>()[3] = v; }

 public:


  bool is_leaf() const { return GetField<2>()[3] != kInternalNodeMaxCount; }


  bool is_internal() const { return !is_leaf(); }

  field_type position() const { return GetField<2>()[0]; }

  field_type start() const {

    assert(GetField<2>()[1] == 0);
    return 0;
  }

  field_type finish() const { return GetField<2>()[2]; }

  field_type count() const {
    assert(finish() >= start());
    return finish() - start();
  }
  field_type max_count() const {


    const field_type max_count = GetField<2>()[3];
    return max_count == field_type{kInternalNodeMaxCount}
               ? field_type{kNodeSlots}
               : max_count;
  }

  btree_node *parent() const { return *GetField<0>(); }



  bool is_root() const { return parent()->is_leaf(); }
  void make_root() {
    assert(parent()->is_root());
    set_generation(parent()->generation());
    set_parent(parent()->parent());
  }

  uint32_t *get_root_generation() const {
    assert(params_type::kEnableGenerations);
    const btree_node *curr = this;
    for (; !curr->is_root(); curr = curr->parent()) continue;
    return const_cast<uint32_t *>(&curr->GetField<1>()[0]);
  }

  uint32_t generation() const {
    return params_type::kEnableGenerations ? *get_root_generation() : 0;
  }


  void set_generation(uint32_t generation) {
    if (params_type::kEnableGenerations) GetField<1>()[0] = generation;
  }

  void next_generation() {
    if (params_type::kEnableGenerations) ++*get_root_generation();
  }

  const key_type &key(size_type i) const { return params_type::key(slot(i)); }
  reference value(size_type i) { return params_type::element(slot(i)); }
  const_reference value(size_type i) const {
    return params_type::element(slot(i));
  }

  btree_node *child(field_type i) const { return GetField<4>()[i]; }
  btree_node *start_child() const { return child(start()); }
  btree_node *&mutable_child(field_type i) { return GetField<4>()[i]; }
  void clear_child(field_type i) {
    absl::container_internal::SanitizerPoisonObject(&mutable_child(i));
  }
  void set_child(field_type i, btree_node *c) {
    absl::container_internal::SanitizerUnpoisonObject(&mutable_child(i));
    mutable_child(i) = c;
    c->set_position(i);
  }
  void init_child(field_type i, btree_node *c) {
    set_child(i, c);
    c->set_parent(this);
  }

  template <typename K>
  SearchResult<size_type, is_key_compare_to::value> lower_bound(
      const K &k, const key_compare &comp) const {
    return use_linear_search::value ? linear_search(k, comp)
                                    : binary_search(k, comp);
  }

  template <typename K>
  size_type upper_bound(const K &k, const key_compare &comp) const {
    auto upper_compare = upper_bound_adapter<key_compare>(comp);
    return use_linear_search::value ? linear_search(k, upper_compare).value
                                    : binary_search(k, upper_compare).value;
  }

  template <typename K, typename Compare>
  SearchResult<size_type, btree_is_key_compare_to<Compare, key_type>::value>
  linear_search(const K &k, const Compare &comp) const {
    return linear_search_impl(k, start(), finish(), comp,
                              btree_is_key_compare_to<Compare, key_type>());
  }

  template <typename K, typename Compare>
  SearchResult<size_type, btree_is_key_compare_to<Compare, key_type>::value>
  binary_search(const K &k, const Compare &comp) const {
    return binary_search_impl(k, start(), finish(), comp,
                              btree_is_key_compare_to<Compare, key_type>());
  }


  template <typename K, typename Compare>
  SearchResult<size_type, false> linear_search_impl(
      const K &k, size_type s, const size_type e, const Compare &comp,
      std::false_type /* IsCompareTo */) const {
    while (s < e) {
      if (!comp(key(s), k)) {
        break;
      }
      ++s;
    }
    return SearchResult<size_type, false>{s};
  }


  template <typename K, typename Compare>
  SearchResult<size_type, true> linear_search_impl(
      const K &k, size_type s, const size_type e, const Compare &comp,
      std::true_type /* IsCompareTo */) const {
    while (s < e) {
      const absl::weak_ordering c = comp(key(s), k);
      if (c == 0) {
        return {s, MatchKind::kEq};
      } else if (c > 0) {
        break;
      }
      ++s;
    }
    return {s, MatchKind::kNe};
  }


  template <typename K, typename Compare>
  SearchResult<size_type, false> binary_search_impl(
      const K &k, size_type s, size_type e, const Compare &comp,
      std::false_type /* IsCompareTo */) const {
    while (s != e) {
      const size_type mid = (s + e) >> 1;
      if (comp(key(mid), k)) {
        s = mid + 1;
      } else {
        e = mid;
      }
    }
    return SearchResult<size_type, false>{s};
  }


  template <typename K, typename CompareTo>
  SearchResult<size_type, true> binary_search_impl(
      const K &k, size_type s, size_type e, const CompareTo &comp,
      std::true_type /* IsCompareTo */) const {
    if (params_type::template can_have_multiple_equivalent_keys<K>()) {
      MatchKind exact_match = MatchKind::kNe;
      while (s != e) {
        const size_type mid = (s + e) >> 1;
        const absl::weak_ordering c = comp(key(mid), k);
        if (c < 0) {
          s = mid + 1;
        } else {
          e = mid;
          if (c == 0) {



            exact_match = MatchKind::kEq;
          }
        }
      }
      return {s, exact_match};
    } else {  // Can't have multiple equivalent keys.
      while (s != e) {
        const size_type mid = (s + e) >> 1;
        const absl::weak_ordering c = comp(key(mid), k);
        if (c < 0) {
          s = mid + 1;
        } else if (c > 0) {
          e = mid;
        } else {
          return {mid, MatchKind::kEq};
        }
      }
      return {s, MatchKind::kNe};
    }
  }


  template <typename... Args>
  void emplace_value(field_type i, allocator_type *alloc, Args &&...args);



  void remove_values(field_type i, field_type to_erase, allocator_type *alloc);

  void rebalance_right_to_left(field_type to_move, btree_node *right,
                               allocator_type *alloc);
  void rebalance_left_to_right(field_type to_move, btree_node *right,
                               allocator_type *alloc);

  void split(int insert_position, btree_node *dest, allocator_type *alloc);


  void merge(btree_node *src, allocator_type *alloc);

  void init_leaf(field_type max_count, btree_node *parent) {
    set_generation(0);
    set_parent(parent);
    set_position(0);
    set_start(0);
    set_finish(0);
    set_max_count(max_count);
    absl::container_internal::SanitizerPoisonMemoryRegion(
        start_slot(), max_count * sizeof(slot_type));
  }
  void init_internal(btree_node *parent) {
    init_leaf(kNodeSlots, parent);


    set_max_count(kInternalNodeMaxCount);
    absl::container_internal::SanitizerPoisonMemoryRegion(
        &mutable_child(start()), (kNodeSlots + 1) * sizeof(btree_node *));
  }

  static void deallocate(const size_type size, btree_node *node,
                         allocator_type *alloc) {
    absl::container_internal::SanitizerUnpoisonMemoryRegion(node, size);
    absl::container_internal::Deallocate<Alignment()>(alloc, node, size);
  }

  static void clear_and_delete(btree_node *node, allocator_type *alloc);

 private:
  template <typename... Args>
  void value_init(const field_type i, allocator_type *alloc, Args &&...args) {
    next_generation();
    absl::container_internal::SanitizerUnpoisonObject(slot(i));
    params_type::construct(alloc, slot(i), std::forward<Args>(args)...);
  }
  void value_destroy(const field_type i, allocator_type *alloc) {
    next_generation();
    params_type::destroy(alloc, slot(i));
    absl::container_internal::SanitizerPoisonObject(slot(i));
  }
  void value_destroy_n(const field_type i, const field_type n,
                       allocator_type *alloc) {
    next_generation();
    for (slot_type *s = slot(i), *end = slot(i + n); s != end; ++s) {
      params_type::destroy(alloc, s);
      absl::container_internal::SanitizerPoisonObject(s);
    }
  }

  static void transfer(slot_type *dest, slot_type *src, allocator_type *alloc) {
    absl::container_internal::SanitizerUnpoisonObject(dest);
    params_type::transfer(alloc, dest, src);
    absl::container_internal::SanitizerPoisonObject(src);
  }

  void transfer(const size_type dest_i, const size_type src_i,
                btree_node *src_node, allocator_type *alloc) {
    next_generation();
    transfer(slot(dest_i), src_node->slot(src_i), alloc);
  }


  void transfer_n(const size_type n, const size_type dest_i,
                  const size_type src_i, btree_node *src_node,
                  allocator_type *alloc) {
    next_generation();
    for (slot_type *src = src_node->slot(src_i), *end = src + n,
                   *dest = slot(dest_i);
         src != end; ++src, ++dest) {
      transfer(dest, src, alloc);
    }
  }


  void transfer_n_backward(const size_type n, const size_type dest_i,
                           const size_type src_i, btree_node *src_node,
                           allocator_type *alloc) {
    next_generation();
    for (slot_type *src = src_node->slot(src_i + n), *end = src - n,
                   *dest = slot(dest_i + n);
         src != end; --src, --dest) {





      transfer(dest - 1, src - 1, alloc);
    }
  }

  template <typename P>
  friend class btree;
  template <typename N, typename R, typename P>
  friend class btree_iterator;
  friend class BtreeNodePeer;
  friend struct btree_access;
};

template <typename Node>
bool AreNodesFromSameContainer(const Node *node_a, const Node *node_b) {



  if (node_a == nullptr || node_b == nullptr) return true;
  while (!node_a->is_root()) node_a = node_a->parent();
  while (!node_b->is_root()) node_b = node_b->parent();
  return node_a == node_b;
}

template <typename Node, typename Reference, typename Pointer>
class btree_iterator {
  using field_type = typename Node::field_type;
  using key_type = typename Node::key_type;
  using size_type = typename Node::size_type;
  using params_type = typename Node::params_type;
  using is_map_container = typename params_type::is_map_container;

  using node_type = Node;
  using normal_node = typename std::remove_const<Node>::type;
  using const_node = const Node;
  using normal_pointer = typename params_type::pointer;
  using normal_reference = typename params_type::reference;
  using const_pointer = typename params_type::const_pointer;
  using const_reference = typename params_type::const_reference;
  using slot_type = typename params_type::slot_type;

  using iterator =
     btree_iterator<normal_node, normal_reference, normal_pointer>;
  using const_iterator =
      btree_iterator<const_node, const_reference, const_pointer>;

 public:

  using difference_type = typename Node::difference_type;
  using value_type = typename params_type::value_type;
  using pointer = Pointer;
  using reference = Reference;
  using iterator_category = std::bidirectional_iterator_tag;

  btree_iterator() : btree_iterator(nullptr, -1) {}
  explicit btree_iterator(Node *n) : btree_iterator(n, n->start()) {}
  btree_iterator(Node *n, int p) : node_(n), position_(p) {
#ifdef ABSL_BTREE_ENABLE_GENERATIONS


    generation_ = n != nullptr ? n->generation() : ~uint32_t{};
#endif
  }



  template <typename N, typename R, typename P,
            absl::enable_if_t<
                std::is_same<btree_iterator<N, R, P>, iterator>::value &&
                    std::is_same<btree_iterator, const_iterator>::value,
                int> = 0>
  btree_iterator(const btree_iterator<N, R, P> other)  // NOLINT
      : node_(other.node_), position_(other.position_) {
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
    generation_ = other.generation_;
#endif
  }

  bool operator==(const iterator &other) const {
    return Equals(other.node_, other.position_);
  }
  bool operator==(const const_iterator &other) const {
    return Equals(other.node_, other.position_);
  }
  bool operator!=(const iterator &other) const {
    return !Equals(other.node_, other.position_);
  }
  bool operator!=(const const_iterator &other) const {
    return !Equals(other.node_, other.position_);
  }


  difference_type operator-(const_iterator other) const {
    if (node_ == other.node_) {
      if (node_->is_leaf()) return position_ - other.position_;
      if (position_ == other.position_) return 0;
    }
    return distance_slow(other);
  }

  reference operator*() const {
    ABSL_HARDENING_ASSERT(node_ != nullptr);
    assert_valid_generation();
    ABSL_HARDENING_ASSERT(position_ >= node_->start());
    if (position_ >= node_->finish()) {
      ABSL_HARDENING_ASSERT(!IsEndIterator() && "Dereferencing end() iterator");
      ABSL_HARDENING_ASSERT(position_ < node_->finish());
    }
    return node_->value(static_cast<field_type>(position_));
  }
  pointer operator->() const { return &operator*(); }

  btree_iterator &operator++() {
    increment();
    return *this;
  }
  btree_iterator &operator--() {
    decrement();
    return *this;
  }
  btree_iterator operator++(int) {
    btree_iterator tmp = *this;
    ++*this;
    return tmp;
  }
  btree_iterator operator--(int) {
    btree_iterator tmp = *this;
    --*this;
    return tmp;
  }

 private:
  friend iterator;
  friend const_iterator;
  template <typename Params>
  friend class btree;
  template <typename Tree>
  friend class btree_container;
  template <typename Tree>
  friend class btree_set_container;
  template <typename Tree>
  friend class btree_map_container;
  template <typename Tree>
  friend class btree_multiset_container;
  template <typename TreeType, typename CheckerType>
  friend class base_checker;
  friend struct btree_access;




  template <typename N, typename R, typename P,
            absl::enable_if_t<
                std::is_same<btree_iterator<N, R, P>, const_iterator>::value &&
                    std::is_same<btree_iterator, iterator>::value,
                int> = 0>
  explicit btree_iterator(const btree_iterator<N, R, P> other)
      : node_(const_cast<node_type *>(other.node_)),
        position_(other.position_) {
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
    generation_ = other.generation_;
#endif
  }

  bool Equals(const node_type *other_node, int other_position) const {
    ABSL_HARDENING_ASSERT(((node_ == nullptr && other_node == nullptr) ||
                           (node_ != nullptr && other_node != nullptr)) &&
                          "Comparing default-constructed iterator with "
                          "non-default-constructed iterator.");



    assert(AreNodesFromSameContainer(node_, other_node) &&
           "Comparing iterators from different containers.");
    return node_ == other_node && position_ == other_position;
  }

  bool IsEndIterator() const {
    if (position_ != node_->finish()) return false;
    node_type *node = node_;
    while (!node->is_root()) {
      if (node->position() != node->parent()->finish()) return false;
      node = node->parent();
    }
    return true;
  }



  difference_type distance_slow(const_iterator other) const;

  void increment() {
    assert_valid_generation();
    if (node_->is_leaf() && ++position_ < node_->finish()) {
      return;
    }
    increment_slow();
  }
  void increment_slow();

  void decrement() {
    assert_valid_generation();
    if (node_->is_leaf() && --position_ >= node_->start()) {
      return;
    }
    decrement_slow();
  }
  void decrement_slow();


  void update_generation() {
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
    if (node_ != nullptr) generation_ = node_->generation();
#endif
  }

  const key_type &key() const {
    return node_->key(static_cast<size_type>(position_));
  }
  decltype(std::declval<Node *>()->slot(0)) slot() {
    return node_->slot(static_cast<size_type>(position_));
  }

  void assert_valid_generation() const {
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
    if (node_ != nullptr && node_->generation() != generation_) {
      ABSL_INTERNAL_LOG(
          FATAL,
          "Attempting to use an invalidated iterator. The corresponding b-tree "
          "container has been mutated since this iterator was constructed.");
    }
#endif
  }

  Node *node_;



  int position_;
#ifdef ABSL_BTREE_ENABLE_GENERATIONS

  uint32_t generation_;
#endif
};

template <typename Params>
class btree {
  using node_type = btree_node<Params>;
  using is_key_compare_to = typename Params::is_key_compare_to;
  using field_type = typename node_type::field_type;


  struct alignas(node_type::Alignment()) EmptyNodeType : node_type {
    using field_type = typename node_type::field_type;
    node_type *parent;
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
    uint32_t generation = 0;
#endif
    field_type position = 0;
    field_type start = 0;
    field_type finish = 0;


    field_type max_count = node_type::kInternalNodeMaxCount + 1;

#ifdef _MSC_VER

    EmptyNodeType() : parent(this) {}
#else
    explicit constexpr EmptyNodeType(node_type *p) : parent(p) {}
#endif
  };

  static node_type *EmptyNode() {
#ifdef _MSC_VER
    static EmptyNodeType *empty_node = new EmptyNodeType;

    assert(empty_node->parent == empty_node);
    return empty_node;
#else
    static constexpr EmptyNodeType empty_node(
        const_cast<EmptyNodeType *>(&empty_node));
    return const_cast<EmptyNodeType *>(&empty_node);
#endif
  }

  enum : uint32_t {
    kNodeSlots = node_type::kNodeSlots,
    kMinNodeValues = kNodeSlots / 2,
  };

  struct node_stats {
    using size_type = typename Params::size_type;

    node_stats(size_type l, size_type i) : leaf_nodes(l), internal_nodes(i) {}

    node_stats &operator+=(const node_stats &other) {
      leaf_nodes += other.leaf_nodes;
      internal_nodes += other.internal_nodes;
      return *this;
    }

    size_type leaf_nodes;
    size_type internal_nodes;
  };

 public:
  using key_type = typename Params::key_type;
  using value_type = typename Params::value_type;
  using size_type = typename Params::size_type;
  using difference_type = typename Params::difference_type;
  using key_compare = typename Params::key_compare;
  using original_key_compare = typename Params::original_key_compare;
  using value_compare = typename Params::value_compare;
  using allocator_type = typename Params::allocator_type;
  using reference = typename Params::reference;
  using const_reference = typename Params::const_reference;
  using pointer = typename Params::pointer;
  using const_pointer = typename Params::const_pointer;
  using iterator =
      typename btree_iterator<node_type, reference, pointer>::iterator;
  using const_iterator = typename iterator::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using node_handle_type = node_handle<Params, Params, allocator_type>;

  using params_type = Params;
  using slot_type = typename Params::slot_type;

 private:




  template <typename Btree>
  void copy_or_move_values_in_order(Btree &other);

  constexpr static bool static_assert_validation();

 public:
  btree(const key_compare &comp, const allocator_type &alloc)
      : root_(EmptyNode()), rightmost_(comp, alloc, EmptyNode()), size_(0) {}

  btree(const btree &other) : btree(other, other.allocator()) {}
  btree(const btree &other, const allocator_type &alloc)
      : btree(other.key_comp(), alloc) {
    copy_or_move_values_in_order(other);
  }
  btree(btree &&other) noexcept
      : root_(absl::exchange(other.root_, EmptyNode())),
        rightmost_(std::move(other.rightmost_)),
        size_(absl::exchange(other.size_, 0u)) {
    other.mutable_rightmost() = EmptyNode();
  }
  btree(btree &&other, const allocator_type &alloc)
      : btree(other.key_comp(), alloc) {
    if (alloc == other.allocator()) {
      swap(other);
    } else {

      copy_or_move_values_in_order(other);
    }
  }

  ~btree() {


    static_assert(static_assert_validation(), "This call must be elided.");
    clear();
  }

  btree &operator=(const btree &other);
  btree &operator=(btree &&other) noexcept;

  iterator begin() { return iterator(leftmost()); }
  const_iterator begin() const { return const_iterator(leftmost()); }
  iterator end() { return iterator(rightmost(), rightmost()->finish()); }
  const_iterator end() const {
    return const_iterator(rightmost(), rightmost()->finish());
  }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  template <typename K>
  iterator lower_bound(const K &key) {
    return internal_end(internal_lower_bound(key).value);
  }
  template <typename K>
  const_iterator lower_bound(const K &key) const {
    return internal_end(internal_lower_bound(key).value);
  }


  template <typename K>
  std::pair<iterator, bool> lower_bound_equal(const K &key) const;

  template <typename K>
  iterator upper_bound(const K &key) {
    return internal_end(internal_upper_bound(key));
  }
  template <typename K>
  const_iterator upper_bound(const K &key) const {
    return internal_end(internal_upper_bound(key));
  }



  template <typename K>
  std::pair<iterator, iterator> equal_range(const K &key);
  template <typename K>
  std::pair<const_iterator, const_iterator> equal_range(const K &key) const {
    return const_cast<btree *>(this)->equal_range(key);
  }




  template <typename K, typename... Args>
  std::pair<iterator, bool> insert_unique(const K &key, Args &&...args);






  template <typename K, typename... Args>
  std::pair<iterator, bool> insert_hint_unique(iterator position, const K &key,
                                               Args &&...args);



  template <typename InputIterator,
            typename = decltype(std::declval<const key_compare &>()(
                params_type::key(*std::declval<InputIterator>()),
                std::declval<const key_type &>()))>
  void insert_iterator_unique(InputIterator b, InputIterator e, int);


  template <typename InputIterator>
  void insert_iterator_unique(InputIterator b, InputIterator e, char);

  template <typename ValueType>
  iterator insert_multi(const key_type &key, ValueType &&v);

  template <typename ValueType>
  iterator insert_multi(ValueType &&v) {
    return insert_multi(params_type::key(v), std::forward<ValueType>(v));
  }




  template <typename ValueType>
  iterator insert_hint_multi(iterator position, ValueType &&v);

  template <typename InputIterator>
  void insert_iterator_multi(InputIterator b, InputIterator e);




  iterator erase(iterator iter);


  std::pair<size_type, iterator> erase_range(iterator begin, iterator end);


  template <typename K>
  iterator find(const K &key) {
    return internal_end(internal_find(key));
  }
  template <typename K>
  const_iterator find(const K &key) const {
    return internal_end(internal_find(key));
  }

  void clear();

  void swap(btree &other);

  const key_compare &key_comp() const noexcept {
    return rightmost_.template get<0>();
  }
  template <typename K1, typename K2>
  bool compare_keys(const K1 &a, const K2 &b) const {
    return compare_internal::compare_result_as_less_than(key_comp()(a, b));
  }

  value_compare value_comp() const {
    return value_compare(original_key_compare(key_comp()));
  }

  void verify() const;

  size_type size() const { return size_; }
  size_type max_size() const { return (std::numeric_limits<size_type>::max)(); }
  bool empty() const { return size_ == 0; }

  size_type height() const {
    size_type h = 0;
    if (!empty()) {




      const node_type *n = root();
      do {
        ++h;
        n = n->parent();
      } while (n != root());
    }
    return h;
  }

  size_type leaf_nodes() const { return internal_stats(root()).leaf_nodes; }
  size_type internal_nodes() const {
    return internal_stats(root()).internal_nodes;
  }
  size_type nodes() const {
    node_stats stats = internal_stats(root());
    return stats.leaf_nodes + stats.internal_nodes;
  }


  size_type bytes_used() const {
    node_stats stats = internal_stats(root());
    if (stats.leaf_nodes == 1 && stats.internal_nodes == 0) {
      return sizeof(*this) + node_type::LeafSize(root()->max_count());
    } else {
      return sizeof(*this) + stats.leaf_nodes * node_type::LeafSize() +
             stats.internal_nodes * node_type::InternalSize();
    }
  }


  static double average_bytes_per_value() {


    const double expected_values_per_node = (kNodeSlots + kMinNodeValues) / 2.0;
    return node_type::LeafSize() / expected_values_per_node;
  }





  double fullness() const {
    if (empty()) return 0.0;
    return static_cast<double>(size()) / (nodes() * kNodeSlots);
  }




  double overhead() const {
    if (empty()) return 0.0;
    return (bytes_used() - size() * sizeof(value_type)) /
           static_cast<double>(size());
  }

  allocator_type get_allocator() const { return allocator(); }

 private:
  friend struct btree_access;

  node_type *root() { return root_; }
  const node_type *root() const { return root_; }
  node_type *&mutable_root() noexcept { return root_; }
  node_type *rightmost() { return rightmost_.template get<2>(); }
  const node_type *rightmost() const { return rightmost_.template get<2>(); }
  node_type *&mutable_rightmost() noexcept {
    return rightmost_.template get<2>();
  }
  key_compare *mutable_key_comp() noexcept {
    return &rightmost_.template get<0>();
  }

  node_type *leftmost() { return root()->parent(); }
  const node_type *leftmost() const { return root()->parent(); }

  allocator_type *mutable_allocator() noexcept {
    return &rightmost_.template get<1>();
  }
  const allocator_type &allocator() const noexcept {
    return rightmost_.template get<1>();
  }


  node_type *allocate(size_type size) {
    return reinterpret_cast<node_type *>(
        absl::container_internal::Allocate<node_type::Alignment()>(
            mutable_allocator(), size));
  }

  node_type *new_internal_node(node_type *parent) {
    node_type *n = allocate(node_type::InternalSize());
    n->init_internal(parent);
    return n;
  }
  node_type *new_leaf_node(node_type *parent) {
    node_type *n = allocate(node_type::LeafSize());
    n->init_leaf(kNodeSlots, parent);
    return n;
  }
  node_type *new_leaf_root_node(field_type max_count) {
    node_type *n = allocate(node_type::LeafSize(max_count));
    n->init_leaf(max_count, /*parent=*/n);
    return n;
  }

  iterator rebalance_after_delete(iterator iter);

  void rebalance_or_split(iterator *iter);


  void merge_nodes(node_type *left, node_type *right);




  bool try_merge_or_rebalance(iterator *iter);

  void try_shrink();

  iterator internal_end(iterator iter) {
    return iter.node_ != nullptr ? iter : end();
  }
  const_iterator internal_end(const_iterator iter) const {
    return iter.node_ != nullptr ? iter : end();
  }


  template <typename... Args>
  iterator internal_emplace(iterator iter, Args &&...args);




  template <typename IterType>
  static IterType internal_last(IterType iter);






  template <typename K>
  SearchResult<iterator, is_key_compare_to::value> internal_locate(
      const K &key) const;

  template <typename K>
  SearchResult<iterator, is_key_compare_to::value> internal_lower_bound(
      const K &key) const;

  template <typename K>
  iterator internal_upper_bound(const K &key) const;

  template <typename K>
  iterator internal_find(const K &key) const;

  size_type internal_verify(const node_type *node, const key_type *lo,
                            const key_type *hi) const;

  node_stats internal_stats(const node_type *node) const {

    if (node == nullptr || (node == root() && empty())) {
      return node_stats(0, 0);
    }
    if (node->is_leaf()) {
      return node_stats(1, 0);
    }
    node_stats res(0, 1);
    for (int i = node->start(); i <= node->finish(); ++i) {
      res += internal_stats(node->child(i));
    }
    return res;
  }

  node_type *root_;



  absl::container_internal::CompressedTuple<key_compare, allocator_type,
                                            node_type *>
      rightmost_;

  size_type size_;
};

// btree_node methods
template <typename P>
template <typename... Args>
inline void btree_node<P>::emplace_value(const field_type i,
                                         allocator_type *alloc,
                                         Args &&...args) {
  assert(i >= start());
  assert(i <= finish());


  if (i < finish()) {
    transfer_n_backward(finish() - i, /*dest_i=*/i + 1, /*src_i=*/i, this,
                        alloc);
  }
  value_init(static_cast<field_type>(i), alloc, std::forward<Args>(args)...);
  set_finish(finish() + 1);

  if (is_internal() && finish() > i + 1) {
    for (field_type j = finish(); j > i + 1; --j) {
      set_child(j, child(j - 1));
    }
    clear_child(i + 1);
  }
}

template <typename P>
inline void btree_node<P>::remove_values(const field_type i,
                                         const field_type to_erase,
                                         allocator_type *alloc) {

  value_destroy_n(i, to_erase, alloc);
  const field_type orig_finish = finish();
  const field_type src_i = i + to_erase;
  transfer_n(orig_finish - src_i, i, src_i, this, alloc);

  if (is_internal()) {

    for (field_type j = 0; j < to_erase; ++j) {
      clear_and_delete(child(i + j + 1), alloc);
    }

    for (field_type j = i + to_erase + 1; j <= orig_finish; ++j) {
      set_child(j - to_erase, child(j));
      clear_child(j);
    }
  }
  set_finish(orig_finish - to_erase);
}

template <typename P>
void btree_node<P>::rebalance_right_to_left(field_type to_move,
                                            btree_node *right,
                                            allocator_type *alloc) {
  assert(parent() == right->parent());
  assert(position() + 1 == right->position());
  assert(right->count() >= count());
  assert(to_move >= 1);
  assert(to_move <= right->count());

  transfer(finish(), position(), parent(), alloc);

  transfer_n(to_move - 1, finish() + 1, right->start(), right, alloc);

  parent()->transfer(position(), right->start() + to_move - 1, right, alloc);

  right->transfer_n(right->count() - to_move, right->start(),
                    right->start() + to_move, right, alloc);

  if (is_internal()) {

    for (field_type i = 0; i < to_move; ++i) {
      init_child(finish() + i + 1, right->child(i));
    }
    for (field_type i = right->start(); i <= right->finish() - to_move; ++i) {
      assert(i + to_move <= right->max_count());
      right->init_child(i, right->child(i + to_move));
      right->clear_child(i + to_move);
    }
  }

  set_finish(finish() + to_move);
  right->set_finish(right->finish() - to_move);
}

template <typename P>
void btree_node<P>::rebalance_left_to_right(field_type to_move,
                                            btree_node *right,
                                            allocator_type *alloc) {
  assert(parent() == right->parent());
  assert(position() + 1 == right->position());
  assert(count() >= right->count());
  assert(to_move >= 1);
  assert(to_move <= count());






  right->transfer_n_backward(right->count(), right->start() + to_move,
                             right->start(), right, alloc);

  right->transfer(right->start() + to_move - 1, position(), parent(), alloc);

  right->transfer_n(to_move - 1, right->start(), finish() - (to_move - 1), this,
                    alloc);

  parent()->transfer(position(), finish() - to_move, this, alloc);

  if (is_internal()) {

    for (field_type i = right->finish() + 1; i > right->start(); --i) {
      right->init_child(i - 1 + to_move, right->child(i - 1));
      right->clear_child(i - 1);
    }
    for (field_type i = 1; i <= to_move; ++i) {
      right->init_child(i - 1, child(finish() - to_move + i));
      clear_child(finish() - to_move + i);
    }
  }

  set_finish(finish() - to_move);
  right->set_finish(right->finish() + to_move);
}

template <typename P>
void btree_node<P>::split(const int insert_position, btree_node *dest,
                          allocator_type *alloc) {
  assert(dest->count() == 0);
  assert(max_count() == kNodeSlots);




  if (insert_position == start()) {
    dest->set_finish(dest->start() + finish() - 1);
  } else if (insert_position == kNodeSlots) {
    dest->set_finish(dest->start());
  } else {
    dest->set_finish(dest->start() + count() / 2);
  }
  set_finish(finish() - dest->count());
  assert(count() >= 1);

  dest->transfer_n(dest->count(), dest->start(), finish(), this, alloc);

  --mutable_finish();
  parent()->emplace_value(position(), alloc, finish_slot());
  value_destroy(finish(), alloc);
  parent()->init_child(position() + 1, dest);

  if (is_internal()) {
    for (field_type i = dest->start(), j = finish() + 1; i <= dest->finish();
         ++i, ++j) {
      assert(child(j) != nullptr);
      dest->init_child(i, child(j));
      clear_child(j);
    }
  }
}

template <typename P>
void btree_node<P>::merge(btree_node *src, allocator_type *alloc) {
  assert(parent() == src->parent());
  assert(position() + 1 == src->position());

  value_init(finish(), alloc, parent()->slot(position()));

  transfer_n(src->count(), finish() + 1, src->start(), src, alloc);

  if (is_internal()) {

    for (field_type i = src->start(), j = finish() + 1; i <= src->finish();
         ++i, ++j) {
      init_child(j, src->child(i));
      src->clear_child(i);
    }
  }

  set_finish(start() + 1 + count() + src->count());
  src->set_finish(src->start());

  parent()->remove_values(position(), /*to_erase=*/1, alloc);
}

template <typename P>
void btree_node<P>::clear_and_delete(btree_node *node, allocator_type *alloc) {
  if (node->is_leaf()) {
    node->value_destroy_n(node->start(), node->count(), alloc);
    deallocate(LeafSize(node->max_count()), node, alloc);
    return;
  }
  if (node->count() == 0) {
    deallocate(InternalSize(), node, alloc);
    return;
  }

  btree_node *delete_root_parent = node->parent();

  while (node->is_internal()) node = node->start_child();
#ifdef ABSL_BTREE_ENABLE_GENERATIONS





  btree_node *leftmost_leaf = node;
#endif


  size_type pos = node->position();
  btree_node *parent = node->parent();
  for (;;) {

    assert(pos <= parent->finish());
    do {
      node = parent->child(static_cast<field_type>(pos));
      if (node->is_internal()) {

        while (node->is_internal()) node = node->start_child();
        pos = node->position();
        parent = node->parent();
      }
      node->value_destroy_n(node->start(), node->count(), alloc);
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
      if (leftmost_leaf != node)
#endif
        deallocate(LeafSize(node->max_count()), node, alloc);
      ++pos;
    } while (pos <= parent->finish());

    assert(pos > parent->finish());
    do {
      node = parent;
      pos = node->position();
      parent = node->parent();
      node->value_destroy_n(node->start(), node->count(), alloc);
      deallocate(InternalSize(), node, alloc);
      if (parent == delete_root_parent) {
#ifdef ABSL_BTREE_ENABLE_GENERATIONS
        deallocate(LeafSize(leftmost_leaf->max_count()), leftmost_leaf, alloc);
#endif
        return;
      }
      ++pos;
    } while (pos > parent->finish());
  }
}

// btree_iterator methods

template <typename N, typename R, typename P>
auto btree_iterator<N, R, P>::distance_slow(const_iterator other) const
    -> difference_type {
  const_iterator begin = other;
  const_iterator end = *this;
  assert(begin.node_ != end.node_ || !begin.node_->is_leaf() ||
         begin.position_ != end.position_);

  const node_type *node = begin.node_;

  difference_type count = node->is_leaf() ? -begin.position_ : 0;

  if (node->is_internal()) {
    ++count;
    node = node->child(begin.position_ + 1);
  }
  while (node->is_internal()) node = node->start_child();


  size_type pos = node->position();
  const node_type *parent = node->parent();
  for (;;) {

    assert(pos <= parent->finish());
    do {
      node = parent->child(static_cast<field_type>(pos));
      if (node->is_internal()) {

        while (node->is_internal()) node = node->start_child();
        pos = node->position();
        parent = node->parent();
      }
      if (node == end.node_) return count + end.position_;
      if (parent == end.node_ && pos == static_cast<size_type>(end.position_))
        return count + node->count();

      count += node->count() + 1;
      ++pos;
    } while (pos <= parent->finish());

    assert(pos > parent->finish());
    do {
      node = parent;
      pos = node->position();
      parent = node->parent();

      if (parent == end.node_ && pos == static_cast<size_type>(end.position_))
        return count - 1;
      ++pos;
    } while (pos > parent->finish());
  }
}

template <typename N, typename R, typename P>
void btree_iterator<N, R, P>::increment_slow() {
  if (node_->is_leaf()) {
    assert(position_ >= node_->finish());
    btree_iterator save(*this);
    while (position_ == node_->finish() && !node_->is_root()) {
      assert(node_->parent()->child(node_->position()) == node_);
      position_ = node_->position();
      node_ = node_->parent();
    }

    if (position_ == node_->finish()) {
      *this = save;
    }
  } else {
    assert(position_ < node_->finish());
    node_ = node_->child(static_cast<field_type>(position_ + 1));
    while (node_->is_internal()) {
      node_ = node_->start_child();
    }
    position_ = node_->start();
  }
}

template <typename N, typename R, typename P>
void btree_iterator<N, R, P>::decrement_slow() {
  if (node_->is_leaf()) {
    assert(position_ <= -1);
    btree_iterator save(*this);
    while (position_ < node_->start() && !node_->is_root()) {
      assert(node_->parent()->child(node_->position()) == node_);
      position_ = node_->position() - 1;
      node_ = node_->parent();
    }

    if (position_ < node_->start()) {
      *this = save;
    }
  } else {
    assert(position_ >= node_->start());
    node_ = node_->child(static_cast<field_type>(position_));
    while (node_->is_internal()) {
      node_ = node_->child(node_->finish());
    }
    position_ = node_->finish() - 1;
  }
}

// btree methods
template <typename P>
template <typename Btree>
void btree<P>::copy_or_move_values_in_order(Btree &other) {
  static_assert(std::is_same<btree, Btree>::value ||
                    std::is_same<const btree, Btree>::value,
                "Btree type must be same or const.");
  assert(empty());


  auto iter = other.begin();
  if (iter == other.end()) return;
  insert_multi(iter.slot());
  ++iter;
  for (; iter != other.end(); ++iter) {


    internal_emplace(end(), iter.slot());
  }
}

template <typename P>
constexpr bool btree<P>::static_assert_validation() {
  static_assert(std::is_nothrow_copy_constructible<key_compare>::value,
                "Key comparison must be nothrow copy constructible");
  static_assert(std::is_nothrow_copy_constructible<allocator_type>::value,
                "Allocator must be nothrow copy constructible");
  static_assert(type_traits_internal::is_trivially_copyable<iterator>::value,
                "iterator not trivially copyable.");


  static_assert(
      kNodeSlots < (1 << (8 * sizeof(typename node_type::field_type))),
      "target node size too large");

  static_assert(
      compare_has_valid_result_type<key_compare, key_type>(),
      "key comparison function must return absl::{weak,strong}_ordering or "
      "bool.");

  static_assert(node_type::MinimumOverhead() >= sizeof(void *) + 4,
                "node space assumption incorrect");

  return true;
}

template <typename P>
template <typename K>
auto btree<P>::lower_bound_equal(const K &key) const
    -> std::pair<iterator, bool> {
  const SearchResult<iterator, is_key_compare_to::value> res =
      internal_lower_bound(key);
  const iterator lower = iterator(internal_end(res.value));
  const bool equal = res.HasMatch()
                         ? res.IsEq()
                         : lower != end() && !compare_keys(key, lower.key());
  return {lower, equal};
}

template <typename P>
template <typename K>
auto btree<P>::equal_range(const K &key) -> std::pair<iterator, iterator> {
  const std::pair<iterator, bool> lower_and_equal = lower_bound_equal(key);
  const iterator lower = lower_and_equal.first;
  if (!lower_and_equal.second) {
    return {lower, lower};
  }

  const iterator next = std::next(lower);
  if (!params_type::template can_have_multiple_equivalent_keys<K>()) {




    assert(next == end() || compare_keys(key, next.key()));
    return {lower, next};
  }




  if (next == end() || compare_keys(key, next.key())) return {lower, next};


  return {lower, upper_bound(key)};
}

template <typename P>
template <typename K, typename... Args>
auto btree<P>::insert_unique(const K &key, Args &&...args)
    -> std::pair<iterator, bool> {
  if (empty()) {
    mutable_root() = mutable_rightmost() = new_leaf_root_node(1);
  }

  SearchResult<iterator, is_key_compare_to::value> res = internal_locate(key);
  iterator iter = res.value;

  if (res.HasMatch()) {
    if (res.IsEq()) {

      return {iter, false};
    }
  } else {
    iterator last = internal_last(iter);
    if (last.node_ && !compare_keys(key, last.key())) {

      return {last, false};
    }
  }
  return {internal_emplace(iter, std::forward<Args>(args)...), true};
}

template <typename P>
template <typename K, typename... Args>
inline auto btree<P>::insert_hint_unique(iterator position, const K &key,
                                         Args &&...args)
    -> std::pair<iterator, bool> {
  if (!empty()) {
    if (position == end() || compare_keys(key, position.key())) {
      if (position == begin() || compare_keys(std::prev(position).key(), key)) {

        return {internal_emplace(position, std::forward<Args>(args)...), true};
      }
    } else if (compare_keys(position.key(), key)) {
      ++position;
      if (position == end() || compare_keys(key, position.key())) {

        return {internal_emplace(position, std::forward<Args>(args)...), true};
      }
    } else {

      return {position, false};
    }
  }
  return insert_unique(key, std::forward<Args>(args)...);
}

template <typename P>
template <typename InputIterator, typename>
void btree<P>::insert_iterator_unique(InputIterator b, InputIterator e, int) {
  for (; b != e; ++b) {
    insert_hint_unique(end(), params_type::key(*b), *b);
  }
}

template <typename P>
template <typename InputIterator>
void btree<P>::insert_iterator_unique(InputIterator b, InputIterator e, char) {
  for (; b != e; ++b) {

    auto node_handle =
        CommonAccess::Construct<node_handle_type>(get_allocator(), *b);
    slot_type *slot = CommonAccess::GetSlot(node_handle);
    insert_hint_unique(end(), params_type::key(slot), slot);
  }
}

template <typename P>
template <typename ValueType>
auto btree<P>::insert_multi(const key_type &key, ValueType &&v) -> iterator {
  if (empty()) {
    mutable_root() = mutable_rightmost() = new_leaf_root_node(1);
  }

  iterator iter = internal_upper_bound(key);
  if (iter.node_ == nullptr) {
    iter = end();
  }
  return internal_emplace(iter, std::forward<ValueType>(v));
}

template <typename P>
template <typename ValueType>
auto btree<P>::insert_hint_multi(iterator position, ValueType &&v) -> iterator {
  if (!empty()) {
    const key_type &key = params_type::key(v);
    if (position == end() || !compare_keys(position.key(), key)) {
      if (position == begin() ||
          !compare_keys(key, std::prev(position).key())) {

        return internal_emplace(position, std::forward<ValueType>(v));
      }
    } else {
      ++position;
      if (position == end() || !compare_keys(position.key(), key)) {

        return internal_emplace(position, std::forward<ValueType>(v));
      }
    }
  }
  return insert_multi(std::forward<ValueType>(v));
}

template <typename P>
template <typename InputIterator>
void btree<P>::insert_iterator_multi(InputIterator b, InputIterator e) {
  for (; b != e; ++b) {
    insert_hint_multi(end(), *b);
  }
}

template <typename P>
auto btree<P>::operator=(const btree &other) -> btree & {
  if (this != &other) {
    clear();

    *mutable_key_comp() = other.key_comp();
    if (absl::allocator_traits<
            allocator_type>::propagate_on_container_copy_assignment::value) {
      *mutable_allocator() = other.allocator();
    }

    copy_or_move_values_in_order(other);
  }
  return *this;
}

template <typename P>
auto btree<P>::operator=(btree &&other) noexcept -> btree & {
  if (this != &other) {
    clear();

    using std::swap;
    if (absl::allocator_traits<
            allocator_type>::propagate_on_container_copy_assignment::value) {
      swap(root_, other.root_);

      swap(rightmost_, other.rightmost_);
      swap(size_, other.size_);
    } else {
      if (allocator() == other.allocator()) {
        swap(mutable_root(), other.mutable_root());
        swap(*mutable_key_comp(), *other.mutable_key_comp());
        swap(mutable_rightmost(), other.mutable_rightmost());
        swap(size_, other.size_);
      } else {





        *mutable_key_comp() = other.key_comp();
        copy_or_move_values_in_order(other);
      }
    }
  }
  return *this;
}

template <typename P>
auto btree<P>::erase(iterator iter) -> iterator {
  iter.node_->value_destroy(static_cast<field_type>(iter.position_),
                            mutable_allocator());
  iter.update_generation();

  const bool internal_delete = iter.node_->is_internal();
  if (internal_delete) {



    iterator internal_iter(iter);
    --iter;
    assert(iter.node_->is_leaf());
    internal_iter.node_->transfer(
        static_cast<size_type>(internal_iter.position_),
        static_cast<size_type>(iter.position_), iter.node_,
        mutable_allocator());
  } else {


    const field_type transfer_from =
        static_cast<field_type>(iter.position_ + 1);
    const field_type num_to_transfer = iter.node_->finish() - transfer_from;
    iter.node_->transfer_n(num_to_transfer,
                           static_cast<size_type>(iter.position_),
                           transfer_from, iter.node_, mutable_allocator());
  }

  iter.node_->set_finish(iter.node_->finish() - 1);
  --size_;







  iterator res = rebalance_after_delete(iter);

  if (internal_delete) {
    ++res;
  }
  return res;
}

template <typename P>
auto btree<P>::rebalance_after_delete(iterator iter) -> iterator {

  iterator res(iter);
  bool first_iteration = true;
  for (;;) {
    if (iter.node_ == root()) {
      try_shrink();
      if (empty()) {
        return end();
      }
      break;
    }
    if (iter.node_->count() >= kMinNodeValues) {
      break;
    }
    bool merged = try_merge_or_rebalance(&iter);


    if (first_iteration) {
      res = iter;
      first_iteration = false;
    }
    if (!merged) {
      break;
    }
    iter.position_ = iter.node_->position();
    iter.node_ = iter.node_->parent();
  }
  res.update_generation();


  if (res.position_ == res.node_->finish()) {
    res.position_ = res.node_->finish() - 1;
    ++res;
  }

  return res;
}

template <typename P>
auto btree<P>::erase_range(iterator begin, iterator end)
    -> std::pair<size_type, iterator> {
  size_type count = static_cast<size_type>(end - begin);
  assert(count >= 0);

  if (count == 0) {
    return {0, begin};
  }

  if (static_cast<size_type>(count) == size_) {
    clear();
    return {count, this->end()};
  }

  if (begin.node_ == end.node_) {
    assert(end.position_ > begin.position_);
    begin.node_->remove_values(
        static_cast<field_type>(begin.position_),
        static_cast<field_type>(end.position_ - begin.position_),
        mutable_allocator());
    size_ -= count;
    return {count, rebalance_after_delete(begin)};
  }

  const size_type target_size = size_ - count;
  while (size_ > target_size) {
    if (begin.node_->is_leaf()) {
      const size_type remaining_to_erase = size_ - target_size;
      const size_type remaining_in_node =
          static_cast<size_type>(begin.node_->finish() - begin.position_);
      const field_type to_erase = static_cast<field_type>(
          (std::min)(remaining_to_erase, remaining_in_node));
      begin.node_->remove_values(static_cast<field_type>(begin.position_),
                                 to_erase, mutable_allocator());
      size_ -= to_erase;
      begin = rebalance_after_delete(begin);
    } else {
      begin = erase(begin);
    }
  }
  begin.update_generation();
  return {count, begin};
}

template <typename P>
void btree<P>::clear() {
  if (!empty()) {
    node_type::clear_and_delete(root(), mutable_allocator());
  }
  mutable_root() = mutable_rightmost() = EmptyNode();
  size_ = 0;
}

template <typename P>
void btree<P>::swap(btree &other) {
  using std::swap;
  if (absl::allocator_traits<
          allocator_type>::propagate_on_container_swap::value) {

    swap(rightmost_, other.rightmost_);
  } else {

    assert(allocator() == other.allocator());
    swap(mutable_rightmost(), other.mutable_rightmost());
    swap(*mutable_key_comp(), *other.mutable_key_comp());
  }
  swap(mutable_root(), other.mutable_root());
  swap(size_, other.size_);
}

template <typename P>
void btree<P>::verify() const {
  assert(root() != nullptr);
  assert(leftmost() != nullptr);
  assert(rightmost() != nullptr);
  assert(empty() || size() == internal_verify(root(), nullptr, nullptr));
  assert(leftmost() == (++const_iterator(root(), -1)).node_);
  assert(rightmost() == (--const_iterator(root(), root()->finish())).node_);
  assert(leftmost()->is_leaf());
  assert(rightmost()->is_leaf());
}

template <typename P>
void btree<P>::rebalance_or_split(iterator *iter) {
  node_type *&node = iter->node_;
  int &insert_position = iter->position_;
  assert(node->count() == node->max_count());
  assert(kNodeSlots == node->max_count());

  node_type *parent = node->parent();
  if (node != root()) {
    if (node->position() > parent->start()) {

      node_type *left = parent->child(node->position() - 1);
      assert(left->max_count() == kNodeSlots);
      if (left->count() < kNodeSlots) {



        field_type to_move =
            (kNodeSlots - left->count()) /
            (1 + (static_cast<field_type>(insert_position) < kNodeSlots));
        to_move = (std::max)(field_type{1}, to_move);

        if (static_cast<field_type>(insert_position) - to_move >=
                node->start() ||
            left->count() + to_move < kNodeSlots) {
          left->rebalance_right_to_left(to_move, node, mutable_allocator());

          assert(node->max_count() - node->count() == to_move);
          insert_position = static_cast<int>(
              static_cast<field_type>(insert_position) - to_move);
          if (insert_position < node->start()) {
            insert_position = insert_position + left->count() + 1;
            node = left;
          }

          assert(node->count() < node->max_count());
          return;
        }
      }
    }

    if (node->position() < parent->finish()) {

      node_type *right = parent->child(node->position() + 1);
      assert(right->max_count() == kNodeSlots);
      if (right->count() < kNodeSlots) {



        field_type to_move = (kNodeSlots - right->count()) /
                             (1 + (insert_position > node->start()));
        to_move = (std::max)(field_type{1}, to_move);

        if (static_cast<field_type>(insert_position) <=
                node->finish() - to_move ||
            right->count() + to_move < kNodeSlots) {
          node->rebalance_left_to_right(to_move, right, mutable_allocator());

          if (insert_position > node->finish()) {
            insert_position = insert_position - node->count() - 1;
            node = right;
          }

          assert(node->count() < node->max_count());
          return;
        }
      }
    }


    assert(parent->max_count() == kNodeSlots);
    if (parent->count() == kNodeSlots) {
      iterator parent_iter(node->parent(), node->position());
      rebalance_or_split(&parent_iter);
    }
  } else {



    parent = new_internal_node(parent);
    parent->set_generation(root()->generation());
    parent->init_child(parent->start(), root());
    mutable_root() = parent;

    assert(parent->start_child()->is_internal() ||
           parent->start_child() == rightmost());
  }

  node_type *split_node;
  if (node->is_leaf()) {
    split_node = new_leaf_node(parent);
    node->split(insert_position, split_node, mutable_allocator());
    if (rightmost() == node) mutable_rightmost() = split_node;
  } else {
    split_node = new_internal_node(parent);
    node->split(insert_position, split_node, mutable_allocator());
  }

  if (insert_position > node->finish()) {
    insert_position = insert_position - node->count() - 1;
    node = split_node;
  }
}

template <typename P>
void btree<P>::merge_nodes(node_type *left, node_type *right) {
  left->merge(right, mutable_allocator());
  if (rightmost() == right) mutable_rightmost() = left;
}

template <typename P>
bool btree<P>::try_merge_or_rebalance(iterator *iter) {
  node_type *parent = iter->node_->parent();
  if (iter->node_->position() > parent->start()) {

    node_type *left = parent->child(iter->node_->position() - 1);
    assert(left->max_count() == kNodeSlots);
    if (1U + left->count() + iter->node_->count() <= kNodeSlots) {
      iter->position_ += 1 + left->count();
      merge_nodes(left, iter->node_);
      iter->node_ = left;
      return true;
    }
  }
  if (iter->node_->position() < parent->finish()) {

    node_type *right = parent->child(iter->node_->position() + 1);
    assert(right->max_count() == kNodeSlots);
    if (1U + iter->node_->count() + right->count() <= kNodeSlots) {
      merge_nodes(iter->node_, right);
      return true;
    }




    if (right->count() > kMinNodeValues &&
        (iter->node_->count() == 0 || iter->position_ > iter->node_->start())) {
      field_type to_move = (right->count() - iter->node_->count()) / 2;
      to_move =
          (std::min)(to_move, static_cast<field_type>(right->count() - 1));
      iter->node_->rebalance_right_to_left(to_move, right, mutable_allocator());
      return false;
    }
  }
  if (iter->node_->position() > parent->start()) {




    node_type *left = parent->child(iter->node_->position() - 1);
    if (left->count() > kMinNodeValues &&
        (iter->node_->count() == 0 ||
         iter->position_ < iter->node_->finish())) {
      field_type to_move = (left->count() - iter->node_->count()) / 2;
      to_move = (std::min)(to_move, static_cast<field_type>(left->count() - 1));
      left->rebalance_left_to_right(to_move, iter->node_, mutable_allocator());
      iter->position_ += to_move;
      return false;
    }
  }
  return false;
}

template <typename P>
void btree<P>::try_shrink() {
  node_type *orig_root = root();
  if (orig_root->count() > 0) {
    return;
  }

  if (orig_root->is_leaf()) {
    assert(size() == 0);
    mutable_root() = mutable_rightmost() = EmptyNode();
  } else {
    node_type *child = orig_root->start_child();
    child->make_root();
    mutable_root() = child;
  }
  node_type::clear_and_delete(orig_root, mutable_allocator());
}

template <typename P>
template <typename IterType>
inline IterType btree<P>::internal_last(IterType iter) {
  assert(iter.node_ != nullptr);
  while (iter.position_ == iter.node_->finish()) {
    iter.position_ = iter.node_->position();
    iter.node_ = iter.node_->parent();
    if (iter.node_->is_leaf()) {
      iter.node_ = nullptr;
      break;
    }
  }
  iter.update_generation();
  return iter;
}

template <typename P>
template <typename... Args>
inline auto btree<P>::internal_emplace(iterator iter, Args &&...args)
    -> iterator {
  if (iter.node_->is_internal()) {


    --iter;
    ++iter.position_;
  }
  const field_type max_count = iter.node_->max_count();
  allocator_type *alloc = mutable_allocator();
  if (iter.node_->count() == max_count) {

    if (max_count < kNodeSlots) {


      assert(iter.node_ == root());
      iter.node_ = new_leaf_root_node(static_cast<field_type>(
          (std::min)(static_cast<int>(kNodeSlots), 2 * max_count)));

      node_type *old_root = root();
      node_type *new_root = iter.node_;
      new_root->transfer_n(old_root->count(), new_root->start(),
                           old_root->start(), old_root, alloc);
      new_root->set_finish(old_root->finish());
      old_root->set_finish(old_root->start());
      new_root->set_generation(old_root->generation());
      node_type::clear_and_delete(old_root, alloc);
      mutable_root() = mutable_rightmost() = new_root;
    } else {
      rebalance_or_split(&iter);
    }
  }
  iter.node_->emplace_value(static_cast<field_type>(iter.position_), alloc,
                            std::forward<Args>(args)...);
  ++size_;
  iter.update_generation();
  return iter;
}

template <typename P>
template <typename K>
inline auto btree<P>::internal_locate(const K &key) const
    -> SearchResult<iterator, is_key_compare_to::value> {
  iterator iter(const_cast<node_type *>(root()));
  for (;;) {
    SearchResult<size_type, is_key_compare_to::value> res =
        iter.node_->lower_bound(key, key_comp());
    iter.position_ = static_cast<int>(res.value);
    if (res.IsEq()) {
      return {iter, MatchKind::kEq};
    }




    if (iter.node_->is_leaf()) {
      break;
    }
    iter.node_ = iter.node_->child(static_cast<field_type>(iter.position_));
  }


  return {iter, MatchKind::kNe};
}

template <typename P>
template <typename K>
auto btree<P>::internal_lower_bound(const K &key) const
    -> SearchResult<iterator, is_key_compare_to::value> {
  if (!params_type::template can_have_multiple_equivalent_keys<K>()) {
    SearchResult<iterator, is_key_compare_to::value> ret = internal_locate(key);
    ret.value = internal_last(ret.value);
    return ret;
  }
  iterator iter(const_cast<node_type *>(root()));
  SearchResult<size_type, is_key_compare_to::value> res;
  bool seen_eq = false;
  for (;;) {
    res = iter.node_->lower_bound(key, key_comp());
    iter.position_ = static_cast<int>(res.value);
    if (iter.node_->is_leaf()) {
      break;
    }
    seen_eq = seen_eq || res.IsEq();
    iter.node_ = iter.node_->child(static_cast<field_type>(iter.position_));
  }
  if (res.IsEq()) return {iter, MatchKind::kEq};
  return {internal_last(iter), seen_eq ? MatchKind::kEq : MatchKind::kNe};
}

template <typename P>
template <typename K>
auto btree<P>::internal_upper_bound(const K &key) const -> iterator {
  iterator iter(const_cast<node_type *>(root()));
  for (;;) {
    iter.position_ = static_cast<int>(iter.node_->upper_bound(key, key_comp()));
    if (iter.node_->is_leaf()) {
      break;
    }
    iter.node_ = iter.node_->child(static_cast<field_type>(iter.position_));
  }
  return internal_last(iter);
}

template <typename P>
template <typename K>
auto btree<P>::internal_find(const K &key) const -> iterator {
  SearchResult<iterator, is_key_compare_to::value> res = internal_locate(key);
  if (res.HasMatch()) {
    if (res.IsEq()) {
      return res.value;
    }
  } else {
    const iterator iter = internal_last(res.value);
    if (iter.node_ != nullptr && !compare_keys(key, iter.key())) {
      return iter;
    }
  }
  return {nullptr, 0};
}

template <typename P>
typename btree<P>::size_type btree<P>::internal_verify(
    const node_type *node, const key_type *lo, const key_type *hi) const {
  assert(node->count() > 0);
  assert(node->count() <= node->max_count());
  if (lo) {
    assert(!compare_keys(node->key(node->start()), *lo));
  }
  if (hi) {
    assert(!compare_keys(*hi, node->key(node->finish() - 1)));
  }
  for (int i = node->start() + 1; i < node->finish(); ++i) {
    assert(!compare_keys(node->key(i), node->key(i - 1)));
  }
  size_type count = node->count();
  if (node->is_internal()) {
    for (field_type i = node->start(); i <= node->finish(); ++i) {
      assert(node->child(i) != nullptr);
      assert(node->child(i)->parent() == node);
      assert(node->child(i)->position() == i);
      count += internal_verify(node->child(i),
                               i == node->start() ? lo : &node->key(i - 1),
                               i == node->finish() ? hi : &node->key(i));
    }
  }
  return count;
}

struct btree_access {
  template <typename BtreeContainer, typename Pred>
  static auto erase_if(BtreeContainer &container, Pred pred) ->
      typename BtreeContainer::size_type {
    const auto initial_size = container.size();
    auto &tree = container.tree_;
    auto *alloc = tree.mutable_allocator();
    for (auto it = container.begin(); it != container.end();) {
      if (!pred(*it)) {
        ++it;
        continue;
      }
      auto *node = it.node_;
      if (node->is_internal()) {

        it = container.erase(it);
        continue;
      }



      int to_pos = it.position_;
      node->value_destroy(it.position_, alloc);
      while (++it.position_ < node->finish()) {
        it.update_generation();
        if (pred(*it)) {
          node->value_destroy(it.position_, alloc);
        } else {
          node->transfer(node->slot(to_pos++), node->slot(it.position_), alloc);
        }
      }
      const int num_deleted = node->finish() - to_pos;
      tree.size_ -= num_deleted;
      node->set_finish(to_pos);
      it.position_ = to_pos;
      it = tree.rebalance_after_delete(it);
    }
    return initial_size - container.size();
  }
};

#undef ABSL_BTREE_ENABLE_GENERATIONS

}  // namespace container_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_INTERNAL_BTREE_H_
