// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MEMORY_DUMP_PROVIDER_INFO_H_
#define BASE_TRACE_EVENT_MEMORY_DUMP_PROVIDER_INFO_H_

#include <memory>
#include <set>

#include "base/base_export.h"
#include "base/memory/ref_counted.h"
#include "base/trace_event/memory_dump_provider.h"

namespace base {

class SequencedTaskRunner;

namespace trace_event {

// MemoryDumpManager(MDM)::RegisterDumpProvider(), holding the extra information
// required to deal with it (which task runner it should be invoked onto,
// whether it has been disabled, etc.)
// More importantly, having a refptr to this object guarantees that a MDP that
// is not thread-bound (hence which can only be unregistered via
// MDM::UnregisterAndDeleteDumpProviderSoon()) will stay alive as long as the
// refptr is held.
//
// Lifetime:
// At any time, there is at most one instance of this class for each instance
// of a given MemoryDumpProvider, but there might be several scoped_refptr
// holding onto each of this. Specifically:
// - In nominal conditions, there is a refptr for each registered MDP in the
//   MDM's |dump_providers_| list.
// - In most cases, the only refptr (in the |dump_providers_| list) is destroyed
//   by MDM::UnregisterDumpProvider().
// - However, when MDM starts a dump, the list of refptrs is copied into the
//   ProcessMemoryDumpAsyncState. That list is pruned as MDP(s) are invoked.
// - If UnregisterDumpProvider() is called on a non-thread-bound MDP while a
//   dump is in progress, the extar extra of the handle is destroyed in
//   MDM::SetupNextMemoryDump() or MDM::InvokeOnMemoryDump(), when the copy
//   inside ProcessMemoryDumpAsyncState is erase()-d.
// - The PeakDetector can keep extra refptrs when enabled.
struct BASE_EXPORT MemoryDumpProviderInfo
    : public RefCountedThreadSafe<MemoryDumpProviderInfo> {
 public:


  struct Comparator {
    bool operator()(const scoped_refptr<MemoryDumpProviderInfo>& a,
                    const scoped_refptr<MemoryDumpProviderInfo>& b) const;
  };
  using OrderedSet =
      std::set<scoped_refptr<MemoryDumpProviderInfo>, Comparator>;

  MemoryDumpProviderInfo(MemoryDumpProvider* dump_provider,
                         const char* name,
                         scoped_refptr<SequencedTaskRunner> task_runner,
                         const MemoryDumpProvider::Options& options,
                         bool allowed_in_background_mode);



  MemoryDumpProvider* const dump_provider;

  const MemoryDumpProvider::Options options;


  const char* const name;



  const scoped_refptr<SequencedTaskRunner> task_runner;

  const bool allowed_in_background_mode;






  std::unique_ptr<MemoryDumpProvider> owned_dump_provider;

  int consecutive_failures;

  bool disabled;

 private:
  friend class base::RefCountedThreadSafe<MemoryDumpProviderInfo>;
  ~MemoryDumpProviderInfo();

  DISALLOW_COPY_AND_ASSIGN(MemoryDumpProviderInfo);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MEMORY_DUMP_PROVIDER_INFO_H_
