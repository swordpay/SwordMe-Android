// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/task_tracker.h"

#include <atomic>
#include <string>
#include <vector>

#include "base/base_switches.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/debug/alias.h"
#include "base/json/json_writer.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/histogram_macros.h"
#include "base/optional.h"
#include "base/sequence_token.h"
#include "base/synchronization/condition_variable.h"
#include "base/task/scoped_set_task_priority_for_current_thread.h"
#include "base/task/task_executor.h"
#include "base/threading/sequence_local_storage_map.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "base/values.h"
#include "build/build_config.h"

namespace base {
namespace internal {

namespace {

constexpr const char* kExecutionModeString[] = {"parallel", "sequenced",
                                                "single thread", "job"};
static_assert(
    size(kExecutionModeString) ==
        static_cast<size_t>(TaskSourceExecutionMode::kMax) + 1,
    "Array kExecutionModeString is out of sync with TaskSourceExecutionMode.");

class TaskTracingInfo : public trace_event::ConvertableToTraceFormat {
 public:
  TaskTracingInfo(const TaskTraits& task_traits,
                  const char* execution_mode,
                  const SequenceToken& sequence_token)
      : task_traits_(task_traits),
        execution_mode_(execution_mode),
        sequence_token_(sequence_token) {}

  void AppendAsTraceFormat(std::string* out) const override;

 private:
  const TaskTraits task_traits_;
  const char* const execution_mode_;
  const SequenceToken sequence_token_;

  DISALLOW_COPY_AND_ASSIGN(TaskTracingInfo);
};

void TaskTracingInfo::AppendAsTraceFormat(std::string* out) const {
  DictionaryValue dict;

  dict.SetStringKey("task_priority",
                    base::TaskPriorityToString(task_traits_.priority()));
  dict.SetStringKey("execution_mode", execution_mode_);
  if (sequence_token_.IsValid())
    dict.SetIntKey("sequence_token", sequence_token_.ToInternalValue());

  std::string tmp;
  JSONWriter::Write(dict, &tmp);
  out->append(tmp);
}

// "ThreadPool.{histogram_name}.{histogram_label}.{task_type_suffix}".
HistogramBase* GetLatencyHistogram(StringPiece histogram_name,
                                   StringPiece histogram_label,
                                   StringPiece task_type_suffix) {
  DCHECK(!histogram_name.empty());
  DCHECK(!task_type_suffix.empty());

  if (histogram_label.empty())
    return nullptr;





  const std::string histogram = JoinString(
      {"ThreadPool", histogram_name, histogram_label, task_type_suffix}, ".");
  return Histogram::FactoryMicrosecondsTimeGet(
      histogram, TimeDelta::FromMicroseconds(1),
      TimeDelta::FromMilliseconds(20), 50,
      HistogramBase::kUmaTargetedHistogramFlag);
}

// "ThreadPool.{histogram_name}.{histogram_label}.{task_type_suffix}".
HistogramBase* GetCountHistogram(StringPiece histogram_name,
                                 StringPiece histogram_label,
                                 StringPiece task_type_suffix) {
  DCHECK(!histogram_name.empty());
  DCHECK(!task_type_suffix.empty());

  if (histogram_label.empty())
    return nullptr;

  const std::string histogram = JoinString(
      {"ThreadPool", histogram_name, histogram_label, task_type_suffix}, ".");



  return Histogram::FactoryGet(histogram, 1, 500, 50,
                               HistogramBase::kUmaTargetedHistogramFlag);
}

// TODO(jessemckenna): use the STATIC_HISTOGRAM_POINTER_GROUP macro from
// histogram_macros.h instead.
HistogramBase* GetHistogramForTaskPriority(TaskPriority task_priority,
                                           HistogramBase* const histograms[3]) {
  return histograms[static_cast<int>(task_priority)];
}

bool HasLogBestEffortTasksSwitch() {


  return CommandLine::InitializedForCurrentProcess() &&
         CommandLine::ForCurrentProcess()->HasSwitch(
             switches::kLogBestEffortTasks);
}

// duration of a threadpool task invocation.
class EphemeralTaskExecutor : public TaskExecutor {
 public:


