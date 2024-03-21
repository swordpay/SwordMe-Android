// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_THREAD_INSTRUCTION_COUNT_H_
#define BASE_TRACE_EVENT_THREAD_INSTRUCTION_COUNT_H_

#include <stdint.h>

#include "base/base_export.h"

namespace base {
namespace trace_event {

// of a thread's performance counters.
class BASE_EXPORT ThreadInstructionDelta {
 public:
  constexpr ThreadInstructionDelta() : delta_(0) {}
  explicit constexpr ThreadInstructionDelta(int64_t delta) : delta_(delta) {}

  constexpr int64_t ToInternalValue() const { return delta_; }

 private:
  int64_t delta_;
};

// instructions that have been retired on the current thread.
class BASE_EXPORT ThreadInstructionCount {
 public:


  static bool IsSupported();


  static ThreadInstructionCount Now();

  constexpr ThreadInstructionCount() : value_(-1) {}
  explicit constexpr ThreadInstructionCount(int64_t value) : value_(value) {}

  constexpr bool is_null() const { return value_ == -1; }

  constexpr ThreadInstructionDelta operator-(
      ThreadInstructionCount other) const {
    return ThreadInstructionDelta(value_ - other.value_);
  }

  constexpr int64_t ToInternalValue() const { return value_; }

 private:
  int64_t value_;
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_THREAD_INSTRUCTION_COUNT_H_
