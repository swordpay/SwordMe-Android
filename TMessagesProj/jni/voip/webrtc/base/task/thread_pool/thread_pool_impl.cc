// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/thread_pool_impl.h"

#include <algorithm>
#include <string>
#include <utility>

#include "base/base_switches.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/feature_list.h"
#include "base/message_loop/message_pump_type.h"
#include "base/metrics/field_trial_params.h"
#include "base/no_destructor.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/task/scoped_set_task_priority_for_current_thread.h"
#include "base/task/task_features.h"
#include "base/task/thread_pool/pooled_parallel_task_runner.h"
#include "base/task/thread_pool/pooled_sequenced_task_runner.h"
#include "base/task/thread_pool/sequence_sort_key.h"
#include "base/task/thread_pool/service_thread.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"
#include "base/task/thread_pool/thread_group_impl.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

#if defined(OS_WIN)
#include "base/task/thread_pool/thread_group_native_win.h"
#endif

#if defined(OS_MACOSX)
#include "base/task/thread_pool/thread_group_native_mac.h"
#endif

namespace base {
namespace internal {

namespace {

constexpr EnvironmentParams kForegroundPoolEnvironmentParams{
    "Foreground", base::ThreadPriority::NORMAL};

constexpr EnvironmentParams kBackgroundPoolEnvironmentParams{
    "Background", base::ThreadPriority::BACKGROUND};

constexpr int kMaxBestEffortTasks = 2;

bool HasDisableBestEffortTasksSwitch() {


  return CommandLine::InitializedForCurrentProcess() &&
         CommandLine::ForCurrentProcess()->HasSwitch(
             switches::kDisableBestEffortTasks);
}

}  // namespace

ThreadPoolImpl::ThreadPoolImpl(StringPiece histogram_label)
    : ThreadPoolImpl(histogram_label,
                     std::make_unique<TaskTrackerImpl>(histogram_label)) {}

ThreadPoolImpl::ThreadPoolImpl(StringPiece histogram_label,
                               std::unique_ptr<TaskTrackerImpl> task_tracker)
    : task_tracker_(std::move(task_tracker)),
      service_thread_(std::make_unique<ServiceThread>(
          task_tracker_.get(),
          BindRepeating(&ThreadPoolImpl::ReportHeartbeatMetrics,
                        Unretained(this)))),
      single_thread_task_runner_manager_(task_tracker_->GetTrackedRef(),
                                         &delayed_task_manager_),
      has_disable_best_effort_switch_(HasDisableBestEffortTasksSwitch()),
      tracked_ref_factory_(this) {
  foreground_thread_group_ = std::make_unique<ThreadGroupImpl>(
      histogram_label.empty()
          ? std::string()
          : JoinString(
                {histogram_label, kForegroundPoolEnvironmentParams.name_suffix},
                "."),
      kForegroundPoolEnvironmentParams.name_suffix,
      kForegroundPoolEnvironmentParams.priority_hint,
      task_tracker_->GetTrackedRef(), tracked_ref_factory_.GetTrackedRef());

  if (CanUseBackgroundPriorityForWorkerThread()) {
    background_thread_group_ = std::make_unique<ThreadGroupImpl>(
        histogram_label.empty()
            ? std::string()
            : JoinString({histogram_label,
                          kBackgroundPoolEnvironmentParams.name_suffix},
                         "."),
        kBackgroundPoolEnvironmentParams.name_suffix,
        kBackgroundPoolEnvironmentParams.priority_hint,
        task_tracker_->GetTrackedRef(), tracked_ref_factory_.GetTrackedRef());
  }
}

ThreadPoolImpl::~ThreadPoolImpl() {
#if DCHECK_IS_ON()
  DCHECK(join_for_testing_returned_.IsSet());
#endif

  foreground_thread_group_.reset();
  background_thread_group_.reset();
}

void ThreadPoolImpl::Start(const ThreadPoolInstance::InitParams& init_params,
                           WorkerThreadObserver* worker_thread_observer) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(!started_);

  internal::InitializeThreadPrioritiesFeature();


  const int max_best_effort_tasks =
      std::min(kMaxBestEffortTasks, init_params.max_num_foreground_threads);


  if (FeatureList::IsEnabled(kAllTasksUserBlocking))
    all_tasks_user_blocking_.Set();

#if HAS_NATIVE_THREAD_POOL()
  if (FeatureList::IsEnabled(kUseNativeThreadPool)) {
    std::unique_ptr<ThreadGroup> pool = std::move(foreground_thread_group_);
    foreground_thread_group_ = std::make_unique<ThreadGroupNativeImpl>(
        task_tracker_->GetTrackedRef(), tracked_ref_factory_.GetTrackedRef(),
        pool.get());
    pool->InvalidateAndHandoffAllTaskSourcesToOtherThreadGroup(
        foreground_thread_group_.get());
  }
#endif



