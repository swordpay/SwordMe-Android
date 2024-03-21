// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_TASK_TRACKER_H_
#define BASE_TASK_THREAD_POOL_TASK_TRACKER_H_

#include <atomic>
#include <functional>
#include <limits>
#include <memory>
#include <queue>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/callback_forward.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/metrics/histogram_base.h"
#include "base/sequence_checker.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/waitable_event.h"
#include "base/task/common/checked_lock.h"
#include "base/task/common/task_annotator.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"
#include "base/task/thread_pool/tracked_ref.h"
#include "base/thread_annotations.h"

namespace base {

class ConditionVariable;

namespace internal {

enum class CanRunPolicy {

  kAll,

  kForegroundOnly,

  kNone,
};

// - A task can be pushed to a task source (WillPostTask).
// - A task source can be queued (WillQueueTaskSource).
// - Tasks for a given priority can run (CanRunPriority).
// - The next task in a queued task source can run (RunAndPopNextTask).
// TaskTracker also sets up the environment to run a task (RunAndPopNextTask)
// and records metrics and trace events. This class is thread-safe.
class BASE_EXPORT TaskTracker {
 public:


  TaskTracker(StringPiece histogram_label);

  virtual ~TaskTracker();



  void StartShutdown();







  void CompleteShutdown();





  void FlushForTesting();





  void FlushAsyncForTesting(OnceClosure flush_callback);



  void SetCanRunPolicy(CanRunPolicy can_run_policy);





  bool WillPostTask(Task* task, TaskShutdownBehavior shutdown_behavior);



  bool WillPostTaskNow(const Task& task,
                       TaskPriority priority) WARN_UNUSED_RESULT;



  RegisteredTaskSource RegisterTaskSource(
      scoped_refptr<TaskSource> task_source);

  bool CanRunPriority(TaskPriority priority) const;






  RegisteredTaskSource RunAndPopNextTask(RegisteredTaskSource task_source);



  bool HasShutdownStarted() const;


  bool IsShutdownComplete() const;







  void RecordHeartbeatLatencyAndTasksRunWhileQueuingHistograms(
      TaskPriority task_priority,
      TimeTicks posted_time,
      int num_tasks_run_when_posted) const;

  int GetNumTasksRun() const;

  TrackedRef<TaskTracker> GetTrackedRef() {
    return tracked_ref_factory_.GetTrackedRef();
  }



  bool HasIncompleteTaskSourcesForTesting() const;

 protected:





  virtual void RunTask(Task task,
                       TaskSource* task_source,
                       const TaskTraits& traits);

 private:
  friend class RegisteredTaskSource;
  class State;

  void PerformShutdown();



  bool BeforeQueueTaskSource(TaskShutdownBehavior shutdown_behavior);



  bool BeforeRunTask(TaskShutdownBehavior shutdown_behavior);


  void AfterRunTask(TaskShutdownBehavior shutdown_behavior);



  scoped_refptr<TaskSource> UnregisterTaskSource(
      scoped_refptr<TaskSource> task_source);

  void DecrementNumItemsBlockingShutdown();


  void DecrementNumIncompleteTaskSources();


  void CallFlushCallbackForTesting();


  void RecordLatencyHistogram(TaskPriority priority,
                              TimeTicks posted_time) const;

  void IncrementNumTasksRun();

  void RunContinueOnShutdown(Task* task);
  void RunSkipOnShutdown(Task* task);
  void RunBlockShutdown(Task* task);
  void RunTaskWithShutdownBehavior(TaskShutdownBehavior shutdown_behavior,
                                   Task* task);

  TaskAnnotator task_annotator_;

  const std::string histogram_label_;


  const bool has_log_best_effort_tasks_switch_;





  const std::unique_ptr<State> state_;





  std::atomic_int num_incomplete_task_sources_{0};

  std::atomic<CanRunPolicy> can_run_policy_;





  mutable CheckedLock flush_lock_;


  const std::unique_ptr<ConditionVariable> flush_cv_;


  OnceClosure flush_callback_for_testing_ GUARDED_BY(flush_lock_);

  mutable CheckedLock shutdown_lock_;


  std::unique_ptr<WaitableEvent> shutdown_event_ GUARDED_BY(shutdown_lock_);


  std::atomic_int num_tasks_run_{0};






  using TaskPriorityType = std::underlying_type<TaskPriority>::type;
  static constexpr TaskPriorityType kNumTaskPriorities =
      static_cast<TaskPriorityType>(TaskPriority::HIGHEST) + 1;
  HistogramBase* const task_latency_histograms_[kNumTaskPriorities];
  HistogramBase* const heartbeat_latency_histograms_[kNumTaskPriorities];
  HistogramBase* const
      num_tasks_run_while_queuing_histograms_[kNumTaskPriorities];



  TrackedRefFactory<TaskTracker> tracked_ref_factory_;

  DISALLOW_COPY_AND_ASSIGN(TaskTracker);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_TASK_TRACKER_H_
