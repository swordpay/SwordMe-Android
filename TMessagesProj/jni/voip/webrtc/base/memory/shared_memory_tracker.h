// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_SHARED_MEMORY_TRACKER_H_
#define BASE_MEMORY_SHARED_MEMORY_TRACKER_H_

#include <map>
#include <string>

#include "base/memory/shared_memory_mapping.h"
#include "base/synchronization/lock.h"
#include "base/trace_event/memory_dump_provider.h"

namespace base {

namespace trace_event {
class MemoryAllocatorDump;
class MemoryAllocatorDumpGuid;
class ProcessMemoryDump;
}

class BASE_EXPORT SharedMemoryTracker : public trace_event::MemoryDumpProvider {
 public:

  static SharedMemoryTracker* GetInstance();

  static std::string GetDumpNameForTracing(const UnguessableToken& id);

  static trace_event::MemoryAllocatorDumpGuid GetGlobalDumpIdForTracing(
      const UnguessableToken& id);



  static const trace_event::MemoryAllocatorDump* GetOrCreateSharedMemoryDump(
      const SharedMemoryMapping& shared_memory,
      trace_event::ProcessMemoryDump* pmd);

  void IncrementMemoryUsage(const SharedMemoryMapping& mapping);

  void DecrementMemoryUsage(const SharedMemoryMapping& mapping);

  static const char kDumpRootName[];

 private:
  SharedMemoryTracker();
  ~SharedMemoryTracker() override;

  bool OnMemoryDump(const trace_event::MemoryDumpArgs& args,
                    trace_event::ProcessMemoryDump* pmd) override;

  static const trace_event::MemoryAllocatorDump*
  GetOrCreateSharedMemoryDumpInternal(void* mapped_memory,
                                      size_t mapped_size,
                                      const UnguessableToken& mapped_id,
                                      trace_event::ProcessMemoryDump* pmd);

  struct UsageInfo {
    UsageInfo(size_t size, const UnguessableToken& id)
        : mapped_size(size), mapped_id(id) {}

    size_t mapped_size;
    UnguessableToken mapped_id;
  };

  Lock usages_lock_;
  std::map<void*, UsageInfo> usages_;

  DISALLOW_COPY_AND_ASSIGN(SharedMemoryTracker);
};

}  // namespace base

#endif  // BASE_MEMORY_SHARED_MEMORY_TRACKER_H_
