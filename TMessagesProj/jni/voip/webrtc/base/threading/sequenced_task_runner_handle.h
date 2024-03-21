// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_
#define BASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/sequenced_task_runner.h"

namespace base {

class ThreadTaskRunnerHandle;

class BASE_EXPORT SequencedTaskRunnerHandle {
 public:




  static const scoped_refptr<SequencedTaskRunner>& Get() WARN_UNUSED_RESULT;





  static bool IsSet() WARN_UNUSED_RESULT;

  explicit SequencedTaskRunnerHandle(
      scoped_refptr<SequencedTaskRunner> task_runner);
  ~SequencedTaskRunnerHandle();

 private:

  friend class ThreadTaskRunnerHandle;

  scoped_refptr<SequencedTaskRunner> task_runner_;

  DISALLOW_COPY_AND_ASSIGN(SequencedTaskRunnerHandle);
};

}  // namespace base

#endif  // BASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_
