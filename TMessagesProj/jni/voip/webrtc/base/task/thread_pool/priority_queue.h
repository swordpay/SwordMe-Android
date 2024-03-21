// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_PRIORITY_QUEUE_H_
#define BASE_TASK_THREAD_POOL_PRIORITY_QUEUE_H_

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/task/common/checked_lock.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/thread_pool/sequence_sort_key.h"
#include "base/task/thread_pool/task_source.h"

namespace base {
namespace internal {

// (requires external synchronization).
class BASE_EXPORT PriorityQueue {
 public:
  PriorityQueue();
  ~PriorityQueue();

  PriorityQueue& operator=(PriorityQueue&& other);

  void Push(TransactionWithRegisteredTaskSource transaction_with_task_source);




  const SequenceSortKey& PeekSortKey() const;



  RegisteredTaskSource& PeekTaskSource() const;


  RegisteredTaskSource PopTaskSource();




  RegisteredTaskSource RemoveTaskSource(const TaskSource& task_source);



  void UpdateSortKey(TaskSource::Transaction transaction);

  bool IsEmpty() const;

  size_t Size() const;

  size_t GetNumTaskSourcesWithPriority(TaskPriority priority) const {
    return num_task_sources_per_priority_[static_cast<int>(priority)];
  }



  void EnableFlushTaskSourcesOnDestroyForTesting();

 private:


  class TaskSourceAndSortKey;

  using ContainerType = IntrusiveHeap<TaskSourceAndSortKey>;

  void DecrementNumTaskSourcesForPriority(TaskPriority priority);
  void IncrementNumTaskSourcesForPriority(TaskPriority priority);

  ContainerType container_;

  std::array<size_t, static_cast<int>(TaskPriority::HIGHEST) + 1>
      num_task_sources_per_priority_ = {};

  bool is_flush_task_sources_on_destroy_enabled_ = false;

  DISALLOW_COPY_AND_ASSIGN(PriorityQueue);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_PRIORITY_QUEUE_H_
