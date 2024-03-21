// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_TASK_SOURCE_H_
#define BASE_TASK_THREAD_POOL_TASK_SOURCE_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "base/sequence_token.h"
#include "base/task/common/checked_lock.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/sequence_sort_key.h"
#include "base/task/thread_pool/task.h"
#include "base/threading/sequence_local_storage_map.h"

namespace base {
namespace internal {

class TaskTracker;

enum class TaskSourceExecutionMode {
  kParallel,
  kSequenced,
  kSingleThread,
  kJob,
  kMax = kJob,
};

struct BASE_EXPORT ExecutionEnvironment {
  SequenceToken token;
  SequenceLocalStorageMap* sequence_local_storage;
};

// executed.
//
// A task source is registered when it's ready to be queued. A task source is
// ready to be queued when either:
// 1- It has new tasks that can run concurrently as a result of external
//    operations, e.g. posting a new task to an empty Sequence or increasing
//    max concurrency of a JobTaskSource;
// 2- A worker finished running a task from it and DidProcessTask() returned
//    true; or
// 3- A worker is about to run a task from it and WillRunTask() returned
//    kAllowedNotSaturated.
//
// A worker may perform the following sequence of operations on a
// RegisteredTaskSource after obtaining it from the queue:
// 1- Check whether a task can run with WillRunTask() (and register/enqueue the
//    task source again if not saturated).
// 2- (optional) Iff (1) determined that a task can run, access the next task
//    with TakeTask().
// 3- (optional) Execute the task.
// 4- Inform the task source that a task was processed with DidProcessTask(),
//    and re-enqueue the task source iff requested.
// When a task source is registered multiple times, many overlapping chains of
// operations may run concurrently, as permitted by WillRunTask(). This allows
// tasks from the same task source to run in parallel.
// However, the following invariants are kept:
// - The number of workers concurrently running tasks never goes over the
//   intended concurrency.
// - If the task source has more tasks that can run concurrently, it must be
//   queued.
//
// Note: there is a known refcounted-ownership cycle in the ThreadPool
// architecture: TaskSource -> TaskRunner -> TaskSource -> ... This is okay so
// long as the other owners of TaskSource (PriorityQueue and WorkerThread in
// alternation and ThreadGroupImpl::WorkerThreadDelegateImpl::GetWork()
// temporarily) keep running it (and taking Tasks from it as a result). A
// dangling reference cycle would only occur should they release their reference
// to it while it's not empty. In other words, it is only correct for them to
// release it when DidProcessTask() returns false.
//
// This class is thread-safe.
class BASE_EXPORT TaskSource : public RefCountedThreadSafe<TaskSource> {
 public:


  enum class RunStatus {

    kDisallowed,


    kAllowedNotSaturated,


    kAllowedSaturated,
  };




  class BASE_EXPORT Transaction {
   public:
    Transaction(Transaction&& other);
    ~Transaction();

    operator bool() const { return !!task_source_; }


    SequenceSortKey GetSortKey() const;

    void UpdatePriority(TaskPriority priority);

    TaskTraits traits() const { return task_source_->traits_; }

    TaskSource* task_source() const { return task_source_; }

   protected:
    explicit Transaction(TaskSource* task_source);

   private:
    friend class TaskSource;

    TaskSource* task_source_;

    DISALLOW_COPY_AND_ASSIGN(Transaction);
  };





  TaskSource(const TaskTraits& traits,
             TaskRunner* task_runner,
             TaskSourceExecutionMode execution_mode);


  Transaction BeginTransaction() WARN_UNUSED_RESULT;

  virtual ExecutionEnvironment GetExecutionEnvironment() = 0;



  virtual size_t GetRemainingConcurrency() const = 0;

  void SetHeapHandle(const HeapHandle& handle);
  void ClearHeapHandle();
  HeapHandle GetHeapHandle() const { return heap_handle_; }

  HeapHandle heap_handle() const { return heap_handle_; }


  TaskShutdownBehavior shutdown_behavior() const {
    return traits_.shutdown_behavior();
  }


