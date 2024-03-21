// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/common/task_annotator.h"

#include <array>

#include "base/debug/activity_tracker.h"
#include "base/debug/alias.h"
#include "base/no_destructor.h"
#include "base/threading/thread_local.h"
#include "base/trace_event/trace_event.h"

namespace base {

namespace {

TaskAnnotator::ObserverForTesting* g_task_annotator_observer = nullptr;

static constexpr int kSentinelSequenceNum =
    static_cast<int>(0xF00DBAADF00DBAAD);

// each thread. Used to allow creating a breadcrumb of program counters on the
// stack to help identify a task's origin in crashes.
ThreadLocalPointer<PendingTask>* GetTLSForCurrentPendingTask() {
  static NoDestructor<ThreadLocalPointer<PendingTask>> instance;
  return instance.get();
}

// been injected by ScopedSetIpcHash solely for the purposes of
// tracking IPC context.
bool IsDummyPendingTask(const PendingTask* task) {
  if (task->sequence_num == kSentinelSequenceNum &&
      !task->posted_from.has_source_info() &&
      !task->posted_from.program_counter()) {
    return true;
  }
  return false;
}

}  // namespace

const PendingTask* TaskAnnotator::CurrentTaskForThread() {
  auto* current_task = GetTLSForCurrentPendingTask()->Get();


  if (current_task && IsDummyPendingTask(current_task))
    return nullptr;
  return current_task;
}

TaskAnnotator::TaskAnnotator() = default;

TaskAnnotator::~TaskAnnotator() = default;

void TaskAnnotator::WillQueueTask(const char* trace_event_name,
                                  PendingTask* pending_task,
                                  const char* task_queue_name) {
  DCHECK(trace_event_name);
  DCHECK(pending_task);
  DCHECK(task_queue_name);
  TRACE_EVENT_WITH_FLOW1(
      TRACE_DISABLED_BY_DEFAULT("toplevel.flow"), trace_event_name,
      TRACE_ID_LOCAL(GetTaskTraceID(*pending_task)), TRACE_EVENT_FLAG_FLOW_OUT,
      "task_queue_name", task_queue_name);

  DCHECK(!pending_task->task_backtrace[0])
      << "Task backtrace was already set, task posted twice??";
  if (pending_task->task_backtrace[0])
    return;

  const auto* parent_task = CurrentTaskForThread();
  if (!parent_task)
    return;

  pending_task->ipc_hash = parent_task->ipc_hash;
  pending_task->task_backtrace[0] = parent_task->posted_from.program_counter();
  std::copy(parent_task->task_backtrace.begin(),
            parent_task->task_backtrace.end() - 1,
            pending_task->task_backtrace.begin() + 1);
  pending_task->task_backtrace_overflow =
      parent_task->task_backtrace_overflow ||
      parent_task->task_backtrace.back() != nullptr;
}

void TaskAnnotator::RunTask(const char* trace_event_name,
                            PendingTask* pending_task) {
  DCHECK(trace_event_name);
  DCHECK(pending_task);

  debug::ScopedTaskRunActivity task_activity(*pending_task);

  TRACE_EVENT1(TRACE_DISABLED_BY_DEFAULT("toplevel.ipc"),
               "TaskAnnotator::RunTask", "ipc_hash", pending_task->ipc_hash);

  TRACE_EVENT_WITH_FLOW0(
      TRACE_DISABLED_BY_DEFAULT("toplevel.flow"), trace_event_name,
      TRACE_ID_LOCAL(GetTaskTraceID(*pending_task)), TRACE_EVENT_FLAG_FLOW_IN);





  static constexpr int kStackTaskTraceSnapshotSize =
      PendingTask::kTaskBacktraceLength + 4;
  std::array<const void*, kStackTaskTraceSnapshotSize> task_backtrace;












  task_backtrace.front() = reinterpret_cast<void*>(0xc001c0ded017d00d);
  task_backtrace.back() = reinterpret_cast<void*>(0x0d00d1d1d178119);

  task_backtrace[1] = pending_task->posted_from.program_counter();
  std::copy(pending_task->task_backtrace.begin(),
            pending_task->task_backtrace.end(), task_backtrace.begin() + 2);
  task_backtrace[kStackTaskTraceSnapshotSize - 2] =
      reinterpret_cast<void*>(pending_task->ipc_hash);
  debug::Alias(&task_backtrace);

  auto* tls = GetTLSForCurrentPendingTask();
  auto* previous_pending_task = tls->Get();
  tls->Set(pending_task);

  if (g_task_annotator_observer)
    g_task_annotator_observer->BeforeRunTask(pending_task);
  std::move(pending_task->task).Run();

  tls->Set(previous_pending_task);
}

uint64_t TaskAnnotator::GetTaskTraceID(const PendingTask& task) const {
  return (static_cast<uint64_t>(task.sequence_num) << 32) |
         ((static_cast<uint64_t>(reinterpret_cast<intptr_t>(this)) << 32) >>
          32);
}

void TaskAnnotator::RegisterObserverForTesting(ObserverForTesting* observer) {
  DCHECK(!g_task_annotator_observer);
  g_task_annotator_observer = observer;
}

void TaskAnnotator::ClearObserverForTesting() {
  g_task_annotator_observer = nullptr;
}

TaskAnnotator::ScopedSetIpcHash::ScopedSetIpcHash(uint32_t ipc_hash) {


  auto* tls = GetTLSForCurrentPendingTask();
  auto* current_task = tls->Get();
  if (!current_task) {
    dummy_pending_task_ = std::make_unique<PendingTask>();
    dummy_pending_task_->sequence_num = kSentinelSequenceNum;
    current_task = dummy_pending_task_.get();
    tls->Set(current_task);
  }

  old_ipc_hash_ = current_task->ipc_hash;
  current_task->ipc_hash = ipc_hash;
}

TaskAnnotator::ScopedSetIpcHash::~ScopedSetIpcHash() {
  auto* tls = GetTLSForCurrentPendingTask();
  auto* current_task = tls->Get();
  DCHECK(current_task);
  if (current_task == dummy_pending_task_.get()) {
    tls->Set(nullptr);
  } else {
    current_task->ipc_hash = old_ipc_hash_;
  }
}

}  // namespace base
