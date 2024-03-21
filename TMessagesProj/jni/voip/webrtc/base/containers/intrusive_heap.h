// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINERS_INTRUSIVE_HEAP_H_
#define BASE_CONTAINERS_INTRUSIVE_HEAP_H_

// facilitate this, each element has associated with it a HeapHandle (an opaque
// wrapper around the index at which the element is stored), which is maintained
// by the heap as elements move within it.
//
// An IntrusiveHeap is implemented as a standard max-heap over a std::vector<T>,
// like std::make_heap. Insertion, removal and updating are amortized O(lg size)
// (occasional O(size) cost if a new vector allocation is required). Retrieving
// an element by handle is O(1). Looking up the top element is O(1). Insertions,
// removals and updates invalidate all iterators, but handles remain valid.
// Similar to a std::set, all iterators are read-only so as to disallow changing
// elements and violating the heap property. That being said, if the type you
// are storing is able to have its sort key be changed externally you can
// repair the heap by resorting the modified element via a call to "Update".
//
// Example usage:
//
//   // Create a heap, wrapping integer elements with WithHeapHandle in order to
//   // endow them with heap handles.
//   IntrusiveHeap<WithHeapHandle<int>> heap;
//
//   // WithHeapHandle<T> is for simple or opaque types. In cases where you
//   // control the type declaration you can also provide HeapHandle storage by
//   // deriving from InternalHeapHandleStorage.
//   class Foo : public InternalHeapHandleStorage {
//    public:
//     explicit Foo(int);
//     ...
//   };
//   IntrusiveHeap<Foo> heap2;
//
//   // Insert some elements. Like most containers, "insert" returns an iterator
//   // to the element in the container.
//   heap.insert(3);
//   heap.insert(1);
//   auto it = heap.insert(4);
//
//   // By default this is a max heap, so the top element should be 4 at this
//   // point.
//   EXPECT_EQ(4, heap.top().value());
//
//   // Iterators are invalidated by further heap operations, but handles are
//   // not. Grab a handle to the current top element so we can track it across
//   // changes.
//   HeapHandle* handle = it->handle();
//
//   // Insert a new max element. 4 should no longer be the top.
//   heap.insert(5);
//   EXPECT_EQ(5, heap.top().value());
//
//   // We can lookup and erase element 4 by its handle, even though it has
//   // moved. Note that erasing the element invalidates the handle to it.
//   EXPECT_EQ(4, heap.at(*handle).value());
//   heap.erase(*handle);
//   handle = nullptr;
//
//   // Popping the current max (5), makes 3 the new max, as we already erased
//   // element 4.
//   heap.pop();
//   EXPECT_EQ(3, heap.top().value());
//
// Under the hood the HeapHandle is managed by an object implementing the
// HeapHandleAccess interface, which is passed as a parameter to the
// IntrusiveHeap template:
//
//   // Gets the heap handle associated with the element. This should return the
//   // most recently set handle value, or HeapHandle::Invalid(). This is only
//   // called in DCHECK builds.
//   HeapHandle GetHeapHandle(const T*);
//
//   // Changes the result of GetHeapHandle. GetHeapHandle() must return the
//   // most recent value provided to SetHeapHandle() or HeapHandle::Invalid().
//   // In some implementations, where GetHeapHandle() can independently
//   // reproduce the correct value, it is possible that SetHeapHandle() does
//   // nothing.
//   void SetHeapHandle(T*, HeapHandle);
//
//   // Clears the heap handle associated with the given element. After calling
//   // this GetHeapHandle() must return HeapHandle::Invalid().
//   void ClearHeapHandle(T*);
//
// The default implementation of HeapHandleAccess assumes that your type
// provides HeapHandle storage and will simply forward these calls to equivalent
// member functions on the type T:
//
//   void T::SetHeapHandle(HeapHandle)
//   void T::ClearHeapHandle()
//   HeapHandle T::GetHeapHandle() const
//
// The WithHeapHandle and InternalHeapHandleStorage classes in turn provide
// implementations of that contract.
//
// In summary, to provide heap handle support for your type, you can do one of
// the following (from most manual / least magical, to least manual / most
// magical):
//
// 0. use a custom HeapHandleAccessor, and implement storage however you want;
// 1. use the default HeapHandleAccessor, and manually provide storage on your
//    your element type and implement the IntrusiveHeap contract;
// 2. use the default HeapHandleAccessor, and endow your type with handle
//    storage by deriving from a helper class (see InternalHeapHandleStorage);
//    or,
// 3. use the default HeapHandleAccessor, and wrap your type in a container that
//    provides handle storage (see WithHeapHandle<T>).
//
// Approach 0 is suitable for custom types that already implement something akin
// to heap handles, via back pointers or any other mechanism, but where the
// storage is external to the objects in the heap. If you already have the
// ability to determine where in a container an object lives despite it
// being moved, then you don't need the overhead of storing an actual HeapHandle
// whose value can be inferred.
//
// Approach 1 is is suitable in cases like the above, but where the data
// allowing you to determine the index of an element in a container is stored
// directly in the object itself.
//
// Approach 2 is suitable for types whose declarations you control, where you
// are able to use inheritance.
//
// Finally, approach 3 is suitable when you are storing PODs, or a type whose
// declaration you can not change.
//
// Most users should be using approach 2 or 3.

