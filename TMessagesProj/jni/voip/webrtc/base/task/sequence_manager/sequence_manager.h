// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_SEQUENCE_MANAGER_H_
#define BASE_TASK_SEQUENCE_MANAGER_SEQUENCE_MANAGER_H_

#include <memory>
#include <utility>

#include "base/macros.h"
#include "base/message_loop/message_pump_type.h"
#include "base/message_loop/timer_slack.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/task/sequence_manager/task_queue_impl.h"
#include "base/task/sequence_manager/task_time_observer.h"
#include "base/time/default_tick_clock.h"

namespace base {

class MessagePump;
class TaskObserver;

namespace sequence_manager {

class TimeDomain;

// a native system task for drawing the UI). As long as this handle is alive,
// the work is considered to be pending.
class NativeWorkHandle {
 public:
  virtual ~NativeWorkHandle();
  NativeWorkHandle(const NativeWorkHandle&) = delete;

 protected:
  NativeWorkHandle() = default;
};

// (e.g. priority, common task type) multiplexing all posted tasks into
// a single backing sequence (currently bound to a single thread, which is
// refererred as *main thread* in the comments below). SequenceManager
// implementation can be used in a various ways to apply scheduling logic.
class BASE_EXPORT SequenceManager {
 public:
  class Observer {
   public:
    virtual ~Observer() = default;

    virtual void OnBeginNestedRunLoop() = 0;
    virtual void OnExitNestedRunLoop() = 0;
  };

  struct MetricRecordingSettings {


    MetricRecordingSettings(double task_sampling_rate_for_recording_cpu_time);






    double task_sampling_rate_for_recording_cpu_time = 0;

    bool records_cpu_time_for_some_tasks() const {
      return task_sampling_rate_for_recording_cpu_time > 0.0;
    }

    bool records_cpu_time_for_all_tasks() const {
      return task_sampling_rate_for_recording_cpu_time == 1.0;
    }
  };


  struct BASE_EXPORT Settings {
    class Builder;

    Settings();


    Settings(Settings&& move_from) noexcept;

    MessagePumpType message_loop_type = MessagePumpType::DEFAULT;
    bool randomised_sampling_enabled = false;
    const TickClock* clock = DefaultTickClock::GetInstance();

    bool add_queue_time_to_tasks = false;

#if DCHECK_IS_ON()

    enum class TaskLogging {
      kNone,
      kEnabled,
      kEnabledWithBacktrace,



      kReorderedOnly,
    };
    TaskLogging task_execution_logging = TaskLogging::kNone;

    bool log_post_task = false;


    bool log_task_delay_expiry = false;

    bool log_runloop_quit_and_quit_when_idle = false;


    std::array<TimeDelta, TaskQueue::kQueuePriorityCount>
        per_priority_cross_thread_task_delay;

    std::array<TimeDelta, TaskQueue::kQueuePriorityCount>
        per_priority_same_thread_task_delay;



    int random_task_selection_seed = 0;

#endif  // DCHECK_IS_ON()

    DISALLOW_COPY_AND_ASSIGN(Settings);
  };

  virtual ~SequenceManager() = default;



  virtual void BindToCurrentThread() = 0;


  virtual scoped_refptr<SequencedTaskRunner> GetTaskRunnerForCurrentTask() = 0;



  virtual void BindToMessagePump(std::unique_ptr<MessagePump> message_pump) = 0;



  virtual void SetObserver(Observer* observer) = 0;

  virtual void AddTaskTimeObserver(TaskTimeObserver* task_time_observer) = 0;
  virtual void RemoveTaskTimeObserver(TaskTimeObserver* task_time_observer) = 0;




  virtual void RegisterTimeDomain(TimeDomain* time_domain) = 0;
  virtual void UnregisterTimeDomain(TimeDomain* time_domain) = 0;

  virtual TimeDomain* GetRealTimeDomain() const = 0;
  virtual const TickClock* GetTickClock() const = 0;
  virtual TimeTicks NowTicks() const = 0;


