// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/thread_pool/environment_config.h"

#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"

namespace base {
namespace internal {

namespace {

bool CanUseBackgroundPriorityForWorkerThreadImpl() {




  if (!Lock::HandlesMultipleThreadPriorities())
    return false;

#if !defined(OS_ANDROID)






  if (!PlatformThread::CanIncreaseThreadPriority(ThreadPriority::NORMAL))
    return false;
#endif  // defined(OS_ANDROID)

  return true;
}

}  // namespace

bool CanUseBackgroundPriorityForWorkerThread() {
  static const bool can_use_background_priority_for_worker_thread =
      CanUseBackgroundPriorityForWorkerThreadImpl();
  return can_use_background_priority_for_worker_thread;
}

}  // namespace internal
}  // namespace base