  TaskPriority priority_racy() const {
    return priority_racy_.load(std::memory_order_relaxed);
  }


  ThreadPolicy thread_policy() const { return traits_.thread_policy(); }




  TaskRunner* task_runner() const { return task_runner_; }

  TaskSourceExecutionMode execution_mode() const { return execution_mode_; }

 protected:
  virtual ~TaskSource();

  virtual RunStatus WillRunTask() = 0;


  virtual Task TakeTask(TaskSource::Transaction* transaction) = 0;
  virtual bool DidProcessTask(TaskSource::Transaction* transaction) = 0;




  virtual Task Clear(TaskSource::Transaction* transaction) = 0;

  virtual SequenceSortKey GetSortKey() const = 0;

  void UpdatePriority(TaskPriority priority);

  TaskTraits traits_;

  std::atomic<TaskPriority> priority_racy_;

  mutable CheckedLock lock_{UniversalPredecessor()};

 private:
  friend class RefCountedThreadSafe<TaskSource>;
  friend class RegisteredTaskSource;


  HeapHandle heap_handle_;




  TaskRunner* task_runner_;

  TaskSourceExecutionMode execution_mode_;

  DISALLOW_COPY_AND_ASSIGN(TaskSource);
};

// RegisteredTaskSource can only be created with TaskTracker and may only be
// used by a single worker at a time. However, the same task source may be
// registered several times, spawning multiple RegisteredTaskSources. A
// RegisteredTaskSource resets to its initial state when WillRunTask() fails
// or after DidProcessTask(), so it can be used again.
class BASE_EXPORT RegisteredTaskSource {
 public:
  RegisteredTaskSource();
  RegisteredTaskSource(std::nullptr_t);
  RegisteredTaskSource(RegisteredTaskSource&& other) noexcept;
  ~RegisteredTaskSource();

  RegisteredTaskSource& operator=(RegisteredTaskSource&& other);

  operator bool() const { return task_source_ != nullptr; }
  TaskSource* operator->() const { return task_source_.get(); }
  TaskSource* get() const { return task_source_.get(); }

  static RegisteredTaskSource CreateForTesting(
      scoped_refptr<TaskSource> task_source,
      TaskTracker* task_tracker = nullptr);




  scoped_refptr<TaskSource> Unregister();



  TaskSource::RunStatus WillRunTask();




  Task TakeTask(TaskSource::Transaction* transaction = nullptr)
      WARN_UNUSED_RESULT;





  bool DidProcessTask(TaskSource::Transaction* transaction = nullptr);



  Task Clear(TaskSource::Transaction* transaction = nullptr) WARN_UNUSED_RESULT;

 private:
  friend class TaskTracker;
  RegisteredTaskSource(scoped_refptr<TaskSource> task_source,
                       TaskTracker* task_tracker);

#if DCHECK_IS_ON()

  enum class State {
    kInitial,       // WillRunTask() may be called.
    kReady,         // After WillRunTask() returned a valid RunStatus.
  };

  State run_step_ = State::kInitial;
#endif  // DCHECK_IS_ON()

  scoped_refptr<TaskSource> task_source_;
  TaskTracker* task_tracker_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(RegisteredTaskSource);
};

// RegisteredTaskSource with an associated Transaction.
// TODO(crbug.com/839091): Rename to RegisteredTaskSourceAndTransaction.
struct BASE_EXPORT TransactionWithRegisteredTaskSource {
 public:
  TransactionWithRegisteredTaskSource(RegisteredTaskSource task_source_in,
                                      TaskSource::Transaction transaction_in);

  TransactionWithRegisteredTaskSource(
      TransactionWithRegisteredTaskSource&& other) = default;
  ~TransactionWithRegisteredTaskSource() = default;

  static TransactionWithRegisteredTaskSource FromTaskSource(
      RegisteredTaskSource task_source_in);

  RegisteredTaskSource task_source;
  TaskSource::Transaction transaction;

  DISALLOW_COPY_AND_ASSIGN(TransactionWithRegisteredTaskSource);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_TASK_SOURCE_H_
