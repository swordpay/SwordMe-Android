// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_STACK_SAMPLING_PROFILER_TEST_UTIL_H_
#define BASE_PROFILER_STACK_SAMPLING_PROFILER_TEST_UTIL_H_

#include <memory>
#include <vector>

#include "base/callback.h"
#include "base/profiler/frame.h"
#include "base/profiler/sampling_profiler_thread_token.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/platform_thread.h"

namespace base {

class Unwinder;

class TargetThread : public PlatformThread::Delegate {
 public:
  TargetThread(OnceClosure to_run);
  ~TargetThread() override;

  void ThreadMain() override;

  SamplingProfilerThreadToken thread_token() const { return thread_token_; }

 private:
  SamplingProfilerThreadToken thread_token_ = {0};
  OnceClosure to_run_;

  DISALLOW_COPY_AND_ASSIGN(TargetThread);
};

struct FunctionAddressRange {
  const void* start;
  const void* end;
};

// StackSamplingProfiler.
class UnwindScenario {
 public:




  using SetupFunction = RepeatingCallback<FunctionAddressRange(OnceClosure)>;

  struct SampleEvents {
    WaitableEvent ready_for_sample;
    WaitableEvent sample_finished;
  };

  explicit UnwindScenario(const SetupFunction& setup_function);
  ~UnwindScenario();

  UnwindScenario(const UnwindScenario&) = delete;
  UnwindScenario& operator=(const UnwindScenario&) = delete;

  FunctionAddressRange GetWaitForSampleAddressRange() const;

  FunctionAddressRange GetSetupFunctionAddressRange() const;


  FunctionAddressRange GetOuterFunctionAddressRange() const;

  void Execute(SampleEvents* events);

 private:
  static FunctionAddressRange InvokeSetupFunction(
      const SetupFunction& setup_function,
      SampleEvents* events);

  static FunctionAddressRange WaitForSample(SampleEvents* events);

  const SetupFunction setup_function_;
};

// any special unwinding setup, to exercise the "normal" unwind scenario.
FunctionAddressRange CallWithPlainFunction(OnceClosure wait_for_sample);

using ProfileCallback = OnceCallback<void(SamplingProfilerThreadToken)>;

// thread. Performs all necessary target thread startup and shutdown work before
// and afterward.
void WithTargetThread(UnwindScenario* scenario,
                      ProfileCallback profile_callback);

using UnwinderFactory = OnceCallback<std::unique_ptr<Unwinder>()>;

std::vector<Frame> SampleScenario(
    UnwindScenario* scenario,
    ModuleCache* module_cache,
    UnwinderFactory aux_unwinder_factory = UnwinderFactory());

std::string FormatSampleForDiagnosticOutput(const std::vector<Frame>& sample);

// ranges, in the specified order.
void ExpectStackContains(const std::vector<Frame>& stack,
                         const std::vector<FunctionAddressRange>& functions);

// address ranges.
void ExpectStackDoesNotContain(
    const std::vector<Frame>& stack,
    const std::vector<FunctionAddressRange>& functions);

}  // namespace base

#endif  // BASE_PROFILER_STACK_SAMPLING_PROFILER_TEST_UTIL_H_
