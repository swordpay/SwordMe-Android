// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_TASK_QUEUE_IMPL_H_
#define BASE_TASK_SEQUENCE_MANAGER_TASK_QUEUE_IMPL_H_

#include <stddef.h>

#include <memory>
#include <queue>
#include <set>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/pending_task.h"
#include "base/task/common/checked_lock.h"
#include "base/task/common/intrusive_heap.h"
#include "base/task/common/operations_controller.h"
#include "base/task/sequence_manager/associated_thread_id.h"
#include "base/task/sequence_manager/atomic_flag_set.h"
#include "base/task/sequence_manager/enqueue_order.h"
#include "base/task/sequence_manager/lazily_deallocated_deque.h"
#include "base/task/sequence_manager/sequenced_task_source.h"
#include "base/task/sequence_manager/task_queue.h"
#include "base/threading/thread_checker.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/traced_value.h"

namespace base {
namespace sequence_manager {

class LazyNow;
class TimeDomain;

namespace internal {

class SequenceManagerImpl;
class WorkQueue;
class WorkQueueSets;

//
// Immediate (non-delayed) tasks:
//    |immediate_incoming_queue| - PostTask enqueues tasks here.
//    |immediate_work_queue| - SequenceManager takes immediate tasks here.
//
// Delayed tasks
//    |delayed_incoming_queue| - PostDelayedTask enqueues tasks here.
//    |delayed_work_queue| - SequenceManager takes delayed tasks here.
//
// The |immediate_incoming_queue| can be accessed from any thread, the other
// queues are main-thread only. To reduce the overhead of locking,
// |immediate_work_queue| is swapped with |immediate_incoming_queue| when
// |immediate_work_queue| becomes empty.
//
// Delayed tasks are initially posted to |delayed_incoming_queue| and a wake-up
// is scheduled with the TimeDomain.  When the delay has elapsed, the TimeDomain
// calls UpdateDelayedWorkQueue and ready delayed tasks are moved into the
// |delayed_work_queue|. Note the EnqueueOrder (used for ordering) for a delayed
// task is not set until it's moved into the |delayed_work_queue|.
//
// TaskQueueImpl uses the WorkQueueSets and the TaskQueueSelector to implement
// prioritization. Task selection is done by the TaskQueueSelector and when a
// queue is selected, it round-robins between the |immediate_work_queue| and
// |delayed_work_queue|.  The reason for this is we want to make sure delayed
// tasks (normally the most common type) don't starve out immediate work.
class BASE_EXPORT TaskQueueImpl {
 public:
  TaskQueueImpl(SequenceManagerImpl* sequence_manager,
                TimeDomain* time_domain,
                const TaskQueue::Spec& spec);

  ~TaskQueueImpl();

  enum class WorkQueueType { kImmediate, kDelayed };

  enum class CurrentThread { kMainThread, kNotMainThread };


  struct DeferredNonNestableTask {
    Task task;
    internal::TaskQueueImpl* task_queue;
    WorkQueueType work_queue_type;
  };

  using OnNextWakeUpChangedCallback = RepeatingCallback<void(TimeTicks)>;
  using OnTaskReadyHandler = RepeatingCallback<void(const Task&, LazyNow*)>;
  using OnTaskStartedHandler =
      RepeatingCallback<void(const Task&, const TaskQueue::TaskTiming&)>;
  using OnTaskCompletedHandler =
      RepeatingCallback<void(const Task&, TaskQueue::TaskTiming*, LazyNow*)>;

  scoped_refptr<SingleThreadTaskRunner> CreateTaskRunner(
      TaskType task_type) const;

