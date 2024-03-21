// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SIMPLE_TASK_EXECUTOR_H_
#define BASE_TASK_SIMPLE_TASK_EXECUTOR_H_

#include "base/task/task_executor.h"
#include "build/build_config.h"

namespace base {

// Must be instantiated and destroyed on the thread that runs tasks for the
// SingleThreadTaskRunner.
class BASE_EXPORT SimpleTaskExecutor : public TaskExecutor {
 public:
  explicit SimpleTaskExecutor(scoped_refptr<SingleThreadTaskRunner> task_queue);

  ~SimpleTaskExecutor() override;

  bool PostDelayedTask(const Location& from_here,
                       const TaskTraits& traits,
                       OnceClosure task,
                       TimeDelta delay) override;

  scoped_refptr<TaskRunner> CreateTaskRunner(const TaskTraits& traits) override;

  scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits) override;

  scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;

#if defined(OS_WIN)
  scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;
#endif  // defined(OS_WIN)

 private:
  const scoped_refptr<SingleThreadTaskRunner> task_queue_;


  TaskExecutor* const previous_task_executor_;
};

}  // namespace base

#endif  // BASE_TASK_SIMPLE_TASK_EXECUTOR_H_
