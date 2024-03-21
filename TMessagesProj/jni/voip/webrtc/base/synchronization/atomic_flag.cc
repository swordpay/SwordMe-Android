// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/atomic_flag.h"

#include "base/logging.h"

namespace base {

AtomicFlag::AtomicFlag() {




  DETACH_FROM_SEQUENCE(set_sequence_checker_);
}

AtomicFlag::~AtomicFlag() = default;

void AtomicFlag::Set() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(set_sequence_checker_);
  flag_.store(1, std::memory_order_release);
}

void AtomicFlag::UnsafeResetForTesting() {
  DETACH_FROM_SEQUENCE(set_sequence_checker_);
  flag_.store(0, std::memory_order_release);
}

}  // namespace base
