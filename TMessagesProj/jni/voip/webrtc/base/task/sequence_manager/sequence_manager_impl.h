// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_SEQUENCE_MANAGER_IMPL_H_
#define BASE_TASK_SEQUENCE_MANAGER_SEQUENCE_MANAGER_IMPL_H_

#include <list>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/atomic_sequence_num.h"
#include "base/cancelable_callback.h"
#include "base/containers/circular_deque.h"
#include "base/debug/crash_logging.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop_current.h"
#include "base/message_loop/message_pump_type.h"
#include "base/pending_task.h"
#include "base/run_loop.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/lock.h"
#include "base/task/common/task_annotator.h"
#include "base/task/sequence_manager/associated_thread_id.h"
#include "base/task/sequence_manager/enqueue_order.h"
#include "base/task/sequence_manager/enqueue_order_generator.h"
#include "base/task/sequence_manager/sequence_manager.h"
#include "base/task/sequence_manager/task_queue_impl.h"
#include "base/task/sequence_manager/task_queue_selector.h"
#include "base/task/sequence_manager/thread_controller.h"
#include "base/threading/thread_checker.h"
#include "base/time/default_tick_clock.h"
#include "build/build_config.h"

namespace base {

namespace trace_event {
class ConvertableToTraceFormat;
}  // namespace trace_event

namespace sequence_manager {

class SequenceManagerForTest;
class TaskQueue;
class TaskTimeObserver;
class TimeDomain;

namespace internal {

class RealTimeDomain;
class TaskQueueImpl;
class ThreadControllerImpl;

// choosing which task queue to service next. Each task queue consists of two
// sub queues:
//
// 1. Incoming task queue. Tasks that are posted get immediately appended here.
//    When a task is appended into an empty incoming queue, the task manager
//    work function (DoWork()) is scheduled to run on the main task runner.
//
// 2. Work queue. If a work queue is empty when DoWork() is entered, tasks from
//    the incoming task queue (if any) are moved here. The work queues are
//    registered with the selector as input to the scheduling decision.
//
class BASE_EXPORT SequenceManagerImpl
    : public SequenceManager,
      public internal::SequencedTaskSource,
      public internal::TaskQueueSelector::Observer,
      public RunLoop::NestingObserver {
 public:
  using Observer = SequenceManager::Observer;

  ~SequenceManagerImpl() override;




  static std::unique_ptr<SequenceManagerImpl> CreateOnCurrentThread(
      SequenceManager::Settings settings = SequenceManager::Settings());




  static std::unique_ptr<SequenceManagerImpl> CreateUnbound(
      SequenceManager::Settings settings);

  void BindToCurrentThread() override;
  scoped_refptr<SequencedTaskRunner> GetTaskRunnerForCurrentTask() override;
  void BindToMessagePump(std::unique_ptr<MessagePump> message_pump) override;
  void SetObserver(Observer* observer) override;
  void AddTaskTimeObserver(TaskTimeObserver* task_time_observer) override;
  void RemoveTaskTimeObserver(TaskTimeObserver* task_time_observer) override;
  void RegisterTimeDomain(TimeDomain* time_domain) override;
  void UnregisterTimeDomain(TimeDomain* time_domain) override;
  TimeDomain* GetRealTimeDomain() const override;
  const TickClock* GetTickClock() const override;
  TimeTicks NowTicks() const override;
  void SetDefaultTaskRunner(
      scoped_refptr<SingleThreadTaskRunner> task_runner) override;
  void ReclaimMemory() override;
  bool GetAndClearSystemIsQuiescentBit() override;
  void SetWorkBatchSize(int work_batch_size) override;
  void SetTimerSlack(TimerSlack timer_slack) override;
  void EnableCrashKeys(const char* async_stack_crash_key) override;
  const MetricRecordingSettings& GetMetricRecordingSettings() const override;
  size_t GetPendingTaskCountForTesting() const override;
  scoped_refptr<TaskQueue> CreateTaskQueue(
      const TaskQueue::Spec& spec) override;
  std::string DescribeAllPendingTasks() const override;
  std::unique_ptr<NativeWorkHandle> OnNativeWorkPending(
      TaskQueue::QueuePriority priority) override;
  void AddTaskObserver(TaskObserver* task_observer) override;
  void RemoveTaskObserver(TaskObserver* task_observer) override;

  Task* SelectNextTask() override;
  void DidRunTask() override;
  TimeDelta DelayTillNextTask(LazyNow* lazy_now) const override;
  bool HasPendingHighResolutionTasks() override;
  bool OnSystemIdle() override;

  void AddDestructionObserver(
      MessageLoopCurrent::DestructionObserver* destruction_observer);
  void RemoveDestructionObserver(
      MessageLoopCurrent::DestructionObserver* destruction_observer);

  void SetTaskRunner(scoped_refptr<SingleThreadTaskRunner> task_runner);

  scoped_refptr<SingleThreadTaskRunner> GetTaskRunner();
  bool IsBoundToCurrentThread() const;
  MessagePump* GetMessagePump() const;
  bool IsType(MessagePumpType type) const;
  void SetAddQueueTimeToTasks(bool enable);
  void SetTaskExecutionAllowed(bool allowed);
  bool IsTaskExecutionAllowed() const;
#if defined(OS_IOS)
  void AttachToMessagePump();
#endif
  bool IsIdleForTesting() override;
  void BindToCurrentThread(std::unique_ptr<MessagePump> pump);
  void DeletePendingTasks();
  bool HasTasks();
  MessagePumpType GetType() const;

  void ScheduleWork();






  void SetNextDelayedDoWork(LazyNow* lazy_now, TimeTicks run_time);


  internal::TaskQueueImpl* currently_executing_task_queue() const;


  void UnregisterTaskQueueImpl(
      std::unique_ptr<internal::TaskQueueImpl> task_queue);

  void ShutdownTaskQueueGracefully(
      std::unique_ptr<internal::TaskQueueImpl> task_queue);

  const scoped_refptr<AssociatedThreadId>& associated_thread() const {
    return associated_thread_;
  }

  const Settings& settings() const { return settings_; }

  WeakPtr<SequenceManagerImpl> GetWeakPtr();

  static constexpr TimeDelta kReclaimMemoryInterval =
      TimeDelta::FromSeconds(30);

 protected:
  static std::unique_ptr<ThreadControllerImpl>
  CreateThreadControllerImplForCurrentThread(const TickClock* clock);


  SequenceManagerImpl(std::unique_ptr<internal::ThreadController> controller,
                      SequenceManager::Settings settings = Settings());

  friend class internal::TaskQueueImpl;
  friend class ::base::sequence_manager::SequenceManagerForTest;

 private:
  class NativeWorkHandleImpl;



  static SequenceManagerImpl* GetCurrent();
  friend class ::base::MessageLoopCurrent;

  enum class ProcessTaskResult {
    kDeferred,
    kExecuted,
    kSequenceManagerDeleted,
  };


  using NonNestableTaskDeque =
      circular_deque<internal::TaskQueueImpl::DeferredNonNestableTask>;




  struct ExecutingTask {
    ExecutingTask(Task&& task,
                  internal::TaskQueueImpl* task_queue,
                  TaskQueue::TaskTiming task_timing)
        : pending_task(std::move(task)),
          task_queue(task_queue),
          task_queue_name(task_queue->GetName()),
          task_timing(task_timing),
          priority(task_queue->GetQueuePriority()),
          task_type(pending_task.task_type) {}

    Task pending_task;
    internal::TaskQueueImpl* task_queue = nullptr;

    const char* task_queue_name;
    TaskQueue::TaskTiming task_timing;

    TaskQueue::QueuePriority priority;


    int task_type;
  };

  struct MainThreadOnly {
    explicit MainThreadOnly(
        const scoped_refptr<AssociatedThreadId>& associated_thread,
        const SequenceManager::Settings& settings);
    ~MainThreadOnly();

    int nesting_depth = 0;
    NonNestableTaskDeque non_nestable_task_queue;


    debug::CrashKeyString* file_name_crash_key = nullptr;
    debug::CrashKeyString* function_name_crash_key = nullptr;
    debug::CrashKeyString* async_stack_crash_key = nullptr;
    std::array<char, static_cast<size_t>(debug::CrashKeySize::Size64)>
        async_stack_buffer = {};

    std::mt19937_64 random_generator;
    std::uniform_real_distribution<double> uniform_distribution;

    internal::TaskQueueSelector selector;
    ObserverList<TaskObserver>::Unchecked task_observers;
    ObserverList<TaskTimeObserver>::Unchecked task_time_observers;
    std::set<TimeDomain*> time_domains;
    std::unique_ptr<internal::RealTimeDomain> real_time_domain;

    bool memory_reclaim_scheduled = false;

    TimeTicks next_time_to_reclaim_memory;










    std::set<internal::TaskQueueImpl*> active_queues;

    std::map<internal::TaskQueueImpl*, std::unique_ptr<internal::TaskQueueImpl>>
        queues_to_gracefully_shutdown;
    std::map<internal::TaskQueueImpl*, std::unique_ptr<internal::TaskQueueImpl>>
        queues_to_delete;

    bool task_was_run_on_quiescence_monitored_queue = false;
    bool nesting_observer_registered_ = false;

    std::vector<ExecutingTask> task_execution_stack;

    Observer* observer = nullptr;  // NOT OWNED

    ObserverList<MessageLoopCurrent::DestructionObserver>::Unchecked
        destruction_observers;

    std::multiset<TaskQueue::QueuePriority> pending_native_work{
        TaskQueue::kBestEffortPriority};
  };

  void CompleteInitializationOnBoundThread();

  void OnTaskQueueEnabled(internal::TaskQueueImpl* queue) override;

  void OnBeginNestedRunLoop() override;
  void OnExitNestedRunLoop() override;



  void WillQueueTask(Task* pending_task, const char* task_queue_name);


  void MoveReadyDelayedTasksToWorkQueues(LazyNow* lazy_now);

  void NotifyWillProcessTask(ExecutingTask* task, LazyNow* time_before_task);
  void NotifyDidProcessTask(ExecutingTask* task, LazyNow* time_after_task);

  EnqueueOrder GetNextSequenceNumber();

  bool GetAddQueueTimeToTasks();

  std::unique_ptr<trace_event::ConvertableToTraceFormat>
  AsValueWithSelectorResult(internal::WorkQueue* selected_work_queue,
                            bool force_verbose) const;
  void AsValueWithSelectorResultInto(trace_event::TracedValue*,
                                     internal::WorkQueue* selected_work_queue,
                                     bool force_verbose) const;





  AtomicFlagSet::AtomicFlag GetFlagToRequestReloadForEmptyQueue(
      TaskQueueImpl* task_queue);


  void ReloadEmptyWorkQueues() const;

  std::unique_ptr<internal::TaskQueueImpl> CreateTaskQueueImpl(
      const TaskQueue::Spec& spec) override;


  void MaybeReclaimMemory();

  void CleanUpQueues();

  void RemoveAllCanceledTasksFromFrontOfWorkQueues();

  TaskQueue::TaskTiming::TimeRecordingPolicy ShouldRecordTaskTiming(
      const internal::TaskQueueImpl* task_queue);
  bool ShouldRecordCPUTimeForTask();
  void RecordCrashKeys(const PendingTask&);


  Task* SelectNextTaskImpl();


  bool ShouldRunTaskOfPriority(TaskQueue::QueuePriority priority) const;

  TimeDelta GetDelayTillNextDelayedTask(LazyNow* lazy_now) const;

#if DCHECK_IS_ON()
  void LogTaskDebugInfo(const internal::WorkQueue* work_queue) const;
#endif


  TaskQueue::TaskTiming InitializeTaskTiming(
      internal::TaskQueueImpl* task_queue);

  scoped_refptr<AssociatedThreadId> associated_thread_;

  EnqueueOrderGenerator enqueue_order_generator_;

  const std::unique_ptr<internal::ThreadController> controller_;
  const Settings settings_;

  const MetricRecordingSettings metric_recording_settings_;

  base::subtle::Atomic32 add_queue_time_to_tasks_;

  AtomicFlagSet empty_queues_to_reload_;


  bool Validate();

  volatile int32_t memory_corruption_sentinel_;

  MainThreadOnly main_thread_only_;
  MainThreadOnly& main_thread_only() {
    DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    return main_thread_only_;
  }
  const MainThreadOnly& main_thread_only() const {
    DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    return main_thread_only_;
  }

  WeakPtrFactory<SequenceManagerImpl> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(SequenceManagerImpl);
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_SEQUENCE_MANAGER_IMPL_H_
