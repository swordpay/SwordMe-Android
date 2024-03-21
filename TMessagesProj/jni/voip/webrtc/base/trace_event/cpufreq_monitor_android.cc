// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/trace_event/cpufreq_monitor_android.h"

#include <fcntl.h>

#include "base/atomicops.h"
#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/memory/scoped_refptr.h"
#include "base/no_destructor.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "base/task/post_task.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/trace_event/trace_event.h"

namespace base {

namespace trace_event {

namespace {

const size_t kNumBytesToReadForSampling = 32;
constexpr const char kTraceCategory[] = TRACE_DISABLED_BY_DEFAULT("power");
const char kEventTitle[] = "CPU Frequency";

}  // namespace

CPUFreqMonitorDelegate::CPUFreqMonitorDelegate() {}

std::string CPUFreqMonitorDelegate::GetScalingCurFreqPathString(
    unsigned int cpu_id) const {
  return base::StringPrintf(
      "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu_id);
}

bool CPUFreqMonitorDelegate::IsTraceCategoryEnabled() const {
  bool enabled;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(kTraceCategory, &enabled);
  return enabled;
}

unsigned int CPUFreqMonitorDelegate::GetKernelMaxCPUs() const {
  std::string str;
  if (!base::ReadFileToString(
          base::FilePath("/sys/devices/system/cpu/kernel_max"), &str)) {

    return 0;
  }

  unsigned int kernel_max_cpu = 0;
  base::StringToUint(str, &kernel_max_cpu);
  return kernel_max_cpu;
}

std::string CPUFreqMonitorDelegate::GetRelatedCPUsPathString(
    unsigned int cpu_id) const {
  return base::StringPrintf(
      "/sys/devices/system/cpu/cpu%d/cpufreq/related_cpus", cpu_id);
}

void CPUFreqMonitorDelegate::GetCPUIds(std::vector<unsigned int>* ids) const {
  ids->clear();
  unsigned int kernel_max_cpu = GetKernelMaxCPUs();


  char cpus_to_monitor[kernel_max_cpu + 1];
  std::memset(cpus_to_monitor, 1, kernel_max_cpu + 1);


  for (unsigned int i = 0; i <= kernel_max_cpu; i++) {
    if (!cpus_to_monitor[i])
      continue;

    std::string filename = GetRelatedCPUsPathString(i);
    std::string line;
    if (!base::ReadFileToString(base::FilePath(filename), &line))
      continue;


    for (auto& str_piece :
         base::SplitString(line, " ", base::WhitespaceHandling::TRIM_WHITESPACE,
                           base::SplitResult::SPLIT_WANT_NONEMPTY)) {
      unsigned int cpu_id;
      if (base::StringToUint(str_piece, &cpu_id)) {
        if (cpu_id != i && cpu_id >= 0 && cpu_id <= kernel_max_cpu)
          cpus_to_monitor[cpu_id] = 0;
      }
    }
    ids->push_back(i);
  }


  if (ids->size() == 0)
    ids->push_back(0);
}

void CPUFreqMonitorDelegate::RecordFrequency(unsigned int cpu_id,
                                             unsigned int freq) {
  TRACE_COUNTER_ID1(kTraceCategory, kEventTitle, cpu_id, freq);
}

scoped_refptr<SingleThreadTaskRunner>
CPUFreqMonitorDelegate::CreateTaskRunner() {
  return base::ThreadPool::CreateSingleThreadTaskRunner(
      {base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN,
       base::TaskPriority::BEST_EFFORT},
      base::SingleThreadTaskRunnerThreadMode::SHARED);
}

CPUFreqMonitor::CPUFreqMonitor()
    : CPUFreqMonitor(std::make_unique<CPUFreqMonitorDelegate>()) {}

CPUFreqMonitor::CPUFreqMonitor(std::unique_ptr<CPUFreqMonitorDelegate> delegate)
    : delegate_(std::move(delegate)) {}

CPUFreqMonitor::~CPUFreqMonitor() {
  Stop();
}

CPUFreqMonitor* CPUFreqMonitor::GetInstance() {
  static base::NoDestructor<CPUFreqMonitor> instance;
  return instance.get();
}

void CPUFreqMonitor::OnTraceLogEnabled() {
  GetOrCreateTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&CPUFreqMonitor::Start, weak_ptr_factory_.GetWeakPtr()));
}

void CPUFreqMonitor::OnTraceLogDisabled() {
  Stop();
}

void CPUFreqMonitor::Start() {



  if (base::subtle::NoBarrier_Load(&is_enabled_) == 1 ||
      !delegate_->IsTraceCategoryEnabled()) {
    return;
  }

  std::vector<unsigned int> cpu_ids;
  delegate_->GetCPUIds(&cpu_ids);

  std::vector<std::pair<unsigned int, base::ScopedFD>> fds;
  for (unsigned int id : cpu_ids) {
    std::string fstr = delegate_->GetScalingCurFreqPathString(id);
    int fd = open(fstr.c_str(), O_RDONLY);
    if (fd == -1)
      continue;

    fds.emplace_back(std::make_pair(id, base::ScopedFD(fd)));
  }

  if (fds.size() == 0)
    return;

  base::subtle::Release_Store(&is_enabled_, 1);

  GetOrCreateTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&CPUFreqMonitor::Sample, weak_ptr_factory_.GetWeakPtr(),
                     std::move(fds)));
}

void CPUFreqMonitor::Stop() {
  base::subtle::Release_Store(&is_enabled_, 0);
}

void CPUFreqMonitor::Sample(
    std::vector<std::pair<unsigned int, base::ScopedFD>> fds) {




  if (base::subtle::NoBarrier_Load(&is_enabled_) == 0)
    return;

  for (auto& id_fd : fds) {
    int fd = id_fd.second.get();
    unsigned int freq = 0;


    lseek(fd, 0L, SEEK_SET);
    char data[kNumBytesToReadForSampling];

    size_t bytes_read = read(fd, data, kNumBytesToReadForSampling);
    if (bytes_read > 0) {
      if (bytes_read < kNumBytesToReadForSampling)
        data[bytes_read] = '\0';
      int ret = sscanf(data, "%d", &freq);
      if (ret == 0 || ret == std::char_traits<char>::eof())
        freq = 0;
    }

    delegate_->RecordFrequency(id_fd.first, freq);
  }

  GetOrCreateTaskRunner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&CPUFreqMonitor::Sample, weak_ptr_factory_.GetWeakPtr(),
                     std::move(fds)),
      base::TimeDelta::FromMilliseconds(kDefaultCPUFreqSampleIntervalMs));
}

bool CPUFreqMonitor::IsEnabledForTesting() {
  return base::subtle::Acquire_Load(&is_enabled_) == 1;
}

const scoped_refptr<SingleThreadTaskRunner>&
CPUFreqMonitor::GetOrCreateTaskRunner() {
  if (!task_runner_)
    task_runner_ = delegate_->CreateTaskRunner();
  return task_runner_;
}

}  // namespace trace_event
}  // namespace base
