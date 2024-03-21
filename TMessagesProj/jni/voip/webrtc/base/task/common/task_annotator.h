// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_COMMON_TASK_ANNOTATOR_H_
#define BASE_TASK_COMMON_TASK_ANNOTATOR_H_

#include <stdint.h>

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/pending_task.h"

namespace base {

// such as task origins, IPC message contexts, queueing durations and memory
// usage.
class BASE_EXPORT TaskAnnotator {
 public:
  class ObserverForTesting {
   public:


    virtual void BeforeRunTask(const PendingTask* pending_task) = 0;
  };


  class ScopedSetIpcHash;

  static const PendingTask* CurrentTaskForThread();

  TaskAnnotator();
  ~TaskAnnotator();




  void WillQueueTask(const char* trace_event_name,
                     PendingTask* pending_task,
                     const char* task_queue_name);

  void RunTask(const char* trace_event_name, PendingTask* pending_task);





  uint64_t GetTaskTraceID(const PendingTask& task) const;

 private:
  friend class TaskAnnotatorBacktraceIntegrationTest;



  static void RegisterObserverForTesting(ObserverForTesting* observer);
  static void ClearObserverForTesting();

  DISALLOW_COPY_AND_ASSIGN(TaskAnnotator);
};

class BASE_EXPORT TaskAnnotator::ScopedSetIpcHash {
 public:
  explicit ScopedSetIpcHash(uint32_t ipc_hash);
  ~ScopedSetIpcHash();

 private:
  std::unique_ptr<PendingTask> dummy_pending_task_;
  uint32_t old_ipc_hash_ = 0;

  DISALLOW_COPY_AND_ASSIGN(ScopedSetIpcHash);
};

}  // namespace base

#endif  // BASE_TASK_COMMON_TASK_ANNOTATOR_H_
