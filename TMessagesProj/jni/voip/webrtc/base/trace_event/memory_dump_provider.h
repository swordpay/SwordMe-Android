// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MEMORY_DUMP_PROVIDER_H_
#define BASE_TRACE_EVENT_MEMORY_DUMP_PROVIDER_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/process/process_handle.h"
#include "base/trace_event/memory_dump_request_args.h"

namespace base {
namespace trace_event {

class ProcessMemoryDump;

class BASE_EXPORT MemoryDumpProvider {
 public:

  struct Options {
    Options() : dumps_on_single_thread_task_runner(false) {}



    bool dumps_on_single_thread_task_runner;
  };

  virtual ~MemoryDumpProvider() = default;







  virtual bool OnMemoryDump(const MemoryDumpArgs& args,
                            ProcessMemoryDump* pmd) = 0;

 protected:
  MemoryDumpProvider() = default;

  DISALLOW_COPY_AND_ASSIGN(MemoryDumpProvider);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MEMORY_DUMP_PROVIDER_H_
