// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACE_LOG_H_
#define BASE_TRACE_EVENT_TRACE_LOG_H_

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/atomicops.h"
#include "base/containers/stack.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/single_thread_task_runner.h"
#include "base/time/time_override.h"
#include "base/trace_event/category_registry.h"
#include "base/trace_event/memory_dump_provider.h"
#include "base/trace_event/trace_config.h"
#include "base/trace_event/trace_event_impl.h"
#include "build/build_config.h"

namespace base {
class RefCountedString;

template <typename T>
class NoDestructor;

namespace trace_event {

struct TraceCategory;
class TraceBuffer;
class TraceBufferChunk;
class TraceEvent;
class TraceEventFilter;
class TraceEventMemoryOverhead;

struct BASE_EXPORT TraceLogStatus {
  TraceLogStatus();
  ~TraceLogStatus();
  uint32_t event_capacity;
  uint32_t event_count;
};

class BASE_EXPORT TraceLog : public MemoryDumpProvider {
 public:

  enum Mode : uint8_t {

    RECORDING_MODE = 1 << 0,


    FILTERING_MODE = 1 << 1
  };

  static TraceLog* GetInstance();

  TraceConfig GetCurrentTraceConfig() const;


  void InitializeThreadLocalEventBufferIfSupported();








  void SetEnabled(const TraceConfig& trace_config, uint8_t modes_to_enable);




  void SetDisabled();
  void SetDisabled(uint8_t modes_to_disable);


  bool IsEnabled() {
    AutoLock lock(lock_);
    return enabled_modes_ & RECORDING_MODE;
  }

  uint8_t enabled_modes() { return enabled_modes_; }





  int GetNumTracesRecorded();

#if defined(OS_ANDROID)
  void StartATrace();
  void StopATrace();
  void AddClockSyncMetadataEvent();
#endif



  class BASE_EXPORT EnabledStateObserver {
   public:
    virtual ~EnabledStateObserver() = default;


    virtual void OnTraceLogEnabled() = 0;


    virtual void OnTraceLogDisabled() = 0;
  };

  void AddEnabledStateObserver(EnabledStateObserver* listener);

  void RemoveEnabledStateObserver(EnabledStateObserver* listener);



  void AddOwnedEnabledStateObserver(
      std::unique_ptr<EnabledStateObserver> listener);
  bool HasEnabledStateObserver(EnabledStateObserver* listener) const;





  class BASE_EXPORT AsyncEnabledStateObserver {
   public:
    virtual ~AsyncEnabledStateObserver() = default;


    virtual void OnTraceLogEnabled() = 0;


    virtual void OnTraceLogDisabled() = 0;
  };




  void AddAsyncEnabledStateObserver(
      WeakPtr<AsyncEnabledStateObserver> listener);
  void RemoveAsyncEnabledStateObserver(AsyncEnabledStateObserver* listener);
  bool HasAsyncEnabledStateObserver(AsyncEnabledStateObserver* listener) const;

  TraceLogStatus GetStatus() const;
  bool BufferIsFull() const;


  void EstimateTraceMemoryOverhead(TraceEventMemoryOverhead* overhead);

  void SetArgumentFilterPredicate(
      const ArgumentFilterPredicate& argument_filter_predicate);
  ArgumentFilterPredicate GetArgumentFilterPredicate() const;

  void SetMetadataFilterPredicate(
      const MetadataFilterPredicate& metadata_filter_predicate);
  MetadataFilterPredicate GetMetadataFilterPredicate() const;










  using OutputCallback =
      base::RepeatingCallback<void(const scoped_refptr<base::RefCountedString>&,
                                   bool has_more_events)>;
  void Flush(const OutputCallback& cb, bool use_worker_thread = false);

  void CancelTracing(const OutputCallback& cb);

  using AddTraceEventOverrideFunction = void (*)(TraceEvent*,
                                                 bool thread_will_flush,
                                                 TraceEventHandle* handle);
  using OnFlushFunction = void (*)();
  using UpdateDurationFunction =
      void (*)(const unsigned char* category_group_enabled,
               const char* name,
               TraceEventHandle handle,
               int thread_id,
               bool explicit_timestamps,
               const TimeTicks& now,
               const ThreadTicks& thread_now,
               ThreadInstructionCount thread_instruction_now);



  void SetAddTraceEventOverrides(
      const AddTraceEventOverrideFunction& add_event_override,
      const OnFlushFunction& on_flush_callback,
      const UpdateDurationFunction& update_duration_callback);



  static const unsigned char* GetCategoryGroupEnabled(const char* name);
  static const char* GetCategoryGroupName(
      const unsigned char* category_group_enabled);
  static constexpr const unsigned char* GetBuiltinCategoryEnabled(
      const char* name) {
    TraceCategory* builtin_category =
        CategoryRegistry::GetBuiltinCategoryByName(name);
    if (builtin_category)
      return builtin_category->state_ptr();
    return nullptr;
  }



