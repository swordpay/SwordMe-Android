// Copyright 2019 The Abseil Authors.
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
// File: inlined_vector.h
// -----------------------------------------------------------------------------
//
// This header file contains the declaration and definition of an "inlined
// vector" which behaves in an equivalent fashion to a `std::vector`, except
// that storage for small sequences of the vector are provided inline without
// requiring any heap allocation.
//
// An `absl::InlinedVector<T, N>` specifies the default capacity `N` as one of
// its template parameters. Instances where `size() <= N` hold contained
// elements in inline space. Typically `N` is very small so that sequences that
// are expected to be short do not require allocations.
//
// An `absl::InlinedVector` does not usually require a specific allocator. If
// the inlined vector grows beyond its initial constraints, it will need to
// allocate (as any normal `std::vector` would). This is usually performed with
// the default allocator (defined as `std::allocator<T>`). Optionally, a custom
// allocator type may be specified as `A` in `absl::InlinedVector<T, N, A>`.

#ifndef ABSL_CONTAINER_INLINED_VECTOR_H_
#define ABSL_CONTAINER_INLINED_VECTOR_H_

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "absl/algorithm/algorithm.h"
#include "absl/base/internal/throw_delegate.h"
#include "absl/base/macros.h"
#include "absl/base/optimization.h"
#include "absl/base/port.h"
#include "absl/container/internal/inlined_vector.h"
#include "absl/memory/memory.h"
#include "absl/meta/type_traits.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
// -----------------------------------------------------------------------------
// InlinedVector
// -----------------------------------------------------------------------------
//
// An `absl::InlinedVector` is designed to be a drop-in replacement for
// `std::vector` for use cases where the vector's size is sufficiently small
// that it can be inlined. If the inlined vector does grow beyond its estimated
// capacity, it will trigger an initial allocation on the heap, and will behave
// as a `std::vector`. The API of the `absl::InlinedVector` within this file is
// designed to cover the same API footprint as covered by `std::vector`.
template <typename T, size_t N, typename A = std::allocator<T>>
class InlinedVector {
  static_assert(N > 0, "`absl::InlinedVector` requires an inlined capacity.");

  using Storage = inlined_vector_internal::Storage<T, N, A>;

  template <typename TheA>
  using AllocatorTraits = inlined_vector_internal::AllocatorTraits<TheA>;
  template <typename TheA>
  using MoveIterator = inlined_vector_internal::MoveIterator<TheA>;
  template <typename TheA>
  using IsMemcpyOk = inlined_vector_internal::IsMemcpyOk<TheA>;
  template <typename TheA>
  using IsMoveAssignOk = inlined_vector_internal::IsMoveAssignOk<TheA>;

  template <typename TheA, typename Iterator>
  using IteratorValueAdapter =
      inlined_vector_internal::IteratorValueAdapter<TheA, Iterator>;
  template <typename TheA>
  using CopyValueAdapter = inlined_vector_internal::CopyValueAdapter<TheA>;
  template <typename TheA>
  using DefaultValueAdapter =
      inlined_vector_internal::DefaultValueAdapter<TheA>;

  template <typename Iterator>
  using EnableIfAtLeastForwardIterator = absl::enable_if_t<
      inlined_vector_internal::IsAtLeastForwardIterator<Iterator>::value, int>;
  template <typename Iterator>
  using DisableIfAtLeastForwardIterator = absl::enable_if_t<
      !inlined_vector_internal::IsAtLeastForwardIterator<Iterator>::value, int>;

  using MemcpyPolicy = typename Storage::MemcpyPolicy;
  using ElementwiseAssignPolicy = typename Storage::ElementwiseAssignPolicy;
  using ElementwiseConstructPolicy =
      typename Storage::ElementwiseConstructPolicy;
  using MoveAssignmentPolicy = typename Storage::MoveAssignmentPolicy;

 public:
  using allocator_type = A;
  using value_type = inlined_vector_internal::ValueType<A>;
  using pointer = inlined_vector_internal::Pointer<A>;
  using const_pointer = inlined_vector_internal::ConstPointer<A>;
  using size_type = inlined_vector_internal::SizeType<A>;
  using difference_type = inlined_vector_internal::DifferenceType<A>;
  using reference = inlined_vector_internal::Reference<A>;
  using const_reference = inlined_vector_internal::ConstReference<A>;
  using iterator = inlined_vector_internal::Iterator<A>;
  using const_iterator = inlined_vector_internal::ConstIterator<A>;
  using reverse_iterator = inlined_vector_internal::ReverseIterator<A>;
  using const_reverse_iterator =
      inlined_vector_internal::ConstReverseIterator<A>;




