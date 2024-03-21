// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_TASK_TRACKER_POSIX_H_
#define BASE_TASK_THREAD_POOL_TASK_TRACKER_POSIX_H_

#include <memory>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/message_loop/message_pump_type.h"
#include "base/task/thread_pool/task_tracker.h"
#include "base/threading/platform_thread.h"

namespace base {
namespace internal {

struct Task;

// a task runs. Used on all POSIX platforms except NaCl SFI.
// set_io_thread_task_runner() must be called before the
// TaskTracker can run tasks.
class BASE_EXPORT TaskTrackerPosix : public TaskTracker {
 public:
  TaskTrackerPosix(StringPiece name);
  ~TaskTrackerPosix() override;






  void set_io_thread_task_runner(
      scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner) {
    io_thread_task_runner_ = std::move(io_thread_task_runner);
  }

 protected:

  void RunTask(Task task,
               TaskSource* task_source,
               const TaskTraits& traits) override;

 private:
  scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(TaskTrackerPosix);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_TASK_TRACKER_POSIX_H_