  bool ShouldAddAfterUpdatingState(char phase,
                                   const unsigned char* category_group_enabled,
                                   const char* name,
                                   unsigned long long id,
                                   int thread_id,
                                   TraceArguments* args);
  TraceEventHandle AddTraceEvent(char phase,
                                 const unsigned char* category_group_enabled,
                                 const char* name,
                                 const char* scope,
                                 unsigned long long id,
                                 TraceArguments* args,
                                 unsigned int flags);
  TraceEventHandle AddTraceEventWithBindId(
      char phase,
      const unsigned char* category_group_enabled,
      const char* name,
      const char* scope,
      unsigned long long id,
      unsigned long long bind_id,
      TraceArguments* args,
      unsigned int flags);
  TraceEventHandle AddTraceEventWithProcessId(
      char phase,
      const unsigned char* category_group_enabled,
      const char* name,
      const char* scope,
      unsigned long long id,
      int process_id,
      TraceArguments* args,
      unsigned int flags);
  TraceEventHandle AddTraceEventWithThreadIdAndTimestamp(
      char phase,
      const unsigned char* category_group_enabled,
      const char* name,
      const char* scope,
      unsigned long long id,
      int thread_id,
      const TimeTicks& timestamp,
      TraceArguments* args,
      unsigned int flags);
  TraceEventHandle AddTraceEventWithThreadIdAndTimestamp(
      char phase,
      const unsigned char* category_group_enabled,
      const char* name,
      const char* scope,
      unsigned long long id,
      unsigned long long bind_id,
      int thread_id,
      const TimeTicks& timestamp,
      TraceArguments* args,
      unsigned int flags);

  void AddMetadataEvent(const unsigned char* category_group_enabled,
                        const char* name,
                        TraceArguments* args,
                        unsigned int flags);

  void UpdateTraceEventDuration(const unsigned char* category_group_enabled,
                                const char* name,
                                TraceEventHandle handle);

  void UpdateTraceEventDurationExplicit(
      const unsigned char* category_group_enabled,
      const char* name,
      TraceEventHandle handle,
      int thread_id,
      bool explicit_timestamps,
      const TimeTicks& now,
      const ThreadTicks& thread_now,
      ThreadInstructionCount thread_instruction_now);

  void EndFilteredEvent(const unsigned char* category_group_enabled,
                        const char* name,
                        TraceEventHandle handle);

  int process_id() const { return process_id_; }
  const std::string& process_name() const { return process_name_; }

  uint64_t MangleEventId(uint64_t id);


  typedef std::unique_ptr<TraceEventFilter> (*FilterFactoryForTesting)(
      const std::string& /* predicate_name */);
  void SetFilterFactoryForTesting(FilterFactoryForTesting factory) {
    filter_factory_for_testing_ = factory;
  }

  static void ResetForTesting();

  TraceEvent* GetEventByHandle(TraceEventHandle handle);

  void SetProcessID(int process_id);



  void SetProcessSortIndex(int sort_index);

  void set_process_name(const std::string& process_name) {
    AutoLock lock(lock_);
    process_name_ = process_name;
  }

  bool IsProcessNameEmpty() const { return process_name_.empty(); }


  void UpdateProcessLabel(int label_id, const std::string& current_label);
  void RemoveProcessLabel(int label_id);



  void SetThreadSortIndex(PlatformThreadId thread_id, int sort_index);


  void SetTimeOffset(TimeDelta offset);

  size_t GetObserverCountForTest() const;



  void SetCurrentThreadBlocksMessageLoop();

#if defined(OS_WIN)



  void UpdateETWCategoryGroupEnabledFlags();
#endif

  void SetTraceBufferForTesting(std::unique_ptr<TraceBuffer> trace_buffer);