#include <algorithm>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"

namespace base {

// IntrusiveHeap. A HeapHandle is associated with each element in an
// IntrusiveHeap, and is maintained by the heap as the object moves around
// within it. It can be used to subsequently remove the element, or update it
// in place.
class BASE_EXPORT HeapHandle {
 public:
  enum : size_t { kInvalidIndex = std::numeric_limits<size_t>::max() };

  constexpr HeapHandle() = default;
  constexpr HeapHandle(const HeapHandle& other) = default;
  HeapHandle(HeapHandle&& other) noexcept
      : index_(std::exchange(other.index_, kInvalidIndex)) {}
  ~HeapHandle() = default;

  HeapHandle& operator=(const HeapHandle& other) = default;
  HeapHandle& operator=(HeapHandle&& other) noexcept {
    index_ = std::exchange(other.index_, kInvalidIndex);
    return *this;
  }

  static HeapHandle Invalid();

  void reset() { index_ = kInvalidIndex; }

  size_t index() const { return index_; }
  bool IsValid() const { return index_ != kInvalidIndex; }

  friend bool operator==(const HeapHandle& lhs, const HeapHandle& rhs) {
    return lhs.index_ == rhs.index_;
  }
  friend bool operator!=(const HeapHandle& lhs, const HeapHandle& rhs) {
    return lhs.index_ != rhs.index_;
  }
  friend bool operator<(const HeapHandle& lhs, const HeapHandle& rhs) {
    return lhs.index_ < rhs.index_;
  }
  friend bool operator>(const HeapHandle& lhs, const HeapHandle& rhs) {
    return lhs.index_ > rhs.index_;
  }
  friend bool operator<=(const HeapHandle& lhs, const HeapHandle& rhs) {
    return lhs.index_ <= rhs.index_;
  }
  friend bool operator>=(const HeapHandle& lhs, const HeapHandle& rhs) {
    return lhs.index_ >= rhs.index_;
  }

 private:
  template <typename T, typename Compare, typename HeapHandleAccessor>
  friend class IntrusiveHeap;

  explicit HeapHandle(size_t index) : index_(index) {}

  size_t index_ = kInvalidIndex;
};

// type.
template <typename T>
struct DefaultHeapHandleAccessor {
  void SetHeapHandle(T* element, HeapHandle handle) const {
    element->SetHeapHandle(handle);
  }

  void ClearHeapHandle(T* element) const { element->ClearHeapHandle(); }

