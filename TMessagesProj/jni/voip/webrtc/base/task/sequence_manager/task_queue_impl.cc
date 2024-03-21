// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/sequence_manager/task_queue_impl.h"

#include <memory>
#include <utility>

#include "base/strings/stringprintf.h"
#include "base/task/common/scoped_defer_task_posting.h"
#include "base/task/sequence_manager/sequence_manager_impl.h"
#include "base/task/sequence_manager/time_domain.h"
#include "base/task/sequence_manager/work_queue.h"
#include "base/task/task_observer.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"
#include "base/trace_event/blame_context.h"
#include "base/trace_event/common/trace_event_common.h"
#include "build/build_config.h"

namespace base {
namespace sequence_manager {

namespace {

base::TimeTicks GetTaskDesiredExecutionTime(const Task& task) {
  if (!task.delayed_run_time.is_null())
    return task.delayed_run_time;

  DCHECK(!task.queue_time.is_null());
  return task.queue_time;
}

}  // namespace

const char* TaskQueue::PriorityToString(TaskQueue::QueuePriority priority) {
  switch (priority) {
    case kControlPriority:
      return "control";
    case kHighestPriority:
      return "highest";
    case kVeryHighPriority:
      return "very_high";
    case kHighPriority:
      return "high";
    case kNormalPriority:
      return "normal";
    case kLowPriority:
      return "low";
    case kBestEffortPriority:
      return "best_effort";
    default:
      NOTREACHED();
      return nullptr;
  }
}

namespace internal {

TaskQueueImpl::GuardedTaskPoster::GuardedTaskPoster(TaskQueueImpl* outer)
    : outer_(outer) {}

TaskQueueImpl::GuardedTaskPoster::~GuardedTaskPoster() {}

bool TaskQueueImpl::GuardedTaskPoster::PostTask(PostedTask task) {


  ScopedDeferTaskPosting disallow_task_posting;

  auto token = operations_controller_.TryBeginOperation();
  if (!token)
    return false;

  outer_->PostTask(std::move(task));
  return true;
}

TaskQueueImpl::TaskRunner::TaskRunner(
    scoped_refptr<GuardedTaskPoster> task_poster,
    scoped_refptr<AssociatedThreadId> associated_thread,
    TaskType task_type)
    : task_poster_(std::move(task_poster)),
      associated_thread_(std::move(associated_thread)),
      task_type_(task_type) {}

TaskQueueImpl::TaskRunner::~TaskRunner() {}

bool TaskQueueImpl::TaskRunner::PostDelayedTask(const Location& location,
                                                OnceClosure callback,
                                                TimeDelta delay) {
  return task_poster_->PostTask(PostedTask(this, std::move(callback), location,
                                           delay, Nestable::kNestable,
                                           task_type_));
}

bool TaskQueueImpl::TaskRunner::PostNonNestableDelayedTask(
    const Location& location,
    OnceClosure callback,
    TimeDelta delay) {
  return task_poster_->PostTask(PostedTask(this, std::move(callback), location,
                                           delay, Nestable::kNonNestable,
                                           task_type_));
}

bool TaskQueueImpl::TaskRunner::RunsTasksInCurrentSequence() const {
  return associated_thread_->IsBoundToCurrentThread();
}

TaskQueueImpl::TaskQueueImpl(SequenceManagerImpl* sequence_manager,
                             TimeDomain* time_domain,
                             const TaskQueue::Spec& spec)
    : name_(spec.name),
      sequence_manager_(sequence_manager),
      associated_thread_(sequence_manager
                             ? sequence_manager->associated_thread()
                             : AssociatedThreadId::CreateBound()),
      task_poster_(MakeRefCounted<GuardedTaskPoster>(this)),
      any_thread_(time_domain),
      main_thread_only_(this, time_domain),
      empty_queues_to_reload_handle_(
          sequence_manager
              ? sequence_manager->GetFlagToRequestReloadForEmptyQueue(this)
              : AtomicFlagSet::AtomicFlag()),
      should_monitor_quiescence_(spec.should_monitor_quiescence),
      should_notify_observers_(spec.should_notify_observers),
      delayed_fence_allowed_(spec.delayed_fence_allowed) {
  DCHECK(time_domain);
  UpdateCrossThreadQueueStateLocked();


  if (sequence_manager_)
    task_poster_->StartAcceptingOperations();
}

TaskQueueImpl::~TaskQueueImpl() {
#if DCHECK_IS_ON()
  base::internal::CheckedAutoLock lock(any_thread_lock_);




  DCHECK(any_thread_.unregistered)
      << "UnregisterTaskQueue must be called first!";
#endif
}

TaskQueueImpl::AnyThread::AnyThread(TimeDomain* time_domain)
    : time_domain(time_domain) {}

TaskQueueImpl::AnyThread::~AnyThread() = default;

TaskQueueImpl::AnyThread::TracingOnly::TracingOnly() = default;
TaskQueueImpl::AnyThread::TracingOnly::~TracingOnly() = default;

TaskQueueImpl::MainThreadOnly::MainThreadOnly(TaskQueueImpl* task_queue,
                                              TimeDomain* time_domain)
    : time_domain(time_domain),
      delayed_work_queue(
          new WorkQueue(task_queue, "delayed", WorkQueue::QueueType::kDelayed)),
      immediate_work_queue(new WorkQueue(task_queue,
                                         "immediate",
                                         WorkQueue::QueueType::kImmediate)) {}

TaskQueueImpl::MainThreadOnly::~MainThreadOnly() = default;

scoped_refptr<SingleThreadTaskRunner> TaskQueueImpl::CreateTaskRunner(
    TaskType task_type) const {
  return MakeRefCounted<TaskRunner>(task_poster_, associated_thread_,
                                    task_type);
}

void TaskQueueImpl::UnregisterTaskQueue() {
  TRACE_EVENT0("base", "TaskQueueImpl::UnregisterTaskQueue");

  {
    ScopedAllowBaseSyncPrimitivesOutsideBlockingScope allow_wait;
    task_poster_->ShutdownAndWaitForZeroOperations();
  }

  TaskDeque immediate_incoming_queue;

  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    any_thread_.unregistered = true;
    any_thread_.time_domain = nullptr;
    immediate_incoming_queue.swap(any_thread_.immediate_incoming_queue);
    any_thread_.task_queue_observer = nullptr;
  }

