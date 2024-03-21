// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/stack_sampling_profiler.h"

#include <algorithm>
#include <map>
#include <utility>

#include "base/atomic_sequence_num.h"
#include "base/atomicops.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/singleton.h"
#include "base/profiler/stack_buffer.h"
#include "base/profiler/stack_sampler.h"
#include "base/profiler/unwinder.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"

namespace base {

// much be dead (thus it should be a fast Join()).
class ScopedAllowThreadRecallForStackSamplingProfiler
    : public ScopedAllowBaseSyncPrimitivesOutsideBlockingScope {};

namespace {

// to MANUAL for correct operation of the IsSignaled() call in Start(). See the
// comment there for why.
constexpr WaitableEvent::ResetPolicy kResetPolicy =
    WaitableEvent::ResetPolicy::MANUAL;

// for referencing the active collection to the SamplingThread.
const int kNullProfilerId = -1;

}  // namespace


class StackSamplingProfiler::SamplingThread : public Thread {
 public:
  class TestPeer {
   public:


    static void Reset();

    static void DisableIdleShutdown();







    static void ShutdownAssumingIdle(bool simulate_intervening_add);

   private:

    static void ShutdownTaskAndSignalEvent(SamplingThread* sampler,
                                           int add_events,
                                           WaitableEvent* event);
  };

  struct CollectionContext {
    CollectionContext(const SamplingParams& params,
                      WaitableEvent* finished,
                      std::unique_ptr<StackSampler> sampler,
                      std::unique_ptr<ProfileBuilder> profile_builder)
        : collection_id(next_collection_id.GetNext()),
          params(params),
          finished(finished),
          sampler(std::move(sampler)),
          profile_builder(std::move(profile_builder)) {}
    ~CollectionContext() = default;


    const int collection_id;

    const SamplingParams params;    // Information about how to sample.
    WaitableEvent* const finished;  // Signaled when all sampling complete.

    std::unique_ptr<StackSampler> sampler;

    std::unique_ptr<ProfileBuilder> profile_builder;

    TimeTicks next_sample_time;

    TimeTicks profile_start_time;

    int sample_count = 0;

    static AtomicSequenceNumber next_collection_id;
  };

  static SamplingThread* GetInstance();



  int Add(std::unique_ptr<CollectionContext> collection);


  void AddAuxUnwinder(int collection_id, std::unique_ptr<Unwinder> unwinder);

  void ApplyMetadataToPastSamples(base::TimeTicks period_start,
                                  base::TimeTicks period_end,
                                  int64_t name_hash,
                                  Optional<int64_t> key,
                                  int64_t value);



  void Remove(int collection_id);

 private:
  friend struct DefaultSingletonTraits<SamplingThread>;

  enum ThreadExecutionState {


    NOT_STARTED,



    RUNNING,





    EXITING,
  };

  SamplingThread();
  ~SamplingThread() override;

  scoped_refptr<SingleThreadTaskRunner> GetOrCreateTaskRunnerForAdd();
  scoped_refptr<SingleThreadTaskRunner> GetTaskRunner(
      ThreadExecutionState* out_state);

  scoped_refptr<SingleThreadTaskRunner> GetTaskRunnerOnSamplingThread();




  void FinishCollection(CollectionContext* collection);

  void ScheduleShutdownIfIdle();

  void AddCollectionTask(std::unique_ptr<CollectionContext> collection);
  void AddAuxUnwinderTask(int collection_id,
                          std::unique_ptr<Unwinder> unwinder);
  void ApplyMetadataToPastSamplesTask(base::TimeTicks period_start,
                                      base::TimeTicks period_end,
                                      int64_t name_hash,
                                      Optional<int64_t> key,
                                      int64_t value);
  void RemoveCollectionTask(int collection_id);
  void RecordSampleTask(int collection_id);
  void ShutdownTask(int add_events);

  void CleanUp() override;



  std::unique_ptr<StackBuffer> stack_buffer_;




  std::map<int, std::unique_ptr<CollectionContext>> active_collections_;