  const char* GetName() const;
  bool IsQueueEnabled() const;
  void SetQueueEnabled(bool enabled);
  void SetShouldReportPostedTasksWhenDisabled(bool should_report);
  bool IsEmpty() const;
  size_t GetNumberOfPendingTasks() const;
  bool HasTaskToRunImmediately() const;
  Optional<TimeTicks> GetNextScheduledWakeUp();
  Optional<DelayedWakeUp> GetNextScheduledWakeUpImpl();
  void SetQueuePriority(TaskQueue::QueuePriority priority);
  TaskQueue::QueuePriority GetQueuePriority() const;
  void AddTaskObserver(TaskObserver* task_observer);
  void RemoveTaskObserver(TaskObserver* task_observer);
  void SetTimeDomain(TimeDomain* time_domain);
  TimeDomain* GetTimeDomain() const;
  void SetBlameContext(trace_event::BlameContext* blame_context);
  void InsertFence(TaskQueue::InsertFencePosition position);
  void InsertFenceAt(TimeTicks time);
  void RemoveFence();
  bool HasActiveFence();
  bool BlockedByFence() const;
  EnqueueOrder GetEnqueueOrderAtWhichWeBecameUnblocked() const;

  void SetObserver(TaskQueue::Observer* observer);

  void UnregisterTaskQueue();



  bool CouldTaskRun(EnqueueOrder enqueue_order) const;



  bool WasBlockedOrLowPriority(EnqueueOrder enqueue_order) const;

  void ReloadEmptyImmediateWorkQueue();

  void AsValueInto(TimeTicks now,
                   trace_event::TracedValue* state,
                   bool force_verbose) const;

  bool GetQuiescenceMonitored() const { return should_monitor_quiescence_; }
  bool GetShouldNotifyObservers() const { return should_notify_observers_; }

  void NotifyWillProcessTask(const Task& task,
                             bool was_blocked_or_low_priority);
  void NotifyDidProcessTask(const Task& task);


  bool HasPendingImmediateWork();
  bool HasPendingImmediateWorkLocked()
      EXCLUSIVE_LOCKS_REQUIRED(any_thread_lock_);

  bool has_pending_high_resolution_tasks() const {
    return main_thread_only()
        .delayed_incoming_queue.has_pending_high_resolution_tasks();
  }

  WorkQueue* delayed_work_queue() {
    return main_thread_only().delayed_work_queue.get();
  }

  const WorkQueue* delayed_work_queue() const {
    return main_thread_only().delayed_work_queue.get();
  }

  WorkQueue* immediate_work_queue() {
    return main_thread_only().immediate_work_queue.get();
  }

  const WorkQueue* immediate_work_queue() const {
    return main_thread_only().immediate_work_queue.get();
  }


  void MoveReadyDelayedTasksToWorkQueue(LazyNow* lazy_now);

  base::internal::HeapHandle heap_handle() const {
    return main_thread_only().heap_handle;
  }

  void set_heap_handle(base::internal::HeapHandle heap_handle) {
    main_thread_only().heap_handle = heap_handle;
  }



  void RequeueDeferredNonNestableTask(DeferredNonNestableTask task);

  void PushImmediateIncomingTaskForTest(Task&& task);


  void ReclaimMemory(TimeTicks now);



  void SetOnTaskReadyHandler(OnTaskReadyHandler handler);


  void SetOnTaskStartedHandler(OnTaskStartedHandler handler);
  void OnTaskStarted(const Task& task,
                     const TaskQueue::TaskTiming& task_timing);





  void SetOnTaskCompletedHandler(OnTaskCompletedHandler handler);
  void OnTaskCompleted(const Task& task,
                       TaskQueue::TaskTiming* task_timing,
                       LazyNow* lazy_now);
  bool RequiresTaskTiming() const;

  WeakPtr<SequenceManagerImpl> GetSequenceManagerWeakPtr();

  SequenceManagerImpl* sequence_manager() const { return sequence_manager_; }


  bool IsUnregistered() const;

  void DeletePendingTasks();


  bool HasTasks() const;

 protected:
  void SetDelayedWakeUpForTesting(Optional<DelayedWakeUp> wake_up);

 private:
  friend class WorkQueue;
  friend class WorkQueueTest;









  class GuardedTaskPoster : public RefCountedThreadSafe<GuardedTaskPoster> {
   public:
    explicit GuardedTaskPoster(TaskQueueImpl* outer);

    bool PostTask(PostedTask task);