  if (main_thread_only().time_domain)
    main_thread_only().time_domain->UnregisterQueue(this);

  main_thread_only().on_task_completed_handler = OnTaskCompletedHandler();
  main_thread_only().time_domain = nullptr;
  main_thread_only().task_queue_observer = nullptr;
  empty_queues_to_reload_handle_.ReleaseAtomicFlag();











  DelayedIncomingQueue delayed_incoming_queue;
  delayed_incoming_queue.swap(&main_thread_only().delayed_incoming_queue);
  std::unique_ptr<WorkQueue> immediate_work_queue =
      std::move(main_thread_only().immediate_work_queue);
  std::unique_ptr<WorkQueue> delayed_work_queue =
      std::move(main_thread_only().delayed_work_queue);
}

const char* TaskQueueImpl::GetName() const {
  return name_;
}

void TaskQueueImpl::PostTask(PostedTask task) {
  CurrentThread current_thread =
      associated_thread_->IsBoundToCurrentThread()
          ? TaskQueueImpl::CurrentThread::kMainThread
          : TaskQueueImpl::CurrentThread::kNotMainThread;

#if DCHECK_IS_ON()
  MaybeLogPostTask(&task);
  MaybeAdjustTaskDelay(&task, current_thread);
#endif  // DCHECK_IS_ON()

  if (task.delay.is_zero()) {
    PostImmediateTaskImpl(std::move(task), current_thread);
  } else {
    PostDelayedTaskImpl(std::move(task), current_thread);
  }
}

void TaskQueueImpl::MaybeLogPostTask(PostedTask* task) {
#if DCHECK_IS_ON()
  if (!sequence_manager_->settings().log_post_task)
    return;

  LOG(INFO) << name_ << " PostTask " << task->location.ToString() << " delay "
            << task->delay;
#endif  // DCHECK_IS_ON()
}

void TaskQueueImpl::MaybeAdjustTaskDelay(PostedTask* task,
                                         CurrentThread current_thread) {
#if DCHECK_IS_ON()
  if (current_thread == TaskQueueImpl::CurrentThread::kNotMainThread) {
    base::internal::CheckedAutoLock lock(any_thread_lock_);


    task->delay +=
        sequence_manager_->settings()
            .per_priority_cross_thread_task_delay[any_thread_.queue_set_index];
  } else {
    task->delay +=
        sequence_manager_->settings().per_priority_same_thread_task_delay
            [main_thread_only().immediate_work_queue->work_queue_set_index()];
  }
#endif  // DCHECK_IS_ON()
}

void TaskQueueImpl::PostImmediateTaskImpl(PostedTask task,
                                          CurrentThread current_thread) {


  CHECK(task.callback);

  bool should_schedule_work = false;
  {


    base::internal::CheckedAutoLock lock(any_thread_lock_);
    LazyNow lazy_now = any_thread_.time_domain->CreateLazyNow();
    bool add_queue_time_to_tasks = sequence_manager_->GetAddQueueTimeToTasks();
    if (add_queue_time_to_tasks || delayed_fence_allowed_)
      task.queue_time = lazy_now.Now();




    EnqueueOrder sequence_number = sequence_manager_->GetNextSequenceNumber();
    bool was_immediate_incoming_queue_empty =
        any_thread_.immediate_incoming_queue.empty();

    base::TimeTicks delayed_run_time;
    any_thread_.immediate_incoming_queue.push_back(Task(
        std::move(task), delayed_run_time, sequence_number, sequence_number));

    if (any_thread_.on_task_ready_handler) {
      any_thread_.on_task_ready_handler.Run(
          any_thread_.immediate_incoming_queue.back(), &lazy_now);
    }

#if DCHECK_IS_ON()
    any_thread_.immediate_incoming_queue.back().cross_thread_ =
        (current_thread == TaskQueueImpl::CurrentThread::kNotMainThread);
#endif

    sequence_manager_->WillQueueTask(
        &any_thread_.immediate_incoming_queue.back(), name_);
    MaybeReportIpcTaskQueuedFromAnyThreadLocked(
        &any_thread_.immediate_incoming_queue.back(), name_);




    if (was_immediate_incoming_queue_empty &&
        any_thread_.immediate_work_queue_empty) {
      empty_queues_to_reload_handle_.SetActive(true);
      should_schedule_work =
          any_thread_.post_immediate_task_should_schedule_work;
    }
  }











  if (should_schedule_work)
    sequence_manager_->ScheduleWork();

  TraceQueueSize();
}

void TaskQueueImpl::PostDelayedTaskImpl(PostedTask task,
                                        CurrentThread current_thread) {


  CHECK(task.callback);
  DCHECK_GT(task.delay, TimeDelta());

  WakeUpResolution resolution = WakeUpResolution::kLow;
#if defined(OS_WIN)




  if (task.delay.InMilliseconds() < (2 * Time::kMinLowResolutionThresholdMs))
    resolution = WakeUpResolution::kHigh;
#endif  // defined(OS_WIN)

  if (current_thread == CurrentThread::kMainThread) {

    EnqueueOrder sequence_number = sequence_manager_->GetNextSequenceNumber();

    TimeTicks time_domain_now = main_thread_only().time_domain->Now();
    TimeTicks time_domain_delayed_run_time = time_domain_now + task.delay;
    if (sequence_manager_->GetAddQueueTimeToTasks())
      task.queue_time = time_domain_now;

    PushOntoDelayedIncomingQueueFromMainThread(
        Task(std::move(task), time_domain_delayed_run_time, sequence_number,
             EnqueueOrder(), resolution),
        time_domain_now, /* notify_task_annotator */ true);
  } else {




    EnqueueOrder sequence_number = sequence_manager_->GetNextSequenceNumber();

    TimeTicks time_domain_now;
    {
      base::internal::CheckedAutoLock lock(any_thread_lock_);
      time_domain_now = any_thread_.time_domain->Now();
    }
    TimeTicks time_domain_delayed_run_time = time_domain_now + task.delay;
    if (sequence_manager_->GetAddQueueTimeToTasks())
      task.queue_time = time_domain_now;

    PushOntoDelayedIncomingQueue(
        Task(std::move(task), time_domain_delayed_run_time, sequence_number,
             EnqueueOrder(), resolution));
  }
}

void TaskQueueImpl::PushOntoDelayedIncomingQueueFromMainThread(
    Task pending_task,
    TimeTicks now,
    bool notify_task_annotator) {
#if DCHECK_IS_ON()
  pending_task.cross_thread_ = false;
#endif

  if (notify_task_annotator) {
    sequence_manager_->WillQueueTask(&pending_task, name_);
    MaybeReportIpcTaskQueuedFromMainThread(&pending_task, name_);
  }
  main_thread_only().delayed_incoming_queue.push(std::move(pending_task));

  LazyNow lazy_now(now);
  UpdateDelayedWakeUp(&lazy_now);

  TraceQueueSize();
}

void TaskQueueImpl::PushOntoDelayedIncomingQueue(Task pending_task) {
  sequence_manager_->WillQueueTask(&pending_task, name_);
  MaybeReportIpcTaskQueuedFromAnyThreadUnlocked(&pending_task, name_);

#if DCHECK_IS_ON()
  pending_task.cross_thread_ = true;
#endif

  auto task_runner = pending_task.task_runner;
  PostImmediateTaskImpl(
      PostedTask(std::move(task_runner),
                 BindOnce(&TaskQueueImpl::ScheduleDelayedWorkTask,
                          Unretained(this), std::move(pending_task)),
                 FROM_HERE, TimeDelta(), Nestable::kNonNestable,
                 pending_task.task_type),
      CurrentThread::kNotMainThread);
}

void TaskQueueImpl::ScheduleDelayedWorkTask(Task pending_task) {
  DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  TimeTicks delayed_run_time = pending_task.delayed_run_time;
  TimeTicks time_domain_now = main_thread_only().time_domain->Now();
  if (delayed_run_time <= time_domain_now) {



    delayed_run_time = time_domain_now;
    pending_task.delayed_run_time = time_domain_now;
    main_thread_only().delayed_incoming_queue.push(std::move(pending_task));
    LazyNow lazy_now(time_domain_now);
    MoveReadyDelayedTasksToWorkQueue(&lazy_now);


  } else {

    PushOntoDelayedIncomingQueueFromMainThread(std::move(pending_task),
                                               time_domain_now, false);
  }
  TraceQueueSize();
}

void TaskQueueImpl::ReloadEmptyImmediateWorkQueue() {
  DCHECK(main_thread_only().immediate_work_queue->Empty());
  main_thread_only().immediate_work_queue->TakeImmediateIncomingQueueTasks();

  if (main_thread_only().task_queue_observer && IsQueueEnabled()) {
    main_thread_only().task_queue_observer->OnQueueNextWakeUpChanged(
        TimeTicks());
  }
}

void TaskQueueImpl::TakeImmediateIncomingQueueTasks(TaskDeque* queue) {
  base::internal::CheckedAutoLock lock(any_thread_lock_);
  DCHECK(queue->empty());
  queue->swap(any_thread_.immediate_incoming_queue);


  any_thread_.immediate_incoming_queue.MaybeShrinkQueue();





  if (main_thread_only().delayed_fence) {
    for (const Task& task : *queue) {
      DCHECK(!task.queue_time.is_null());
      DCHECK(task.delayed_run_time.is_null());
      if (task.queue_time >= main_thread_only().delayed_fence.value()) {
        main_thread_only().delayed_fence = nullopt;
        DCHECK(!main_thread_only().current_fence);
        main_thread_only().current_fence = task.enqueue_order();


        main_thread_only().immediate_work_queue->InsertFenceSilently(
            main_thread_only().current_fence);
        main_thread_only().delayed_work_queue->InsertFenceSilently(
            main_thread_only().current_fence);
        break;
      }
    }
  }

  UpdateCrossThreadQueueStateLocked();
}

bool TaskQueueImpl::IsEmpty() const {
  if (!main_thread_only().delayed_work_queue->Empty() ||
      !main_thread_only().delayed_incoming_queue.empty() ||
      !main_thread_only().immediate_work_queue->Empty()) {
    return false;
  }

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  return any_thread_.immediate_incoming_queue.empty();
}

size_t TaskQueueImpl::GetNumberOfPendingTasks() const {
  size_t task_count = 0;
  task_count += main_thread_only().delayed_work_queue->Size();
  task_count += main_thread_only().delayed_incoming_queue.size();
  task_count += main_thread_only().immediate_work_queue->Size();

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  task_count += any_thread_.immediate_incoming_queue.size();
  return task_count;
}

bool TaskQueueImpl::HasTaskToRunImmediately() const {

  if (!main_thread_only().delayed_work_queue->Empty() ||
      !main_thread_only().immediate_work_queue->Empty()) {
    return true;
  }


  if (!main_thread_only().delayed_incoming_queue.empty() &&
      main_thread_only().delayed_incoming_queue.top().delayed_run_time <=
          main_thread_only().time_domain->CreateLazyNow().Now()) {
    return true;
  }

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  return !any_thread_.immediate_incoming_queue.empty();
}

Optional<DelayedWakeUp> TaskQueueImpl::GetNextScheduledWakeUpImpl() {

  if (main_thread_only().delayed_incoming_queue.empty() || !IsQueueEnabled())
    return nullopt;

  return main_thread_only().delayed_incoming_queue.top().delayed_wake_up();
}

Optional<TimeTicks> TaskQueueImpl::GetNextScheduledWakeUp() {
  Optional<DelayedWakeUp> wake_up = GetNextScheduledWakeUpImpl();
  if (!wake_up)
    return nullopt;
  return wake_up->time;
}

void TaskQueueImpl::MoveReadyDelayedTasksToWorkQueue(LazyNow* lazy_now) {


  WorkQueue::TaskPusher delayed_work_queue_task_pusher(
      main_thread_only().delayed_work_queue->CreateTaskPusher());

  while (!main_thread_only().delayed_incoming_queue.empty()) {
    Task* task =
        const_cast<Task*>(&main_thread_only().delayed_incoming_queue.top());

    sequence_manager_->RecordCrashKeys(*task);
    if (!task->task || task->task.IsCancelled()) {
      main_thread_only().delayed_incoming_queue.pop();
      continue;
    }
    if (task->delayed_run_time > lazy_now->Now())
      break;
#if DCHECK_IS_ON()
    if (sequence_manager_->settings().log_task_delay_expiry)
      VLOG(0) << name_ << " Delay expired for " << task->posted_from.ToString();
#endif  // DCHECK_IS_ON()

    ActivateDelayedFenceIfNeeded(GetTaskDesiredExecutionTime(*task));
    DCHECK(!task->enqueue_order_set());
    task->set_enqueue_order(sequence_manager_->GetNextSequenceNumber());

    if (main_thread_only().on_task_ready_handler)
      main_thread_only().on_task_ready_handler.Run(*task, lazy_now);

    delayed_work_queue_task_pusher.Push(task);
    main_thread_only().delayed_incoming_queue.pop();
  }

  UpdateDelayedWakeUp(lazy_now);
}

void TaskQueueImpl::TraceQueueSize() const {
  bool is_tracing;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(
      TRACE_DISABLED_BY_DEFAULT("sequence_manager"), &is_tracing);
  if (!is_tracing)
    return;


  if (!associated_thread_->IsBoundToCurrentThread())
    return;

  size_t total_task_count;
  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    total_task_count = any_thread_.immediate_incoming_queue.size() +
                       main_thread_only().immediate_work_queue->Size() +
                       main_thread_only().delayed_work_queue->Size() +
                       main_thread_only().delayed_incoming_queue.size();
  }
  TRACE_COUNTER1(TRACE_DISABLED_BY_DEFAULT("sequence_manager"), GetName(),
                 total_task_count);
}

void TaskQueueImpl::SetQueuePriority(TaskQueue::QueuePriority priority) {
  const TaskQueue::QueuePriority previous_priority = GetQueuePriority();
  if (priority == previous_priority)
    return;
  sequence_manager_->main_thread_only().selector.SetQueuePriority(this,
                                                                  priority);

  static_assert(TaskQueue::QueuePriority::kLowPriority >
                    TaskQueue::QueuePriority::kNormalPriority,
                "Priorities are not ordered as expected");
  if (priority > TaskQueue::QueuePriority::kNormalPriority) {

    main_thread_only()
        .enqueue_order_at_which_we_became_unblocked_with_normal_priority =
        EnqueueOrder::max();
  } else if (previous_priority > TaskQueue::QueuePriority::kNormalPriority) {


    DCHECK_EQ(
        main_thread_only()
            .enqueue_order_at_which_we_became_unblocked_with_normal_priority,
        EnqueueOrder::max());
    main_thread_only()
        .enqueue_order_at_which_we_became_unblocked_with_normal_priority =
        sequence_manager_->GetNextSequenceNumber();
  }
}

TaskQueue::QueuePriority TaskQueueImpl::GetQueuePriority() const {
  size_t set_index = immediate_work_queue()->work_queue_set_index();
  DCHECK_EQ(set_index, delayed_work_queue()->work_queue_set_index());
  return static_cast<TaskQueue::QueuePriority>(set_index);
}

void TaskQueueImpl::AsValueInto(TimeTicks now,
                                trace_event::TracedValue* state,
                                bool force_verbose) const {
  base::internal::CheckedAutoLock lock(any_thread_lock_);
  state->BeginDictionary();
  state->SetString("name", GetName());
  if (any_thread_.unregistered) {
    state->SetBoolean("unregistered", true);
    state->EndDictionary();
    return;
  }
  DCHECK(main_thread_only().time_domain);
  DCHECK(main_thread_only().delayed_work_queue);
  DCHECK(main_thread_only().immediate_work_queue);

  state->SetString(
      "task_queue_id",
      StringPrintf("0x%" PRIx64,
                   static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this))));
  state->SetBoolean("enabled", IsQueueEnabled());
  state->SetString("time_domain_name",
                   main_thread_only().time_domain->GetName());
  state->SetInteger("any_thread_.immediate_incoming_queuesize",
                    any_thread_.immediate_incoming_queue.size());
  state->SetInteger("delayed_incoming_queue_size",
                    main_thread_only().delayed_incoming_queue.size());
  state->SetInteger("immediate_work_queue_size",
                    main_thread_only().immediate_work_queue->Size());
  state->SetInteger("delayed_work_queue_size",
                    main_thread_only().delayed_work_queue->Size());

  state->SetInteger("any_thread_.immediate_incoming_queuecapacity",
                    any_thread_.immediate_incoming_queue.capacity());
  state->SetInteger("immediate_work_queue_capacity",
                    immediate_work_queue()->Capacity());
  state->SetInteger("delayed_work_queue_capacity",
                    delayed_work_queue()->Capacity());

  if (!main_thread_only().delayed_incoming_queue.empty()) {
    TimeDelta delay_to_next_task =
        (main_thread_only().delayed_incoming_queue.top().delayed_run_time -
         main_thread_only().time_domain->CreateLazyNow().Now());
    state->SetDouble("delay_to_next_task_ms",
                     delay_to_next_task.InMillisecondsF());
  }
  if (main_thread_only().current_fence)
    state->SetInteger("current_fence", main_thread_only().current_fence);
  if (main_thread_only().delayed_fence) {
    state->SetDouble(
        "delayed_fence_seconds_from_now",
        (main_thread_only().delayed_fence.value() - now).InSecondsF());
  }

  bool verbose = false;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(
      TRACE_DISABLED_BY_DEFAULT("sequence_manager.verbose_snapshots"),
      &verbose);

  if (verbose || force_verbose) {
    state->BeginArray("immediate_incoming_queue");
    QueueAsValueInto(any_thread_.immediate_incoming_queue, now, state);
    state->EndArray();
    state->BeginArray("delayed_work_queue");
    main_thread_only().delayed_work_queue->AsValueInto(now, state);
    state->EndArray();
    state->BeginArray("immediate_work_queue");
    main_thread_only().immediate_work_queue->AsValueInto(now, state);
    state->EndArray();
    state->BeginArray("delayed_incoming_queue");
    main_thread_only().delayed_incoming_queue.AsValueInto(now, state);
    state->EndArray();
  }
  state->SetString("priority", TaskQueue::PriorityToString(GetQueuePriority()));
  state->EndDictionary();
}

