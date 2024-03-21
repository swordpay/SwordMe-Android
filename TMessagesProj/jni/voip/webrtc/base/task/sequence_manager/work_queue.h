// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_WORK_QUEUE_H_
#define BASE_TASK_SEQUENCE_MANAGER_WORK_QUEUE_H_

#include "base/base_export.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/sequence_manager/enqueue_order.h"
#include "base/task/sequence_manager/sequenced_task_source.h"
#include "base/task/sequence_manager/task_queue_impl.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/traced_value.h"

namespace base {
namespace sequence_manager {
namespace internal {

class WorkQueueSets;

// now. It interfaces deeply with WorkQueueSets which keeps track of which queue
// (with a given priority) contains the oldest task.
//
// If a fence is inserted, WorkQueue behaves normally up until
// TakeTaskFromWorkQueue reaches or exceeds the fence.  At that point it the
// API subset used by WorkQueueSets pretends the WorkQueue is empty until the
// fence is removed.  This functionality is a primitive intended for use by
// throttling mechanisms.
class BASE_EXPORT WorkQueue {
 public:
  using QueueType = internal::TaskQueueImpl::WorkQueueType;

  WorkQueue(TaskQueueImpl* task_queue, const char* name, QueueType queue_type);
  ~WorkQueue();


  void AssignToWorkQueueSets(WorkQueueSets* work_queue_sets);

  void AssignSetIndex(size_t work_queue_set_index);

  void AsValueInto(TimeTicks now, trace_event::TracedValue* state) const;

  bool Empty() const { return tasks_.empty(); }



  bool GetFrontTaskEnqueueOrder(EnqueueOrder* enqueue_order) const;


  const Task* GetFrontTask() const;


  const Task* GetBackTask() const;


  void Push(Task task);

  class BASE_EXPORT TaskPusher {
   public:
    TaskPusher(const TaskPusher&) = delete;
    TaskPusher(TaskPusher&& other);
    ~TaskPusher();

    void Push(Task* task);

   private:
    friend class WorkQueue;

    explicit TaskPusher(WorkQueue* work_queue);

    WorkQueue* work_queue_;
    const bool was_empty_;
  };

  TaskPusher CreateTaskPusher();



  void PushNonNestableTaskToFront(Task task);



  void TakeImmediateIncomingQueueTasks();

  size_t Size() const { return tasks_.size(); }

  size_t Capacity() const { return tasks_.capacity(); }



  Task TakeTaskFromWorkQueue();


  bool RemoveAllCanceledTasksFromFront();

  const char* name() const { return name_; }

  TaskQueueImpl* task_queue() const { return task_queue_; }

  WorkQueueSets* work_queue_sets() const { return work_queue_sets_; }

  size_t work_queue_set_index() const { return work_queue_set_index_; }

  base::internal::HeapHandle heap_handle() const { return heap_handle_; }

  void set_heap_handle(base::internal::HeapHandle handle) {
    heap_handle_ = handle;
  }

  QueueType queue_type() const { return queue_type_; }



  bool ShouldRunBefore(const WorkQueue* other_queue) const;





  bool InsertFence(EnqueueOrder fence);



  void InsertFenceSilently(EnqueueOrder fence);



  bool RemoveFence();



  bool BlockedByFence() const;

  void MaybeShrinkQueue();

  void DeletePendingTasks();

  void PopTaskForTesting();


  void CollectTasksOlderThan(EnqueueOrder reference,
                             std::vector<const Task*>* result) const;

 private:
  bool InsertFenceImpl(EnqueueOrder fence);

  TaskQueueImpl::TaskDeque tasks_;
  WorkQueueSets* work_queue_sets_ = nullptr;  // NOT OWNED.
  TaskQueueImpl* const task_queue_;           // NOT OWNED.
  size_t work_queue_set_index_ = 0;



  base::internal::HeapHandle heap_handle_;
  const char* const name_;
  EnqueueOrder fence_;
  const QueueType queue_type_;

  DISALLOW_COPY_AND_ASSIGN(WorkQueue);
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_WORK_QUEUE_H_