  HeapHandle GetHeapHandle(const T* element) const {
    return element->GetHeapHandle();
  }
};

// removal are similar, objects don't have a fixed address in memory) crossed
// with a std::set (elements are considered immutable once they're in the
// container).
template <typename T,
          typename Compare = std::less<T>,
          typename HeapHandleAccessor = DefaultHeapHandleAccessor<T>>
class IntrusiveHeap {
 private:
  using UnderlyingType = std::vector<T>;

 public:



  using value_type = typename UnderlyingType::value_type;
  using size_type = typename UnderlyingType::size_type;
  using difference_type = typename UnderlyingType::difference_type;
  using value_compare = Compare;
  using heap_handle_accessor = HeapHandleAccessor;

  using reference = typename UnderlyingType::reference;
  using const_reference = typename UnderlyingType::const_reference;
  using pointer = typename UnderlyingType::pointer;
  using const_pointer = typename UnderlyingType::const_pointer;

  using iterator = typename UnderlyingType::const_iterator;
  using const_iterator = typename UnderlyingType::const_iterator;
  using reverse_iterator = typename UnderlyingType::const_reverse_iterator;
  using const_reverse_iterator =
      typename UnderlyingType::const_reverse_iterator;



  IntrusiveHeap() = default;
  IntrusiveHeap(const value_compare& comp, const heap_handle_accessor& access)
      : impl_(comp, access) {}

  template <class InputIterator>
  IntrusiveHeap(InputIterator first,
                InputIterator last,
                const value_compare& comp = value_compare(),
                const heap_handle_accessor& access = heap_handle_accessor())
      : impl_(comp, access) {
    insert(first, last);
  }


  IntrusiveHeap(IntrusiveHeap&& other) = default;

  IntrusiveHeap(const IntrusiveHeap&);

  template <typename U>
  IntrusiveHeap(std::initializer_list<U> ilist,
                const value_compare& comp = value_compare(),
                const heap_handle_accessor& access = heap_handle_accessor())
      : impl_(comp, access) {
    insert(std::begin(ilist), std::end(ilist));
  }

  ~IntrusiveHeap();



  IntrusiveHeap& operator=(IntrusiveHeap&&) noexcept;
  IntrusiveHeap& operator=(const IntrusiveHeap&);
  IntrusiveHeap& operator=(std::initializer_list<value_type> ilist);









  const_reference at(size_type pos) const { return impl_.heap_.at(pos); }
  const_reference at(HeapHandle pos) const {
    return impl_.heap_.at(pos.index());
  }
  const_reference operator[](size_type pos) const { return impl_.heap_[pos]; }
  const_reference operator[](HeapHandle pos) const {
    return impl_.heap_[pos.index()];
  }
  const_reference front() const { return impl_.heap_.front(); }
  const_reference back() const { return impl_.heap_.back(); }
  const_reference top() const { return impl_.heap_.front(); }

  const_pointer data() const { return impl_.heap_.data(); }



  void reserve(size_type new_capacity) { impl_.heap_.reserve(new_capacity); }
  size_type capacity() const { return impl_.heap_.capacity(); }
  void shrink_to_fit() { impl_.heap_.shrink_to_fit(); }



  void clear();
  size_type size() const { return impl_.heap_.size(); }
  size_type max_size() const { return impl_.heap_.max_size(); }
  bool empty() const { return impl_.heap_.empty(); }





  const_iterator begin() const { return impl_.heap_.cbegin(); }
  const_iterator cbegin() const { return impl_.heap_.cbegin(); }

  const_iterator end() const { return impl_.heap_.cend(); }
  const_iterator cend() const { return impl_.heap_.cend(); }

  const_reverse_iterator rbegin() const { return impl_.heap_.crbegin(); }
  const_reverse_iterator crbegin() const { return impl_.heap_.crbegin(); }

  const_reverse_iterator rend() const { return impl_.heap_.crend(); }
  const_reverse_iterator crend() const { return impl_.heap_.crend(); }