 private:
  typedef unsigned int InternalTraceOptions;

  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture,
                           TraceBufferRingBufferGetReturnChunk);
  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture,
                           TraceBufferRingBufferHalfIteration);
  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture,
                           TraceBufferRingBufferFullIteration);
  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture, TraceBufferVectorReportFull);
  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture,
                           ConvertTraceConfigToInternalOptions);
  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture,
                           TraceRecordAsMuchAsPossibleMode);
  FRIEND_TEST_ALL_PREFIXES(TraceEventTestFixture, ConfigTraceBufferLimit);

  friend class base::NoDestructor<TraceLog>;

  bool OnMemoryDump(const MemoryDumpArgs& args,
                    ProcessMemoryDump* pmd) override;





  void UpdateCategoryRegistry();
  void UpdateCategoryState(TraceCategory* category);

  void CreateFiltersForTraceConfig();

  InternalTraceOptions GetInternalOptionsFromTraceConfig(
      const TraceConfig& config);

  class ThreadLocalEventBuffer;
  class OptionalAutoLock;
  struct RegisteredAsyncObserver;

  TraceLog();
  ~TraceLog() override;
  void AddMetadataEventsWhileLocked();
  template <typename T>
  void AddMetadataEventWhileLocked(int thread_id,
                                   const char* metadata_name,
                                   const char* arg_name,
                                   const T& value);

  InternalTraceOptions trace_options() const {
    return static_cast<InternalTraceOptions>(
        subtle::NoBarrier_Load(&trace_options_));
  }

  TraceBuffer* trace_buffer() const { return logged_events_.get(); }
  TraceBuffer* CreateTraceBuffer();

  std::string EventToConsoleMessage(unsigned char phase,
                                    const TimeTicks& timestamp,
                                    TraceEvent* trace_event);

  TraceEvent* AddEventToThreadSharedChunkWhileLocked(TraceEventHandle* handle,
                                                     bool check_buffer_is_full);
  void CheckIfBufferIsFullWhileLocked();
  void SetDisabledWhileLocked(uint8_t modes);

  TraceEvent* GetEventByHandleInternal(TraceEventHandle handle,
                                       OptionalAutoLock* lock);

  void FlushInternal(const OutputCallback& cb,
                     bool use_worker_thread,
                     bool discard_events);


  void FlushCurrentThread(int generation, bool discard_events);

  static void ConvertTraceEventsToTraceFormat(
      std::unique_ptr<TraceBuffer> logged_events,
      const TraceLog::OutputCallback& flush_output_callback,
      const ArgumentFilterPredicate& argument_filter_predicate);
  void FinishFlush(int generation, bool discard_events);
  void OnFlushTimeout(int generation, bool discard_events);

  int generation() const {
    return static_cast<int>(subtle::NoBarrier_Load(&generation_));
  }
  bool CheckGeneration(int generation) const {
    return generation == this->generation();
  }
  void UseNextTraceBuffer();

  TimeTicks OffsetNow() const {

    return OffsetTimestamp(base::subtle::TimeTicksNowIgnoringOverride());
  }
  TimeTicks OffsetTimestamp(const TimeTicks& timestamp) const {
    return timestamp - time_offset_;
  }


  static const InternalTraceOptions kInternalNone;
  static const InternalTraceOptions kInternalRecordUntilFull;
  static const InternalTraceOptions kInternalRecordContinuously;
  static const InternalTraceOptions kInternalEchoToConsole;
  static const InternalTraceOptions kInternalRecordAsMuchAsPossible;
  static const InternalTraceOptions kInternalEnableArgumentFilter;


  mutable Lock lock_;


  Lock thread_info_lock_;
  uint8_t enabled_modes_;  // See TraceLog::Mode.
  int num_traces_recorded_;
  std::unique_ptr<TraceBuffer> logged_events_;
  std::vector<std::unique_ptr<TraceEvent>> metadata_events_;

  mutable Lock observers_lock_;
  bool dispatching_to_observers_ = false;
  std::vector<EnabledStateObserver*> enabled_state_observers_;
  std::map<AsyncEnabledStateObserver*, RegisteredAsyncObserver>
      async_observers_;


  std::vector<std::unique_ptr<EnabledStateObserver>>
      owned_enabled_state_observer_copy_;

  std::string process_name_;
  std::unordered_map<int, std::string> process_labels_;
  int process_sort_index_;
  std::unordered_map<int, int> thread_sort_indices_;
  std::unordered_map<int, std::string> thread_names_;
  base::Time process_creation_time_;

  std::unordered_map<int, base::stack<TimeTicks>> thread_event_start_times_;
  std::unordered_map<std::string, int> thread_colors_;

  TimeTicks buffer_limit_reached_timestamp_;

  unsigned long long process_id_hash_;

  int process_id_;

  TimeDelta time_offset_;

  subtle::AtomicWord /* Options */ trace_options_;

  TraceConfig trace_config_;
  TraceConfig::EventFilters enabled_event_filters_;

  ThreadLocalPointer<ThreadLocalEventBuffer> thread_local_event_buffer_;
  ThreadLocalBoolean thread_blocks_message_loop_;
  ThreadLocalBoolean thread_is_in_trace_event_;


  std::unordered_map<int, scoped_refptr<SingleThreadTaskRunner>>
      thread_task_runners_;


  std::unique_ptr<TraceBufferChunk> thread_shared_chunk_;
  size_t thread_shared_chunk_index_;

  OutputCallback flush_output_callback_;
  scoped_refptr<SequencedTaskRunner> flush_task_runner_;
  ArgumentFilterPredicate argument_filter_predicate_;
  MetadataFilterPredicate metadata_filter_predicate_;
  subtle::AtomicWord generation_;
  bool use_worker_thread_;
  std::atomic<AddTraceEventOverrideFunction> add_trace_event_override_{nullptr};
  std::atomic<OnFlushFunction> on_flush_override_{nullptr};
  std::atomic<UpdateDurationFunction> update_duration_override_{nullptr};

  FilterFactoryForTesting filter_factory_for_testing_;

  DISALLOW_COPY_AND_ASSIGN(TraceLog);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACE_LOG_H_
