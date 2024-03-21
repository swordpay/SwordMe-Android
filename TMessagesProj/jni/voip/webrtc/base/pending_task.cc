// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/pending_task.h"


namespace base {

PendingTask::PendingTask() = default;

PendingTask::PendingTask(const Location& posted_from,
                         OnceClosure task,
                         TimeTicks delayed_run_time,
                         Nestable nestable)
    : task(std::move(task)),
      posted_from(posted_from),
      delayed_run_time(delayed_run_time),
      nestable(nestable) {}

PendingTask::PendingTask(PendingTask&& other) = default;

PendingTask::~PendingTask() = default;

PendingTask& PendingTask::operator=(PendingTask&& other) = default;

bool PendingTask::operator<(const PendingTask& other) const {




  if (delayed_run_time < other.delayed_run_time)
    return false;

  if (delayed_run_time > other.delayed_run_time)
    return true;


  return (sequence_num - other.sequence_num) > 0;
}

}  // namespace base