  InlinedVector() noexcept(noexcept(allocator_type())) : storage_() {}

  explicit InlinedVector(const allocator_type& allocator) noexcept
      : storage_(allocator) {}

  explicit InlinedVector(size_type n,
                         const allocator_type& allocator = allocator_type())
      : storage_(allocator) {
    storage_.Initialize(DefaultValueAdapter<A>(), n);
  }

  InlinedVector(size_type n, const_reference v,
                const allocator_type& allocator = allocator_type())
      : storage_(allocator) {
    storage_.Initialize(CopyValueAdapter<A>(std::addressof(v)), n);
  }

  InlinedVector(std::initializer_list<value_type> list,
                const allocator_type& allocator = allocator_type())
      : InlinedVector(list.begin(), list.end(), allocator) {}






  template <typename ForwardIterator,
            EnableIfAtLeastForwardIterator<ForwardIterator> = 0>
  InlinedVector(ForwardIterator first, ForwardIterator last,
                const allocator_type& allocator = allocator_type())
      : storage_(allocator) {
    storage_.Initialize(IteratorValueAdapter<A, ForwardIterator>(first),
                        static_cast<size_t>(std::distance(first, last)));
  }


  template <typename InputIterator,
            DisableIfAtLeastForwardIterator<InputIterator> = 0>
  InlinedVector(InputIterator first, InputIterator last,
                const allocator_type& allocator = allocator_type())
      : storage_(allocator) {
    std::copy(first, last, std::back_inserter(*this));
  }


  InlinedVector(const InlinedVector& other)
      : InlinedVector(other, other.storage_.GetAllocator()) {}


  InlinedVector(const InlinedVector& other, const allocator_type& allocator)
      : storage_(allocator) {
    if (other.empty()) {

    } else if (IsMemcpyOk<A>::value && !other.storage_.GetIsAllocated()) {

      storage_.MemcpyFrom(other.storage_);
    } else {
      storage_.InitFrom(other.storage_);
    }
  }














  InlinedVector(InlinedVector&& other) noexcept(
      absl::allocator_is_nothrow<allocator_type>::value ||
      std::is_nothrow_move_constructible<value_type>::value)
      : storage_(other.storage_.GetAllocator()) {
    if (IsMemcpyOk<A>::value) {
      storage_.MemcpyFrom(other.storage_);

      other.storage_.SetInlinedSize(0);
    } else if (other.storage_.GetIsAllocated()) {
      storage_.SetAllocation({other.storage_.GetAllocatedData(),
                              other.storage_.GetAllocatedCapacity()});
      storage_.SetAllocatedSize(other.storage_.GetSize());

      other.storage_.SetInlinedSize(0);
    } else {
      IteratorValueAdapter<A, MoveIterator<A>> other_values(
          MoveIterator<A>(other.storage_.GetInlinedData()));

      inlined_vector_internal::ConstructElements<A>(
          storage_.GetAllocator(), storage_.GetInlinedData(), other_values,
          other.storage_.GetSize());

      storage_.SetInlinedSize(other.storage_.GetSize());
    }
  }







  InlinedVector(
      InlinedVector&& other,
      const allocator_type&
          allocator) noexcept(absl::allocator_is_nothrow<allocator_type>::value)
      : storage_(allocator) {
    if (IsMemcpyOk<A>::value) {
      storage_.MemcpyFrom(other.storage_);

      other.storage_.SetInlinedSize(0);
    } else if ((storage_.GetAllocator() == other.storage_.GetAllocator()) &&
               other.storage_.GetIsAllocated()) {
      storage_.SetAllocation({other.storage_.GetAllocatedData(),
                              other.storage_.GetAllocatedCapacity()});
      storage_.SetAllocatedSize(other.storage_.GetSize());

      other.storage_.SetInlinedSize(0);
    } else {
      storage_.Initialize(IteratorValueAdapter<A, MoveIterator<A>>(
                              MoveIterator<A>(other.data())),
                          other.size());
    }
  }

  ~InlinedVector() {}






  bool empty() const noexcept { return !size(); }



  size_type size() const noexcept { return storage_.GetSize(); }



