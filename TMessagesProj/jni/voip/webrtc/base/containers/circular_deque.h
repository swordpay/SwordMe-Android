// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINERS_CIRCULAR_DEQUE_H_
#define BASE_CONTAINERS_CIRCULAR_DEQUE_H_

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include "base/containers/vector_buffer.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/stl_util.h"
#include "base/template_util.h"

// storage is provided in a flat circular buffer conceptually similar to a
// vector. The beginning and end will wrap around as necessary so that
// pushes and pops will be constant time as long as a capacity expansion is
// not required.
//
// The API should be identical to std::deque with the following differences:
//
//  - ITERATORS ARE NOT STABLE. Mutating the container will invalidate all
//    iterators.
//
//  - Insertions may resize the vector and so are not constant time (std::deque
//    guarantees constant time for insertions at the ends).
//
//  - Container-wide comparisons are not implemented. If you want to compare
//    two containers, use an algorithm so the expensive iteration is explicit.
//
// If you want a similar container with only a queue API, use base::queue in
// base/containers/queue.h.
//
// Constructors:
//   circular_deque();
//   circular_deque(size_t count);
//   circular_deque(size_t count, const T& value);
//   circular_deque(InputIterator first, InputIterator last);
//   circular_deque(const circular_deque&);
//   circular_deque(circular_deque&&);
//   circular_deque(std::initializer_list<value_type>);
//
// Assignment functions:
//   circular_deque& operator=(const circular_deque&);
//   circular_deque& operator=(circular_deque&&);
//   circular_deque& operator=(std::initializer_list<T>);
//   void assign(size_t count, const T& value);
//   void assign(InputIterator first, InputIterator last);
//   void assign(std::initializer_list<T> value);
//
// Random accessors:
//   T& at(size_t);
//   const T& at(size_t) const;
//   T& operator[](size_t);
//   const T& operator[](size_t) const;
//
// End accessors:
//   T& front();
//   const T& front() const;
//   T& back();
//   const T& back() const;
//
// Iterator functions:
//   iterator               begin();
//   const_iterator         begin() const;
//   const_iterator         cbegin() const;
//   iterator               end();
//   const_iterator         end() const;
//   const_iterator         cend() const;
//   reverse_iterator       rbegin();
//   const_reverse_iterator rbegin() const;
//   const_reverse_iterator crbegin() const;
//   reverse_iterator       rend();
//   const_reverse_iterator rend() const;
//   const_reverse_iterator crend() const;
//
// Memory management:
//   void reserve(size_t);  // SEE IMPLEMENTATION FOR SOME GOTCHAS.
//   size_t capacity() const;
//   void shrink_to_fit();
//
// Size management:
//   void clear();
//   bool empty() const;
//   size_t size() const;
//   void resize(size_t);
//   void resize(size_t count, const T& value);
//
// Positional insert and erase:
//   void insert(const_iterator pos, size_type count, const T& value);
//   void insert(const_iterator pos,
//               InputIterator first, InputIterator last);
//   iterator insert(const_iterator pos, const T& value);
//   iterator insert(const_iterator pos, T&& value);
//   iterator emplace(const_iterator pos, Args&&... args);
//   iterator erase(const_iterator pos);
//   iterator erase(const_iterator first, const_iterator last);
//
// End insert and erase:
//   void push_front(const T&);
//   void push_front(T&&);
//   void push_back(const T&);
//   void push_back(T&&);
//   T& emplace_front(Args&&...);
//   T& emplace_back(Args&&...);
//   void pop_front();
//   void pop_back();
//
// General:
//   void swap(circular_deque&);

namespace base {

template <class T>
class circular_deque;

namespace internal {

// external capacity so the internal buffer will be one larger (= 4) which is
// more even for the allocator. See the descriptions of internal vs. external
// capacity on the comment above the buffer_ variable below.
constexpr size_t kCircularBufferInitialCapacity = 3;

template <typename T>
class circular_deque_const_iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = const T*;
  using reference = const T&;
  using iterator_category = std::random_access_iterator_tag;

