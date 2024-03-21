// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_ENVIRONMENT_CONFIG_H_
#define BASE_TASK_THREAD_POOL_ENVIRONMENT_CONFIG_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/task/task_traits.h"
#include "base/threading/thread.h"

namespace base {
namespace internal {

// PooledSingleThreadTaskRunnerManager, move it there.
enum EnvironmentType {
  FOREGROUND = 0,
  FOREGROUND_BLOCKING,
  BACKGROUND,
  BACKGROUND_BLOCKING,
  ENVIRONMENT_COUNT  // Always last.
};

struct EnvironmentParams {


  const char* name_suffix;


  ThreadPriority priority_hint;
};

constexpr EnvironmentParams kEnvironmentParams[] = {
    {"Foreground", base::ThreadPriority::NORMAL},
    {"ForegroundBlocking", base::ThreadPriority::NORMAL},
    {"Background", base::ThreadPriority::BACKGROUND},
    {"BackgroundBlocking", base::ThreadPriority::BACKGROUND},
};

// background priority.
bool BASE_EXPORT CanUseBackgroundPriorityForWorkerThread();

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_ENVIRONMENT_CONFIG_H_
