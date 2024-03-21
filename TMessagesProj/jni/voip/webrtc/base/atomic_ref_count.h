// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// counting.  Please use base/memory/ref_counted.h directly instead.

#ifndef BASE_ATOMIC_REF_COUNT_H_
#define BASE_ATOMIC_REF_COUNT_H_

#include <atomic>

namespace base {

class AtomicRefCount {
 public:
  constexpr AtomicRefCount() : ref_count_(0) {}
  explicit constexpr AtomicRefCount(int initial_value)
      : ref_count_(initial_value) {}


  int Increment() { return Increment(1); }


  int Increment(int increment) {
    return ref_count_.fetch_add(increment, std::memory_order_relaxed);
  }



  bool Decrement() {




    return ref_count_.fetch_sub(1, std::memory_order_acq_rel) != 1;
  }






  bool IsOne() const { return ref_count_.load(std::memory_order_acquire) == 1; }



  bool IsZero() const {
    return ref_count_.load(std::memory_order_acquire) == 0;
  }


  int SubtleRefCountForDebug() const {
    return ref_count_.load(std::memory_order_relaxed);
  }

 private:
  std::atomic_int ref_count_;
};

}  // namespace base

#endif  // BASE_ATOMIC_REF_COUNT_H_
