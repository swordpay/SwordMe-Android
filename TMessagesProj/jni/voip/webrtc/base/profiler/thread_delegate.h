// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_THREAD_DELEGATE_H_
#define BASE_PROFILER_THREAD_DELEGATE_H_

#include <vector>

#include "base/base_export.h"
#include "base/profiler/register_context.h"
#include "base/threading/platform_thread.h"

namespace base {

// platform-independent stack copying/walking implementation in
// StackSamplerImpl. Provides the common interface across signal- and
// suspend-based stack copy implementations.
class BASE_EXPORT ThreadDelegate {
 public:
  ThreadDelegate() = default;
  virtual ~ThreadDelegate() = default;

  ThreadDelegate(const ThreadDelegate&) = delete;
  ThreadDelegate& operator=(const ThreadDelegate&) = delete;

  virtual PlatformThreadId GetThreadId() const = 0;

  virtual uintptr_t GetStackBaseAddress() const = 0;



  virtual std::vector<uintptr_t*> GetRegistersToRewrite(
      RegisterContext* thread_context) = 0;
};

}  // namespace base

#endif  // BASE_PROFILER_THREAD_DELEGATE_H_