  virtual void SetDefaultTaskRunner(
      scoped_refptr<SingleThreadTaskRunner> task_runner) = 0;


  virtual void ReclaimMemory() = 0;


  virtual bool GetAndClearSystemIsQuiescentBit() = 0;



  virtual void SetWorkBatchSize(int work_batch_size) = 0;


  virtual void SetTimerSlack(TimerSlack timer_slack) = 0;



  virtual void EnableCrashKeys(const char* async_stack_crash_key) = 0;

  virtual const MetricRecordingSettings& GetMetricRecordingSettings() const = 0;



  template <typename TaskQueueType, typename... Args>
  scoped_refptr<TaskQueueType> CreateTaskQueueWithType(
      const TaskQueue::Spec& spec,
      Args&&... args) {
    return WrapRefCounted(new TaskQueueType(CreateTaskQueueImpl(spec), spec,
                                            std::forward<Args>(args)...));
  }



  virtual scoped_refptr<TaskQueue> CreateTaskQueue(
      const TaskQueue::Spec& spec) = 0;






  virtual bool IsIdleForTesting() = 0;

  virtual size_t GetPendingTaskCountForTesting() const = 0;

  virtual std::string DescribeAllPendingTasks() const = 0;








  virtual std::unique_ptr<NativeWorkHandle> OnNativeWorkPending(
      TaskQueue::QueuePriority priority) = 0;


  virtual void AddTaskObserver(TaskObserver* task_observer) = 0;


  virtual void RemoveTaskObserver(TaskObserver* task_observer) = 0;

 protected:
  virtual std::unique_ptr<internal::TaskQueueImpl> CreateTaskQueueImpl(
      const TaskQueue::Spec& spec) = 0;
};

class BASE_EXPORT SequenceManager::Settings::Builder {
 public:
  Builder();
  ~Builder();

  Builder& SetMessagePumpType(MessagePumpType message_loop_type);

  Builder& SetRandomisedSamplingEnabled(bool randomised_sampling_enabled);

  Builder& SetTickClock(const TickClock* clock);

  Builder& SetAddQueueTimeToTasks(bool add_queue_time_to_tasks);

#if DCHECK_IS_ON()

  Builder& SetTaskLogging(TaskLogging task_execution_logging);

  Builder& SetLogPostTask(bool log_post_task);


  Builder& SetLogTaskDelayExpiry(bool log_task_delay_expiry);

  Builder& SetLogRunloopQuitAndQuitWhenIdle(
      bool log_runloop_quit_and_quit_when_idle);


  Builder& SetPerPriorityCrossThreadTaskDelay(
      std::array<TimeDelta, TaskQueue::kQueuePriorityCount>
          per_priority_cross_thread_task_delay);


  Builder& SetPerPrioritySameThreadTaskDelay(
      std::array<TimeDelta, TaskQueue::kQueuePriorityCount>
          per_priority_same_thread_task_delay);



  Builder& SetRandomTaskSelectionSeed(int random_task_selection_seed);

#endif  // DCHECK_IS_ON()

  Settings Build();

 private:
  Settings settings_;
};

// Implementation is located in sequence_manager_impl.cc.
// TODO(scheduler-dev): Remove after every thread has a SequenceManager.
BASE_EXPORT std::unique_ptr<SequenceManager>
CreateSequenceManagerOnCurrentThread(SequenceManager::Settings settings);

// MessagePump instances can be created with
// MessagePump::CreateMessagePumpForType().
BASE_EXPORT std::unique_ptr<SequenceManager>
CreateSequenceManagerOnCurrentThreadWithPump(
    std::unique_ptr<MessagePump> message_pump,
    SequenceManager::Settings settings = SequenceManager::Settings());

// additional setup is required before binding). The SequenceManager can be
// initialized on the current thread and then needs to be bound and initialized
// on the target thread by calling one of the Bind*() methods.
BASE_EXPORT std::unique_ptr<SequenceManager> CreateUnboundSequenceManager(
    SequenceManager::Settings settings = SequenceManager::Settings());

}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_SEQUENCE_MANAGER_H_