  const_iterator insert(const value_type& value) { return InsertImpl(value); }
  const_iterator insert(value_type&& value) {
    return InsertImpl(std::move_if_noexcept(value));
  }

  template <class InputIterator>
  void insert(InputIterator first, InputIterator last);

  template <typename... Args>
  const_iterator emplace(Args&&... args);












  value_type take(size_type pos);


  template <typename P>
  value_type take(P pos) {
    return take(ToIndex(pos));
  }

  value_type take_top() { return take(0u); }

  void erase(size_type pos);

  template <typename P>
  void erase(P pos) {
    erase(ToIndex(pos));
  }


  void pop() { erase(0u); }





  const_iterator Replace(size_type pos, const T& element) {
    return ReplaceImpl(pos, element);
  }
  const_iterator Replace(size_type pos, T&& element) {
    return ReplaceImpl(pos, std::move_if_noexcept(element));
  }

  template <typename P>
  const_iterator Replace(P pos, const T& element) {
    return ReplaceImpl(ToIndex(pos), element);
  }
  template <typename P>
  const_iterator Replace(P pos, T&& element) {
    return ReplaceImpl(ToIndex(pos), std::move_if_noexcept(element));
  }

  const_iterator ReplaceTop(const T& element) {
    return ReplaceTopImpl(element);
  }
  const_iterator ReplaceTop(T&& element) {
    return ReplaceTopImpl(std::move_if_noexcept(element));
  }




  const_iterator Update(size_type pos);
  template <typename P>
  const_iterator Update(P pos) {
    return Update(ToIndex(pos));
  }



  const value_compare& value_comp() const { return impl_.get_value_compare(); }

  const heap_handle_accessor& heap_handle_access() const {
    return impl_.get_heap_handle_access();
  }



  void swap(IntrusiveHeap& other) noexcept;
  friend void swap(IntrusiveHeap& lhs, IntrusiveHeap& rhs) { lhs.swap(rhs); }



  friend bool operator==(const IntrusiveHeap& lhs, const IntrusiveHeap& rhs) {
    return lhs.impl_.heap_ == rhs.impl_.heap_;
  }
  friend bool operator!=(const IntrusiveHeap& lhs, const IntrusiveHeap& rhs) {
    return lhs.impl_.heap_ != rhs.impl_.heap_;
  }




  size_type ToIndex(HeapHandle handle) { return handle.index(); }
  size_type ToIndex(const_iterator pos);
  size_type ToIndex(const_reverse_iterator pos);

 private:


  template <typename I, typename = std::enable_if_t<std::is_integral<I>::value>>
  size_type ToIndex(I pos) {
    return static_cast<size_type>(pos);
  }

  size_type GetLastIndex() const { return impl_.heap_.size() - 1; }

  void SetHeapHandle(size_type i);
  void ClearHeapHandle(size_type i);
  HeapHandle GetHeapHandle(size_type i);


  bool Less(size_type i, size_type j);
  bool Less(const T& element, size_type i);
  bool Less(size_type i, const T& element);








  void MakeHole(size_type pos);
  template <typename U>
  void FillHole(size_type hole, U element);
  void MoveHole(size_type new_hole_pos, size_type old_hole_pos);


  template <typename U>
  size_type MoveHoleUpAndFill(size_type hole_pos, U element);




  struct WithLeafElement {
    static constexpr bool kIsLeafElement = true;
  };
  struct WithElement {
    static constexpr bool kIsLeafElement = false;
  };
  template <typename FillElementType, typename U>
  size_type MoveHoleDownAndFill(size_type hole_pos, U element);


  template <typename U>
  const_iterator InsertImpl(U element);
  template <typename U>
  const_iterator ReplaceImpl(size_type pos, U element);
  template <typename U>
  const_iterator ReplaceTopImpl(U element);