void TaskQueueImpl::AddTaskObserver(TaskObserver* task_observer) {
  main_thread_only().task_observers.AddObserver(task_observer);
}

void TaskQueueImpl::RemoveTaskObserver(TaskObserver* task_observer) {
  main_thread_only().task_observers.RemoveObserver(task_observer);
}

void TaskQueueImpl::NotifyWillProcessTask(const Task& task,
                                          bool was_blocked_or_low_priority) {
  DCHECK(should_notify_observers_);

  if (main_thread_only().blame_context)
    main_thread_only().blame_context->Enter();

  for (auto& observer : main_thread_only().task_observers)
    observer.WillProcessTask(task, was_blocked_or_low_priority);
}

void TaskQueueImpl::NotifyDidProcessTask(const Task& task) {
  DCHECK(should_notify_observers_);
  for (auto& observer : main_thread_only().task_observers)
    observer.DidProcessTask(task);
  if (main_thread_only().blame_context)
    main_thread_only().blame_context->Leave();
}

void TaskQueueImpl::SetTimeDomain(TimeDomain* time_domain) {
  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    DCHECK(time_domain);
    DCHECK(!any_thread_.unregistered);
    if (any_thread_.unregistered)
      return;
    DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    if (time_domain == main_thread_only().time_domain)
      return;

    any_thread_.time_domain = time_domain;
  }

  main_thread_only().time_domain->UnregisterQueue(this);
  main_thread_only().time_domain = time_domain;

  LazyNow lazy_now = time_domain->CreateLazyNow();




  main_thread_only().scheduled_wake_up = nullopt;
  UpdateDelayedWakeUp(&lazy_now);
}

