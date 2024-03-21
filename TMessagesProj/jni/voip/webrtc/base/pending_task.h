// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PENDING_TASK_H_
#define BASE_PENDING_TASK_H_

#include <array>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/containers/queue.h"
#include "base/location.h"
#include "base/optional.h"
#include "base/time/time.h"

namespace base {

enum class Nestable : uint8_t {
  kNonNestable,
  kNestable,
};

// for use by classes that queue and execute tasks.
struct BASE_EXPORT PendingTask {
  PendingTask();
  PendingTask(const Location& posted_from,
              OnceClosure task,
              TimeTicks delayed_run_time = TimeTicks(),
              Nestable nestable = Nestable::kNestable);
  PendingTask(PendingTask&& other);
  ~PendingTask();

  PendingTask& operator=(PendingTask&& other);

  bool operator<(const PendingTask& other) const;

  OnceClosure task;

  Location posted_from;

  base::TimeTicks delayed_run_time;






  TimeTicks queue_time;

  static constexpr size_t kTaskBacktraceLength = 4;
  std::array<const void*, kTaskBacktraceLength> task_backtrace = {};








  uint32_t ipc_hash = 0;

  int sequence_num = 0;

  bool task_backtrace_overflow = false;

  Nestable nestable;

  bool is_high_res = false;
};

using TaskQueue = base::queue<PendingTask>;

using DelayedTaskQueue = std::priority_queue<base::PendingTask>;

}  // namespace base

#endif  // BASE_PENDING_TASK_H_
