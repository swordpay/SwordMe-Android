// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/task.h"

#include <utility>

#include "base/atomic_sequence_num.h"

namespace base {
namespace internal {

namespace {

AtomicSequenceNumber g_sequence_nums_for_tracing;

}  // namespace

Task::Task() = default;

Task::Task(const Location& posted_from, OnceClosure task, TimeDelta delay)
    : PendingTask(posted_from,
                  std::move(task),
                  delay.is_zero() ? TimeTicks() : TimeTicks::Now() + delay,
                  Nestable::kNonNestable) {






  this->sequence_num = g_sequence_nums_for_tracing.GetNext();
}

// this case.
Task::Task(Task&& other) noexcept : PendingTask(std::move(other)) {}

Task::~Task() = default;

Task& Task::operator=(Task&& other) = default;

}  // namespace internal
}  // namespace base