  circular_deque_const_iterator() : parent_deque_(nullptr), index_(0) {
#if DCHECK_IS_ON()
    created_generation_ = 0;
#endif  // DCHECK_IS_ON()
  }

  const T& operator*() const {
    CheckUnstableUsage();
    parent_deque_->CheckValidIndex(index_);
    return parent_deque_->buffer_[index_];
  }
  const T* operator->() const {
    CheckUnstableUsage();
    parent_deque_->CheckValidIndex(index_);
    return &parent_deque_->buffer_[index_];
  }
  const value_type& operator[](difference_type i) const { return *(*this + i); }

  circular_deque_const_iterator& operator++() {
    Increment();
    return *this;
  }
  circular_deque_const_iterator operator++(int) {
    circular_deque_const_iterator ret = *this;
    Increment();
    return ret;
  }
  circular_deque_const_iterator& operator--() {
    Decrement();
    return *this;
  }
  circular_deque_const_iterator operator--(int) {
    circular_deque_const_iterator ret = *this;
    Decrement();
    return ret;
  }

  friend circular_deque_const_iterator operator+(
      const circular_deque_const_iterator& iter,
      difference_type offset) {
    circular_deque_const_iterator ret = iter;
    ret.Add(offset);
    return ret;
  }
  circular_deque_const_iterator& operator+=(difference_type offset) {
    Add(offset);
    return *this;
  }
  friend circular_deque_const_iterator operator-(
      const circular_deque_const_iterator& iter,
      difference_type offset) {
    circular_deque_const_iterator ret = iter;
    ret.Add(-offset);
    return ret;
  }
  circular_deque_const_iterator& operator-=(difference_type offset) {
    Add(-offset);
    return *this;
  }

  friend std::ptrdiff_t operator-(const circular_deque_const_iterator& lhs,
                                  const circular_deque_const_iterator& rhs) {
    lhs.CheckComparable(rhs);
    return lhs.OffsetFromBegin() - rhs.OffsetFromBegin();
  }

