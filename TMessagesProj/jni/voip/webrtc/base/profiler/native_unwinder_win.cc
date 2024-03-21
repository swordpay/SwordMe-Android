// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/native_unwinder_win.h"

#include <winnt.h>

#include "base/profiler/native_unwinder.h"
#include "base/profiler/win32_stack_frame_unwinder.h"

namespace base {

bool NativeUnwinderWin::CanUnwindFrom(const Frame& current_frame) const {
  return current_frame.module && current_frame.module->IsNative();
}

// successful appends frames onto the stack and returns true. Otherwise
// returns false.
UnwindResult NativeUnwinderWin::TryUnwind(RegisterContext* thread_context,
                                          uintptr_t stack_top,
                                          ModuleCache* module_cache,
                                          std::vector<Frame>* stack) const {


  DCHECK_GT(stack->size(), 0u);

  Win32StackFrameUnwinder frame_unwinder;
  for (;;) {
    if (!stack->back().module) {












      return UnwindResult::ABORTED;
    }

    if (!stack->back().module->IsNative()) {



      return UnwindResult::UNRECOGNIZED_FRAME;
    }

    uintptr_t prev_stack_pointer = RegisterContextStackPointer(thread_context);
    if (!frame_unwinder.TryUnwind(stack->size() == 1u, thread_context,
                                  stack->back().module)) {
      return UnwindResult::ABORTED;
    }

    if (ContextPC(thread_context) == 0)
      return UnwindResult::COMPLETED;

    struct {
      uintptr_t start;
      uintptr_t end;
    } expected_stack_pointer_range = {prev_stack_pointer, stack_top};

#if defined(ARCH_CPU_ARM64)


    if (stack->size() == 1u) {
      expected_stack_pointer_range.start--;
    }
#endif
    if (RegisterContextStackPointer(thread_context) <=
            expected_stack_pointer_range.start ||
        RegisterContextStackPointer(thread_context) >=
            expected_stack_pointer_range.end) {
      return UnwindResult::ABORTED;
    }

    stack->emplace_back(
        ContextPC(thread_context),
        module_cache->GetModuleForAddress(ContextPC(thread_context)));
  }

  NOTREACHED();
  return UnwindResult::COMPLETED;
}

std::unique_ptr<Unwinder> CreateNativeUnwinder(ModuleCache* module_cache) {
  return std::make_unique<NativeUnwinderWin>();
}

}  // namespace base
