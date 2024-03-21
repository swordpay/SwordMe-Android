// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MEMORY_DUMP_MANAGER_H_
#define BASE_TRACE_EVENT_MEMORY_DUMP_MANAGER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <unordered_set>
#include <vector>

#include "base/atomicops.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "base/trace_event/memory_allocator_dump.h"
#include "base/trace_event/memory_dump_provider_info.h"
#include "base/trace_event/memory_dump_request_args.h"
#include "base/trace_event/process_memory_dump.h"
#include "base/trace_event/trace_event.h"

namespace base {

class SequencedTaskRunner;
class SingleThreadTaskRunner;
class Thread;

namespace trace_event {

class MemoryDumpProvider;

// memory tracing. The main entry point for clients is represented by
// RequestDumpPoint(). The extension by Un(RegisterDumpProvider).
class BASE_EXPORT MemoryDumpManager {
 public:
  using RequestGlobalDumpFunction =
      RepeatingCallback<void(MemoryDumpType, MemoryDumpLevelOfDetail)>;

  static constexpr const char* const kTraceCategory =
      TRACE_DISABLED_BY_DEFAULT("memory-infra");


  static const uint64_t kInvalidTracingProcessId;

  static MemoryDumpManager* GetInstance();
  static std::unique_ptr<MemoryDumpManager> CreateInstanceForTesting();












  void Initialize(RequestGlobalDumpFunction request_dump_function,
                  bool is_coordinator);












  void RegisterDumpProvider(MemoryDumpProvider* mdp,
                            const char* name,
                            scoped_refptr<SingleThreadTaskRunner> task_runner);
  void RegisterDumpProvider(MemoryDumpProvider* mdp,
                            const char* name,
                            scoped_refptr<SingleThreadTaskRunner> task_runner,
                            MemoryDumpProvider::Options options);
  void RegisterDumpProviderWithSequencedTaskRunner(
      MemoryDumpProvider* mdp,
      const char* name,
      scoped_refptr<SequencedTaskRunner> task_runner,
      MemoryDumpProvider::Options options);
  void UnregisterDumpProvider(MemoryDumpProvider* mdp);







  void UnregisterAndDeleteDumpProviderSoon(
      std::unique_ptr<MemoryDumpProvider> mdp);



  void SetupForTracing(const TraceConfig::MemoryDumpConfig&);


  void TeardownForTracing();




  void CreateProcessDump(const MemoryDumpRequestArgs& args,
                         ProcessMemoryDumpCallback callback);

  bool IsDumpProviderRegisteredForTesting(MemoryDumpProvider*);




  uint64_t GetTracingProcessId() const { return tracing_process_id_; }
  void set_tracing_process_id(uint64_t tracing_process_id) {
    tracing_process_id_ = tracing_process_id;
  }




  const char* system_allocator_pool_name() const {
    return kSystemAllocatorPoolName;
  }

  void set_dumper_registrations_ignored_for_testing(bool ignored) {
    dumper_registrations_ignored_for_testing_ = ignored;
  }

  scoped_refptr<SequencedTaskRunner> GetDumpThreadTaskRunner();

 private:
  friend std::default_delete<MemoryDumpManager>;  // For the testing instance.
  friend struct DefaultSingletonTraits<MemoryDumpManager>;
  friend class MemoryDumpManagerTest;
  FRIEND_TEST_ALL_PREFIXES(MemoryDumpManagerTest,
                           NoStackOverflowWithTooManyMDPs);




  struct ProcessMemoryDumpAsyncState {
    ProcessMemoryDumpAsyncState(
        MemoryDumpRequestArgs req_args,
        const MemoryDumpProviderInfo::OrderedSet& dump_providers,
        ProcessMemoryDumpCallback callback,
        scoped_refptr<SequencedTaskRunner> dump_thread_task_runner);
    ~ProcessMemoryDumpAsyncState();

    std::unique_ptr<ProcessMemoryDump> process_memory_dump;

    const MemoryDumpRequestArgs req_args;



    std::vector<scoped_refptr<MemoryDumpProviderInfo>> pending_dump_providers;

    ProcessMemoryDumpCallback callback;



    const scoped_refptr<SingleThreadTaskRunner> callback_task_runner;





    const scoped_refptr<SequencedTaskRunner> dump_thread_task_runner;

   private:
    DISALLOW_COPY_AND_ASSIGN(ProcessMemoryDumpAsyncState);
  };

  static const int kMaxConsecutiveFailuresCount;
  static const char* const kSystemAllocatorPoolName;

  MemoryDumpManager();
  virtual ~MemoryDumpManager();

  static void SetInstanceForTesting(MemoryDumpManager* instance);

  scoped_refptr<base::SequencedTaskRunner> GetOrCreateBgTaskRunnerLocked();




  void ContinueAsyncProcessDump(
      ProcessMemoryDumpAsyncState* owned_pmd_async_state);


  void InvokeOnMemoryDump(MemoryDumpProviderInfo* mdpinfo,
                          ProcessMemoryDump* pmd);

  void FinishAsyncProcessDump(
      std::unique_ptr<ProcessMemoryDumpAsyncState> pmd_async_state);

  void RegisterDumpProviderInternal(
      MemoryDumpProvider* mdp,
      const char* name,
      scoped_refptr<SequencedTaskRunner> task_runner,
      const MemoryDumpProvider::Options& options);

  void UnregisterDumpProviderInternal(MemoryDumpProvider* mdp,
                                      bool take_mdp_ownership_and_delete_async);

  bool can_request_global_dumps() const {
    return !request_dump_function_.is_null();
  }


  MemoryDumpProviderInfo::OrderedSet dump_providers_;

  RequestGlobalDumpFunction request_dump_function_;

  bool is_coordinator_;


  Lock lock_;


  std::unique_ptr<Thread> dump_thread_;


  uint64_t tracing_process_id_;

  bool dumper_registrations_ignored_for_testing_;

  DISALLOW_COPY_AND_ASSIGN(MemoryDumpManager);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MEMORY_DUMP_MANAGER_H_
