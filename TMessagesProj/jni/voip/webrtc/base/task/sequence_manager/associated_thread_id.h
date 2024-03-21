// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_ASSOCIATED_THREAD_ID_H_
#define BASE_TASK_SEQUENCE_MANAGER_ASSOCIATED_THREAD_ID_H_

#include <atomic>
#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "base/sequence_checker.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_checker.h"

namespace base {
namespace sequence_manager {
namespace internal {

// refactor has happened (https://crbug.com/865411).
//
// This class is thread-safe. But see notes about memory ordering guarantees for
// the various methods.
class BASE_EXPORT AssociatedThreadId
    : public base::RefCountedThreadSafe<AssociatedThreadId> {
 public:
  AssociatedThreadId();

  THREAD_CHECKER(thread_checker);
  SEQUENCE_CHECKER(sequence_checker);

  static scoped_refptr<AssociatedThreadId> CreateUnbound() {
    return MakeRefCounted<AssociatedThreadId>();
  }

  static scoped_refptr<AssociatedThreadId> CreateBound() {
    auto associated_thread = MakeRefCounted<AssociatedThreadId>();
    associated_thread->BindToCurrentThread();
    return associated_thread;
  }





  void BindToCurrentThread();









  Optional<PlatformThreadId> GetBoundThreadId() const {
    auto thread_id = thread_id_.load(std::memory_order_acquire);
    if (thread_id == kInvalidThreadId) {
      return nullopt;
    } else {
      return thread_id;
    }
  }








  bool IsBound() const {
    return thread_id_.load(std::memory_order_acquire) != kInvalidThreadId;
  }









  bool IsBoundToCurrentThread() const {
    return thread_id_.load(std::memory_order_relaxed) ==
           PlatformThread::CurrentId();
  }



 private:
  friend class base::RefCountedThreadSafe<AssociatedThreadId>;
  ~AssociatedThreadId();


  std::atomic<PlatformThreadId> thread_id_{kInvalidThreadId};
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_ASSOCIATED_THREAD_ID_H_
