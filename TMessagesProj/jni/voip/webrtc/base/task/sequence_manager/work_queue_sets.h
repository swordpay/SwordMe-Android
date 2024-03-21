// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_WORK_QUEUE_SETS_H_
#define BASE_TASK_SEQUENCE_MANAGER_WORK_QUEUE_SETS_H_

#include <array>
#include <map>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/sequence_manager/sequence_manager.h"
#include "base/task/sequence_manager/task_queue_impl.h"
#include "base/task/sequence_manager/work_queue.h"
#include "base/trace_event/traced_value.h"

namespace base {
namespace sequence_manager {
namespace internal {

// uses a EnqueueOrderToWorkQueueMap to keep track of which queue in the set has
// the oldest task (i.e. the one that should be run next if the
// TaskQueueSelector chooses to run a task a given priority).  The reason this
// works is because std::map is a tree based associative container and all the
// values are kept in sorted order.
class BASE_EXPORT WorkQueueSets {
 public:
  class Observer {
   public:
    virtual ~Observer() {}

    virtual void WorkQueueSetBecameEmpty(size_t set_index) = 0;

    virtual void WorkQueueSetBecameNonEmpty(size_t set_index) = 0;
  };

  WorkQueueSets(const char* name,
                Observer* observer,
                const SequenceManager::Settings& settings);
  ~WorkQueueSets();

  void AddQueue(WorkQueue* queue, size_t set_index);

  void RemoveQueue(WorkQueue* work_queue);

  void ChangeSetIndex(WorkQueue* queue, size_t set_index);

  void OnQueuesFrontTaskChanged(WorkQueue* queue);

  void OnTaskPushedToEmptyQueue(WorkQueue* work_queue);



  void OnPopMinQueueInSet(WorkQueue* work_queue);

  void OnQueueBlocked(WorkQueue* work_queue);

  WorkQueue* GetOldestQueueInSet(size_t set_index) const;

  WorkQueue* GetOldestQueueAndEnqueueOrderInSet(
      size_t set_index,
      EnqueueOrder* out_enqueue_order) const;

#if DCHECK_IS_ON()

  WorkQueue* GetRandomQueueInSet(size_t set_index) const;

  WorkQueue* GetRandomQueueAndEnqueueOrderInSet(
      size_t set_index,
      EnqueueOrder* out_enqueue_order) const;
#endif

  bool IsSetEmpty(size_t set_index) const;

#if DCHECK_IS_ON() || !defined(NDEBUG)


  bool ContainsWorkQueueForTest(const WorkQueue* queue) const;
#endif

  const char* GetName() const { return name_; }


  void CollectSkippedOverLowerPriorityTasks(
      const internal::WorkQueue* selected_work_queue,
      std::vector<const Task*>* result) const;

 private:
  struct OldestTaskEnqueueOrder {
    EnqueueOrder key;
    WorkQueue* value;

    bool operator<=(const OldestTaskEnqueueOrder& other) const {
      return key <= other.key;
    }

    void SetHeapHandle(base::internal::HeapHandle handle) {
      value->set_heap_handle(handle);
    }

    void ClearHeapHandle() {
      value->set_heap_handle(base::internal::HeapHandle());
    }

    HeapHandle GetHeapHandle() const { return value->heap_handle(); }
  };

  const char* const name_;


  std::array<base::internal::IntrusiveHeap<OldestTaskEnqueueOrder>,
             TaskQueue::kQueuePriorityCount>
      work_queue_heaps_;

#if DCHECK_IS_ON()
  static inline uint64_t MurmurHash3(uint64_t value) {
    value ^= value >> 33;
    value *= uint64_t{0xFF51AFD7ED558CCD};
    value ^= value >> 33;
    value *= uint64_t{0xC4CEB9FE1A85EC53};
    value ^= value >> 33;
    return value;
  }



  uint64_t Random() const {
    last_rand_ = MurmurHash3(last_rand_);
    return last_rand_;
  }

  mutable uint64_t last_rand_;
#endif

  Observer* const observer_;

  DISALLOW_COPY_AND_ASSIGN(WorkQueueSets);
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_WORK_QUEUE_SETS_H_