  EphemeralTaskExecutor(SequencedTaskRunner* sequenced_task_runner,
                        SingleThreadTaskRunner* single_thread_task_runner,
                        const TaskTraits* sequence_traits)
      : sequenced_task_runner_(sequenced_task_runner),
        single_thread_task_runner_(single_thread_task_runner),
        sequence_traits_(sequence_traits) {
    SetTaskExecutorForCurrentThread(this);
  }

  ~EphemeralTaskExecutor() override {
    SetTaskExecutorForCurrentThread(nullptr);
  }

  bool PostDelayedTask(const Location& from_here,
                       const TaskTraits& traits,
                       OnceClosure task,
                       TimeDelta delay) override {
    CheckTraitsCompatibleWithSequenceTraits(traits);
    return sequenced_task_runner_->PostDelayedTask(from_here, std::move(task),
                                                   delay);
  }

  scoped_refptr<TaskRunner> CreateTaskRunner(
      const TaskTraits& traits) override {
    CheckTraitsCompatibleWithSequenceTraits(traits);
    return sequenced_task_runner_;
  }

  scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits) override {
    CheckTraitsCompatibleWithSequenceTraits(traits);
    return sequenced_task_runner_;
  }

  scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override {
    CheckTraitsCompatibleWithSequenceTraits(traits);
    return single_thread_task_runner_;
  }

#if defined(OS_WIN)
  scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override {
    CheckTraitsCompatibleWithSequenceTraits(traits);
    return single_thread_task_runner_;
  }
#endif  // defined(OS_WIN)

 private:

  void CheckTraitsCompatibleWithSequenceTraits(const TaskTraits& traits) {
    if (traits.shutdown_behavior_set_explicitly()) {
      DCHECK_EQ(traits.shutdown_behavior(),
                sequence_traits_->shutdown_behavior());
    }

    DCHECK(!traits.may_block() ||
           traits.may_block() == sequence_traits_->may_block());

    DCHECK(!traits.with_base_sync_primitives() ||
           traits.with_base_sync_primitives() ==
               sequence_traits_->with_base_sync_primitives());
  }

  SequencedTaskRunner* const sequenced_task_runner_;
  SingleThreadTaskRunner* const single_thread_task_runner_;
  const TaskTraits* const sequence_traits_;
};

}  // namespace

// Shutdown. An "item" consist of either:
// - A running SKIP_ON_SHUTDOWN task
// - A queued/running BLOCK_SHUTDOWN TaskSource.
// Sequential consistency shouldn't be assumed from these calls (i.e. a thread
// reading |HasShutdownStarted() == true| isn't guaranteed to see all writes
// made before |StartShutdown()| on the thread that invoked it).
class TaskTracker::State {
 public:
  State() = default;


  bool StartShutdown() {
    const auto new_value =
        subtle::NoBarrier_AtomicIncrement(&bits_, kShutdownHasStartedMask);


    DCHECK(new_value & kShutdownHasStartedMask);

    const auto num_items_blocking_shutdown =
        new_value >> kNumItemsBlockingShutdownBitOffset;
    return num_items_blocking_shutdown != 0;
  }

  bool HasShutdownStarted() const {
    return subtle::NoBarrier_Load(&bits_) & kShutdownHasStartedMask;
  }

  bool AreItemsBlockingShutdown() const {
    const auto num_items_blocking_shutdown =
        subtle::NoBarrier_Load(&bits_) >> kNumItemsBlockingShutdownBitOffset;
    DCHECK_GE(num_items_blocking_shutdown, 0);
    return num_items_blocking_shutdown != 0;
  }


  bool IncrementNumItemsBlockingShutdown() {
#if DCHECK_IS_ON()

    const auto num_items_blocking_shutdown =
        subtle::NoBarrier_Load(&bits_) >> kNumItemsBlockingShutdownBitOffset;
    DCHECK_LT(num_items_blocking_shutdown,
              std::numeric_limits<subtle::Atomic32>::max() -
                  kNumItemsBlockingShutdownIncrement);
#endif

    const auto new_bits = subtle::NoBarrier_AtomicIncrement(
        &bits_, kNumItemsBlockingShutdownIncrement);
    return new_bits & kShutdownHasStartedMask;
  }


