// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_WORK_ID_PROVIDER_H_
#define BASE_MESSAGE_LOOP_WORK_ID_PROVIDER_H_

#include <atomic>

#include "base/base_export.h"
#include "base/threading/thread_checker.h"

namespace base {
namespace sequence_manager {
namespace internal {
class ThreadControllerWithMessagePumpImpl;
}
}  // namespace sequence_manager

// reflecting the current work item being executed by the message loop. The item
// is accessed lock-free from other threads to provide a snapshot of the
// currently-executing work item.
//
// The expected user is the ThreadProfiler which samples the id along with the
// thread's stack to identify cases where the same task spans multiple
// samples. The state is stored in TLS rather than on the MessageLoop or the
// ThreadProfiler because the lifetime relationship between the two classes
// varies depending on which thread is being profiled, plus the fact that
// MessageLoop doesn't have a well-defined creation point/owner on some threads.
class BASE_EXPORT WorkIdProvider {
 public:


  static WorkIdProvider* GetForCurrentThread();






  unsigned int GetWorkId();

  ~WorkIdProvider();

  void SetCurrentWorkIdForTesting(unsigned int id);
  void IncrementWorkIdForTesting();

  WorkIdProvider(const WorkIdProvider&) = delete;
  WorkIdProvider& operator=(const WorkIdProvider&) = delete;

 private:

  friend class sequence_manager::internal::ThreadControllerWithMessagePumpImpl;

  WorkIdProvider();

  void IncrementWorkId();

  std::atomic_uint work_id_;

  THREAD_CHECKER(thread_checker_);
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_WORK_ID_PROVIDER_H_
