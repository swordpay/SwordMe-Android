// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_TASK_OBSERVER_H_
#define BASE_TASK_TASK_OBSERVER_H_

#include "base/base_export.h"
#include "base/pending_task.h"

namespace base {

// processed on the thread it's associated with.
//
// NOTE: A TaskObserver implementation should be extremely fast!

class BASE_EXPORT TaskObserver {
 public:



  virtual void WillProcessTask(const PendingTask& pending_task,
                               bool was_blocked_or_low_priority) = 0;

  virtual void DidProcessTask(const PendingTask& pending_task) = 0;

 protected:
  virtual ~TaskObserver() = default;
};

}  // namespace base

#endif  // BASE_TASK_TASK_OBSERVER_H_