TimeDomain* TaskQueueImpl::GetTimeDomain() const {
  DCHECK(associated_thread_->IsBoundToCurrentThread() ||
         !associated_thread_->IsBound());
  return main_thread_only().time_domain;
}

void TaskQueueImpl::SetBlameContext(trace_event::BlameContext* blame_context) {
  main_thread_only().blame_context = blame_context;
}

void TaskQueueImpl::InsertFence(TaskQueue::InsertFencePosition position) {

  main_thread_only().delayed_fence = nullopt;

  EnqueueOrder previous_fence = main_thread_only().current_fence;
  EnqueueOrder current_fence = position == TaskQueue::InsertFencePosition::kNow
                                   ? sequence_manager_->GetNextSequenceNumber()
                                   : EnqueueOrder::blocking_fence();


  main_thread_only().current_fence = current_fence;
  bool front_task_unblocked =
      main_thread_only().immediate_work_queue->InsertFence(current_fence);
  front_task_unblocked |=
      main_thread_only().delayed_work_queue->InsertFence(current_fence);

  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    if (!front_task_unblocked && previous_fence &&
        previous_fence < current_fence) {
      if (!any_thread_.immediate_incoming_queue.empty() &&
          any_thread_.immediate_incoming_queue.front().enqueue_order() >
              previous_fence &&
          any_thread_.immediate_incoming_queue.front().enqueue_order() <
              current_fence) {
        front_task_unblocked = true;
      }
    }

    UpdateCrossThreadQueueStateLocked();
  }

  if (IsQueueEnabled() && front_task_unblocked) {
    OnQueueUnblocked();
    sequence_manager_->ScheduleWork();
  }
}