  size_type max_size() const noexcept {




    return (std::min)(AllocatorTraits<A>::max_size(storage_.GetAllocator()),
                      (std::numeric_limits<size_type>::max)() / 2);
  }









  size_type capacity() const noexcept {
    return storage_.GetIsAllocated() ? storage_.GetAllocatedCapacity()
                                     : storage_.GetInlinedCapacity();
  }






  pointer data() noexcept {
    return storage_.GetIsAllocated() ? storage_.GetAllocatedData()
                                     : storage_.GetInlinedData();
  }





  const_pointer data() const noexcept {
    return storage_.GetIsAllocated() ? storage_.GetAllocatedData()
                                     : storage_.GetInlinedData();
  }



  reference operator[](size_type i) {
    ABSL_HARDENING_ASSERT(i < size());
    return data()[i];
  }


  const_reference operator[](size_type i) const {
    ABSL_HARDENING_ASSERT(i < size());
    return data()[i];
  }






  reference at(size_type i) {
    if (ABSL_PREDICT_FALSE(i >= size())) {
      base_internal::ThrowStdOutOfRange(
          "`InlinedVector::at(size_type)` failed bounds check");
    }
    return data()[i];
  }





  const_reference at(size_type i) const {
    if (ABSL_PREDICT_FALSE(i >= size())) {
      base_internal::ThrowStdOutOfRange(
          "`InlinedVector::at(size_type) const` failed bounds check");
    }
    return data()[i];
  }



  reference front() {
    ABSL_HARDENING_ASSERT(!empty());
    return data()[0];
  }


  const_reference front() const {
    ABSL_HARDENING_ASSERT(!empty());
    return data()[0];
  }



  reference back() {
    ABSL_HARDENING_ASSERT(!empty());
    return data()[size() - 1];
  }


  const_reference back() const {
    ABSL_HARDENING_ASSERT(!empty());
    return data()[size() - 1];
  }



  iterator begin() noexcept { return data(); }


  const_iterator begin() const noexcept { return data(); }



  iterator end() noexcept { return data() + size(); }


  const_iterator end() const noexcept { return data() + size(); }



  const_iterator cbegin() const noexcept { return begin(); }



  const_iterator cend() const noexcept { return end(); }



  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }


  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }



  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }


  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }



  const_reverse_iterator crbegin() const noexcept { return rbegin(); }




  const_reverse_iterator crend() const noexcept { return rend(); }



  allocator_type get_allocator() const { return storage_.GetAllocator(); }







  InlinedVector& operator=(std::initializer_list<value_type> list) {
    assign(list.begin(), list.end());

    return *this;
  }


  InlinedVector& operator=(const InlinedVector& other) {
    if (ABSL_PREDICT_TRUE(this != std::addressof(other))) {
      const_pointer other_data = other.data();
      assign(other_data, other_data + other.size());
    }

    return *this;
  }





  InlinedVector& operator=(InlinedVector&& other) {
    if (ABSL_PREDICT_TRUE(this != std::addressof(other))) {
      MoveAssignment(MoveAssignmentPolicy{}, std::move(other));
    }

    return *this;
  }



  void assign(size_type n, const_reference v) {
    storage_.Assign(CopyValueAdapter<A>(std::addressof(v)), n);
  }


  void assign(std::initializer_list<value_type> list) {
    assign(list.begin(), list.end());
  }




  template <typename ForwardIterator,
            EnableIfAtLeastForwardIterator<ForwardIterator> = 0>
  void assign(ForwardIterator first, ForwardIterator last) {
    storage_.Assign(IteratorValueAdapter<A, ForwardIterator>(first),
                    static_cast<size_t>(std::distance(first, last)));
  }




  template <typename InputIterator,
            DisableIfAtLeastForwardIterator<InputIterator> = 0>
  void assign(InputIterator first, InputIterator last) {
    size_type i = 0;
    for (; i < size() && first != last; ++i, static_cast<void>(++first)) {
      data()[i] = *first;
    }

    erase(data() + i, data() + size());
    std::copy(first, last, std::back_inserter(*this));
  }






  void resize(size_type n) {
    ABSL_HARDENING_ASSERT(n <= max_size());
    storage_.Resize(DefaultValueAdapter<A>(), n);
  }





  void resize(size_type n, const_reference v) {
    ABSL_HARDENING_ASSERT(n <= max_size());
    storage_.Resize(CopyValueAdapter<A>(std::addressof(v)), n);
  }




  iterator insert(const_iterator pos, const_reference v) {
    return emplace(pos, v);
  }


  iterator insert(const_iterator pos, value_type&& v) {
    return emplace(pos, std::move(v));
  }



  iterator insert(const_iterator pos, size_type n, const_reference v) {
    ABSL_HARDENING_ASSERT(pos >= begin());
    ABSL_HARDENING_ASSERT(pos <= end());

    if (ABSL_PREDICT_TRUE(n != 0)) {
      value_type dealias = v;





#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
      return storage_.Insert(pos, CopyValueAdapter<A>(std::addressof(dealias)),
                             n);
#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    } else {
      return const_cast<iterator>(pos);
    }
  }



  iterator insert(const_iterator pos, std::initializer_list<value_type> list) {
    return insert(pos, list.begin(), list.end());
  }





  template <typename ForwardIterator,
            EnableIfAtLeastForwardIterator<ForwardIterator> = 0>
  iterator insert(const_iterator pos, ForwardIterator first,
                  ForwardIterator last) {
    ABSL_HARDENING_ASSERT(pos >= begin());
    ABSL_HARDENING_ASSERT(pos <= end());

    if (ABSL_PREDICT_TRUE(first != last)) {
      return storage_.Insert(
          pos, IteratorValueAdapter<A, ForwardIterator>(first),
          static_cast<size_type>(std::distance(first, last)));
    } else {
      return const_cast<iterator>(pos);
    }
  }





  template <typename InputIterator,
            DisableIfAtLeastForwardIterator<InputIterator> = 0>
  iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
    ABSL_HARDENING_ASSERT(pos >= begin());
    ABSL_HARDENING_ASSERT(pos <= end());

    size_type index = static_cast<size_type>(std::distance(cbegin(), pos));
    for (size_type i = index; first != last; ++i, static_cast<void>(++first)) {
      insert(data() + i, *first);
    }

    return iterator(data() + index);
  }




  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    ABSL_HARDENING_ASSERT(pos >= begin());
    ABSL_HARDENING_ASSERT(pos <= end());

    value_type dealias(std::forward<Args>(args)...);





