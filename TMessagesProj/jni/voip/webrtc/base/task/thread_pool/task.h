// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_TASK_H_
#define BASE_TASK_THREAD_POOL_TASK_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/pending_task.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/time/time.h"

namespace base {
namespace internal {

// profiling inherited from PendingTask.
// TODO(etiennep): This class is now equivalent to PendingTask, remove it.
struct BASE_EXPORT Task : public PendingTask {
  Task();


  Task(const Location& posted_from, OnceClosure task, TimeDelta delay);


  Task(Task&& other) noexcept;

  ~Task();

  Task& operator=(Task&& other);

 private:
  DISALLOW_COPY_AND_ASSIGN(Task);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_TASK_H_
