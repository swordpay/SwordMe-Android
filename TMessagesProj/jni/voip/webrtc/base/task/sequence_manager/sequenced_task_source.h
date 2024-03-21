// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_SEQUENCED_TASK_SOURCE_H_
#define BASE_TASK_SEQUENCE_MANAGER_SEQUENCED_TASK_SOURCE_H_

#include "base/optional.h"
#include "base/pending_task.h"
#include "base/task/sequence_manager/lazy_now.h"
#include "base/task/sequence_manager/tasks.h"

namespace base {
namespace sequence_manager {
namespace internal {

class SequencedTaskSource {
 public:
  virtual ~SequencedTaskSource() = default;



  virtual Task* SelectNextTask() = 0;


  virtual void DidRunTask() = 0;


  virtual TimeDelta DelayTillNextTask(LazyNow* lazy_now) const = 0;


  virtual bool HasPendingHighResolutionTasks() = 0;



  virtual bool OnSystemIdle() = 0;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_SEQUENCED_TASK_SOURCE_H_
