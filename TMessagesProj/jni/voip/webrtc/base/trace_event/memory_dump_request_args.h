// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MEMORY_DUMP_REQUEST_ARGS_H_
#define BASE_TRACE_EVENT_MEMORY_DUMP_REQUEST_ARGS_H_

// These are also used in the IPCs for coordinating inter-process memory dumps.

#include <stdint.h>
#include <map>
#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/optional.h"
#include "base/process/process_handle.h"

namespace base {
namespace trace_event {

class ProcessMemoryDump;

// selective enabling of dumps, filtering and post-processing. Keep this
// consistent with memory_instrumentation.mojo and
// memory_instrumentation_struct_traits.{h,cc}
enum class MemoryDumpType {
  PERIODIC_INTERVAL,     // Dumping memory at periodic intervals.
  EXPLICITLY_TRIGGERED,  // Non maskable dump request.
  SUMMARY_ONLY,          // Calculate just the summary & don't add to the trace.
  LAST = SUMMARY_ONLY
};

// Keep this consistent with memory_instrumentation.mojo and
// memory_instrumentation_struct_traits.{h,cc}
enum class MemoryDumpLevelOfDetail : uint32_t {
  FIRST,



  BACKGROUND = FIRST,




  LIGHT,

  DETAILED,

  LAST = DETAILED
};

// more deterministic by forcing garbage collection.
// Keep this consistent with memory_instrumentation.mojo and
// memory_instrumentation_struct_traits.{h,cc}
enum class MemoryDumpDeterminism : uint32_t { NONE, FORCE_GC };

// memory_instrumentation_struct_traits.{h,cc}
struct BASE_EXPORT MemoryDumpRequestArgs {



  uint64_t dump_guid;

  MemoryDumpType dump_type;
  MemoryDumpLevelOfDetail level_of_detail;
  MemoryDumpDeterminism determinism;
};

// providers. Dump providers are expected to read the args for creating dumps.
struct MemoryDumpArgs {

  MemoryDumpLevelOfDetail level_of_detail;

  MemoryDumpDeterminism determinism;



  uint64_t dump_guid;
};

using ProcessMemoryDumpCallback = OnceCallback<
    void(bool success, uint64_t dump_guid, std::unique_ptr<ProcessMemoryDump>)>;

BASE_EXPORT const char* MemoryDumpTypeToString(const MemoryDumpType& dump_type);

BASE_EXPORT MemoryDumpType StringToMemoryDumpType(const std::string& str);

BASE_EXPORT const char* MemoryDumpLevelOfDetailToString(
    const MemoryDumpLevelOfDetail& level_of_detail);

BASE_EXPORT MemoryDumpLevelOfDetail
StringToMemoryDumpLevelOfDetail(const std::string& str);

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MEMORY_DUMP_REQUEST_ARGS_H_