    void StartAcceptingOperations() {
      operations_controller_.StartAcceptingOperations();
    }

    void ShutdownAndWaitForZeroOperations() {
      operations_controller_.ShutdownAndWaitForZeroOperations();
    }

   private:
    friend class RefCountedThreadSafe<GuardedTaskPoster>;

    ~GuardedTaskPoster();

    base::internal::OperationsController operations_controller_;

    TaskQueueImpl* const outer_;
  };

  class TaskRunner : public SingleThreadTaskRunner {
   public:
    explicit TaskRunner(scoped_refptr<GuardedTaskPoster> task_poster,
                        scoped_refptr<AssociatedThreadId> associated_thread,
                        TaskType task_type);

    bool PostDelayedTask(const Location& location,
                         OnceClosure callback,
                         TimeDelta delay) final;
    bool PostNonNestableDelayedTask(const Location& location,
                                    OnceClosure callback,
                                    TimeDelta delay) final;
    bool RunsTasksInCurrentSequence() const final;

   private:
    ~TaskRunner() final;

    bool PostTask(PostedTask task) const;

    const scoped_refptr<GuardedTaskPoster> task_poster_;
    const scoped_refptr<AssociatedThreadId> associated_thread_;
    const TaskType task_type_;
  };

  struct DelayedIncomingQueue {
   public:
    DelayedIncomingQueue();
    ~DelayedIncomingQueue();

    void push(Task&& task);
    void pop();
    bool empty() const { return queue_.empty(); }
    size_t size() const { return queue_.size(); }
    const Task& top() const { return queue_.top(); }
    void swap(DelayedIncomingQueue* other);

    bool has_pending_high_resolution_tasks() const {
      return pending_high_res_tasks_;
    }

    void SweepCancelledTasks();
    std::priority_queue<Task> TakeTasks() { return std::move(queue_); }
    void AsValueInto(TimeTicks now, trace_event::TracedValue* state) const;

   private:
    struct PQueue : public std::priority_queue<Task> {

      using std::priority_queue<Task>::c;
      using std::priority_queue<Task>::comp;
    };

    PQueue queue_;

    int pending_high_res_tasks_ = 0;

    DISALLOW_COPY_AND_ASSIGN(DelayedIncomingQueue);
  };

  struct MainThreadOnly {
    MainThreadOnly(TaskQueueImpl* task_queue, TimeDomain* time_domain);
    ~MainThreadOnly();


    TimeDomain* time_domain;

    TaskQueue::Observer* task_queue_observer = nullptr;

    std::unique_ptr<WorkQueue> delayed_work_queue;
    std::unique_ptr<WorkQueue> immediate_work_queue;
    DelayedIncomingQueue delayed_incoming_queue;
    ObserverList<TaskObserver>::Unchecked task_observers;
    base::internal::HeapHandle heap_handle;
    bool is_enabled = true;
    trace_event::BlameContext* blame_context = nullptr;  // Not owned.
    EnqueueOrder current_fence;
    Optional<TimeTicks> delayed_fence;




    EnqueueOrder enqueue_order_at_which_we_became_unblocked;
















    EnqueueOrder
        enqueue_order_at_which_we_became_unblocked_with_normal_priority;
    OnTaskReadyHandler on_task_ready_handler;
    OnTaskStartedHandler on_task_started_handler;
    OnTaskCompletedHandler on_task_completed_handler;


    Optional<DelayedWakeUp> scheduled_wake_up;

    bool is_enabled_for_test = true;


    Optional<TimeTicks> disabled_time;


    bool should_report_posted_tasks_when_disabled = false;
  };

  void PostTask(PostedTask task);

  void PostImmediateTaskImpl(PostedTask task, CurrentThread current_thread);
  void PostDelayedTaskImpl(PostedTask task, CurrentThread current_thread);


  void PushOntoDelayedIncomingQueueFromMainThread(Task pending_task,
                                                  TimeTicks now,
                                                  bool notify_task_annotator);


  void PushOntoDelayedIncomingQueue(Task pending_task);