#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    return storage_.Insert(pos,
                           IteratorValueAdapter<A, MoveIterator<A>>(
                               MoveIterator<A>(std::addressof(dealias))),
                           1);
#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
  }




  template <typename... Args>
  reference emplace_back(Args&&... args) {
    return storage_.EmplaceBack(std::forward<Args>(args)...);
  }



  void push_back(const_reference v) { static_cast<void>(emplace_back(v)); }


  void push_back(value_type&& v) {
    static_cast<void>(emplace_back(std::move(v)));
  }



  void pop_back() noexcept {
    ABSL_HARDENING_ASSERT(!empty());

    AllocatorTraits<A>::destroy(storage_.GetAllocator(), data() + (size() - 1));
    storage_.SubtractSize(1);
  }






  iterator erase(const_iterator pos) {
    ABSL_HARDENING_ASSERT(pos >= begin());
    ABSL_HARDENING_ASSERT(pos < end());

    return storage_.Erase(pos, pos + 1);
  }





  iterator erase(const_iterator from, const_iterator to) {
    ABSL_HARDENING_ASSERT(from >= begin());
    ABSL_HARDENING_ASSERT(from <= to);
    ABSL_HARDENING_ASSERT(to <= end());

    if (ABSL_PREDICT_TRUE(from != to)) {
      return storage_.Erase(from, to);
    } else {
      return const_cast<iterator>(from);
    }
  }




  void clear() noexcept {
    inlined_vector_internal::DestroyAdapter<A>::DestroyElements(
        storage_.GetAllocator(), data(), size());
    storage_.DeallocateIfAllocated();

    storage_.SetInlinedSize(0);
  }



  void reserve(size_type n) { storage_.Reserve(n); }








  void shrink_to_fit() {
    if (storage_.GetIsAllocated()) {
      storage_.ShrinkToFit();
    }
  }



  void swap(InlinedVector& other) {
    if (ABSL_PREDICT_TRUE(this != std::addressof(other))) {
      storage_.Swap(std::addressof(other.storage_));
    }
  }

 private:
  template <typename H, typename TheT, size_t TheN, typename TheA>
  friend H AbslHashValue(H h, const absl::InlinedVector<TheT, TheN, TheA>& a);

  void MoveAssignment(MemcpyPolicy, InlinedVector&& other) {
    inlined_vector_internal::DestroyAdapter<A>::DestroyElements(
        storage_.GetAllocator(), data(), size());
    storage_.DeallocateIfAllocated();
    storage_.MemcpyFrom(other.storage_);

    other.storage_.SetInlinedSize(0);
  }

  void MoveAssignment(ElementwiseAssignPolicy, InlinedVector&& other) {
    if (other.storage_.GetIsAllocated()) {
      MoveAssignment(MemcpyPolicy{}, std::move(other));
    } else {
      storage_.Assign(IteratorValueAdapter<A, MoveIterator<A>>(
                          MoveIterator<A>(other.storage_.GetInlinedData())),
                      other.size());
    }
  }

  void MoveAssignment(ElementwiseConstructPolicy, InlinedVector&& other) {
    if (other.storage_.GetIsAllocated()) {
      MoveAssignment(MemcpyPolicy{}, std::move(other));
    } else {
      inlined_vector_internal::DestroyAdapter<A>::DestroyElements(
          storage_.GetAllocator(), data(), size());
      storage_.DeallocateIfAllocated();

      IteratorValueAdapter<A, MoveIterator<A>> other_values(
          MoveIterator<A>(other.storage_.GetInlinedData()));
      inlined_vector_internal::ConstructElements<A>(
          storage_.GetAllocator(), storage_.GetInlinedData(), other_values,
          other.storage_.GetSize());
      storage_.SetInlinedSize(other.storage_.GetSize());
    }
  }

  Storage storage_;
};