  friend bool operator==(const circular_deque_const_iterator& lhs,
                         const circular_deque_const_iterator& rhs) {
    lhs.CheckComparable(rhs);
    return lhs.index_ == rhs.index_;
  }
  friend bool operator!=(const circular_deque_const_iterator& lhs,
                         const circular_deque_const_iterator& rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(const circular_deque_const_iterator& lhs,
                        const circular_deque_const_iterator& rhs) {
    lhs.CheckComparable(rhs);
    return lhs.OffsetFromBegin() < rhs.OffsetFromBegin();
  }
  friend bool operator<=(const circular_deque_const_iterator& lhs,
                         const circular_deque_const_iterator& rhs) {
    return !(lhs > rhs);
  }
  friend bool operator>(const circular_deque_const_iterator& lhs,
                        const circular_deque_const_iterator& rhs) {
    lhs.CheckComparable(rhs);
    return lhs.OffsetFromBegin() > rhs.OffsetFromBegin();
  }
  friend bool operator>=(const circular_deque_const_iterator& lhs,
                         const circular_deque_const_iterator& rhs) {
    return !(lhs < rhs);
  }

 protected:
  friend class circular_deque<T>;

  circular_deque_const_iterator(const circular_deque<T>* parent, size_t index)
      : parent_deque_(parent), index_(index) {
#if DCHECK_IS_ON()
    created_generation_ = parent->generation_;
#endif  // DCHECK_IS_ON()
  }


  size_t OffsetFromBegin() const {
    if (index_ >= parent_deque_->begin_)
      return index_ - parent_deque_->begin_;  // On the same side as begin.
    return parent_deque_->buffer_.capacity() - parent_deque_->begin_ + index_;
  }

  void Increment() {
    CheckUnstableUsage();
    parent_deque_->CheckValidIndex(index_);
    index_++;
    if (index_ == parent_deque_->buffer_.capacity())
      index_ = 0;
  }
  void Decrement() {
    CheckUnstableUsage();
    parent_deque_->CheckValidIndexOrEnd(index_);
    if (index_ == 0)
      index_ = parent_deque_->buffer_.capacity() - 1;
    else
      index_--;
  }
  void Add(difference_type delta) {
    CheckUnstableUsage();
#if DCHECK_IS_ON()
    if (delta <= 0)
      parent_deque_->CheckValidIndexOrEnd(index_);
    else
      parent_deque_->CheckValidIndex(index_);
#endif




    if (delta == 0)
      return;

    difference_type new_offset = OffsetFromBegin() + delta;
    DCHECK(new_offset >= 0 &&
           new_offset <= static_cast<difference_type>(parent_deque_->size()));
    index_ = (new_offset + parent_deque_->begin_) %
             parent_deque_->buffer_.capacity();
  }

#if DCHECK_IS_ON()
  void CheckUnstableUsage() const {
    DCHECK(parent_deque_);



    DCHECK_EQ(created_generation_, parent_deque_->generation_)
        << "circular_deque iterator dereferenced after mutation.";
  }
  void CheckComparable(const circular_deque_const_iterator& other) const {
    DCHECK_EQ(parent_deque_, other.parent_deque_);




    DCHECK_EQ(created_generation_, other.created_generation_);
  }
#else
  inline void CheckUnstableUsage() const {}
  inline void CheckComparable(const circular_deque_const_iterator&) const {}
#endif  // DCHECK_IS_ON()

  const circular_deque<T>* parent_deque_;
  size_t index_;

#if DCHECK_IS_ON()



  uint64_t created_generation_;
#endif  // DCHECK_IS_ON()
};

template <typename T>
class circular_deque_iterator : public circular_deque_const_iterator<T> {
  using base = circular_deque_const_iterator<T>;

 public:
  friend class circular_deque<T>;

  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::random_access_iterator_tag;

  circular_deque_iterator() : circular_deque_const_iterator<T>() {}

  T& operator*() const { return const_cast<T&>(base::operator*()); }
  T* operator->() const { return const_cast<T*>(base::operator->()); }
  T& operator[](difference_type i) {
    return const_cast<T&>(base::operator[](i));
  }

  friend circular_deque_iterator operator+(const circular_deque_iterator& iter,
                                           difference_type offset) {
    circular_deque_iterator ret = iter;
    ret.Add(offset);
    return ret;
  }
  circular_deque_iterator& operator+=(difference_type offset) {
    base::Add(offset);
    return *this;
  }
  friend circular_deque_iterator operator-(const circular_deque_iterator& iter,
                                           difference_type offset) {
    circular_deque_iterator ret = iter;
    ret.Add(-offset);
    return ret;
  }
  circular_deque_iterator& operator-=(difference_type offset) {
    base::Add(-offset);
    return *this;
  }

  circular_deque_iterator& operator++() {
    base::Increment();
    return *this;
  }
  circular_deque_iterator operator++(int) {
    circular_deque_iterator ret = *this;
    base::Increment();
    return ret;
  }
  circular_deque_iterator& operator--() {
    base::Decrement();
    return *this;
  }
  circular_deque_iterator operator--(int) {
    circular_deque_iterator ret = *this;
    base::Decrement();
    return ret;
  }

 private:
  circular_deque_iterator(const circular_deque<T>* parent, size_t index)
      : circular_deque_const_iterator<T>(parent, index) {}
};

}  // namespace internal

template <typename T>
class circular_deque {
 private:
  using VectorBuffer = internal::VectorBuffer<T>;

 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  using iterator = internal::circular_deque_iterator<T>;
  using const_iterator = internal::circular_deque_const_iterator<T>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;



