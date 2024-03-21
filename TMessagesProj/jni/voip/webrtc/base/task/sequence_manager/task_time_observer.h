// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_TASK_TIME_OBSERVER_H_
#define BASE_TASK_SEQUENCE_MANAGER_TASK_TIME_OBSERVER_H_

#include "base/time/time.h"

namespace base {
namespace sequence_manager {

class TaskTimeObserver {
 public:
  TaskTimeObserver() = default;
  virtual ~TaskTimeObserver() = default;

  virtual void WillProcessTask(TimeTicks start_time) = 0;

  virtual void DidProcessTask(TimeTicks start_time, TimeTicks end_time) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(TaskTimeObserver);
};

}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_TASK_TIME_OBSERVER_H_
