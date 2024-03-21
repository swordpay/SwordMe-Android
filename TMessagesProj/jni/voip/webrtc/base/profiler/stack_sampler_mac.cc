// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/stack_sampler.h"

#include "base/profiler/native_unwinder_mac.h"
#include "base/profiler/stack_copier_suspend.h"
#include "base/profiler/stack_sampler_impl.h"
#include "base/profiler/suspendable_thread_delegate_mac.h"

namespace base {

std::unique_ptr<StackSampler> StackSampler::Create(
    SamplingProfilerThreadToken thread_token,
    ModuleCache* module_cache,
    StackSamplerTestDelegate* test_delegate,
    std::unique_ptr<Unwinder> native_unwinder) {
  DCHECK(!native_unwinder);
  return std::make_unique<StackSamplerImpl>(
      std::make_unique<StackCopierSuspend>(
          std::make_unique<SuspendableThreadDelegateMac>(thread_token)),
      std::make_unique<NativeUnwinderMac>(module_cache), module_cache,
      test_delegate);
}

size_t StackSampler::GetStackBufferSize() {
  size_t stack_size = PlatformThread::GetDefaultThreadStackSize();


  return stack_size > 0 ? stack_size : 12 * 1024 * 1024;
}

}  // namespace base