  constexpr circular_deque() = default;

  circular_deque(size_type count) { resize(count); }
  circular_deque(size_type count, const T& value) { resize(count, value); }

  template <class InputIterator>
  circular_deque(InputIterator first, InputIterator last) {
    assign(first, last);
  }

  circular_deque(const circular_deque& other) : buffer_(other.size() + 1) {
    assign(other.begin(), other.end());
  }
  circular_deque(circular_deque&& other) noexcept
      : buffer_(std::move(other.buffer_)),
        begin_(other.begin_),
        end_(other.end_) {
    other.begin_ = 0;
    other.end_ = 0;
  }

  circular_deque(std::initializer_list<value_type> init) { assign(init); }

  ~circular_deque() { DestructRange(begin_, end_); }





  circular_deque& operator=(const circular_deque& other) {
    if (&other == this)
      return *this;

    reserve(other.size());
    assign(other.begin(), other.end());
    return *this;
  }
  circular_deque& operator=(circular_deque&& other) noexcept {
    if (&other == this)
      return *this;


    ClearRetainCapacity();
    buffer_ = std::move(other.buffer_);
    begin_ = other.begin_;
    end_ = other.end_;

    other.begin_ = 0;
    other.end_ = 0;

    IncrementGeneration();
    return *this;
  }
  circular_deque& operator=(std::initializer_list<value_type> ilist) {
    reserve(ilist.size());
    assign(std::begin(ilist), std::end(ilist));
    return *this;
  }

  void assign(size_type count, const value_type& value) {
    ClearRetainCapacity();
    reserve(count);
    for (size_t i = 0; i < count; i++)
      emplace_back(value);
    IncrementGeneration();
  }

  template <typename InputIterator>
  typename std::enable_if<::base::internal::is_iterator<InputIterator>::value,
                          void>::type
  assign(InputIterator first, InputIterator last) {



    ClearRetainCapacity();
    for (; first != last; ++first)
      emplace_back(*first);
    IncrementGeneration();
  }

  void assign(std::initializer_list<value_type> value) {
    reserve(std::distance(value.begin(), value.end()));
    assign(value.begin(), value.end());
  }





  const value_type& at(size_type i) const {
    DCHECK(i < size());
    size_t right_size = buffer_.capacity() - begin_;
    if (begin_ <= end_ || i < right_size)
      return buffer_[begin_ + i];
    return buffer_[i - right_size];
  }
  value_type& at(size_type i) {
    return const_cast<value_type&>(as_const(*this).at(i));
  }

  value_type& operator[](size_type i) {
    return const_cast<value_type&>(as_const(*this)[i]);
  }

  const value_type& operator[](size_type i) const { return at(i); }

  value_type& front() {
    DCHECK(!empty());
    return buffer_[begin_];
  }
  const value_type& front() const {
    DCHECK(!empty());
    return buffer_[begin_];
  }

  value_type& back() {
    DCHECK(!empty());
    return *(--end());
  }
  const value_type& back() const {
    DCHECK(!empty());
    return *(--end());
  }



  iterator begin() { return iterator(this, begin_); }
  const_iterator begin() const { return const_iterator(this, begin_); }
  const_iterator cbegin() const { return const_iterator(this, begin_); }

