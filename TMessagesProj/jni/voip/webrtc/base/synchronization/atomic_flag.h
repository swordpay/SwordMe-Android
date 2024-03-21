// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYNCHRONIZATION_ATOMIC_FLAG_H_
#define BASE_SYNCHRONIZATION_ATOMIC_FLAG_H_

#include <stdint.h>

#include <atomic>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/sequence_checker.h"

namespace base {

//
// This class IS NOT intended for synchronization between threads.
class BASE_EXPORT AtomicFlag {
 public:
  AtomicFlag();
  ~AtomicFlag();

  void Set();



  bool IsSet() const {

    return flag_.load(std::memory_order_acquire) != 0;
  }


  void UnsafeResetForTesting();

 private:
  std::atomic<uint_fast8_t> flag_{0};
  SEQUENCE_CHECKER(set_sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AtomicFlag);
};

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_ATOMIC_FLAG_H_