  ServiceThread::Options service_thread_options;
  service_thread_options.message_pump_type =
#if defined(OS_POSIX) && !defined(OS_NACL_SFI)
      MessagePumpType::IO;
#else
      MessagePumpType::DEFAULT;
#endif
  service_thread_options.timer_slack = TIMER_SLACK_MAXIMUM;
  CHECK(service_thread_->StartWithOptions(service_thread_options));

#if defined(OS_POSIX) && !defined(OS_NACL_SFI)


  task_tracker_->set_io_thread_task_runner(service_thread_->task_runner());
#endif  // defined(OS_POSIX) && !defined(OS_NACL_SFI)

  UpdateCanRunPolicy();

  auto service_thread_task_runner = service_thread_->task_runner();
  delayed_task_manager_.Start(service_thread_task_runner);

  single_thread_task_runner_manager_.Start(worker_thread_observer);

  ThreadGroup::WorkerEnvironment worker_environment;
  switch (init_params.common_thread_pool_environment) {
    case InitParams::CommonThreadPoolEnvironment::DEFAULT:
      worker_environment = ThreadGroup::WorkerEnvironment::NONE;
      break;
#if defined(OS_WIN)
    case InitParams::CommonThreadPoolEnvironment::COM_MTA:
      worker_environment = ThreadGroup::WorkerEnvironment::COM_MTA;
      break;
    case InitParams::CommonThreadPoolEnvironment::
        DEPRECATED_COM_STA_IN_FOREGROUND_GROUP:
      worker_environment = ThreadGroup::WorkerEnvironment::COM_STA;
      break;
#endif
  }

  const base::TimeDelta suggested_reclaim_time =
      FeatureList::IsEnabled(kUseFiveMinutesThreadReclaimTime)
          ? base::TimeDelta::FromMinutes(5)
          : init_params.suggested_reclaim_time;

#if HAS_NATIVE_THREAD_POOL()
  if (FeatureList::IsEnabled(kUseNativeThreadPool)) {
    static_cast<ThreadGroupNative*>(foreground_thread_group_.get())
        ->Start(worker_environment);
  } else
#endif
  {





    static_cast<ThreadGroupImpl*>(foreground_thread_group_.get())
        ->Start(init_params.max_num_foreground_threads, max_best_effort_tasks,
                suggested_reclaim_time, service_thread_task_runner,
                worker_thread_observer, worker_environment);
  }

  if (background_thread_group_) {
    background_thread_group_->Start(
        max_best_effort_tasks, max_best_effort_tasks, suggested_reclaim_time,
        service_thread_task_runner, worker_thread_observer,
#if defined(OS_WIN)


        worker_environment == ThreadGroup::WorkerEnvironment::COM_STA
            ? ThreadGroup::WorkerEnvironment::NONE
            :
#endif
            worker_environment);
  }

  started_ = true;
}

bool ThreadPoolImpl::PostDelayedTask(const Location& from_here,
                                     const TaskTraits& traits,
                                     OnceClosure task,
                                     TimeDelta delay) {

  const TaskTraits new_traits = VerifyAndAjustIncomingTraits(traits);
  return PostTaskWithSequence(
      Task(from_here, std::move(task), delay),
      MakeRefCounted<Sequence>(new_traits, nullptr,
                               TaskSourceExecutionMode::kParallel));
}

scoped_refptr<TaskRunner> ThreadPoolImpl::CreateTaskRunner(
    const TaskTraits& traits) {
  const TaskTraits new_traits = VerifyAndAjustIncomingTraits(traits);
  return MakeRefCounted<PooledParallelTaskRunner>(new_traits, this);
}

scoped_refptr<SequencedTaskRunner> ThreadPoolImpl::CreateSequencedTaskRunner(
    const TaskTraits& traits) {
  const TaskTraits new_traits = VerifyAndAjustIncomingTraits(traits);
  return MakeRefCounted<PooledSequencedTaskRunner>(new_traits, this);
}

scoped_refptr<SingleThreadTaskRunner>
ThreadPoolImpl::CreateSingleThreadTaskRunner(
    const TaskTraits& traits,
    SingleThreadTaskRunnerThreadMode thread_mode) {
  return single_thread_task_runner_manager_.CreateSingleThreadTaskRunner(
      VerifyAndAjustIncomingTraits(traits), thread_mode);
}

#if defined(OS_WIN)
scoped_refptr<SingleThreadTaskRunner> ThreadPoolImpl::CreateCOMSTATaskRunner(
    const TaskTraits& traits,
    SingleThreadTaskRunnerThreadMode thread_mode) {
  return single_thread_task_runner_manager_.CreateCOMSTATaskRunner(
      VerifyAndAjustIncomingTraits(traits), thread_mode);
}
#endif  // defined(OS_WIN)

scoped_refptr<UpdateableSequencedTaskRunner>
ThreadPoolImpl::CreateUpdateableSequencedTaskRunner(const TaskTraits& traits) {
  const TaskTraits new_traits = VerifyAndAjustIncomingTraits(traits);
  return MakeRefCounted<PooledSequencedTaskRunner>(new_traits, this);
}

Optional<TimeTicks> ThreadPoolImpl::NextScheduledRunTimeForTesting() const {
  if (task_tracker_->HasIncompleteTaskSourcesForTesting())
    return TimeTicks::Now();
  return delayed_task_manager_.NextScheduledRunTime();
}

void ThreadPoolImpl::ProcessRipeDelayedTasksForTesting() {
  delayed_task_manager_.ProcessRipeTasks();
}

int ThreadPoolImpl::GetMaxConcurrentNonBlockedTasksWithTraitsDeprecated(
    const TaskTraits& traits) const {


  DCHECK_NE(traits.priority(), TaskPriority::BEST_EFFORT);
  return GetThreadGroupForTraits(traits)
      ->GetMaxConcurrentNonBlockedTasksDeprecated();
}

void ThreadPoolImpl::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);






  service_thread_->Stop();

  task_tracker_->StartShutdown();



  UpdateCanRunPolicy();

  task_tracker_->CompleteShutdown();
}

