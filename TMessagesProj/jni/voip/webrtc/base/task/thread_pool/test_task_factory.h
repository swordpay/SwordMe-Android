// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_TEST_TASK_FACTORY_H_
#define BASE_TASK_THREAD_POOL_TEST_TASK_FACTORY_H_

#include <stddef.h>

#include <unordered_set>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/test_utils.h"
#include "base/task_runner.h"
#include "base/threading/thread_checker_impl.h"

namespace base {
namespace internal {
namespace test {

// expected. Generates a test failure when:
// - The RunsTasksInCurrentSequence() method of the SequencedTaskRunner
//   (kSequenced or kSingleThread modes) returns false on a thread on which a
//   Task is run.
// - The TaskRunnerHandles set in the context of the task don't match what's
//   expected for the tested TaskSourceExecutionMode.
// - The TaskSourceExecutionMode of the TaskRunner is kSequenced or
//   kSingleThread and Tasks don't run in posting order.
// - The TaskSourceExecutionMode of the TaskRunner is kSingleThread and Tasks
//   don't run on the same thread.
// - A Task runs more than once.
class TestTaskFactory {
 public:
  enum class PostNestedTask {
    YES,
    NO,
  };


  TestTaskFactory(scoped_refptr<TaskRunner> task_runner,
                  TaskSourceExecutionMode execution_mode);

  ~TestTaskFactory();





  bool PostTask(PostNestedTask post_nested_task,
                OnceClosure after_task_closure);


  void WaitForAllTasksToRun() const;

  const TaskRunner* task_runner() const { return task_runner_.get(); }

 private:
  void RunTaskCallback(size_t task_index,
                       PostNestedTask post_nested_task,
                       OnceClosure after_task_closure);

  mutable Lock lock_;

  mutable ConditionVariable cv_;

  const scoped_refptr<TaskRunner> task_runner_;

  const TaskSourceExecutionMode execution_mode_;

  size_t num_posted_tasks_ = 0;

  std::unordered_set<size_t> ran_tasks_;


  ThreadCheckerImpl thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(TestTaskFactory);
};

}  // namespace test
}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_TEST_TASK_FACTORY_H_