  Lock thread_execution_state_lock_;  // Protects all thread_execution_state_*
  ThreadExecutionState thread_execution_state_
      GUARDED_BY(thread_execution_state_lock_) = NOT_STARTED;
  scoped_refptr<SingleThreadTaskRunner> thread_execution_state_task_runner_
      GUARDED_BY(thread_execution_state_lock_);
  bool thread_execution_state_disable_idle_shutdown_for_testing_
      GUARDED_BY(thread_execution_state_lock_) = false;




  int thread_execution_state_add_events_
      GUARDED_BY(thread_execution_state_lock_) = 0;

  DISALLOW_COPY_AND_ASSIGN(SamplingThread);
};

void StackSamplingProfiler::SamplingThread::TestPeer::Reset() {
  SamplingThread* sampler = SamplingThread::GetInstance();

  ThreadExecutionState state;
  {
    AutoLock lock(sampler->thread_execution_state_lock_);
    state = sampler->thread_execution_state_;
    DCHECK(sampler->active_collections_.empty());
  }


  if (state == RUNNING) {
    ShutdownAssumingIdle(false);
    state = EXITING;
  }

  if (state == EXITING)
    sampler->Stop();

  {
    AutoLock lock(sampler->thread_execution_state_lock_);
    sampler->thread_execution_state_ = NOT_STARTED;
    sampler->thread_execution_state_task_runner_ = nullptr;
    sampler->thread_execution_state_disable_idle_shutdown_for_testing_ = false;
    sampler->thread_execution_state_add_events_ = 0;
  }
}

void StackSamplingProfiler::SamplingThread::TestPeer::DisableIdleShutdown() {
  SamplingThread* sampler = SamplingThread::GetInstance();

  {
    AutoLock lock(sampler->thread_execution_state_lock_);
    sampler->thread_execution_state_disable_idle_shutdown_for_testing_ = true;
  }
}

void StackSamplingProfiler::SamplingThread::TestPeer::ShutdownAssumingIdle(
    bool simulate_intervening_add) {
  SamplingThread* sampler = SamplingThread::GetInstance();

  ThreadExecutionState state;
  scoped_refptr<SingleThreadTaskRunner> task_runner =
      sampler->GetTaskRunner(&state);
  DCHECK_EQ(RUNNING, state);
  DCHECK(task_runner);

  int add_events;
  {
    AutoLock lock(sampler->thread_execution_state_lock_);
    add_events = sampler->thread_execution_state_add_events_;
    if (simulate_intervening_add)
      ++sampler->thread_execution_state_add_events_;
  }

  WaitableEvent executed(WaitableEvent::ResetPolicy::MANUAL,
                         WaitableEvent::InitialState::NOT_SIGNALED);


  task_runner->PostTask(
      FROM_HERE, BindOnce(&ShutdownTaskAndSignalEvent, Unretained(sampler),
                          add_events, Unretained(&executed)));
  executed.Wait();
}

void StackSamplingProfiler::SamplingThread::TestPeer::
    ShutdownTaskAndSignalEvent(SamplingThread* sampler,
                               int add_events,
                               WaitableEvent* event) {
  sampler->ShutdownTask(add_events);
  event->Signal();
}

AtomicSequenceNumber StackSamplingProfiler::SamplingThread::CollectionContext::
    next_collection_id;

StackSamplingProfiler::SamplingThread::SamplingThread()
    : Thread("StackSamplingProfiler") {}

StackSamplingProfiler::SamplingThread::~SamplingThread() = default;

StackSamplingProfiler::SamplingThread*
StackSamplingProfiler::SamplingThread::GetInstance() {
  return Singleton<SamplingThread, LeakySingletonTraits<SamplingThread>>::get();
}

int StackSamplingProfiler::SamplingThread::Add(
    std::unique_ptr<CollectionContext> collection) {


  int collection_id = collection->collection_id;
  scoped_refptr<SingleThreadTaskRunner> task_runner =
      GetOrCreateTaskRunnerForAdd();

  task_runner->PostTask(
      FROM_HERE, BindOnce(&SamplingThread::AddCollectionTask, Unretained(this),
                          std::move(collection)));

  return collection_id;
}

void StackSamplingProfiler::SamplingThread::AddAuxUnwinder(
    int collection_id,
    std::unique_ptr<Unwinder> unwinder) {
  ThreadExecutionState state;
  scoped_refptr<SingleThreadTaskRunner> task_runner = GetTaskRunner(&state);
  if (state != RUNNING)
    return;
  DCHECK(task_runner);
  task_runner->PostTask(
      FROM_HERE, BindOnce(&SamplingThread::AddAuxUnwinderTask, Unretained(this),
                          collection_id, std::move(unwinder)));
}

void StackSamplingProfiler::SamplingThread::ApplyMetadataToPastSamples(
    base::TimeTicks period_start,
    base::TimeTicks period_end,
    int64_t name_hash,
    Optional<int64_t> key,
    int64_t value) {
  ThreadExecutionState state;
  scoped_refptr<SingleThreadTaskRunner> task_runner = GetTaskRunner(&state);
  if (state != RUNNING)
    return;
  DCHECK(task_runner);
  task_runner->PostTask(
      FROM_HERE, BindOnce(&SamplingThread::ApplyMetadataToPastSamplesTask,
                          Unretained(this), period_start, period_end, name_hash,
                          key, value));
}

void StackSamplingProfiler::SamplingThread::Remove(int collection_id) {


  ThreadExecutionState state;
  scoped_refptr<SingleThreadTaskRunner> task_runner = GetTaskRunner(&state);
  if (state != RUNNING)
    return;
  DCHECK(task_runner);



  task_runner->PostTask(FROM_HERE,
                        BindOnce(&SamplingThread::RemoveCollectionTask,
                                 Unretained(this), collection_id));
}

scoped_refptr<SingleThreadTaskRunner>
StackSamplingProfiler::SamplingThread::GetOrCreateTaskRunnerForAdd() {
  AutoLock lock(thread_execution_state_lock_);


  ++thread_execution_state_add_events_;

  if (thread_execution_state_ == RUNNING) {
    DCHECK(thread_execution_state_task_runner_);


    DCHECK_NE(GetThreadId(), PlatformThread::CurrentId());
    return thread_execution_state_task_runner_;
  }

  if (thread_execution_state_ == EXITING) {









    ScopedAllowThreadRecallForStackSamplingProfiler allow_thread_join;
    Stop();
  }

  DCHECK(!stack_buffer_);
  stack_buffer_ = StackSampler::CreateStackBuffer();






  Start();
  thread_execution_state_ = RUNNING;
  thread_execution_state_task_runner_ = Thread::task_runner();


  DetachFromSequence();

  return thread_execution_state_task_runner_;
}

scoped_refptr<SingleThreadTaskRunner>
StackSamplingProfiler::SamplingThread::GetTaskRunner(
    ThreadExecutionState* out_state) {
  AutoLock lock(thread_execution_state_lock_);
  if (out_state)
    *out_state = thread_execution_state_;
  if (thread_execution_state_ == RUNNING) {


    DCHECK_NE(GetThreadId(), PlatformThread::CurrentId());
    DCHECK(thread_execution_state_task_runner_);
  } else {
    DCHECK(!thread_execution_state_task_runner_);
  }

  return thread_execution_state_task_runner_;
}

scoped_refptr<SingleThreadTaskRunner>
StackSamplingProfiler::SamplingThread::GetTaskRunnerOnSamplingThread() {


  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  return Thread::task_runner();
}

void StackSamplingProfiler::SamplingThread::FinishCollection(
    CollectionContext* collection) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());
  DCHECK_EQ(0u, active_collections_.count(collection->collection_id));

  TimeDelta profile_duration = TimeTicks::Now() -
                               collection->profile_start_time +
                               collection->params.sampling_interval;

  collection->profile_builder->OnProfileCompleted(
      profile_duration, collection->params.sampling_interval);

  collection->finished->Signal();

  ScheduleShutdownIfIdle();
}