  struct Impl : private value_compare, private heap_handle_accessor {
    Impl(const value_compare& value_comp,
         const heap_handle_accessor& heap_handle_access)
        : value_compare(value_comp), heap_handle_accessor(heap_handle_access) {}

    Impl() = default;
    Impl(Impl&&) = default;
    Impl(const Impl&) = default;
    Impl& operator=(Impl&& other) = default;
    Impl& operator=(const Impl& other) = default;

    const value_compare& get_value_compare() const { return *this; }
    value_compare& get_value_compare() { return *this; }

    const heap_handle_accessor& get_heap_handle_access() const { return *this; }
    heap_handle_accessor& get_heap_handle_access() { return *this; }

    UnderlyingType heap_;
  } impl_;
};

// from this type you endow your class with self-owned storage for a HeapHandle.
// This is a move-only type so that the handle follows the element across moves
// and resizes of the underlying vector.
class BASE_EXPORT InternalHeapHandleStorage {
 public:
  InternalHeapHandleStorage();
  InternalHeapHandleStorage(const InternalHeapHandleStorage&) = delete;
  InternalHeapHandleStorage(InternalHeapHandleStorage&& other) noexcept;
  virtual ~InternalHeapHandleStorage();

  InternalHeapHandleStorage& operator=(const InternalHeapHandleStorage&) =
      delete;
  InternalHeapHandleStorage& operator=(
      InternalHeapHandleStorage&& other) noexcept;


  HeapHandle* handle() const { return handle_.get(); }


  void SetHeapHandle(HeapHandle handle) {
    DCHECK(handle.IsValid());
    if (handle_)
      *handle_ = handle;
  }
  void ClearHeapHandle() {
    if (handle_)
      handle_->reset();
  }
  HeapHandle GetHeapHandle() const {
    if (handle_)
      return *handle_;
    return HeapHandle::Invalid();
  }

  void swap(InternalHeapHandleStorage& other) noexcept;
  friend void swap(InternalHeapHandleStorage& lhs,
                   InternalHeapHandleStorage& rhs) {
    lhs.swap(rhs);
  }

 private:
  std::unique_ptr<HeapHandle> handle_;
};

// to wrap arbitrary types and provide them with a HeapHandle, making them
// appropriate for use in an IntrusiveHeap. This is a move-only type.
template <typename T>
class WithHeapHandle : public InternalHeapHandleStorage {
 public:
  WithHeapHandle() = default;


  template <typename U>
  WithHeapHandle(U value) : value_(std::move_if_noexcept(value)) {}
  WithHeapHandle(T&& value) noexcept : value_(std::move(value)) {}

  template <class... Args>
  explicit WithHeapHandle(Args&&... args);
  WithHeapHandle(const WithHeapHandle&) = delete;
  WithHeapHandle(WithHeapHandle&& other) noexcept = default;
  ~WithHeapHandle() override = default;

  WithHeapHandle& operator=(const WithHeapHandle&) = delete;
  WithHeapHandle& operator=(WithHeapHandle&& other) = default;

  T& value() { return value_; }
  const T& value() const { return value_; }

  void swap(WithHeapHandle& other) noexcept;
  friend void swap(WithHeapHandle& lhs, WithHeapHandle& rhs) { lhs.swap(rhs); }

  friend bool operator==(const WithHeapHandle& lhs, const WithHeapHandle& rhs) {
    return lhs.value_ == rhs.value_;
  }
  friend bool operator!=(const WithHeapHandle& lhs, const WithHeapHandle& rhs) {
    return lhs.value_ != rhs.value_;
  }
  friend bool operator<=(const WithHeapHandle& lhs, const WithHeapHandle& rhs) {
    return lhs.value_ <= rhs.value_;
  }
  friend bool operator<(const WithHeapHandle& lhs, const WithHeapHandle& rhs) {
    return lhs.value_ < rhs.value_;
  }
  friend bool operator>=(const WithHeapHandle& lhs, const WithHeapHandle& rhs) {
    return lhs.value_ >= rhs.value_;
  }
  friend bool operator>(const WithHeapHandle& lhs, const WithHeapHandle& rhs) {
    return lhs.value_ > rhs.value_;
  }

