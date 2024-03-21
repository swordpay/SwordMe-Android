// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINERS_RING_BUFFER_H_
#define BASE_CONTAINERS_RING_BUFFER_H_

#include <stddef.h>

#include "base/logging.h"
#include "base/macros.h"

namespace base {

// std::deque, and so, one can access only the last |kSize| elements. Also, you
// can add elements to the front and read/modify random elements, but cannot
// remove elements from the back. Therefore, it does not have a |Size| method,
// only |BufferSize|, which is a constant, and |CurrentIndex|, which is the
// number of elements added so far.
//
// If the above is sufficient for your use case, base::RingBuffer should be more
// efficient than base::circular_deque.
template <typename T, size_t kSize>
class RingBuffer {
 public:
  RingBuffer() : current_index_(0) {}

  size_t BufferSize() const { return kSize; }

  size_t CurrentIndex() const { return current_index_; }

  bool IsFilledIndex(size_t n) const {
    return IsFilledIndexByBufferIndex(BufferIndex(n));
  }




  const T& ReadBuffer(size_t n) const {
    const size_t buffer_index = BufferIndex(n);
    CHECK(IsFilledIndexByBufferIndex(buffer_index));
    return buffer_[buffer_index];
  }

  T* MutableReadBuffer(size_t n) {
    const size_t buffer_index = BufferIndex(n);
    CHECK(IsFilledIndexByBufferIndex(buffer_index));
    return &buffer_[buffer_index];
  }

  void SaveToBuffer(const T& value) {
    buffer_[BufferIndex(0)] = value;
    current_index_++;
  }

  void Clear() { current_index_ = 0; }

  class Iterator {
   public:
    size_t index() const { return index_; }

    const T* operator->() const { return &buffer_.ReadBuffer(index_); }
    const T* operator*() const { return &buffer_.ReadBuffer(index_); }

    Iterator& operator++() {
      index_++;
      if (index_ == kSize)
        out_of_range_ = true;
      return *this;
    }

    Iterator& operator--() {
      if (index_ == 0)
        out_of_range_ = true;
      index_--;
      return *this;
    }

    operator bool() const {
      return !out_of_range_ && buffer_.IsFilledIndex(index_);
    }

   private:
    Iterator(const RingBuffer<T, kSize>& buffer, size_t index)
        : buffer_(buffer), index_(index), out_of_range_(false) {}

    const RingBuffer<T, kSize>& buffer_;
    size_t index_;
    bool out_of_range_;

    friend class RingBuffer<T, kSize>;
  };



  Iterator Begin() const {
    if (current_index_ < kSize)
      return Iterator(*this, kSize - current_index_);
    return Iterator(*this, 0);
  }



  Iterator End() const { return Iterator(*this, kSize - 1); }

 private:
  inline size_t BufferIndex(size_t n) const {
    return (current_index_ + n) % kSize;
  }




  inline bool IsFilledIndexByBufferIndex(size_t buffer_index) const {
    return buffer_index < current_index_;
  }

  T buffer_[kSize];
  size_t current_index_;

  DISALLOW_COPY_AND_ASSIGN(RingBuffer);
};

}  // namespace base

#endif  // BASE_CONTAINERS_RING_BUFFER_H_
