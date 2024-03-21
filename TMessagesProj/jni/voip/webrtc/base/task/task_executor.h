// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_TASK_EXECUTOR_H_
#define BASE_TASK_TASK_EXECUTOR_H_

#include <stdint.h>

#include "base/base_export.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task/single_thread_task_runner_thread_mode.h"
#include "base/task_runner.h"
#include "build/build_config.h"

namespace base {

class Location;
class TaskTraits;

// handle Tasks posted via the //base/task/post_task.h API, the TaskExecutor
// should be registered by calling RegisterTaskExecutor().
class BASE_EXPORT TaskExecutor {
 public:
  virtual ~TaskExecutor() = default;



  virtual bool PostDelayedTask(const Location& from_here,
                               const TaskTraits& traits,
                               OnceClosure task,
                               TimeDelta delay) = 0;


  virtual scoped_refptr<TaskRunner> CreateTaskRunner(
      const TaskTraits& traits) = 0;


  virtual scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits) = 0;




  virtual scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) = 0;

#if defined(OS_WIN)





  virtual scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) = 0;
#endif  // defined(OS_WIN)
};

// process for tasks subsequently posted with a TaskTraits extension with the
// given |extension_id|. All executors need to be registered before any tasks
// are posted with |extension_id|. Only one executor per |extension_id| is
// supported.
void BASE_EXPORT RegisterTaskExecutor(uint8_t extension_id,
                                      TaskExecutor* task_executor);
void BASE_EXPORT UnregisterTaskExecutorForTesting(uint8_t extension_id);

// tasks with the CurrentThread() trait.
void BASE_EXPORT SetTaskExecutorForCurrentThread(TaskExecutor* task_executor);

BASE_EXPORT TaskExecutor* GetTaskExecutorForCurrentThread();

// |traits| and, if so, returns a pointer to it. Otherwise, returns |nullptr|.
TaskExecutor* GetRegisteredTaskExecutorForTraits(const TaskTraits& traits);

}  // namespace base

#endif  // BASE_TASK_TASK_EXECUTOR_H_