void ThreadPoolImpl::FlushForTesting() {
  task_tracker_->FlushForTesting();
}

void ThreadPoolImpl::FlushAsyncForTesting(OnceClosure flush_callback) {
  task_tracker_->FlushAsyncForTesting(std::move(flush_callback));
}

void ThreadPoolImpl::JoinForTesting() {
#if DCHECK_IS_ON()
  DCHECK(!join_for_testing_returned_.IsSet());
#endif




  service_thread_->Stop();
  single_thread_task_runner_manager_.JoinForTesting();
  foreground_thread_group_->JoinForTesting();
  if (background_thread_group_)
    background_thread_group_->JoinForTesting();
#if DCHECK_IS_ON()
  join_for_testing_returned_.Set();
#endif
}

void ThreadPoolImpl::BeginFence() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  ++num_fences_;
  UpdateCanRunPolicy();
}

void ThreadPoolImpl::EndFence() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_GT(num_fences_, 0);
  --num_fences_;
  UpdateCanRunPolicy();
}

void ThreadPoolImpl::BeginBestEffortFence() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  ++num_best_effort_fences_;
  UpdateCanRunPolicy();
}

void ThreadPoolImpl::EndBestEffortFence() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_GT(num_best_effort_fences_, 0);
  --num_best_effort_fences_;
  UpdateCanRunPolicy();
}

bool ThreadPoolImpl::PostTaskWithSequenceNow(Task task,
                                             scoped_refptr<Sequence> sequence) {
  auto transaction = sequence->BeginTransaction();
  const bool sequence_should_be_queued = transaction.WillPushTask();
  RegisteredTaskSource task_source;
  if (sequence_should_be_queued) {
    task_source = task_tracker_->RegisterTaskSource(sequence);

    if (!task_source)
      return false;
  }
  if (!task_tracker_->WillPostTaskNow(task, transaction.traits().priority()))
    return false;
  transaction.PushTask(std::move(task));
  if (task_source) {
    const TaskTraits traits = transaction.traits();
    GetThreadGroupForTraits(traits)->PushTaskSourceAndWakeUpWorkers(
        {std::move(task_source), std::move(transaction)});
  }
  return true;
}

bool ThreadPoolImpl::PostTaskWithSequence(Task task,
                                          scoped_refptr<Sequence> sequence) {


  CHECK(task.task);
  DCHECK(sequence);

  if (!task_tracker_->WillPostTask(&task, sequence->shutdown_behavior()))
    return false;

  if (task.delayed_run_time.is_null()) {
    return PostTaskWithSequenceNow(std::move(task), std::move(sequence));
  } else {


    scoped_refptr<TaskRunner> task_runner = sequence->task_runner();
    delayed_task_manager_.AddDelayedTask(
        std::move(task),
        BindOnce(
            [](scoped_refptr<Sequence> sequence,
               ThreadPoolImpl* thread_pool_impl, Task task) {
              thread_pool_impl->PostTaskWithSequenceNow(std::move(task),
                                                        std::move(sequence));
            },
            std::move(sequence), Unretained(this)),
        std::move(task_runner));
  }

  return true;
}