  iterator end() { return iterator(this, end_); }
  const_iterator end() const { return const_iterator(this, end_); }
  const_iterator cend() const { return const_iterator(this, end_); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const { return rbegin(); }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const { return rend(); }










  void reserve(size_type new_capacity) {
    if (new_capacity > capacity())
      SetCapacityTo(new_capacity);
  }

  size_type capacity() const {

    return buffer_.capacity() == 0 ? 0 : buffer_.capacity() - 1;
  }

  void shrink_to_fit() {
    if (empty()) {


      if (buffer_.capacity())
        buffer_ = VectorBuffer();
    } else {
      SetCapacityTo(size());
    }
  }



  void clear() {


    ClearRetainCapacity();
    buffer_ = VectorBuffer();
  }

  bool empty() const { return begin_ == end_; }

  size_type size() const {
    if (begin_ <= end_)
      return end_ - begin_;
    return buffer_.capacity() - begin_ + end_;
  }











  void resize(size_type count) {

    if (count > size()) {




      ExpandCapacityIfNecessary(count - size());
      while (size() < count)
        emplace_back();
    } else if (count < size()) {
      size_t new_end = (begin_ + count) % buffer_.capacity();
      DestructRange(new_end, end_);
      end_ = new_end;

      ShrinkCapacityIfNecessary();
    }
    IncrementGeneration();
  }
  void resize(size_type count, const value_type& value) {

    if (count > size()) {
      ExpandCapacityIfNecessary(count - size());
      while (size() < count)
        emplace_back(value);
    } else if (count < size()) {
      size_t new_end = (begin_ + count) % buffer_.capacity();
      DestructRange(new_end, end_);
      end_ = new_end;

      ShrinkCapacityIfNecessary();
    }
    IncrementGeneration();
  }














  void insert(const_iterator pos, size_type count, const T& value) {
    ValidateIterator(pos);

    if (pos == begin()) {
      ExpandCapacityIfNecessary(count);
      for (size_t i = 0; i < count; i++)
        push_front(value);
      return;
    }

    iterator insert_cur(this, pos.index_);
    iterator insert_end;
    MakeRoomFor(count, &insert_cur, &insert_end);
    while (insert_cur < insert_end) {
      new (&buffer_[insert_cur.index_]) T(value);
      ++insert_cur;
    }

    IncrementGeneration();
  }


  template <class InputIterator>
  typename std::enable_if<::base::internal::is_iterator<InputIterator>::value,
                          void>::type
  insert(const_iterator pos, InputIterator first, InputIterator last) {
    ValidateIterator(pos);

    size_t inserted_items = std::distance(first, last);
    if (inserted_items == 0)
      return;  // Can divide by 0 when doing modulo below, so return early.

    iterator insert_cur;
    iterator insert_end;
    if (pos == begin()) {


      ExpandCapacityIfNecessary(inserted_items);
      insert_end = begin();
      begin_ =
          (begin_ + buffer_.capacity() - inserted_items) % buffer_.capacity();
      insert_cur = begin();
    } else {
      insert_cur = iterator(this, pos.index_);
      MakeRoomFor(inserted_items, &insert_cur, &insert_end);
    }

    while (insert_cur < insert_end) {
      new (&buffer_[insert_cur.index_]) T(*first);
      ++insert_cur;
      ++first;
    }

    IncrementGeneration();
  }


  iterator insert(const_iterator pos, const T& value) {
    return emplace(pos, value);
  }
  iterator insert(const_iterator pos, T&& value) {
    return emplace(pos, std::move(value));
  }
  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    ValidateIterator(pos);

    if (pos == cbegin()) {
      emplace_front(std::forward<Args>(args)...);
      return begin();
    }

    IncrementGeneration();

    iterator insert_begin(this, pos.index_);
    iterator insert_end;
    MakeRoomFor(1, &insert_begin, &insert_end);
    new (&buffer_[insert_begin.index_]) T(std::forward<Args>(args)...);

    return insert_begin;
  }