// InlinedVector Non-Member Functions
// -----------------------------------------------------------------------------

//
// Swaps the contents of two inlined vectors.
template <typename T, size_t N, typename A>
void swap(absl::InlinedVector<T, N, A>& a,
          absl::InlinedVector<T, N, A>& b) noexcept(noexcept(a.swap(b))) {
  a.swap(b);
}

//
// Tests for value-equality of two inlined vectors.
template <typename T, size_t N, typename A>
bool operator==(const absl::InlinedVector<T, N, A>& a,
                const absl::InlinedVector<T, N, A>& b) {
  auto a_data = a.data();
  auto b_data = b.data();
  return absl::equal(a_data, a_data + a.size(), b_data, b_data + b.size());
}

//
// Tests for value-inequality of two inlined vectors.
template <typename T, size_t N, typename A>
bool operator!=(const absl::InlinedVector<T, N, A>& a,
                const absl::InlinedVector<T, N, A>& b) {
  return !(a == b);
}

//
// Tests whether the value of an inlined vector is less than the value of
// another inlined vector using a lexicographical comparison algorithm.
template <typename T, size_t N, typename A>
bool operator<(const absl::InlinedVector<T, N, A>& a,
               const absl::InlinedVector<T, N, A>& b) {
  auto a_data = a.data();
  auto b_data = b.data();
  return std::lexicographical_compare(a_data, a_data + a.size(), b_data,
                                      b_data + b.size());
}

//
// Tests whether the value of an inlined vector is greater than the value of
// another inlined vector using a lexicographical comparison algorithm.
template <typename T, size_t N, typename A>
bool operator>(const absl::InlinedVector<T, N, A>& a,
               const absl::InlinedVector<T, N, A>& b) {
  return b < a;
}

//
// Tests whether the value of an inlined vector is less than or equal to the
// value of another inlined vector using a lexicographical comparison algorithm.
template <typename T, size_t N, typename A>
bool operator<=(const absl::InlinedVector<T, N, A>& a,
                const absl::InlinedVector<T, N, A>& b) {
  return !(b < a);
}

//
// Tests whether the value of an inlined vector is greater than or equal to the
// value of another inlined vector using a lexicographical comparison algorithm.
template <typename T, size_t N, typename A>
bool operator>=(const absl::InlinedVector<T, N, A>& a,
                const absl::InlinedVector<T, N, A>& b) {
  return !(a < b);
}

//
// Provides `absl::Hash` support for `absl::InlinedVector`. It is uncommon to
// call this directly.
template <typename H, typename T, size_t N, typename A>
H AbslHashValue(H h, const absl::InlinedVector<T, N, A>& a) {
  auto size = a.size();
  return H::combine(H::combine_contiguous(std::move(h), a.data(), size), size);
}

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_INLINED_VECTOR_H_