void StackSamplingProfiler::SamplingThread::ScheduleShutdownIfIdle() {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  if (!active_collections_.empty())
    return;

  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::SamplingThread::ScheduleShutdownIfIdle");

  int add_events;
  {
    AutoLock lock(thread_execution_state_lock_);
    if (thread_execution_state_disable_idle_shutdown_for_testing_)
      return;
    add_events = thread_execution_state_add_events_;
  }

  GetTaskRunnerOnSamplingThread()->PostDelayedTask(
      FROM_HERE,
      BindOnce(&SamplingThread::ShutdownTask, Unretained(this), add_events),
      TimeDelta::FromSeconds(60));
}

void StackSamplingProfiler::SamplingThread::AddAuxUnwinderTask(
    int collection_id,
    std::unique_ptr<Unwinder> unwinder) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  auto loc = active_collections_.find(collection_id);
  if (loc == active_collections_.end())
    return;

  loc->second->sampler->AddAuxUnwinder(std::move(unwinder));
}

void StackSamplingProfiler::SamplingThread::ApplyMetadataToPastSamplesTask(
    base::TimeTicks period_start,
    base::TimeTicks period_end,
    int64_t name_hash,
    Optional<int64_t> key,
    int64_t value) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());
  ProfileBuilder::MetadataItem item(name_hash, key, value);
  for (auto& id_collection_pair : active_collections_) {
    id_collection_pair.second->profile_builder->ApplyMetadataRetrospectively(
        period_start, period_end, item);
  }
}

