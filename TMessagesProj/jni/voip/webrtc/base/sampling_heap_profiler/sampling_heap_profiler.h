// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SAMPLING_HEAP_PROFILER_SAMPLING_HEAP_PROFILER_H_
#define BASE_SAMPLING_HEAP_PROFILER_SAMPLING_HEAP_PROFILER_H_

#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/sampling_heap_profiler/poisson_allocation_sampler.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"
#include "base/threading/thread_id_name_manager.h"

namespace base {

template <typename T>
class NoDestructor;

// It uses PoissonAllocationSampler to aggregate the heap allocations and
// record samples.
// The recorded samples can then be retrieved using GetSamples method.
class BASE_EXPORT SamplingHeapProfiler
    : private PoissonAllocationSampler::SamplesObserver,
      public base::ThreadIdNameManager::Observer {
 public:
  class BASE_EXPORT Sample {
   public:
    Sample(const Sample&);
    ~Sample();

    size_t size;

    size_t total;

    PoissonAllocationSampler::AllocatorType allocator;

    const char* context = nullptr;

    const char* thread_name = nullptr;






    std::vector<void*> stack;

   private:
    friend class SamplingHeapProfiler;

    Sample(size_t size, size_t total, uint32_t ordinal);

    uint32_t ordinal;
  };



  uint32_t Start();

  void Stop();

  void SetSamplingInterval(size_t sampling_interval);

  void SetRecordThreadNames(bool value);

  static const char* CachedThreadName();





  std::vector<Sample> GetSamples(uint32_t profile_id);

  std::vector<const char*> GetStrings();



  static void** CaptureStackTrace(void** frames,
                                  size_t max_entries,
                                  size_t* count);

  static void Init();
  static SamplingHeapProfiler* Get();

  void OnThreadNameChanged(const char* name) override;

 private:
  SamplingHeapProfiler();
  ~SamplingHeapProfiler() override;

  void SampleAdded(void* address,
                   size_t size,
                   size_t total,
                   PoissonAllocationSampler::AllocatorType type,
                   const char* context) override;
  void SampleRemoved(void* address) override;

  void CaptureMixedStack(const char* context, Sample* sample);
  void CaptureNativeStack(const char* context, Sample* sample);
  const char* RecordString(const char* string) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Lock mutex_;

  std::unordered_map<void*, Sample> samples_ GUARDED_BY(mutex_);






  std::unordered_set<const char*> strings_ GUARDED_BY(mutex_);


  Lock start_stop_mutex_;

  int running_sessions_ = 0;

  std::atomic<uint32_t> last_sample_ordinal_{1};

  std::atomic<bool> record_thread_names_{false};

  friend class NoDestructor<SamplingHeapProfiler>;
  friend class SamplingHeapProfilerTest;

  DISALLOW_COPY_AND_ASSIGN(SamplingHeapProfiler);
};

}  // namespace base

#endif  // BASE_SAMPLING_HEAP_PROFILER_SAMPLING_HEAP_PROFILER_H_
