// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_STACK_SAMPLING_PROFILER_H_
#define BASE_PROFILER_STACK_SAMPLING_PROFILER_H_

#include <memory>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/profiler/profile_builder.h"
#include "base/profiler/sampling_profiler_thread_token.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"

namespace base {

class Unwinder;
class StackSampler;
class StackSamplerTestDelegate;

// the purpose of collecting information about which code paths are
// executing. This information is used in aggregate by UMA to identify hot
// and/or janky code paths.
//
// Sample StackSamplingProfiler usage:
//
//   // Create and customize params as desired.
//   base::StackStackSamplingProfiler::SamplingParams params;
//
//   // To process the profiles, use a custom ProfileBuilder subclass:
//   class SubProfileBuilder : public base::ProfileBuilder {...}
//
//   // Then create the profiler:
//   base::StackSamplingProfiler profiler(base::PlatformThread::CurrentId(),
//       params, std::make_unique<SubProfileBuilder>(...));
//
//   // On Android the |sampler| is not implemented in base. So, client can pass
//   // in |sampler| to use while profiling.
//   base::StackSamplingProfiler profiler(base::PlatformThread::CurrentId(),
//       params, std::make_unique<SubProfileBuilder>(...), <optional> sampler);
//
//   // Then start the profiling.
//   profiler.Start();
//   // ... work being done on the target thread here ...
//   // Optionally stop collection before complete per params.
//   profiler.Stop();
//
// The default SamplingParams causes stacks to be recorded in a single profile
// at a 10Hz interval for a total of 30 seconds. All of these parameters may be
// altered as desired.
//
// When a call stack profile is complete, or the profiler is stopped,
// ProfileBuilder's OnProfileCompleted function is called from a thread created
// by the profiler.
class BASE_EXPORT StackSamplingProfiler {
 public:

  struct BASE_EXPORT SamplingParams {

    TimeDelta initial_delay = TimeDelta::FromMilliseconds(0);

    int samples_per_profile = 300;


    TimeDelta sampling_interval = TimeDelta::FromMilliseconds(100);








    bool keep_consistent_sampling_interval = true;
  };





  StackSamplingProfiler(SamplingProfilerThreadToken thread_token,
                        const SamplingParams& params,
                        std::unique_ptr<ProfileBuilder> profile_builder,
                        StackSamplerTestDelegate* test_delegate = nullptr);


  StackSamplingProfiler(const SamplingParams& params,
                        std::unique_ptr<ProfileBuilder> profile_builder,
                        std::unique_ptr<StackSampler> sampler);



  ~StackSamplingProfiler();



  void Start();






  void Stop();


  void AddAuxUnwinder(std::unique_ptr<Unwinder> unwinder);





  class BASE_EXPORT TestPeer {
   public:


    static void Reset();

    static bool IsSamplingThreadRunning();

    static void DisableIdleShutdown();









    static void PerformSamplingThreadIdleShutdown(
        bool simulate_intervening_start);
  };

 private:


  class SamplingThread;


  friend void ApplyMetadataToPastSamplesImpl(TimeTicks period_start,
                                             TimeTicks period_end,
                                             int64_t name_hash,
                                             Optional<int64_t> key,
                                             int64_t value);


  static void ApplyMetadataToPastSamples(TimeTicks period_start,
                                         TimeTicks period_end,
                                         int64_t name_hash,
                                         Optional<int64_t> key,
                                         int64_t value);

  SamplingProfilerThreadToken thread_token_;

  const SamplingParams params_;



  std::unique_ptr<ProfileBuilder> profile_builder_;



  std::unique_ptr<StackSampler> sampler_;


  std::unique_ptr<Unwinder> pending_aux_unwinder_;



  WaitableEvent profiling_inactive_;


  int profiler_id_;

  DISALLOW_COPY_AND_ASSIGN(StackSamplingProfiler);
};

}  // namespace base

#endif  // BASE_PROFILER_STACK_SAMPLING_PROFILER_H_