  iterator erase(const_iterator pos) { return erase(pos, pos + 1); }
  iterator erase(const_iterator first, const_iterator last) {
    ValidateIterator(first);
    ValidateIterator(last);

    IncrementGeneration();

    if (first.index_ == last.index_) {


      return iterator(this, first.index_);
    } else if (first.index_ < last.index_) {

      buffer_.DestructRange(&buffer_[first.index_], &buffer_[last.index_]);
    } else {

      buffer_.DestructRange(&buffer_[first.index_],
                            &buffer_[buffer_.capacity()]);
      buffer_.DestructRange(&buffer_[0], &buffer_[last.index_]);
    }

    if (first.index_ == begin_) {


      begin_ = last.index_;
      return iterator(this, last.index_);
    }


    iterator move_src(this, last.index_);
    iterator move_src_end = end();
    iterator move_dest(this, first.index_);
    for (; move_src < move_src_end; move_src++, move_dest++) {
      buffer_.MoveRange(&buffer_[move_src.index_],
                        &buffer_[move_src.index_ + 1],
                        &buffer_[move_dest.index_]);
    }

    end_ = move_dest.index_;



    return iterator(this, first.index_);
  }



  void push_front(const T& value) { emplace_front(value); }
  void push_front(T&& value) { emplace_front(std::move(value)); }

  void push_back(const T& value) { emplace_back(value); }
  void push_back(T&& value) { emplace_back(std::move(value)); }

  template <class... Args>
  reference emplace_front(Args&&... args) {
    ExpandCapacityIfNecessary(1);
    if (begin_ == 0)
      begin_ = buffer_.capacity() - 1;
    else
      begin_--;
    IncrementGeneration();
    new (&buffer_[begin_]) T(std::forward<Args>(args)...);
    return front();
  }

  template <class... Args>
  reference emplace_back(Args&&... args) {
    ExpandCapacityIfNecessary(1);
    new (&buffer_[end_]) T(std::forward<Args>(args)...);
    if (end_ == buffer_.capacity() - 1)
      end_ = 0;
    else
      end_++;
    IncrementGeneration();
    return back();
  }

  void pop_front() {
    DCHECK(size());
    buffer_.DestructRange(&buffer_[begin_], &buffer_[begin_ + 1]);
    begin_++;
    if (begin_ == buffer_.capacity())
      begin_ = 0;

    ShrinkCapacityIfNecessary();




    IncrementGeneration();
  }
  void pop_back() {
    DCHECK(size());
    if (end_ == 0)
      end_ = buffer_.capacity() - 1;
    else
      end_--;
    buffer_.DestructRange(&buffer_[end_], &buffer_[end_ + 1]);

    ShrinkCapacityIfNecessary();

    IncrementGeneration();
  }



  void swap(circular_deque& other) {
    std::swap(buffer_, other.buffer_);
    std::swap(begin_, other.begin_);
    std::swap(end_, other.end_);
    IncrementGeneration();
  }

  friend void swap(circular_deque& lhs, circular_deque& rhs) { lhs.swap(rhs); }

 private:
  friend internal::circular_deque_iterator<T>;
  friend internal::circular_deque_const_iterator<T>;



  static void MoveBuffer(VectorBuffer& from_buf,
                         size_t from_begin,
                         size_t from_end,
                         VectorBuffer* to_buf,
                         size_t* to_begin,
                         size_t* to_end) {
    size_t from_capacity = from_buf.capacity();

    *to_begin = 0;
    if (from_begin < from_end) {

      from_buf.MoveRange(&from_buf[from_begin], &from_buf[from_end],
                         to_buf->begin());
      *to_end = from_end - from_begin;
    } else if (from_begin > from_end) {

      from_buf.MoveRange(&from_buf[from_begin], &from_buf[from_capacity],
                         to_buf->begin());
      size_t right_size = from_capacity - from_begin;

      from_buf.MoveRange(&from_buf[0], &from_buf[from_end],
                         &(*to_buf)[right_size]);
      *to_end = right_size + from_end;
    } else {

      *to_end = 0;
    }
  }


