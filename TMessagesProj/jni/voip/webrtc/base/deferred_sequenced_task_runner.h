// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_DEFERRED_SEQUENCED_TASK_RUNNER_H_
#define BASE_DEFERRED_SEQUENCED_TASK_RUNNER_H_

#include <vector>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

namespace base {

// queues up all requests until the first call to Start() is issued.
// DeferredSequencedTaskRunner may be created in two ways:
// . with an explicit SequencedTaskRunner that the events are flushed to
// . without a SequencedTaskRunner. In this configuration the
//   SequencedTaskRunner is supplied in StartWithTaskRunner().
class BASE_EXPORT DeferredSequencedTaskRunner : public SequencedTaskRunner {
 public:
  explicit DeferredSequencedTaskRunner(
      scoped_refptr<SequencedTaskRunner> target_runner);


  DeferredSequencedTaskRunner();

  bool PostDelayedTask(const Location& from_here,
                       OnceClosure task,
                       TimeDelta delay) override;
  bool RunsTasksInCurrentSequence() const override;

  bool PostNonNestableDelayedTask(const Location& from_here,
                                  OnceClosure task,
                                  TimeDelta delay) override;




  void Start();

  void StartWithTaskRunner(
      scoped_refptr<SequencedTaskRunner> target_task_runner);

 private:
  struct DeferredTask  {
    DeferredTask();
    DeferredTask(DeferredTask&& other);
    ~DeferredTask();
    DeferredTask& operator=(DeferredTask&& other);

    Location posted_from;
    OnceClosure task;

    TimeDelta delay;
    bool is_non_nestable;
  };

  ~DeferredSequencedTaskRunner() override;

  void StartImpl();

  void QueueDeferredTask(const Location& from_here,
                         OnceClosure task,
                         TimeDelta delay,
                         bool is_non_nestable);

  mutable Lock lock_;

  const PlatformThreadId created_thread_id_;

  bool started_ = false;
  scoped_refptr<SequencedTaskRunner> target_task_runner_;
  std::vector<DeferredTask> deferred_tasks_queue_;

  DISALLOW_COPY_AND_ASSIGN(DeferredSequencedTaskRunner);
};

}  // namespace base

#endif  // BASE_DEFERRED_SEQUENCED_TASK_RUNNER_H_
