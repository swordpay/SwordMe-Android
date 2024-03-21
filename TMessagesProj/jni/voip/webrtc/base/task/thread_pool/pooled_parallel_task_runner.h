// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_POOLED_PARALLEL_TASK_RUNNER_H_
#define BASE_TASK_THREAD_POOL_POOLED_PARALLEL_TASK_RUNNER_H_

#include "base/base_export.h"
#include "base/callback_forward.h"
#include "base/containers/flat_set.h"
#include "base/location.h"
#include "base/task/common/checked_lock.h"
#include "base/task/task_traits.h"
#include "base/task_runner.h"
#include "base/thread_annotations.h"
#include "base/time/time.h"

namespace base {
namespace internal {

class Sequence;
class PooledTaskRunnerDelegate;

class BASE_EXPORT PooledParallelTaskRunner : public TaskRunner {
 public:

  PooledParallelTaskRunner(
      const TaskTraits& traits,
      PooledTaskRunnerDelegate* pooled_task_runner_delegate);

  bool PostDelayedTask(const Location& from_here,
                       OnceClosure closure,
                       TimeDelta delay) override;

  void UnregisterSequence(Sequence* sequence);

 private:
  ~PooledParallelTaskRunner() override;

  const TaskTraits traits_;
  PooledTaskRunnerDelegate* const pooled_task_runner_delegate_;

  CheckedLock lock_;



  base::flat_set<Sequence*> sequences_ GUARDED_BY(lock_);

  DISALLOW_COPY_AND_ASSIGN(PooledParallelTaskRunner);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_POOLED_PARALLEL_TASK_RUNNER_H_