void TaskQueueImpl::InsertFenceAt(TimeTicks time) {
  DCHECK(delayed_fence_allowed_)
      << "Delayed fences are not supported for this queue. Enable them "
         "explicitly in TaskQueue::Spec when creating the queue";

  RemoveFence();
  main_thread_only().delayed_fence = time;
}

void TaskQueueImpl::RemoveFence() {
  EnqueueOrder previous_fence = main_thread_only().current_fence;
  main_thread_only().current_fence = EnqueueOrder::none();
  main_thread_only().delayed_fence = nullopt;

  bool front_task_unblocked =
      main_thread_only().immediate_work_queue->RemoveFence();
  front_task_unblocked |= main_thread_only().delayed_work_queue->RemoveFence();

  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    if (!front_task_unblocked && previous_fence) {
      if (!any_thread_.immediate_incoming_queue.empty() &&
          any_thread_.immediate_incoming_queue.front().enqueue_order() >
              previous_fence) {
        front_task_unblocked = true;
      }
    }

    UpdateCrossThreadQueueStateLocked();
  }

  if (IsQueueEnabled() && front_task_unblocked) {
    OnQueueUnblocked();
    sequence_manager_->ScheduleWork();
  }
}

bool TaskQueueImpl::BlockedByFence() const {
  if (!main_thread_only().current_fence)
    return false;

  if (!main_thread_only().immediate_work_queue->BlockedByFence() ||
      !main_thread_only().delayed_work_queue->BlockedByFence()) {
    return false;
  }

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  if (any_thread_.immediate_incoming_queue.empty())
    return true;

  return any_thread_.immediate_incoming_queue.front().enqueue_order() >
         main_thread_only().current_fence;
}

bool TaskQueueImpl::HasActiveFence() {
  if (main_thread_only().delayed_fence &&
      main_thread_only().time_domain->Now() >
          main_thread_only().delayed_fence.value()) {
    return true;
  }
  return !!main_thread_only().current_fence;
}

