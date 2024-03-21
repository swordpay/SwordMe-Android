// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_POST_JOB_H_
#define BASE_TASK_POST_JOB_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/task/task_traits.h"
#include "base/time/time.h"

namespace base {
namespace internal {
class JobTaskSource;
class PooledTaskRunnerDelegate;
}

// communicate with the scheduler.
class BASE_EXPORT JobDelegate {
 public:





  JobDelegate(internal::JobTaskSource* task_source,
              internal::PooledTaskRunnerDelegate* pooled_task_runner_delegate);
  ~JobDelegate();



  bool ShouldYield();





  void YieldIfNeeded();


  void NotifyConcurrencyIncrease();

 private:



  void AssertExpectedConcurrency(size_t expected_max_concurrency);

  internal::JobTaskSource* const task_source_;
  internal::PooledTaskRunnerDelegate* const pooled_task_runner_delegate_;

#if DCHECK_IS_ON()


  size_t recorded_max_concurrency_;

  size_t recorded_increase_version_;

  bool last_should_yield_ = false;
#endif

  DISALLOW_COPY_AND_ASSIGN(JobDelegate);
};

// the posted Job.
class BASE_EXPORT JobHandle {
 public:
  JobHandle();


  ~JobHandle();

  JobHandle(JobHandle&&);
  JobHandle& operator=(JobHandle&&);

  explicit operator bool() const { return task_source_ != nullptr; }

  void UpdatePriority(TaskPriority new_priority);


  void NotifyConcurrencyIncrease();



  void Join();


  void Cancel();



  void CancelAndDetach();

  void Detach();

 private:
  friend class internal::JobTaskSource;

  explicit JobHandle(scoped_refptr<internal::JobTaskSource> task_source);

  scoped_refptr<internal::JobTaskSource> task_source_;

  DISALLOW_COPY_AND_ASSIGN(JobHandle);
};

// base::ThreadPool.
// Returns a JobHandle associated with the Job, which can be joined, canceled or
// detached.
// To avoid scheduling overhead, |worker_task| should do as much work as
// possible in a loop when invoked, and JobDelegate::ShouldYield() should be
// periodically invoked to conditionally exit and let the scheduler prioritize
// work.
//
// A canonical implementation of |worker_task| looks like:
//   void WorkerTask(JobDelegate* job_delegate) {
//     while (!job_delegate->ShouldYield()) {
//       auto work_item = worker_queue.TakeWorkItem(); // Smallest unit of work.
//       if (!work_item)
//         return:
//       ProcessWork(work_item);
//     }
//   }
//
// |max_concurrency_callback| controls the maximum number of threads calling
// |worker_task| concurrently. |worker_task| is only invoked if the number of
// threads previously running |worker_task| was less than the value returned by
// |max_concurrency_callback|. In general, |max_concurrency_callback| should
// return the latest number of incomplete work items (smallest unit of work)
// left to processed. JobHandle/JobDelegate::NotifyConcurrencyIncrease() *must*
// be invoked shortly after |max_concurrency_callback| starts returning a value
// larger than previously returned values. This usually happens when new work
// items are added and the API user wants additional threads to invoke
// |worker_task| concurrently. The callbacks may be called concurrently on any
// thread until the job is complete. If the job handle is detached, the
// callbacks may still be called, so they must not access global state that
// could be destroyed.
//
// |traits| requirements:
// - base::ThreadPolicy must be specified if the priority of the task runner
//   will ever be increased from BEST_EFFORT.
JobHandle BASE_EXPORT
PostJob(const Location& from_here,
        const TaskTraits& traits,
        RepeatingCallback<void(JobDelegate*)> worker_task,
        RepeatingCallback<size_t()> max_concurrency_callback);

}  // namespace base

#endif  // BASE_TASK_POST_JOB_H_
