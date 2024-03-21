// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_UNWINDER_H_
#define BASE_PROFILER_UNWINDER_H_

#include <vector>

#include "base/macros.h"
#include "base/profiler/frame.h"
#include "base/profiler/module_cache.h"
#include "base/profiler/register_context.h"

namespace base {

enum class UnwindResult {

  COMPLETED,


  UNRECOGNIZED_FRAME,

  ABORTED,
};

// use with the StackSamplingProfiler. The profiler is expected to call
// CanUnwind() to determine if the Unwinder thinks it can unwind from the frame
// represented by the context values, then TryUnwind() to attempt the
// unwind.
class Unwinder {
 public:
  virtual ~Unwinder() = default;


  virtual void AddInitialModules(ModuleCache* module_cache) {}





  virtual void OnStackCapture() {}



  virtual void UpdateModules(ModuleCache* module_cache) {}






  virtual bool CanUnwindFrom(const Frame& current_frame) const = 0;










  virtual UnwindResult TryUnwind(RegisterContext* thread_context,
                                 uintptr_t stack_top,
                                 ModuleCache* module_cache,
                                 std::vector<Frame>* stack) const = 0;

  Unwinder(const Unwinder&) = delete;
  Unwinder& operator=(const Unwinder&) = delete;

 protected:
  Unwinder() = default;
};

}  // namespace base

#endif  // BASE_PROFILER_UNWINDER_H_