void StackSamplingProfiler::SamplingThread::AddCollectionTask(
    std::unique_ptr<CollectionContext> collection) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  const int collection_id = collection->collection_id;
  const TimeDelta initial_delay = collection->params.initial_delay;

  active_collections_.insert(
      std::make_pair(collection_id, std::move(collection)));

  GetTaskRunnerOnSamplingThread()->PostDelayedTask(
      FROM_HERE,
      BindOnce(&SamplingThread::RecordSampleTask, Unretained(this),
               collection_id),
      initial_delay);



  {
    AutoLock lock(thread_execution_state_lock_);
    ++thread_execution_state_add_events_;
  }
}

void StackSamplingProfiler::SamplingThread::RemoveCollectionTask(
    int collection_id) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  auto found = active_collections_.find(collection_id);
  if (found == active_collections_.end())
    return;

  std::unique_ptr<CollectionContext> collection = std::move(found->second);
  size_t count = active_collections_.erase(collection_id);
  DCHECK_EQ(1U, count);

  FinishCollection(collection.get());
}

void StackSamplingProfiler::SamplingThread::RecordSampleTask(
    int collection_id) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  auto found = active_collections_.find(collection_id);

  if (found == active_collections_.end())
    return;

  CollectionContext* collection = found->second.get();

  if (collection->sample_count == 0) {
    collection->profile_start_time = TimeTicks::Now();
    collection->next_sample_time = TimeTicks::Now();
  }

  collection->sampler->RecordStackFrames(stack_buffer_.get(),
                                         collection->profile_builder.get());

  if (++collection->sample_count < collection->params.samples_per_profile) {
    if (!collection->params.keep_consistent_sampling_interval)
      collection->next_sample_time = TimeTicks::Now();
    collection->next_sample_time += collection->params.sampling_interval;
    bool success = GetTaskRunnerOnSamplingThread()->PostDelayedTask(
        FROM_HERE,
        BindOnce(&SamplingThread::RecordSampleTask, Unretained(this),
                 collection_id),
        std::max(collection->next_sample_time - TimeTicks::Now(), TimeDelta()));
    DCHECK(success);
    return;
  }

  std::unique_ptr<CollectionContext> owned_collection =
      std::move(found->second);
  size_t count = active_collections_.erase(collection_id);
  DCHECK_EQ(1U, count);

  FinishCollection(collection);
}

void StackSamplingProfiler::SamplingThread::ShutdownTask(int add_events) {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());



  AutoLock lock(thread_execution_state_lock_);


  if (thread_execution_state_add_events_ != add_events)
    return;

  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::SamplingThread::ShutdownTask");




  DCHECK(active_collections_.empty());
  StopSoon();




  DetachFromSequence();



  thread_execution_state_ = EXITING;
  thread_execution_state_task_runner_ = nullptr;
  stack_buffer_.reset();
}

