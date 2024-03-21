// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_PROCESS_MEMORY_DUMP_H_
#define BASE_TRACE_EVENT_PROCESS_MEMORY_DUMP_H_

#include <stddef.h>

#include <map>
#include <unordered_map>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/trace_event/heap_profiler_allocation_context.h"
#include "base/trace_event/memory_allocator_dump.h"
#include "base/trace_event/memory_allocator_dump_guid.h"
#include "base/trace_event/memory_dump_request_args.h"
#include "build/build_config.h"

// resident memory.
#if !defined(OS_NACL)
#define COUNT_RESIDENT_BYTES_SUPPORTED
#endif

namespace base {

class UnguessableToken;

namespace trace_event {

class TracedValue;

// produced by the MemoryDumpProvider(s) for a specific process.
class BASE_EXPORT ProcessMemoryDump {
 public:
  struct BASE_EXPORT MemoryAllocatorDumpEdge {
    bool operator==(const MemoryAllocatorDumpEdge&) const;
    bool operator!=(const MemoryAllocatorDumpEdge&) const;

    MemoryAllocatorDumpGuid source;
    MemoryAllocatorDumpGuid target;
    int importance = 0;
    bool overridable = false;
  };


  using AllocatorDumpsMap =
      std::map<std::string, std::unique_ptr<MemoryAllocatorDump>>;

  using AllocatorDumpEdgesMap =
      std::map<MemoryAllocatorDumpGuid, MemoryAllocatorDumpEdge>;

#if defined(COUNT_RESIDENT_BYTES_SUPPORTED)




  static size_t GetSystemPageSize();




  static size_t CountResidentBytes(void* start_address, size_t mapped_size);


  static base::Optional<size_t> CountResidentBytesInSharedMemory(
      void* start_address,
      size_t mapped_size);
#endif

  explicit ProcessMemoryDump(const MemoryDumpArgs& dump_args);
  ProcessMemoryDump(ProcessMemoryDump&&);
  ~ProcessMemoryDump();

  ProcessMemoryDump& operator=(ProcessMemoryDump&&);












  MemoryAllocatorDump* CreateAllocatorDump(const std::string& absolute_name);
  MemoryAllocatorDump* CreateAllocatorDump(const std::string& absolute_name,
                                           const MemoryAllocatorDumpGuid& guid);


  MemoryAllocatorDump* GetAllocatorDump(const std::string& absolute_name) const;







  MemoryAllocatorDump* GetOrCreateAllocatorDump(
      const std::string& absolute_name);




  MemoryAllocatorDump* CreateSharedGlobalAllocatorDump(
      const MemoryAllocatorDumpGuid& guid);






  MemoryAllocatorDump* CreateWeakSharedGlobalAllocatorDump(
      const MemoryAllocatorDumpGuid& guid);

  MemoryAllocatorDump* GetSharedGlobalAllocatorDump(
      const MemoryAllocatorDumpGuid& guid) const;

  const AllocatorDumpsMap& allocator_dumps() const { return allocator_dumps_; }

  AllocatorDumpsMap* mutable_allocator_dumps_for_serialization() const {


    return const_cast<AllocatorDumpsMap*>(&allocator_dumps_);
  }
  void SetAllocatorDumpsForSerialization(
      std::vector<std::unique_ptr<MemoryAllocatorDump>>);

  std::vector<MemoryAllocatorDumpEdge> GetAllEdgesForSerialization() const;
  void SetAllEdgesForSerialization(const std::vector<MemoryAllocatorDumpEdge>&);

  void DumpHeapUsage(
      const std::unordered_map<base::trace_event::AllocationContext,
                               base::trace_event::AllocationMetrics>&
          metrics_by_context,
      base::trace_event::TraceEventMemoryOverhead& overhead,
      const char* allocator_name);







  void AddOwnershipEdge(const MemoryAllocatorDumpGuid& source,
                        const MemoryAllocatorDumpGuid& target,
                        int importance);
  void AddOwnershipEdge(const MemoryAllocatorDumpGuid& source,
                        const MemoryAllocatorDumpGuid& target);



  void AddOverridableOwnershipEdge(const MemoryAllocatorDumpGuid& source,
                                   const MemoryAllocatorDumpGuid& target,
                                   int importance);











  void CreateSharedMemoryOwnershipEdge(
      const MemoryAllocatorDumpGuid& client_local_dump_guid,
      const UnguessableToken& shared_memory_guid,
      int importance);
  void CreateWeakSharedMemoryOwnershipEdge(
      const MemoryAllocatorDumpGuid& client_local_dump_guid,
      const UnguessableToken& shared_memory_guid,
      int importance);

  const AllocatorDumpEdgesMap& allocator_dumps_edges() const {
    return allocator_dumps_edges_;
  }





  void AddSuballocation(const MemoryAllocatorDumpGuid& source,
                        const std::string& target_node_name);


  void Clear();






  void TakeAllDumpsFrom(ProcessMemoryDump* other);


  void SerializeAllocatorDumpsInto(TracedValue* value) const;

  const MemoryDumpArgs& dump_args() const { return dump_args_; }

 private:
  FRIEND_TEST_ALL_PREFIXES(ProcessMemoryDumpTest, BackgroundModeTest);
  FRIEND_TEST_ALL_PREFIXES(ProcessMemoryDumpTest, SharedMemoryOwnershipTest);
  FRIEND_TEST_ALL_PREFIXES(ProcessMemoryDumpTest, GuidsTest);

  MemoryAllocatorDump* AddAllocatorDumpInternal(
      std::unique_ptr<MemoryAllocatorDump> mad);



  const UnguessableToken& process_token() const { return process_token_; }
  void set_process_token_for_testing(UnguessableToken token) {
    process_token_ = token;
  }



  MemoryAllocatorDumpGuid GetDumpId(const std::string& absolute_name);

  void CreateSharedMemoryOwnershipEdgeInternal(
      const MemoryAllocatorDumpGuid& client_local_dump_guid,
      const UnguessableToken& shared_memory_guid,
      int importance,
      bool is_weak);

  MemoryAllocatorDump* GetBlackHoleMad();

  UnguessableToken process_token_;
  AllocatorDumpsMap allocator_dumps_;

  AllocatorDumpEdgesMap allocator_dumps_edges_;

  MemoryDumpArgs dump_args_;



  std::unique_ptr<MemoryAllocatorDump> black_hole_mad_;


  static bool is_black_hole_non_fatal_for_testing_;

  DISALLOW_COPY_AND_ASSIGN(ProcessMemoryDump);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_PROCESS_MEMORY_DUMP_H_
