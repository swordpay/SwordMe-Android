// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/sequence_manager/work_queue.h"

#include "base/task/sequence_manager/sequence_manager_impl.h"
#include "base/task/sequence_manager/work_queue_sets.h"

namespace base {
namespace sequence_manager {
namespace internal {

WorkQueue::WorkQueue(TaskQueueImpl* task_queue,
                     const char* name,
                     QueueType queue_type)
    : task_queue_(task_queue), name_(name), queue_type_(queue_type) {}

void WorkQueue::AsValueInto(TimeTicks now,
                            trace_event::TracedValue* state) const {
  for (const Task& task : tasks_) {
    TaskQueueImpl::TaskAsValueInto(task, now, state);
  }
}

WorkQueue::~WorkQueue() {
  DCHECK(!work_queue_sets_) << task_queue_->GetName() << " : "
                            << work_queue_sets_->GetName() << " : " << name_;
}

const Task* WorkQueue::GetFrontTask() const {
  if (tasks_.empty())
    return nullptr;
  return &tasks_.front();
}

const Task* WorkQueue::GetBackTask() const {
  if (tasks_.empty())
    return nullptr;
  return &tasks_.back();
}

bool WorkQueue::BlockedByFence() const {
  if (!fence_)
    return false;



  return tasks_.empty() || tasks_.front().enqueue_order() >= fence_;
}

bool WorkQueue::GetFrontTaskEnqueueOrder(EnqueueOrder* enqueue_order) const {
  if (tasks_.empty() || BlockedByFence())
    return false;

  DCHECK_LE(tasks_.front().enqueue_order(), tasks_.back().enqueue_order())
      << task_queue_->GetName() << " : " << work_queue_sets_->GetName() << " : "
      << name_;
  *enqueue_order = tasks_.front().enqueue_order();
  return true;
}

void WorkQueue::Push(Task task) {
  bool was_empty = tasks_.empty();
#ifndef NDEBUG
  DCHECK(task.enqueue_order_set());
#endif

  DCHECK(was_empty || tasks_.back().enqueue_order() < task.enqueue_order());

  tasks_.push_back(std::move(task));

  if (!was_empty)
    return;

  if (work_queue_sets_ && !BlockedByFence())
    work_queue_sets_->OnTaskPushedToEmptyQueue(this);
}

WorkQueue::TaskPusher::TaskPusher(WorkQueue* work_queue)
    : work_queue_(work_queue), was_empty_(work_queue->Empty()) {}

WorkQueue::TaskPusher::TaskPusher(TaskPusher&& other)
    : work_queue_(other.work_queue_), was_empty_(other.was_empty_) {
  other.work_queue_ = nullptr;
}

void WorkQueue::TaskPusher::Push(Task* task) {
  DCHECK(work_queue_);

#ifndef NDEBUG
  DCHECK(task->enqueue_order_set());
#endif

  DCHECK(work_queue_->tasks_.empty() ||
         work_queue_->tasks_.back().enqueue_order() < task->enqueue_order());

  work_queue_->tasks_.push_back(std::move(*task));
}

WorkQueue::TaskPusher::~TaskPusher() {


  if (was_empty_ && work_queue_ && !work_queue_->Empty() &&
      work_queue_->work_queue_sets_ && !work_queue_->BlockedByFence()) {
    work_queue_->work_queue_sets_->OnTaskPushedToEmptyQueue(work_queue_);
  }
}

WorkQueue::TaskPusher WorkQueue::CreateTaskPusher() {
  return TaskPusher(this);
}

void WorkQueue::PushNonNestableTaskToFront(Task task) {
  DCHECK(task.nestable == Nestable::kNonNestable);

  bool was_empty = tasks_.empty();
  bool was_blocked = BlockedByFence();
#ifndef NDEBUG
  DCHECK(task.enqueue_order_set());
#endif

  if (!was_empty) {

    DCHECK_LE(task.enqueue_order(), tasks_.front().enqueue_order())
        << task_queue_->GetName() << " : " << work_queue_sets_->GetName()
        << " : " << name_;
  }

  tasks_.push_front(std::move(task));

  if (!work_queue_sets_)
    return;

  if (BlockedByFence())
    return;

  if (was_empty || was_blocked) {
    work_queue_sets_->OnTaskPushedToEmptyQueue(this);
  } else {
    work_queue_sets_->OnQueuesFrontTaskChanged(this);
  }
}

void WorkQueue::TakeImmediateIncomingQueueTasks() {
  DCHECK(tasks_.empty());

  task_queue_->TakeImmediateIncomingQueueTasks(&tasks_);
  if (tasks_.empty())
    return;

  if (work_queue_sets_ && !BlockedByFence())
    work_queue_sets_->OnTaskPushedToEmptyQueue(this);
}

Task WorkQueue::TakeTaskFromWorkQueue() {
  DCHECK(work_queue_sets_);
  DCHECK(!tasks_.empty());

  Task pending_task = std::move(tasks_.front());
  tasks_.pop_front();

  if (tasks_.empty()) {

    if (queue_type_ == QueueType::kImmediate) {


      task_queue_->TakeImmediateIncomingQueueTasks(&tasks_);
    }


    tasks_.MaybeShrinkQueue();
  }

  DCHECK(work_queue_sets_);
#if DCHECK_IS_ON()


  work_queue_sets_->OnQueuesFrontTaskChanged(this);
#else


  work_queue_sets_->OnPopMinQueueInSet(this);
#endif
  task_queue_->TraceQueueSize();
  return pending_task;
}

bool WorkQueue::RemoveAllCanceledTasksFromFront() {
  if (!work_queue_sets_)
    return false;
  bool task_removed = false;
  while (!tasks_.empty() &&
         (!tasks_.front().task || tasks_.front().task.IsCancelled())) {
    tasks_.pop_front();
    task_removed = true;
  }
  if (task_removed) {
    if (tasks_.empty()) {

      if (queue_type_ == QueueType::kImmediate) {


        task_queue_->TakeImmediateIncomingQueueTasks(&tasks_);
      }


      tasks_.MaybeShrinkQueue();
    }


    if (heap_handle_.IsValid())
      work_queue_sets_->OnQueuesFrontTaskChanged(this);
    task_queue_->TraceQueueSize();
  }
  return task_removed;
}

void WorkQueue::AssignToWorkQueueSets(WorkQueueSets* work_queue_sets) {
  work_queue_sets_ = work_queue_sets;
}

void WorkQueue::AssignSetIndex(size_t work_queue_set_index) {
  work_queue_set_index_ = work_queue_set_index;
}

bool WorkQueue::InsertFenceImpl(EnqueueOrder fence) {
  DCHECK_NE(fence, 0u);
  DCHECK(fence >= fence_ || fence == EnqueueOrder::blocking_fence());
  bool was_blocked_by_fence = BlockedByFence();
  fence_ = fence;
  return was_blocked_by_fence;
}

void WorkQueue::InsertFenceSilently(EnqueueOrder fence) {

  DCHECK(!fence_ || fence_ == EnqueueOrder::blocking_fence());
  InsertFenceImpl(fence);
}

bool WorkQueue::InsertFence(EnqueueOrder fence) {
  bool was_blocked_by_fence = InsertFenceImpl(fence);
  if (!work_queue_sets_)
    return false;

  if (!tasks_.empty() && was_blocked_by_fence && !BlockedByFence()) {
    work_queue_sets_->OnTaskPushedToEmptyQueue(this);
    return true;
  }

  if (BlockedByFence())
    work_queue_sets_->OnQueueBlocked(this);
  return false;
}

bool WorkQueue::RemoveFence() {
  bool was_blocked_by_fence = BlockedByFence();
  fence_ = EnqueueOrder::none();
  if (work_queue_sets_ && !tasks_.empty() && was_blocked_by_fence) {
    work_queue_sets_->OnTaskPushedToEmptyQueue(this);
    return true;
  }
  return false;
}

bool WorkQueue::ShouldRunBefore(const WorkQueue* other_queue) const {
  DCHECK(!tasks_.empty());
  DCHECK(!other_queue->tasks_.empty());
  EnqueueOrder enqueue_order;
  EnqueueOrder other_enqueue_order;
  bool have_task = GetFrontTaskEnqueueOrder(&enqueue_order);
  bool have_other_task =
      other_queue->GetFrontTaskEnqueueOrder(&other_enqueue_order);
  DCHECK(have_task);
  DCHECK(have_other_task);
  return enqueue_order < other_enqueue_order;
}

void WorkQueue::MaybeShrinkQueue() {
  tasks_.MaybeShrinkQueue();
}

void WorkQueue::DeletePendingTasks() {
  tasks_.clear();

  if (work_queue_sets_ && heap_handle().IsValid())
    work_queue_sets_->OnQueuesFrontTaskChanged(this);
  DCHECK(!heap_handle_.IsValid());
}

void WorkQueue::PopTaskForTesting() {
  if (tasks_.empty())
    return;
  tasks_.pop_front();
}

void WorkQueue::CollectTasksOlderThan(EnqueueOrder reference,
                                      std::vector<const Task*>* result) const {
  for (const Task& task : tasks_) {
    if (task.enqueue_order() >= reference)
      break;

    result->push_back(&task);
  }
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base