  void ScheduleDelayedWorkTask(Task pending_task);

  void MoveReadyImmediateTasksToImmediateWorkQueueLocked()
      EXCLUSIVE_LOCKS_REQUIRED(any_thread_lock_);


  using TaskDeque =
      LazilyDeallocatedDeque<Task, subtle::TimeTicksNowIgnoringOverride>;



  void TakeImmediateIncomingQueueTasks(TaskDeque* queue);

  void TraceQueueSize() const;
  static void QueueAsValueInto(const TaskDeque& queue,
                               TimeTicks now,
                               trace_event::TracedValue* state);
  static void QueueAsValueInto(const std::priority_queue<Task>& queue,
                               TimeTicks now,
                               trace_event::TracedValue* state);
  static void TaskAsValueInto(const Task& task,
                              TimeTicks now,
                              trace_event::TracedValue* state);

  void UpdateDelayedWakeUp(LazyNow* lazy_now);
  void UpdateDelayedWakeUpImpl(LazyNow* lazy_now,
                               Optional<DelayedWakeUp> wake_up);

  void ActivateDelayedFenceIfNeeded(TimeTicks now);

  void UpdateCrossThreadQueueStateLocked()
      EXCLUSIVE_LOCKS_REQUIRED(any_thread_lock_);

  void MaybeLogPostTask(PostedTask* task);
  void MaybeAdjustTaskDelay(PostedTask* task, CurrentThread current_thread);


  void MaybeReportIpcTaskQueuedFromMainThread(Task* pending_task,
                                              const char* task_queue_name);
  bool ShouldReportIpcTaskQueuedFromAnyThreadLocked(
      base::TimeDelta* time_since_disabled)
      EXCLUSIVE_LOCKS_REQUIRED(any_thread_lock_);
  void MaybeReportIpcTaskQueuedFromAnyThreadLocked(Task* pending_task,
                                                   const char* task_queue_name)
      EXCLUSIVE_LOCKS_REQUIRED(any_thread_lock_);
  void MaybeReportIpcTaskQueuedFromAnyThreadUnlocked(
      Task* pending_task,
      const char* task_queue_name);
  void ReportIpcTaskQueued(Task* pending_task,
                           const char* task_queue_name,
                           const base::TimeDelta& time_since_disabled);

  void OnQueueUnblocked();

  const char* name_;
  SequenceManagerImpl* const sequence_manager_;

  scoped_refptr<AssociatedThreadId> associated_thread_;

  const scoped_refptr<GuardedTaskPoster> task_poster_;

  mutable base::internal::CheckedLock any_thread_lock_;

  struct AnyThread {

    struct TracingOnly {
      TracingOnly();
      ~TracingOnly();

      bool is_enabled = true;
      Optional<TimeTicks> disabled_time;
      bool should_report_posted_tasks_when_disabled = false;
    };

    explicit AnyThread(TimeDomain* time_domain);
    ~AnyThread();



    TimeDomain* time_domain;

    TaskQueue::Observer* task_queue_observer = nullptr;

    TaskDeque immediate_incoming_queue;

    bool immediate_work_queue_empty = true;

    bool post_immediate_task_should_schedule_work = true;

    bool unregistered = false;

    OnTaskReadyHandler on_task_ready_handler;

#if DCHECK_IS_ON()




    int queue_set_index = 0;
#endif

    TracingOnly tracing_only;
  };

  AnyThread any_thread_ GUARDED_BY(any_thread_lock_);

  MainThreadOnly main_thread_only_;
  MainThreadOnly& main_thread_only() {
    DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    return main_thread_only_;
  }
  const MainThreadOnly& main_thread_only() const {
    DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    return main_thread_only_;
  }





  AtomicFlagSet::AtomicFlag empty_queues_to_reload_handle_;

  const bool should_monitor_quiescence_;
  const bool should_notify_observers_;
  const bool delayed_fence_allowed_;

  DISALLOW_COPY_AND_ASSIGN(TaskQueueImpl);
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_TASK_QUEUE_IMPL_H_
