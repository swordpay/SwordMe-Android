// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_WORKER_THREAD_STACK_H_
#define BASE_TASK_THREAD_POOL_WORKER_THREAD_STACK_H_

#include <stddef.h>

#include <vector>

#include "base/base_export.h"
#include "base/macros.h"

namespace base {
namespace internal {

class WorkerThread;

// of the stack as being "in-use" (so its time in that position doesn't count
// towards being inactive / reclaimable). Supports removal of arbitrary
// WorkerThreads. DCHECKs when a WorkerThread is inserted multiple times.
// WorkerThreads are not owned by the stack. Push() is amortized O(1). Pop(),
// Peek(), Size() and Empty() are O(1). Contains() and Remove() are O(n). This
// class is NOT thread-safe.
class BASE_EXPORT WorkerThreadStack {
 public:
  WorkerThreadStack();
  ~WorkerThreadStack();



  void Push(WorkerThread* worker);



  WorkerThread* Pop();

  WorkerThread* Peek() const;

  bool Contains(const WorkerThread* worker) const;


  void Remove(const WorkerThread* worker);

  size_t Size() const { return stack_.size(); }

  bool IsEmpty() const { return stack_.empty(); }

 private:
  std::vector<WorkerThread*> stack_;

  DISALLOW_COPY_AND_ASSIGN(WorkerThreadStack);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_WORKER_THREAD_STACK_H_
