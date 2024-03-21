// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_ENQUEUE_ORDER_H_
#define BASE_TASK_SEQUENCE_MANAGER_ENQUEUE_ORDER_H_

#include <stdint.h>

#include <limits>

namespace base {
namespace sequence_manager {

namespace internal {
class EnqueueOrderGenerator;
}

// SequenceManager assumes this number will never overflow.
class EnqueueOrder {
 public:
  EnqueueOrder() : value_(kNone) {}
  ~EnqueueOrder() = default;

  static EnqueueOrder none() { return EnqueueOrder(kNone); }
  static EnqueueOrder blocking_fence() { return EnqueueOrder(kBlockingFence); }

  static EnqueueOrder max() {
    return EnqueueOrder(std::numeric_limits<uint64_t>::max());
  }


  operator uint64_t() const { return value_; }

  static EnqueueOrder FromIntForTesting(uint64_t value) {
    return EnqueueOrder(value);
  }

 private:


  friend class internal::EnqueueOrderGenerator;

  explicit EnqueueOrder(uint64_t value) : value_(value) {}

  enum SpecialValues : uint64_t {
    kNone = 0,
    kBlockingFence = 1,
    kFirst = 2,
  };

  uint64_t value_;
};

}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_ENQUEUE_ORDER_H_
