// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_STACK_SAMPLER_H_
#define BASE_PROFILER_STACK_SAMPLER_H_

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/profiler/sampling_profiler_thread_token.h"
#include "base/threading/platform_thread.h"

namespace base {

class Unwinder;
class ModuleCache;
class ProfileBuilder;
class StackBuffer;
class StackSamplerTestDelegate;

// abstracts the native implementation required to record a set of stack frames
// for a given thread.
class BASE_EXPORT StackSampler {
 public:
  virtual ~StackSampler();



  static std::unique_ptr<StackSampler> Create(
      SamplingProfilerThreadToken thread_token,
      ModuleCache* module_cache,
      StackSamplerTestDelegate* test_delegate,
      std::unique_ptr<Unwinder> native_unwinder = nullptr);

  static size_t GetStackBufferSize();


  static std::unique_ptr<StackBuffer> CreateStackBuffer();




  virtual void AddAuxUnwinder(std::unique_ptr<Unwinder> unwinder) = 0;

  virtual void RecordStackFrames(StackBuffer* stackbuffer,
                                 ProfileBuilder* profile_builder) = 0;

 protected:
  StackSampler();

 private:
  DISALLOW_COPY_AND_ASSIGN(StackSampler);
};

// collection.
class BASE_EXPORT StackSamplerTestDelegate {
 public:
  virtual ~StackSamplerTestDelegate();


  virtual void OnPreStackWalk() = 0;

 protected:
  StackSamplerTestDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(StackSamplerTestDelegate);
};

}  // namespace base

#endif  // BASE_PROFILER_STACK_SAMPLER_H_