bool ThreadPoolImpl::ShouldYield(const TaskSource* task_source) const {
  const TaskPriority priority = task_source->priority_racy();
  auto* const thread_group =
      GetThreadGroupForTraits({priority, task_source->thread_policy()});


  if (!thread_group->IsBoundToCurrentThread())
    return true;
  return GetThreadGroupForTraits({priority, task_source->thread_policy()})
      ->ShouldYield(priority);
}

bool ThreadPoolImpl::EnqueueJobTaskSource(
    scoped_refptr<JobTaskSource> task_source) {
  auto registered_task_source =
      task_tracker_->RegisterTaskSource(std::move(task_source));
  if (!registered_task_source)
    return false;
  auto transaction = registered_task_source->BeginTransaction();
  const TaskTraits traits = transaction.traits();
  GetThreadGroupForTraits(traits)->PushTaskSourceAndWakeUpWorkers(
      {std::move(registered_task_source), std::move(transaction)});
  return true;
}

void ThreadPoolImpl::RemoveJobTaskSource(
    scoped_refptr<JobTaskSource> task_source) {
  auto transaction = task_source->BeginTransaction();
  ThreadGroup* const current_thread_group =
      GetThreadGroupForTraits(transaction.traits());
  current_thread_group->RemoveTaskSource(*task_source);
}

void ThreadPoolImpl::UpdatePriority(scoped_refptr<TaskSource> task_source,
                                    TaskPriority priority) {
  auto transaction = task_source->BeginTransaction();

  if (transaction.traits().priority() == priority)
    return;

  if (transaction.traits().priority() == TaskPriority::BEST_EFFORT) {
    DCHECK(transaction.traits().thread_policy_set_explicitly())
        << "A ThreadPolicy must be specified in the TaskTraits of an "
           "UpdateableSequencedTaskRunner whose priority is increased from "
           "BEST_EFFORT. See ThreadPolicy documentation.";
  }

  ThreadGroup* const current_thread_group =
      GetThreadGroupForTraits(transaction.traits());
  transaction.UpdatePriority(priority);
  ThreadGroup* const new_thread_group =
      GetThreadGroupForTraits(transaction.traits());

  if (new_thread_group == current_thread_group) {


    current_thread_group->UpdateSortKey(std::move(transaction));
  } else {


    auto registered_task_source =
        current_thread_group->RemoveTaskSource(*task_source);
    if (registered_task_source) {
      DCHECK(task_source);
      new_thread_group->PushTaskSourceAndWakeUpWorkers(
          {std::move(registered_task_source), std::move(transaction)});
    }
  }
}

const ThreadGroup* ThreadPoolImpl::GetThreadGroupForTraits(
    const TaskTraits& traits) const {
  return const_cast<ThreadPoolImpl*>(this)->GetThreadGroupForTraits(traits);
}

ThreadGroup* ThreadPoolImpl::GetThreadGroupForTraits(const TaskTraits& traits) {
  if (traits.priority() == TaskPriority::BEST_EFFORT &&
      traits.thread_policy() == ThreadPolicy::PREFER_BACKGROUND &&
      background_thread_group_) {
    return background_thread_group_.get();
  }

  return foreground_thread_group_.get();
}

void ThreadPoolImpl::UpdateCanRunPolicy() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  CanRunPolicy can_run_policy;
  if ((num_fences_ == 0 && num_best_effort_fences_ == 0 &&
       !has_disable_best_effort_switch_) ||
      task_tracker_->HasShutdownStarted()) {
    can_run_policy = CanRunPolicy::kAll;
  } else if (num_fences_ != 0) {
    can_run_policy = CanRunPolicy::kNone;
  } else {
    DCHECK(num_best_effort_fences_ > 0 || has_disable_best_effort_switch_);
    can_run_policy = CanRunPolicy::kForegroundOnly;
  }

  task_tracker_->SetCanRunPolicy(can_run_policy);
  foreground_thread_group_->DidUpdateCanRunPolicy();
  if (background_thread_group_)
    background_thread_group_->DidUpdateCanRunPolicy();
  single_thread_task_runner_manager_.DidUpdateCanRunPolicy();
}

TaskTraits ThreadPoolImpl::VerifyAndAjustIncomingTraits(
    TaskTraits traits) const {
  DCHECK_EQ(traits.extension_id(),
            TaskTraitsExtensionStorage::kInvalidExtensionId)
      << "Extension traits cannot be used with the ThreadPool API.";
  if (all_tasks_user_blocking_.IsSet())
    traits.UpdatePriority(TaskPriority::USER_BLOCKING);
  return traits;
}

void ThreadPoolImpl::ReportHeartbeatMetrics() const {
  foreground_thread_group_->ReportHeartbeatMetrics();
  if (background_thread_group_)
    background_thread_group_->ReportHeartbeatMetrics();
}

}  // namespace internal
}  // namespace base