EnqueueOrder TaskQueueImpl::GetEnqueueOrderAtWhichWeBecameUnblocked() const {
  return main_thread_only().enqueue_order_at_which_we_became_unblocked;
}

bool TaskQueueImpl::CouldTaskRun(EnqueueOrder enqueue_order) const {
  if (!IsQueueEnabled())
    return false;

  if (!main_thread_only().current_fence)
    return true;

  return enqueue_order < main_thread_only().current_fence;
}

bool TaskQueueImpl::WasBlockedOrLowPriority(EnqueueOrder enqueue_order) const {
  return enqueue_order <
         main_thread_only()
             .enqueue_order_at_which_we_became_unblocked_with_normal_priority;
}

void TaskQueueImpl::QueueAsValueInto(const TaskDeque& queue,
                                     TimeTicks now,
                                     trace_event::TracedValue* state) {
  for (const Task& task : queue) {
    TaskAsValueInto(task, now, state);
  }
}

void TaskQueueImpl::TaskAsValueInto(const Task& task,
                                    TimeTicks now,
                                    trace_event::TracedValue* state) {
  state->BeginDictionary();
  state->SetString("posted_from", task.posted_from.ToString());
  if (task.enqueue_order_set())
    state->SetInteger("enqueue_order", task.enqueue_order());
  state->SetInteger("sequence_num", task.sequence_num);
  state->SetBoolean("nestable", task.nestable == Nestable::kNestable);
  state->SetBoolean("is_high_res", task.is_high_res);
  state->SetBoolean("is_cancelled", task.task.IsCancelled());
  state->SetDouble("delayed_run_time",
                   (task.delayed_run_time - TimeTicks()).InMillisecondsF());
  const TimeDelta delayed_run_time_milliseconds_from_now =
      task.delayed_run_time.is_null() ? TimeDelta()
                                      : (task.delayed_run_time - now);
  state->SetDouble("delayed_run_time_milliseconds_from_now",
                   delayed_run_time_milliseconds_from_now.InMillisecondsF());
  state->EndDictionary();
}

bool TaskQueueImpl::IsQueueEnabled() const {
  return main_thread_only().is_enabled;
}

void TaskQueueImpl::SetQueueEnabled(bool enabled) {
  if (main_thread_only().is_enabled == enabled)
    return;

  main_thread_only().is_enabled = enabled;
  main_thread_only().disabled_time = nullopt;
  if (!enabled) {
    bool tracing_enabled = false;
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                                       &tracing_enabled);
    main_thread_only().disabled_time = main_thread_only().time_domain->Now();
  } else {

    main_thread_only().should_report_posted_tasks_when_disabled = false;
  }

  LazyNow lazy_now = main_thread_only().time_domain->CreateLazyNow();
  UpdateDelayedWakeUp(&lazy_now);

  bool has_pending_immediate_work = false;

  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    UpdateCrossThreadQueueStateLocked();
    has_pending_immediate_work = HasPendingImmediateWorkLocked();

    any_thread_.tracing_only.is_enabled = enabled;
    any_thread_.tracing_only.disabled_time = main_thread_only().disabled_time;
    any_thread_.tracing_only.should_report_posted_tasks_when_disabled =
        main_thread_only().should_report_posted_tasks_when_disabled;
  }

  if (!sequence_manager_)
    return;

  if (enabled) {
    if (has_pending_immediate_work && main_thread_only().task_queue_observer) {

      main_thread_only().task_queue_observer->OnQueueNextWakeUpChanged(
          TimeTicks());
    }


    sequence_manager_->main_thread_only().selector.EnableQueue(this);

    if (!BlockedByFence())
      OnQueueUnblocked();
  } else {
    sequence_manager_->main_thread_only().selector.DisableQueue(this);
  }
}

void TaskQueueImpl::SetShouldReportPostedTasksWhenDisabled(bool should_report) {
  if (main_thread_only().should_report_posted_tasks_when_disabled ==
      should_report)
    return;

  if (should_report) {
    bool tracing_enabled = false;
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                                       &tracing_enabled);
    if (!tracing_enabled)
      return;
  }

  main_thread_only().should_report_posted_tasks_when_disabled = should_report;

  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    any_thread_.tracing_only.should_report_posted_tasks_when_disabled =
        should_report;
  }
}

void TaskQueueImpl::UpdateCrossThreadQueueStateLocked() {
  any_thread_.immediate_work_queue_empty =
      main_thread_only().immediate_work_queue->Empty();

  if (main_thread_only().task_queue_observer) {



    any_thread_.post_immediate_task_should_schedule_work = IsQueueEnabled();
  } else {


    any_thread_.post_immediate_task_should_schedule_work =
        IsQueueEnabled() && !main_thread_only().current_fence;
  }

#if DCHECK_IS_ON()
  any_thread_.queue_set_index =
      main_thread_only().immediate_work_queue->work_queue_set_index();
#endif
}

void TaskQueueImpl::ReclaimMemory(TimeTicks now) {
  if (main_thread_only().delayed_incoming_queue.empty())
    return;
  main_thread_only().delayed_incoming_queue.SweepCancelledTasks();

  main_thread_only().delayed_work_queue->MaybeShrinkQueue();
  main_thread_only().immediate_work_queue->MaybeShrinkQueue();

  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    any_thread_.immediate_incoming_queue.MaybeShrinkQueue();
  }

  LazyNow lazy_now(now);
  UpdateDelayedWakeUp(&lazy_now);
}

void TaskQueueImpl::PushImmediateIncomingTaskForTest(Task&& task) {
  base::internal::CheckedAutoLock lock(any_thread_lock_);
  any_thread_.immediate_incoming_queue.push_back(std::move(task));
}

