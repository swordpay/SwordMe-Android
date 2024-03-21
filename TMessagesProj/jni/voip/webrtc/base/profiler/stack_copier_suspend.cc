// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/stack_copier_suspend.h"

#include "base/profiler/stack_buffer.h"
#include "base/profiler/suspendable_thread_delegate.h"

namespace base {

StackCopierSuspend::StackCopierSuspend(
    std::unique_ptr<SuspendableThreadDelegate> thread_delegate)
    : thread_delegate_(std::move(thread_delegate)) {}

StackCopierSuspend::~StackCopierSuspend() = default;

// copied stack state includes the stack itself, the top address of the stack
// copy, and the register context. Returns true on success, and returns the
// copied state via the params.
//
// NO HEAP ALLOCATIONS within the ScopedSuspendThread scope.
bool StackCopierSuspend::CopyStack(StackBuffer* stack_buffer,
                                   uintptr_t* stack_top,
                                   TimeTicks* timestamp,
                                   RegisterContext* thread_context,
                                   Delegate* delegate) {
  const uintptr_t top = thread_delegate_->GetStackBaseAddress();
  uintptr_t bottom = 0;
  const uint8_t* stack_copy_bottom = nullptr;
  {


    std::unique_ptr<SuspendableThreadDelegate::ScopedSuspendThread>
        suspend_thread = thread_delegate_->CreateScopedSuspendThread();


    *timestamp = TimeTicks::Now();

    if (!suspend_thread->WasSuccessful())
      return false;

    if (!thread_delegate_->GetThreadContext(thread_context))
      return false;

    bottom = RegisterContextStackPointer(thread_context);




    if ((top - bottom) > stack_buffer->size())
      return false;

    if (!thread_delegate_->CanCopyStack(bottom))
      return false;

    delegate->OnStackCopy();

    stack_copy_bottom = CopyStackContentsAndRewritePointers(
        reinterpret_cast<uint8_t*>(bottom), reinterpret_cast<uintptr_t*>(top),
        StackBuffer::kPlatformStackAlignment, stack_buffer->buffer());
  }

  delegate->OnThreadResume();

  *stack_top = reinterpret_cast<uintptr_t>(stack_copy_bottom) + (top - bottom);

  for (uintptr_t* reg :
       thread_delegate_->GetRegistersToRewrite(thread_context)) {
    *reg = RewritePointerIfInOriginalStack(reinterpret_cast<uint8_t*>(bottom),
                                           reinterpret_cast<uintptr_t*>(top),
                                           stack_copy_bottom, *reg);
  }

  return true;
}

}  // namespace base
