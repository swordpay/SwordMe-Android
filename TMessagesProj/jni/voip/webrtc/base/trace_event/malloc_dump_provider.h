// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MALLOC_DUMP_PROVIDER_H_
#define BASE_TRACE_EVENT_MALLOC_DUMP_PROVIDER_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "base/trace_event/memory_dump_provider.h"
#include "build/build_config.h"

#if defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_WIN) || \
    (defined(OS_MACOSX) && !defined(OS_IOS))
#define MALLOC_MEMORY_TRACING_SUPPORTED
#endif

namespace base {
namespace trace_event {

class BASE_EXPORT MallocDumpProvider : public MemoryDumpProvider {
 public:


  static const char kAllocatedObjects[];

  static MallocDumpProvider* GetInstance();

  bool OnMemoryDump(const MemoryDumpArgs& args,
                    ProcessMemoryDump* pmd) override;



  void EnableMetrics();
  void DisableMetrics();

 private:
  friend struct DefaultSingletonTraits<MallocDumpProvider>;

  MallocDumpProvider();
  ~MallocDumpProvider() override;

  bool emit_metrics_on_memory_dump_ = true;
  base::Lock emit_metrics_on_memory_dump_lock_;

  DISALLOW_COPY_AND_ASSIGN(MallocDumpProvider);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MALLOC_DUMP_PROVIDER_H_
