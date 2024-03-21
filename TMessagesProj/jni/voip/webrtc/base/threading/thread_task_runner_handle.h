// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
#define BASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_

#include "base/base_export.h"
#include "base/callback_helpers.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

// in thread-local storage.  Callers can then retrieve the TaskRunner
// for the current thread by calling ThreadTaskRunnerHandle::Get().
// At most one TaskRunner may be bound to each thread at a time.
// Prefer SequencedTaskRunnerHandle to this unless thread affinity is required.
class BASE_EXPORT ThreadTaskRunnerHandle {
 public:

  static const scoped_refptr<SingleThreadTaskRunner>& Get() WARN_UNUSED_RESULT;


  static bool IsSet() WARN_UNUSED_RESULT;








  static ScopedClosureRunner OverrideForTesting(
      scoped_refptr<SingleThreadTaskRunner> overriding_task_runner)
      WARN_UNUSED_RESULT;


  explicit ThreadTaskRunnerHandle(
      scoped_refptr<SingleThreadTaskRunner> task_runner);
  ~ThreadTaskRunnerHandle();

 private:
  scoped_refptr<SingleThreadTaskRunner> task_runner_;


  SequencedTaskRunnerHandle sequenced_task_runner_handle_;

  DISALLOW_COPY_AND_ASSIGN(ThreadTaskRunnerHandle);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