  void SetCapacityTo(size_t new_capacity) {


    VectorBuffer new_buffer(new_capacity + 1);
    MoveBuffer(buffer_, begin_, end_, &new_buffer, &begin_, &end_);
    buffer_ = std::move(new_buffer);
  }
  void ExpandCapacityIfNecessary(size_t additional_elts) {
    size_t min_new_capacity = size() + additional_elts;
    if (capacity() >= min_new_capacity)
      return;  // Already enough room.

    min_new_capacity =
        std::max(min_new_capacity, internal::kCircularBufferInitialCapacity);



    size_t new_capacity =
        std::max(min_new_capacity, capacity() + capacity() / 4);
    SetCapacityTo(new_capacity);
  }

  void ShrinkCapacityIfNecessary() {

    if (capacity() <= internal::kCircularBufferInitialCapacity)
      return;

    size_t sz = size();
    size_t empty_spaces = capacity() - sz;
    if (empty_spaces < sz)
      return;


    size_t new_capacity =
        std::max(internal::kCircularBufferInitialCapacity, sz + sz / 4);
    if (new_capacity < capacity()) {

      SetCapacityTo(new_capacity);
    }
  }

  void ClearRetainCapacity() {


    DestructRange(begin_, end_);
    begin_ = 0;
    end_ = 0;
    IncrementGeneration();
  }



  void DestructRange(size_t begin, size_t end) {
    if (end == begin) {
      return;
    } else if (end > begin) {
      buffer_.DestructRange(&buffer_[begin], &buffer_[end]);
    } else {
      buffer_.DestructRange(&buffer_[begin], &buffer_[buffer_.capacity()]);
      buffer_.DestructRange(&buffer_[0], &buffer_[end]);
    }
  }




  void MakeRoomFor(size_t count, iterator* insert_begin, iterator* insert_end) {
    if (count == 0) {
      *insert_end = *insert_begin;
      return;
    }

    size_t begin_offset = insert_begin->OffsetFromBegin();
    ExpandCapacityIfNecessary(count);

    insert_begin->index_ = (begin_ + begin_offset) % buffer_.capacity();
    *insert_end =
        iterator(this, (insert_begin->index_ + count) % buffer_.capacity());

    iterator src = end();
    end_ = (end_ + count) % buffer_.capacity();
    iterator dest = end();


    while (true) {
      if (src == *insert_begin)
        break;
      --src;
      --dest;
      buffer_.MoveRange(&buffer_[src.index_], &buffer_[src.index_ + 1],
                        &buffer_[dest.index_]);
    }
  }

#if DCHECK_IS_ON()



  void CheckValidIndex(size_t i) const {
    if (begin_ <= end_)
      DCHECK(i >= begin_ && i < end_);
    else
      DCHECK((i >= begin_ && i < buffer_.capacity()) || i < end_);
  }

  void CheckValidIndexOrEnd(size_t i) const {
    if (i != end_)
      CheckValidIndex(i);
  }

  void ValidateIterator(const const_iterator& i) const {
    DCHECK(i.parent_deque_ == this);
    i.CheckUnstableUsage();
  }

  void IncrementGeneration() { generation_++; }
#else

  void CheckValidIndex(size_t) const {}
  void CheckValidIndexOrEnd(size_t) const {}
  void ValidateIterator(const const_iterator& i) const {}
  void IncrementGeneration() {}
#endif










  VectorBuffer buffer_;
  size_type begin_ = 0;
  size_type end_ = 0;

#if DCHECK_IS_ON()


  uint64_t generation_ = 0;
#endif
};

template <class T, class Value>
size_t Erase(circular_deque<T>& container, const Value& value) {
  auto it = std::remove(container.begin(), container.end(), value);
  size_t removed = std::distance(it, container.end());
  container.erase(it, container.end());
  return removed;
}

template <class T, class Predicate>
size_t EraseIf(circular_deque<T>& container, Predicate pred) {
  auto it = std::remove_if(container.begin(), container.end(), pred);
  size_t removed = std::distance(it, container.end());
  container.erase(it, container.end());
  return removed;
}

}  // namespace base

#endif  // BASE_CONTAINERS_CIRCULAR_DEQUE_H_
