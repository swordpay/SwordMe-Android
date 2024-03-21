// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/platform_thread.h"

#include <memory>

#include "base/feature_list.h"

namespace base {

namespace {

// PlatformThread::SetCurrentThreadPriority() no-ops.
const Feature kThreadPrioritiesFeature{"ThreadPriorities",
                                       FEATURE_ENABLED_BY_DEFAULT};

//
// PlatformThread::SetCurrentThreadPriority() doesn't query the state of the
// feature directly because FeatureList initialization is not always
// synchronized with PlatformThread::SetCurrentThreadPriority().
std::atomic<bool> g_use_thread_priorities(true);

}  // namespace

void PlatformThread::SetCurrentThreadPriority(ThreadPriority priority) {
  if (g_use_thread_priorities.load())
    SetCurrentThreadPriorityImpl(priority);
}

namespace internal {

void InitializeThreadPrioritiesFeature() {




  if (FeatureList::GetInstance() &&
      !FeatureList::IsEnabled(kThreadPrioritiesFeature)) {
    g_use_thread_priorities.store(false);
  }
}

}  // namespace internal

}  // namespace base