  bool DecrementNumItemsBlockingShutdown() {
    const auto new_bits = subtle::NoBarrier_AtomicIncrement(
        &bits_, -kNumItemsBlockingShutdownIncrement);
    const bool shutdown_has_started = new_bits & kShutdownHasStartedMask;
    const auto num_items_blocking_shutdown =
        new_bits >> kNumItemsBlockingShutdownBitOffset;
    DCHECK_GE(num_items_blocking_shutdown, 0);
    return shutdown_has_started && num_items_blocking_shutdown == 0;
  }

 private:
  static constexpr subtle::Atomic32 kShutdownHasStartedMask = 1;
  static constexpr subtle::Atomic32 kNumItemsBlockingShutdownBitOffset = 1;
  static constexpr subtle::Atomic32 kNumItemsBlockingShutdownIncrement =
      1 << kNumItemsBlockingShutdownBitOffset;











  subtle::Atomic32 bits_ = 0;

  DISALLOW_COPY_AND_ASSIGN(State);
};

TaskTracker::TaskTracker(StringPiece histogram_label)
    : histogram_label_(histogram_label),
      has_log_best_effort_tasks_switch_(HasLogBestEffortTasksSwitch()),
      state_(new State),
      can_run_policy_(CanRunPolicy::kAll),
      flush_cv_(flush_lock_.CreateConditionVariable()),
      shutdown_lock_(&flush_lock_),
      task_latency_histograms_{GetLatencyHistogram("TaskLatencyMicroseconds",
                                                   histogram_label,
                                                   "BackgroundTaskPriority"),
                               GetLatencyHistogram("TaskLatencyMicroseconds",
                                                   histogram_label,
                                                   "UserVisibleTaskPriority"),
                               GetLatencyHistogram("TaskLatencyMicroseconds",
                                                   histogram_label,
                                                   "UserBlockingTaskPriority")},
      heartbeat_latency_histograms_{
          GetLatencyHistogram("HeartbeatLatencyMicroseconds",
                              histogram_label,
                              "BackgroundTaskPriority"),
          GetLatencyHistogram("HeartbeatLatencyMicroseconds",
                              histogram_label,
                              "UserVisibleTaskPriority"),
          GetLatencyHistogram("HeartbeatLatencyMicroseconds",
                              histogram_label,
                              "UserBlockingTaskPriority")},
      num_tasks_run_while_queuing_histograms_{
          GetCountHistogram("NumTasksRunWhileQueuing",
                            histogram_label,
                            "BackgroundTaskPriority"),
          GetCountHistogram("NumTasksRunWhileQueuing",
                            histogram_label,
                            "UserVisibleTaskPriority"),
          GetCountHistogram("NumTasksRunWhileQueuing",
                            histogram_label,
                            "UserBlockingTaskPriority")},
      tracked_ref_factory_(this) {}

TaskTracker::~TaskTracker() = default;

void TaskTracker::StartShutdown() {
  CheckedAutoLock auto_lock(shutdown_lock_);

  DCHECK(!shutdown_event_);
  DCHECK(!state_->HasShutdownStarted());

  shutdown_event_ = std::make_unique<WaitableEvent>();

  const bool tasks_are_blocking_shutdown = state_->StartShutdown();



  if (!tasks_are_blocking_shutdown) {





    shutdown_event_->Signal();
    return;
  }
}

void TaskTracker::CompleteShutdown() {



  DCHECK(TS_UNCHECKED_READ(shutdown_event_));
  {
    base::ScopedAllowBaseSyncPrimitives allow_wait;
    TS_UNCHECKED_READ(shutdown_event_)->Wait();
  }


  {
    CheckedAutoLock auto_lock(flush_lock_);
    flush_cv_->Signal();
  }
  CallFlushCallbackForTesting();
}

void TaskTracker::FlushForTesting() {
  CheckedAutoLock auto_lock(flush_lock_);
  while (num_incomplete_task_sources_.load(std::memory_order_acquire) != 0 &&
         !IsShutdownComplete()) {
    flush_cv_->Wait();
  }
}

void TaskTracker::FlushAsyncForTesting(OnceClosure flush_callback) {
  DCHECK(flush_callback);
  {
    CheckedAutoLock auto_lock(flush_lock_);
    DCHECK(!flush_callback_for_testing_)
        << "Only one FlushAsyncForTesting() may be pending at any time.";
    flush_callback_for_testing_ = std::move(flush_callback);
  }

  if (num_incomplete_task_sources_.load(std::memory_order_acquire) == 0 ||
      IsShutdownComplete()) {
    CallFlushCallbackForTesting();
  }
}

void TaskTracker::SetCanRunPolicy(CanRunPolicy can_run_policy) {
  can_run_policy_.store(can_run_policy);
}

bool TaskTracker::WillPostTask(Task* task,
                               TaskShutdownBehavior shutdown_behavior) {
  DCHECK(task);
  DCHECK(task->task);

  if (state_->HasShutdownStarted()) {


    if (shutdown_behavior != TaskShutdownBehavior::BLOCK_SHUTDOWN ||
        !task->delayed_run_time.is_null()) {
      return false;
    }


    CheckedAutoLock auto_lock(shutdown_lock_);
    DCHECK(shutdown_event_);
    DCHECK(!shutdown_event_->IsSignaled());
  }

  task_annotator_.WillQueueTask("ThreadPool_PostTask", task, "");

  return true;
}

bool TaskTracker::WillPostTaskNow(const Task& task, TaskPriority priority) {
  if (!task.delayed_run_time.is_null() && state_->HasShutdownStarted())
    return false;
  if (has_log_best_effort_tasks_switch_ &&
      priority == TaskPriority::BEST_EFFORT) {

    LOG(INFO) << task.posted_from.ToString();
  }
  return true;
}

RegisteredTaskSource TaskTracker::RegisterTaskSource(
    scoped_refptr<TaskSource> task_source) {
  DCHECK(task_source);

  TaskShutdownBehavior shutdown_behavior = task_source->shutdown_behavior();
  if (!BeforeQueueTaskSource(shutdown_behavior))
    return nullptr;

  num_incomplete_task_sources_.fetch_add(1, std::memory_order_relaxed);
  return RegisteredTaskSource(std::move(task_source), this);
}

bool TaskTracker::CanRunPriority(TaskPriority priority) const {
  auto can_run_policy = can_run_policy_.load();

  if (can_run_policy == CanRunPolicy::kAll)
    return true;

  if (can_run_policy == CanRunPolicy::kForegroundOnly &&
      priority >= TaskPriority::USER_VISIBLE) {
    return true;
  }

  return false;
}

RegisteredTaskSource TaskTracker::RunAndPopNextTask(
    RegisteredTaskSource task_source) {
  DCHECK(task_source);

  const bool should_run_tasks = BeforeRunTask(task_source->shutdown_behavior());

  Optional<Task> task;
  TaskTraits traits;
  {
    auto transaction = task_source->BeginTransaction();
    task = should_run_tasks ? task_source.TakeTask(&transaction)
                            : task_source.Clear(&transaction);
    traits = transaction.traits();
  }

  if (task) {

    RunTask(std::move(task.value()), task_source.get(), traits);
  }
  if (should_run_tasks)
    AfterRunTask(task_source->shutdown_behavior());
  const bool task_source_must_be_queued = task_source.DidProcessTask();

  if (task_source_must_be_queued)
    return task_source;
  return nullptr;
}

bool TaskTracker::HasShutdownStarted() const {
  return state_->HasShutdownStarted();
}

bool TaskTracker::IsShutdownComplete() const {
  CheckedAutoLock auto_lock(shutdown_lock_);
  return shutdown_event_ && shutdown_event_->IsSignaled();
}

void TaskTracker::RecordLatencyHistogram(TaskPriority priority,
                                         TimeTicks posted_time) const {
  if (histogram_label_.empty())
    return;

  const TimeDelta task_latency = TimeTicks::Now() - posted_time;
  GetHistogramForTaskPriority(priority, task_latency_histograms_)
      ->AddTimeMicrosecondsGranularity(task_latency);
}

void TaskTracker::RecordHeartbeatLatencyAndTasksRunWhileQueuingHistograms(
    TaskPriority priority,
    TimeTicks posted_time,
    int num_tasks_run_when_posted) const {
  if (histogram_label_.empty())
    return;

  const TimeDelta task_latency = TimeTicks::Now() - posted_time;
  GetHistogramForTaskPriority(priority, heartbeat_latency_histograms_)
      ->AddTimeMicrosecondsGranularity(task_latency);

  GetHistogramForTaskPriority(priority, num_tasks_run_while_queuing_histograms_)
      ->Add(GetNumTasksRun() - num_tasks_run_when_posted);
}

int TaskTracker::GetNumTasksRun() const {
  return num_tasks_run_.load(std::memory_order_relaxed);
}

void TaskTracker::IncrementNumTasksRun() {
  num_tasks_run_.fetch_add(1, std::memory_order_relaxed);
}

void TaskTracker::RunTask(Task task,
                          TaskSource* task_source,
                          const TaskTraits& traits) {
  DCHECK(task_source);
  RecordLatencyHistogram(traits.priority(), task.queue_time);

  const auto environment = task_source->GetExecutionEnvironment();

  const bool previous_singleton_allowed =
      ThreadRestrictions::SetSingletonAllowed(
          traits.shutdown_behavior() !=
          TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN);
  const bool previous_io_allowed =
      ThreadRestrictions::SetIOAllowed(traits.may_block());
  const bool previous_wait_allowed =
      ThreadRestrictions::SetWaitAllowed(traits.with_base_sync_primitives());

  {
    DCHECK(environment.token.IsValid());
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(environment.token);
    ScopedSetTaskPriorityForCurrentThread
        scoped_set_task_priority_for_current_thread(traits.priority());

    Optional<SequenceLocalStorageMap> local_storage_map;
    if (!environment.sequence_local_storage)
      local_storage_map.emplace();

    ScopedSetSequenceLocalStorageMapForCurrentThread
        scoped_set_sequence_local_storage_map_for_current_thread(
            environment.sequence_local_storage
                ? environment.sequence_local_storage
                : &local_storage_map.value());

    Optional<SequencedTaskRunnerHandle> sequenced_task_runner_handle;
    Optional<ThreadTaskRunnerHandle> single_thread_task_runner_handle;
    Optional<EphemeralTaskExecutor> ephemeral_task_executor;
    switch (task_source->execution_mode()) {
      case TaskSourceExecutionMode::kJob:
      case TaskSourceExecutionMode::kParallel:
        break;
      case TaskSourceExecutionMode::kSequenced:
        DCHECK(task_source->task_runner());
        sequenced_task_runner_handle.emplace(
            static_cast<SequencedTaskRunner*>(task_source->task_runner()));
        ephemeral_task_executor.emplace(
            static_cast<SequencedTaskRunner*>(task_source->task_runner()),
            nullptr, &traits);
        break;
      case TaskSourceExecutionMode::kSingleThread:
        DCHECK(task_source->task_runner());
        single_thread_task_runner_handle.emplace(
            static_cast<SingleThreadTaskRunner*>(task_source->task_runner()));
        ephemeral_task_executor.emplace(
            static_cast<SequencedTaskRunner*>(task_source->task_runner()),
            static_cast<SingleThreadTaskRunner*>(task_source->task_runner()),
            &traits);
        break;
    }

    TRACE_TASK_EXECUTION("ThreadPool_RunTask", task);



    TRACE_EVENT1("thread_pool", "ThreadPool_TaskInfo", "task_info",
                 std::make_unique<TaskTracingInfo>(
                     traits,
                     kExecutionModeString[static_cast<size_t>(
                         task_source->execution_mode())],
                     environment.token));

    RunTaskWithShutdownBehavior(traits.shutdown_behavior(), &task);


    task.task = OnceClosure();
  }

  ThreadRestrictions::SetWaitAllowed(previous_wait_allowed);
  ThreadRestrictions::SetIOAllowed(previous_io_allowed);
  ThreadRestrictions::SetSingletonAllowed(previous_singleton_allowed);
}

bool TaskTracker::HasIncompleteTaskSourcesForTesting() const {
  return num_incomplete_task_sources_.load(std::memory_order_acquire) != 0;
}

bool TaskTracker::BeforeQueueTaskSource(
    TaskShutdownBehavior shutdown_behavior) {
  if (shutdown_behavior == TaskShutdownBehavior::BLOCK_SHUTDOWN) {


    const bool shutdown_started = state_->IncrementNumItemsBlockingShutdown();

    if (shutdown_started) {


      CheckedAutoLock auto_lock(shutdown_lock_);
      DCHECK(shutdown_event_);
      DCHECK(!shutdown_event_->IsSignaled());
    }

    return true;
  }


  return !state_->HasShutdownStarted();
}

bool TaskTracker::BeforeRunTask(TaskShutdownBehavior shutdown_behavior) {
  switch (shutdown_behavior) {
    case TaskShutdownBehavior::BLOCK_SHUTDOWN: {


      DCHECK(state_->AreItemsBlockingShutdown());




      DCHECK(!state_->HasShutdownStarted() || !IsShutdownComplete());

      return true;
    }

    case TaskShutdownBehavior::SKIP_ON_SHUTDOWN: {

      const bool shutdown_started = state_->IncrementNumItemsBlockingShutdown();

      if (shutdown_started) {



        DecrementNumItemsBlockingShutdown();
        return false;
      }

      return true;
    }

    case TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN: {
      return !state_->HasShutdownStarted();
    }
  }

  NOTREACHED();
  return false;
}

void TaskTracker::AfterRunTask(TaskShutdownBehavior shutdown_behavior) {
  IncrementNumTasksRun();
  if (shutdown_behavior == TaskShutdownBehavior::SKIP_ON_SHUTDOWN) {
    DecrementNumItemsBlockingShutdown();
  }
}

scoped_refptr<TaskSource> TaskTracker::UnregisterTaskSource(
    scoped_refptr<TaskSource> task_source) {
  DCHECK(task_source);
  if (task_source->shutdown_behavior() ==
      TaskShutdownBehavior::BLOCK_SHUTDOWN) {
    DecrementNumItemsBlockingShutdown();
  }
  DecrementNumIncompleteTaskSources();
  return task_source;
}

void TaskTracker::DecrementNumItemsBlockingShutdown() {
  const bool shutdown_started_and_no_items_block_shutdown =
      state_->DecrementNumItemsBlockingShutdown();
  if (!shutdown_started_and_no_items_block_shutdown)
    return;

  CheckedAutoLock auto_lock(shutdown_lock_);
  DCHECK(shutdown_event_);
  shutdown_event_->Signal();
}

void TaskTracker::DecrementNumIncompleteTaskSources() {
  const auto prev_num_incomplete_task_sources =
      num_incomplete_task_sources_.fetch_sub(1);
  DCHECK_GE(prev_num_incomplete_task_sources, 1);
  if (prev_num_incomplete_task_sources == 1) {
    {
      CheckedAutoLock auto_lock(flush_lock_);
      flush_cv_->Signal();
    }
    CallFlushCallbackForTesting();
  }
}

void TaskTracker::CallFlushCallbackForTesting() {
  OnceClosure flush_callback;
  {
    CheckedAutoLock auto_lock(flush_lock_);
    flush_callback = std::move(flush_callback_for_testing_);
  }
  if (flush_callback)
    std::move(flush_callback).Run();
}

NOINLINE void TaskTracker::RunContinueOnShutdown(Task* task) {
  const int line_number = __LINE__;
  task_annotator_.RunTask("ThreadPool_RunTask_ContinueOnShutdown", task);
  base::debug::Alias(&line_number);
}

NOINLINE void TaskTracker::RunSkipOnShutdown(Task* task) {
  const int line_number = __LINE__;
  task_annotator_.RunTask("ThreadPool_RunTask_SkipOnShutdown", task);
  base::debug::Alias(&line_number);
}

NOINLINE void TaskTracker::RunBlockShutdown(Task* task) {
  const int line_number = __LINE__;
  task_annotator_.RunTask("ThreadPool_RunTask_BlockShutdown", task);
  base::debug::Alias(&line_number);
}

void TaskTracker::RunTaskWithShutdownBehavior(
    TaskShutdownBehavior shutdown_behavior,
    Task* task) {
  switch (shutdown_behavior) {
    case TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN:
      RunContinueOnShutdown(task);
      return;
    case TaskShutdownBehavior::SKIP_ON_SHUTDOWN:
      RunSkipOnShutdown(task);
      return;
    case TaskShutdownBehavior::BLOCK_SHUTDOWN:
      RunBlockShutdown(task);
      return;
  }
}

}  // namespace internal
}  // namespace base