void TaskQueueImpl::RequeueDeferredNonNestableTask(
    DeferredNonNestableTask task) {
  DCHECK(task.task.nestable == Nestable::kNonNestable);




  if (task.work_queue_type == WorkQueueType::kDelayed) {
    main_thread_only().delayed_work_queue->PushNonNestableTaskToFront(
        std::move(task.task));
  } else {





    if (main_thread_only().immediate_work_queue->Empty()) {
      base::internal::CheckedAutoLock lock(any_thread_lock_);
      empty_queues_to_reload_handle_.SetActive(false);

      any_thread_.immediate_work_queue_empty = false;
      main_thread_only().immediate_work_queue->PushNonNestableTaskToFront(
          std::move(task.task));

    } else {
      main_thread_only().immediate_work_queue->PushNonNestableTaskToFront(
          std::move(task.task));
    }
  }
}

void TaskQueueImpl::SetObserver(TaskQueue::Observer* observer) {
  if (observer) {
    DCHECK(!main_thread_only().task_queue_observer)
        << "Can't assign two different observers to "
           "base::sequence_manager:TaskQueue";
  }

  main_thread_only().task_queue_observer = observer;

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  any_thread_.task_queue_observer = observer;
}

void TaskQueueImpl::UpdateDelayedWakeUp(LazyNow* lazy_now) {
  return UpdateDelayedWakeUpImpl(lazy_now, GetNextScheduledWakeUpImpl());
}

void TaskQueueImpl::UpdateDelayedWakeUpImpl(LazyNow* lazy_now,
                                            Optional<DelayedWakeUp> wake_up) {
  if (main_thread_only().scheduled_wake_up == wake_up)
    return;
  main_thread_only().scheduled_wake_up = wake_up;

  if (wake_up && main_thread_only().task_queue_observer &&
      !HasPendingImmediateWork()) {
    main_thread_only().task_queue_observer->OnQueueNextWakeUpChanged(
        wake_up->time);
  }

  WakeUpResolution resolution = has_pending_high_resolution_tasks()
                                    ? WakeUpResolution::kHigh
                                    : WakeUpResolution::kLow;
  main_thread_only().time_domain->SetNextWakeUpForQueue(this, wake_up,
                                                        resolution, lazy_now);
}

void TaskQueueImpl::SetDelayedWakeUpForTesting(
    Optional<DelayedWakeUp> wake_up) {
  LazyNow lazy_now = main_thread_only().time_domain->CreateLazyNow();
  UpdateDelayedWakeUpImpl(&lazy_now, wake_up);
}

bool TaskQueueImpl::HasPendingImmediateWork() {

  if (!main_thread_only().delayed_work_queue->Empty() ||
      !main_thread_only().immediate_work_queue->Empty()) {
    return true;
  }

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  return !any_thread_.immediate_incoming_queue.empty();
}

bool TaskQueueImpl::HasPendingImmediateWorkLocked() {
  return !main_thread_only().delayed_work_queue->Empty() ||
         !main_thread_only().immediate_work_queue->Empty() ||
         !any_thread_.immediate_incoming_queue.empty();
}

void TaskQueueImpl::SetOnTaskReadyHandler(
    TaskQueueImpl::OnTaskReadyHandler handler) {
  DCHECK(should_notify_observers_ || handler.is_null());
  main_thread_only().on_task_ready_handler = handler;

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  DCHECK_NE(!!any_thread_.on_task_ready_handler, !!handler);
  any_thread_.on_task_ready_handler = std::move(handler);
}

void TaskQueueImpl::SetOnTaskStartedHandler(
    TaskQueueImpl::OnTaskStartedHandler handler) {
  DCHECK(should_notify_observers_ || handler.is_null());
  main_thread_only().on_task_started_handler = std::move(handler);
}

void TaskQueueImpl::OnTaskStarted(const Task& task,
                                  const TaskQueue::TaskTiming& task_timing) {
  if (!main_thread_only().on_task_started_handler.is_null())
    main_thread_only().on_task_started_handler.Run(task, task_timing);
}

void TaskQueueImpl::SetOnTaskCompletedHandler(
    TaskQueueImpl::OnTaskCompletedHandler handler) {
  DCHECK(should_notify_observers_ || handler.is_null());
  main_thread_only().on_task_completed_handler = std::move(handler);
}

void TaskQueueImpl::OnTaskCompleted(const Task& task,
                                    TaskQueue::TaskTiming* task_timing,
                                    LazyNow* lazy_now) {
  if (!main_thread_only().on_task_completed_handler.is_null()) {
    main_thread_only().on_task_completed_handler.Run(task, task_timing,
                                                     lazy_now);
  }
}

bool TaskQueueImpl::RequiresTaskTiming() const {
  return !main_thread_only().on_task_started_handler.is_null() ||
         !main_thread_only().on_task_completed_handler.is_null();
}

bool TaskQueueImpl::IsUnregistered() const {
  base::internal::CheckedAutoLock lock(any_thread_lock_);
  return any_thread_.unregistered;
}

WeakPtr<SequenceManagerImpl> TaskQueueImpl::GetSequenceManagerWeakPtr() {
  return sequence_manager_->GetWeakPtr();
}

void TaskQueueImpl::ActivateDelayedFenceIfNeeded(TimeTicks now) {
  if (!main_thread_only().delayed_fence)
    return;
  if (main_thread_only().delayed_fence.value() > now)
    return;
  InsertFence(TaskQueue::InsertFencePosition::kNow);
  main_thread_only().delayed_fence = nullopt;
}

void TaskQueueImpl::DeletePendingTasks() {
  main_thread_only().delayed_work_queue->DeletePendingTasks();
  main_thread_only().immediate_work_queue->DeletePendingTasks();

  DelayedIncomingQueue queue_to_delete;
  main_thread_only().delayed_incoming_queue.swap(&queue_to_delete);

  TaskDeque deque;
  {


    base::internal::CheckedAutoLock lock(any_thread_lock_);
    deque.swap(any_thread_.immediate_incoming_queue);
    any_thread_.immediate_work_queue_empty = true;
    empty_queues_to_reload_handle_.SetActive(false);
  }

  LazyNow lazy_now = main_thread_only().time_domain->CreateLazyNow();
  UpdateDelayedWakeUp(&lazy_now);
}

