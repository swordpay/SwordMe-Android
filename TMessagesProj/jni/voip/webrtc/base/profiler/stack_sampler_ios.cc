// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// supported.

#include "base/profiler/stack_sampler.h"

namespace base {

std::unique_ptr<StackSampler> StackSampler::Create(
    SamplingProfilerThreadToken thread_token,
    ModuleCache* module_cache,
    StackSamplerTestDelegate* test_delegate,
    std::unique_ptr<Unwinder> native_unwinder) {
  return nullptr;
}

size_t StackSampler::GetStackBufferSize() {
  return 0;
}

}  // namespace base