 private:
  T value_;
};

// IMPLEMENTATION DETAILS

namespace intrusive_heap {

BASE_EXPORT inline size_t ParentIndex(size_t i) {
  DCHECK_NE(0u, i);
  return (i - 1) / 2;
}

BASE_EXPORT inline size_t LeftIndex(size_t i) {
  return 2 * i + 1;
}

template <typename HandleType>
bool IsInvalid(const HandleType& handle) {
  return !handle || !handle->IsValid();
}

BASE_EXPORT inline void CheckInvalidOrEqualTo(HeapHandle handle, size_t index) {
  if (handle.IsValid())
    DCHECK_EQ(index, handle.index());
}

}  // namespace intrusive_heap

// IntrusiveHeap

template <typename T, typename Compare, typename HeapHandleAccessor>
IntrusiveHeap<T, Compare, HeapHandleAccessor>::IntrusiveHeap(
    const IntrusiveHeap& other)
    : impl_(other.impl_) {
  for (size_t i = 0; i < size(); ++i) {
    SetHeapHandle(i);
  }
}

template <typename T, typename Compare, typename HeapHandleAccessor>
IntrusiveHeap<T, Compare, HeapHandleAccessor>::~IntrusiveHeap() {
  clear();
}

template <typename T, typename Compare, typename HeapHandleAccessor>
IntrusiveHeap<T, Compare, HeapHandleAccessor>&
IntrusiveHeap<T, Compare, HeapHandleAccessor>::operator=(
    IntrusiveHeap&& other) noexcept {
  clear();
  impl_ = std::move(other.impl_);
  return *this;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
IntrusiveHeap<T, Compare, HeapHandleAccessor>&
IntrusiveHeap<T, Compare, HeapHandleAccessor>::operator=(
    const IntrusiveHeap& other) {
  clear();
  impl_ = other.impl_;
  for (size_t i = 0; i < size(); ++i) {
    SetHeapHandle(i);
  }
  return *this;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
IntrusiveHeap<T, Compare, HeapHandleAccessor>&
IntrusiveHeap<T, Compare, HeapHandleAccessor>::operator=(
    std::initializer_list<value_type> ilist) {
  clear();
  insert(std::begin(ilist), std::end(ilist));
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::clear() {

  for (size_type i = 0; i < size(); ++i) {
    ClearHeapHandle(i);
  }

  impl_.heap_.clear();
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <class InputIterator>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::insert(InputIterator first,
                                                           InputIterator last) {
  for (auto it = first; it != last; ++it) {
    insert(value_type(*it));
  }
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename... Args>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::const_iterator
IntrusiveHeap<T, Compare, HeapHandleAccessor>::emplace(Args&&... args) {
  value_type value(std::forward<Args>(args)...);
  return InsertImpl(std::move_if_noexcept(value));
}

template <typename T, typename Compare, typename HeapHandleAccessor>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::value_type
IntrusiveHeap<T, Compare, HeapHandleAccessor>::take(size_type pos) {

  MakeHole(pos);
  value_type val = std::move(impl_.heap_[pos]);


  if (pos != GetLastIndex()) {
    MakeHole(GetLastIndex());


    MoveHoleDownAndFill<WithLeafElement>(
        pos, std::move(impl_.heap_[GetLastIndex()]));
  }

  impl_.heap_.pop_back();

  return val;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::erase(size_type pos) {
  DCHECK_LT(pos, size());

  MakeHole(pos);


  if (pos != GetLastIndex()) {
    MakeHole(GetLastIndex());


    MoveHoleDownAndFill<WithLeafElement>(
        pos, std::move_if_noexcept(impl_.heap_[GetLastIndex()]));
  }

  impl_.heap_.pop_back();
}

template <typename T, typename Compare, typename HeapHandleAccessor>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::const_iterator
IntrusiveHeap<T, Compare, HeapHandleAccessor>::Update(size_type pos) {
  DCHECK_LT(pos, size());
  MakeHole(pos);

  bool child_greater_eq_parent = false;
  size_type i = 0;
  if (pos > 0) {
    i = intrusive_heap::ParentIndex(pos);
    child_greater_eq_parent = !Less(pos, i);
  }

  if (child_greater_eq_parent) {
    i = MoveHoleUpAndFill(pos, std::move_if_noexcept(impl_.heap_[pos]));
  } else {
    i = MoveHoleDownAndFill<WithElement>(
        pos, std::move_if_noexcept(impl_.heap_[pos]));
  }

  return cbegin() + i;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::swap(
    IntrusiveHeap& other) noexcept {
  std::swap(impl_.get_value_compare(), other.impl_.get_value_compare());
  std::swap(impl_.get_heap_handle_access(),
            other.impl_.get_heap_handle_access());
  std::swap(impl_.heap_, other.impl_.heap_);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::size_type
IntrusiveHeap<T, Compare, HeapHandleAccessor>::ToIndex(const_iterator pos) {
  DCHECK(cbegin() <= pos);
  DCHECK(pos <= cend());
  if (pos == cend())
    return HeapHandle::kInvalidIndex;
  return pos - cbegin();
}

template <typename T, typename Compare, typename HeapHandleAccessor>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::size_type
IntrusiveHeap<T, Compare, HeapHandleAccessor>::ToIndex(
    const_reverse_iterator pos) {
  DCHECK(crbegin() <= pos);
  DCHECK(pos <= crend());
  if (pos == crend())
    return HeapHandle::kInvalidIndex;
  return (pos.base() - cbegin()) - 1;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::SetHeapHandle(size_type i) {
  impl_.get_heap_handle_access().SetHeapHandle(&impl_.heap_[i], HeapHandle(i));
  intrusive_heap::CheckInvalidOrEqualTo(GetHeapHandle(i), i);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::ClearHeapHandle(
    size_type i) {
  impl_.get_heap_handle_access().ClearHeapHandle(&impl_.heap_[i]);
  DCHECK(!GetHeapHandle(i).IsValid());
}

template <typename T, typename Compare, typename HeapHandleAccessor>
HeapHandle IntrusiveHeap<T, Compare, HeapHandleAccessor>::GetHeapHandle(
    size_type i) {
  return impl_.get_heap_handle_access().GetHeapHandle(&impl_.heap_[i]);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
bool IntrusiveHeap<T, Compare, HeapHandleAccessor>::Less(size_type i,
                                                         size_type j) {
  DCHECK_LT(i, size());
  DCHECK_LT(j, size());
  return impl_.get_value_compare()(impl_.heap_[i], impl_.heap_[j]);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
bool IntrusiveHeap<T, Compare, HeapHandleAccessor>::Less(const T& element,
                                                         size_type i) {
  DCHECK_LT(i, size());
  return impl_.get_value_compare()(element, impl_.heap_[i]);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
bool IntrusiveHeap<T, Compare, HeapHandleAccessor>::Less(size_type i,
                                                         const T& element) {
  DCHECK_LT(i, size());
  return impl_.get_value_compare()(impl_.heap_[i], element);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::MakeHole(size_type pos) {
  DCHECK_LT(pos, size());
  ClearHeapHandle(pos);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename U>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::FillHole(size_type hole_pos,
                                                             U element) {


  DCHECK_LE(hole_pos, size());
  if (hole_pos == size()) {
    impl_.heap_.push_back(std::move_if_noexcept(element));
  } else {
    impl_.heap_[hole_pos] = std::move_if_noexcept(element);
  }
  SetHeapHandle(hole_pos);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
void IntrusiveHeap<T, Compare, HeapHandleAccessor>::MoveHole(
    size_type new_hole_pos,
    size_type old_hole_pos) {


  DCHECK_NE(new_hole_pos, old_hole_pos);
  DCHECK_LT(new_hole_pos, size());
  DCHECK_LE(old_hole_pos, size());

  if (old_hole_pos == size()) {
    impl_.heap_.push_back(std::move_if_noexcept(impl_.heap_[new_hole_pos]));
  } else {
    impl_.heap_[old_hole_pos] =
        std::move_if_noexcept(impl_.heap_[new_hole_pos]);
  }
  SetHeapHandle(old_hole_pos);
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename U>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::size_type
IntrusiveHeap<T, Compare, HeapHandleAccessor>::MoveHoleUpAndFill(
    size_type hole_pos,
    U element) {


  DCHECK_LE(hole_pos, size());

  while (hole_pos != 0) {

    size_type parent = intrusive_heap::ParentIndex(hole_pos);
    if (!Less(parent, element))
      break;

    MoveHole(parent, hole_pos);
    hole_pos = parent;
  }

  FillHole(hole_pos, std::move_if_noexcept(element));
  return hole_pos;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename FillElementType, typename U>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::size_type
IntrusiveHeap<T, Compare, HeapHandleAccessor>::MoveHoleDownAndFill(
    size_type hole_pos,
    U element) {
  DCHECK_LT(hole_pos, size());


  const size_type n = size() - (FillElementType::kIsLeafElement ? 1 : 0);

  DCHECK_LT(hole_pos, n);
  DCHECK(!GetHeapHandle(hole_pos).IsValid());

  while (true) {

    size_type left = intrusive_heap::LeftIndex(hole_pos);
    if (left >= n)
      break;
    size_type right = left + 1;

    size_type largest = left;
    if (right < n && Less(left, right))
      largest = right;


    if (!FillElementType::kIsLeafElement && !Less(element, largest))
      break;

    MoveHole(largest, hole_pos);
    hole_pos = largest;
  }

  if (FillElementType::kIsLeafElement) {


    hole_pos = MoveHoleUpAndFill(hole_pos, std::move_if_noexcept(element));
  } else {
    FillHole(hole_pos, std::move_if_noexcept(element));
  }
  return hole_pos;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename U>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::const_iterator
IntrusiveHeap<T, Compare, HeapHandleAccessor>::InsertImpl(U element) {



  size_t i = MoveHoleUpAndFill(size(), std::move_if_noexcept(element));
  return cbegin() + i;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename U>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::const_iterator
IntrusiveHeap<T, Compare, HeapHandleAccessor>::ReplaceImpl(size_type pos,
                                                           U element) {


  MakeHole(pos);
  size_type i = 0;
  if (!Less(element, pos)) {
    i = MoveHoleUpAndFill(pos, std::move_if_noexcept(element));
  } else {
    i = MoveHoleDownAndFill<WithElement>(pos, std::move_if_noexcept(element));
  }
  return cbegin() + i;
}

template <typename T, typename Compare, typename HeapHandleAccessor>
template <typename U>
typename IntrusiveHeap<T, Compare, HeapHandleAccessor>::const_iterator
IntrusiveHeap<T, Compare, HeapHandleAccessor>::ReplaceTopImpl(U element) {
  MakeHole(0u);
  size_type i =
      MoveHoleDownAndFill<WithElement>(0u, std::move_if_noexcept(element));
  return cbegin() + i;
}

// WithHeapHandle

template <typename T>
template <class... Args>
WithHeapHandle<T>::WithHeapHandle(Args&&... args)
    : value_(std::forward<Args>(args)...) {}

template <typename T>
void WithHeapHandle<T>::swap(WithHeapHandle& other) noexcept {
  InternalHeapHandleStorage::swap(other);
  std::swap(value_, other.value_);
}

}  // namespace base

#endif  // BASE_CONTAINERS_INTRUSIVE_HEAP_H_