void StackSamplingProfiler::SamplingThread::CleanUp() {
  DCHECK_EQ(GetThreadId(), PlatformThread::CurrentId());

  DCHECK(active_collections_.empty());

  Thread::CleanUp();
}


void StackSamplingProfiler::TestPeer::Reset() {
  SamplingThread::TestPeer::Reset();
}

bool StackSamplingProfiler::TestPeer::IsSamplingThreadRunning() {
  return SamplingThread::GetInstance()->IsRunning();
}

void StackSamplingProfiler::TestPeer::DisableIdleShutdown() {
  SamplingThread::TestPeer::DisableIdleShutdown();
}

void StackSamplingProfiler::TestPeer::PerformSamplingThreadIdleShutdown(
    bool simulate_intervening_start) {
  SamplingThread::TestPeer::ShutdownAssumingIdle(simulate_intervening_start);
}

StackSamplingProfiler::StackSamplingProfiler(
    SamplingProfilerThreadToken thread_token,
    const SamplingParams& params,
    std::unique_ptr<ProfileBuilder> profile_builder,
    StackSamplerTestDelegate* test_delegate)
    : StackSamplingProfiler(params, std::move(profile_builder), nullptr) {
  sampler_ = StackSampler::Create(
      thread_token, profile_builder_->GetModuleCache(), test_delegate);
}

StackSamplingProfiler::StackSamplingProfiler(
    const SamplingParams& params,
    std::unique_ptr<ProfileBuilder> profile_builder,
    std::unique_ptr<StackSampler> sampler)
    : params_(params),
      profile_builder_(std::move(profile_builder)),
      sampler_(std::move(sampler)),


      profiling_inactive_(kResetPolicy, WaitableEvent::InitialState::SIGNALED),
      profiler_id_(kNullProfilerId) {
  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::StackSamplingProfiler");
  DCHECK(profile_builder_);
}

StackSamplingProfiler::~StackSamplingProfiler() {
  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::~StackSamplingProfiler");



  Stop();









  ScopedAllowBaseSyncPrimitivesOutsideBlockingScope allow_wait;
  profiling_inactive_.Wait();
}

void StackSamplingProfiler::Start() {
  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::Start");



  DCHECK(profile_builder_);


  if (!sampler_)
    return;

  if (pending_aux_unwinder_)
    sampler_->AddAuxUnwinder(std::move(pending_aux_unwinder_));


  static_assert(kResetPolicy == WaitableEvent::ResetPolicy::MANUAL,
                "The reset policy must be set to MANUAL");



  if (!profiling_inactive_.IsSignaled())
    profiling_inactive_.Wait();
  profiling_inactive_.Reset();

  DCHECK_EQ(kNullProfilerId, profiler_id_);
  profiler_id_ = SamplingThread::GetInstance()->Add(
      std::make_unique<SamplingThread::CollectionContext>(
          params_, &profiling_inactive_, std::move(sampler_),
          std::move(profile_builder_)));
  DCHECK_NE(kNullProfilerId, profiler_id_);

  TRACE_EVENT1(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::Started", "profiler_id", profiler_id_);
}

void StackSamplingProfiler::Stop() {
  TRACE_EVENT1(TRACE_DISABLED_BY_DEFAULT("cpu_profiler"),
               "StackSamplingProfiler::Stop", "profiler_id", profiler_id_);

  SamplingThread::GetInstance()->Remove(profiler_id_);
  profiler_id_ = kNullProfilerId;
}

void StackSamplingProfiler::AddAuxUnwinder(std::unique_ptr<Unwinder> unwinder) {
  if (profiler_id_ == kNullProfilerId) {


    pending_aux_unwinder_ = std::move(unwinder);
    return;
  }

  SamplingThread::GetInstance()->AddAuxUnwinder(profiler_id_,
                                                std::move(unwinder));
}

void StackSamplingProfiler::ApplyMetadataToPastSamples(
    base::TimeTicks period_start,
    base::TimeTicks period_end,
    int64_t name_hash,
    Optional<int64_t> key,
    int64_t value) {
  SamplingThread::GetInstance()->ApplyMetadataToPastSamples(
      period_start, period_end, name_hash, key, value);
}

}  // namespace base
