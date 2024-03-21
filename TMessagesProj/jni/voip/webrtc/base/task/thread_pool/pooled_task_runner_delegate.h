// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_POOLED_TASK_RUNNER_DELEGATE_H_
#define BASE_TASK_THREAD_POOL_POOLED_TASK_RUNNER_DELEGATE_H_

#include "base/base_export.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/job_task_source.h"
#include "base/task/thread_pool/sequence.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"

namespace base {
namespace internal {

// PooledSequencedTaskRunner.
class BASE_EXPORT PooledTaskRunnerDelegate {
 public:
  PooledTaskRunnerDelegate();
  virtual ~PooledTaskRunnerDelegate();



  static bool Exists();



  virtual bool ShouldYield(const TaskSource* task_source) const = 0;




  virtual bool PostTaskWithSequence(Task task,
                                    scoped_refptr<Sequence> sequence) = 0;




  virtual bool EnqueueJobTaskSource(
      scoped_refptr<JobTaskSource> task_source) = 0;

  virtual void RemoveJobTaskSource(
      scoped_refptr<JobTaskSource> task_source) = 0;




  virtual void UpdatePriority(scoped_refptr<TaskSource> task_source,
                              TaskPriority priority) = 0;
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_POOLED_TASK_RUNNER_DELEGATE_H_
