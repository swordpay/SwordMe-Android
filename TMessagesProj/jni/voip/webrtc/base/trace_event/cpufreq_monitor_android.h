// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_CPUFREQ_MONITOR_ANDROID_H_
#define BASE_TRACE_EVENT_CPUFREQ_MONITOR_ANDROID_H_

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/files/scoped_file.h"
#include "base/memory/scoped_refptr.h"
#include "base/trace_event/trace_log.h"

namespace base {

class SingleThreadTaskRunner;

namespace trace_event {

class BASE_EXPORT CPUFreqMonitorDelegate {
 public:
  CPUFreqMonitorDelegate();
  virtual ~CPUFreqMonitorDelegate() = default;



  virtual void GetCPUIds(std::vector<unsigned int>* ids) const;


  virtual unsigned int GetKernelMaxCPUs() const;

  virtual void RecordFrequency(unsigned int cpu_id, unsigned int freq);


  virtual bool IsTraceCategoryEnabled() const;

  virtual std::string GetScalingCurFreqPathString(unsigned int cpu_id) const;
  virtual std::string GetRelatedCPUsPathString(unsigned int cpu_id) const;


  virtual scoped_refptr<SingleThreadTaskRunner> CreateTaskRunner();

 private:
  DISALLOW_COPY_AND_ASSIGN(CPUFreqMonitorDelegate);
};

class BASE_EXPORT CPUFreqMonitor : public TraceLog::EnabledStateObserver {
 public:


  static const size_t kDefaultCPUFreqSampleIntervalMs = 50;

  CPUFreqMonitor();
  ~CPUFreqMonitor() override;

  static CPUFreqMonitor* GetInstance();

  void Start();
  void Stop();

  void OnTraceLogEnabled() override;
  void OnTraceLogDisabled() override;

  bool IsEnabledForTesting();

 private:
  friend class CPUFreqMonitorTest;

  CPUFreqMonitor(std::unique_ptr<CPUFreqMonitorDelegate> delegate);

  void Sample(std::vector<std::pair<unsigned int, base::ScopedFD>> fds);



  const scoped_refptr<SingleThreadTaskRunner>& GetOrCreateTaskRunner();

  base::subtle::Atomic32 is_enabled_ = 0;
  scoped_refptr<SingleThreadTaskRunner> task_runner_;
  std::unique_ptr<CPUFreqMonitorDelegate> delegate_;
  base::WeakPtrFactory<CPUFreqMonitor> weak_ptr_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(CPUFreqMonitor);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_CPUFREQ_MONITOR_ANDROID_H_