bool TaskQueueImpl::HasTasks() const {
  if (!main_thread_only().delayed_work_queue->Empty())
    return true;
  if (!main_thread_only().immediate_work_queue->Empty())
    return true;
  if (!main_thread_only().delayed_incoming_queue.empty())
    return true;

  base::internal::CheckedAutoLock lock(any_thread_lock_);
  if (!any_thread_.immediate_incoming_queue.empty())
    return true;

  return false;
}

void TaskQueueImpl::MaybeReportIpcTaskQueuedFromMainThread(
    Task* pending_task,
    const char* task_queue_name) {
  if (!pending_task->ipc_hash)
    return;


  if (!main_thread_only().disabled_time)
    return;

  bool tracing_enabled = false;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                                     &tracing_enabled);
  if (!tracing_enabled)
    return;

  if (main_thread_only().is_enabled ||
      !main_thread_only().should_report_posted_tasks_when_disabled) {
    return;
  }

  base::TimeDelta time_since_disabled =
      main_thread_only().time_domain->Now() -
      main_thread_only().disabled_time.value();

  ReportIpcTaskQueued(pending_task, task_queue_name, time_since_disabled);
}

bool TaskQueueImpl::ShouldReportIpcTaskQueuedFromAnyThreadLocked(
    base::TimeDelta* time_since_disabled) {


  if (!any_thread_.tracing_only.disabled_time)
    return false;

  if (any_thread_.tracing_only.is_enabled ||
      any_thread_.tracing_only.should_report_posted_tasks_when_disabled) {
    return false;
  }

  *time_since_disabled = any_thread_.time_domain->Now() -
                         any_thread_.tracing_only.disabled_time.value();
  return true;
}

void TaskQueueImpl::MaybeReportIpcTaskQueuedFromAnyThreadLocked(
    Task* pending_task,
    const char* task_queue_name) {
  if (!pending_task->ipc_hash)
    return;

  bool tracing_enabled = false;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                                     &tracing_enabled);
  if (!tracing_enabled)
    return;

  base::TimeDelta time_since_disabled;
  if (ShouldReportIpcTaskQueuedFromAnyThreadLocked(&time_since_disabled))
    ReportIpcTaskQueued(pending_task, task_queue_name, time_since_disabled);
}

void TaskQueueImpl::MaybeReportIpcTaskQueuedFromAnyThreadUnlocked(
    Task* pending_task,
    const char* task_queue_name) {
  if (!pending_task->ipc_hash)
    return;

  bool tracing_enabled = false;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                                     &tracing_enabled);
  if (!tracing_enabled)
    return;

  base::TimeDelta time_since_disabled;
  bool should_report = false;
  {
    base::internal::CheckedAutoLock lock(any_thread_lock_);
    should_report =
        ShouldReportIpcTaskQueuedFromAnyThreadLocked(&time_since_disabled);
  }

  ReportIpcTaskQueued(pending_task, task_queue_name, time_since_disabled);
}

void TaskQueueImpl::ReportIpcTaskQueued(
    Task* pending_task,
    const char* task_queue_name,
    const base::TimeDelta& time_since_disabled) {

  TRACE_EVENT_BEGIN2(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                     "task_posted_to_disabled_queue", "task_queue_name",
                     task_queue_name, "time_since_disabled_ms",
                     time_since_disabled.InMilliseconds());
  TRACE_EVENT_END2(TRACE_DISABLED_BY_DEFAULT("lifecycles"),
                   "task_posted_to_disabled_queue", "ipc_hash",
                   pending_task->ipc_hash, "location",
                   pending_task->posted_from.program_counter());
}

void TaskQueueImpl::OnQueueUnblocked() {
  DCHECK(IsQueueEnabled());
  DCHECK(!BlockedByFence());

  main_thread_only().enqueue_order_at_which_we_became_unblocked =
      sequence_manager_->GetNextSequenceNumber();

  static_assert(TaskQueue::QueuePriority::kLowPriority >
                    TaskQueue::QueuePriority::kNormalPriority,
                "Priorities are not ordered as expected");
  if (GetQueuePriority() <= TaskQueue::QueuePriority::kNormalPriority) {


    main_thread_only()
        .enqueue_order_at_which_we_became_unblocked_with_normal_priority =
        main_thread_only().enqueue_order_at_which_we_became_unblocked;
  }
}

TaskQueueImpl::DelayedIncomingQueue::DelayedIncomingQueue() = default;
TaskQueueImpl::DelayedIncomingQueue::~DelayedIncomingQueue() = default;

void TaskQueueImpl::DelayedIncomingQueue::push(Task&& task) {
  if (task.is_high_res)
    pending_high_res_tasks_++;
  queue_.push(std::move(task));
}

void TaskQueueImpl::DelayedIncomingQueue::pop() {
  DCHECK(!empty());
  if (top().is_high_res) {
    pending_high_res_tasks_--;
    DCHECK_GE(pending_high_res_tasks_, 0);
  }
  queue_.pop();
}

void TaskQueueImpl::DelayedIncomingQueue::swap(DelayedIncomingQueue* rhs) {
  std::swap(pending_high_res_tasks_, rhs->pending_high_res_tasks_);
  std::swap(queue_, rhs->queue_);
}

void TaskQueueImpl::DelayedIncomingQueue::SweepCancelledTasks() {



  bool task_deleted = false;
  auto it = queue_.c.begin();
  while (it != queue_.c.end()) {
    if (it->task.IsCancelled()) {
      if (it->is_high_res)
        pending_high_res_tasks_--;
      *it = std::move(queue_.c.back());
      queue_.c.pop_back();
      task_deleted = true;
    } else {
      it++;
    }
  }

  if (task_deleted)
    std::make_heap(queue_.c.begin(), queue_.c.end(), queue_.comp);
}

void TaskQueueImpl::DelayedIncomingQueue::AsValueInto(
    TimeTicks now,
    trace_event::TracedValue* state) const {
  for (const Task& task : queue_.c) {
    TaskAsValueInto(task, now, state);
  }
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base
