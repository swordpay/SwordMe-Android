// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_SERVICE_THREAD_H_
#define BASE_TASK_THREAD_POOL_SERVICE_THREAD_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace base {
namespace internal {

class TaskTracker;

// for handling async events (e.g. delayed tasks and async I/O). Its role is to
// merely forward such events to their destination (hence staying mostly idle
// and highly responsive).
// It aliases Thread::Run() to enforce that ServiceThread::Run() be on the stack
// and make it easier to identify the service thread in stack traces.
class BASE_EXPORT ServiceThread : public Thread {
 public:





  explicit ServiceThread(const TaskTracker* task_tracker,
                         RepeatingClosure report_heartbeat_metrics_callback);

  ~ServiceThread() override;



  static void SetHeartbeatIntervalForTesting(TimeDelta heartbeat);

 private:

  void Init() override;
  void Run(RunLoop* run_loop) override;

  void ReportHeartbeatMetrics() const;


  void PerformHeartbeatLatencyReport() const;

  const TaskTracker* const task_tracker_;



  base::RepeatingTimer heartbeat_metrics_timer_;

  RepeatingClosure report_heartbeat_metrics_callback_;

  DISALLOW_COPY_AND_ASSIGN(ServiceThread);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_SERVICE_THREAD_H_
