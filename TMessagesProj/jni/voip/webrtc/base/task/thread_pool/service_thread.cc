// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/service_thread.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/debug/alias.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/task/thread_pool/task_tracker.h"
#include "base/task/thread_pool/thread_pool_instance.h"

namespace base {
namespace internal {

namespace {

TimeDelta g_heartbeat_for_testing = TimeDelta();

}  // namespace

ServiceThread::ServiceThread(const TaskTracker* task_tracker,
                             RepeatingClosure report_heartbeat_metrics_callback)
    : Thread("ThreadPoolServiceThread"),
      task_tracker_(task_tracker),
      report_heartbeat_metrics_callback_(
          std::move(report_heartbeat_metrics_callback)) {}

ServiceThread::~ServiceThread() = default;

void ServiceThread::SetHeartbeatIntervalForTesting(TimeDelta heartbeat) {
  g_heartbeat_for_testing = heartbeat;
}

void ServiceThread::Init() {



  if (ThreadPoolInstance::Get()) {



    constexpr TimeDelta kHeartbeat = TimeDelta::FromMinutes(59);

    heartbeat_metrics_timer_.Start(
        FROM_HERE,
        g_heartbeat_for_testing.is_zero() ? kHeartbeat
                                          : g_heartbeat_for_testing,
        BindRepeating(&ServiceThread::ReportHeartbeatMetrics,
                      Unretained(this)));
  }
}

NOINLINE void ServiceThread::Run(RunLoop* run_loop) {
  const int line_number = __LINE__;
  Thread::Run(run_loop);
  base::debug::Alias(&line_number);
}

void ServiceThread::ReportHeartbeatMetrics() const {
  report_heartbeat_metrics_callback_.Run();
  PerformHeartbeatLatencyReport();
}

void ServiceThread::PerformHeartbeatLatencyReport() const {
  if (!task_tracker_)
    return;







  const TaskPriority profiled_priority = static_cast<TaskPriority>(
      RandInt(static_cast<int>(TaskPriority::LOWEST),
              static_cast<int>(TaskPriority::HIGHEST)));




  ThreadPool::PostTask(
      FROM_HERE, {profiled_priority},
      BindOnce(
          &TaskTracker::RecordHeartbeatLatencyAndTasksRunWhileQueuingHistograms,
          Unretained(task_tracker_), profiled_priority, TimeTicks::Now(),
          task_tracker_->GetNumTasksRun()));
}

}  // namespace internal
}  // namespace base
