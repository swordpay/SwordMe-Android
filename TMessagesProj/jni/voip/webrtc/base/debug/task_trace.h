// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_DEBUG_TASK_TRACE_H_
#define BASE_DEBUG_TASK_TRACE_H_

#include <iosfwd>
#include <string>

#include "base/base_export.h"
#include "base/containers/span.h"
#include "base/debug/stack_trace.h"
#include "base/optional.h"

namespace base {
namespace debug {

// base::TaskRunner::PostTask() that led to the TaskTrace() constructor call.
// Analogous to base::StackTrace, but for posted tasks rather than function
// calls.
//
// Example usage:
//   TaskTrace().Print();
//
// Example output:
//   Task trace:
//   #0 content::ServiceWorkerContextWrapper::DidCheckHasServiceWorker()
//   #1 content::ServiceWorkerStorage::FindForDocumentInDB()
//   #2 content::ServiceWorkerStorage::FindRegistrationForDocument()
//   #3 content::ServiceWorkerContextWrapper::CheckHasServiceWorker()
//   #4 content::ManifestIconDownloader::ScaleIcon()
//   Task trace buffer limit hit, update PendingTask::kTaskBacktraceLength to
//   increase.
class BASE_EXPORT TaskTrace {
 public:
  TaskTrace();

  bool empty() const;

  void Print() const;

  void OutputToStream(std::ostream* os) const;

  std::string ToString() const;

  base::span<const void* const> AddressesForTesting() const;

 private:
  base::Optional<StackTrace> stack_trace_;
  bool trace_overflow_ = false;
};

BASE_EXPORT std::ostream& operator<<(std::ostream& os,
                                     const TaskTrace& task_trace);

}  // namespace debug
}  // namespace base

#endif  // BASE_DEBUG_TASK_TRACE_H_
