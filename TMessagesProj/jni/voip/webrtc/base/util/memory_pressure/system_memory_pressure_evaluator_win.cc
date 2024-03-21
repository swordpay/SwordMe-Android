// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/util/memory_pressure/system_memory_pressure_evaluator_win.h"

#include <windows.h>

#include "base/bind.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/time/time.h"
#include "base/util/memory_pressure/multi_source_memory_pressure_monitor.h"

namespace util {
namespace win {

namespace {

static const DWORDLONG kMBBytes = 1024 * 1024;

}  // namespace

// memory pressure monitor. The values were determined experimentally to ensure
// sufficient responsiveness of the memory pressure subsystem, and minimal
// overhead.
const int SystemMemoryPressureEvaluator::kModeratePressureCooldownMs = 10000;


// memory available for use by the memory manager (not reserved for hardware
// and drivers). This is a fuzzy version of the ~2GB discussed below.
const int SystemMemoryPressureEvaluator::kLargeMemoryThresholdMb = 1536;

// memory. Such systems have been observed to always maintain ~100MB of
// available memory, paging until that is the case. To try to avoid paging a
// threshold slightly above this is chosen. The moderate threshold is slightly
// less grounded in reality and chosen as 2.5x critical.
const int
    SystemMemoryPressureEvaluator::kSmallMemoryDefaultModerateThresholdMb = 500;
const int
    SystemMemoryPressureEvaluator::kSmallMemoryDefaultCriticalThresholdMb = 200;

// memory. Such systems have been observed to always maintain ~300MB of
// available memory, paging until that is the case.
const int
    SystemMemoryPressureEvaluator::kLargeMemoryDefaultModerateThresholdMb =
        1000;
const int
    SystemMemoryPressureEvaluator::kLargeMemoryDefaultCriticalThresholdMb = 400;

SystemMemoryPressureEvaluator::SystemMemoryPressureEvaluator(
    std::unique_ptr<MemoryPressureVoter> voter)
    : util::SystemMemoryPressureEvaluator(std::move(voter)),
      moderate_threshold_mb_(0),
      critical_threshold_mb_(0),
      moderate_pressure_repeat_count_(0) {
  InferThresholds();
  StartObserving();
}

SystemMemoryPressureEvaluator::SystemMemoryPressureEvaluator(
    int moderate_threshold_mb,
    int critical_threshold_mb,
    std::unique_ptr<MemoryPressureVoter> voter)
    : util::SystemMemoryPressureEvaluator(std::move(voter)),
      moderate_threshold_mb_(moderate_threshold_mb),
      critical_threshold_mb_(critical_threshold_mb),
      moderate_pressure_repeat_count_(0) {
  DCHECK_GE(moderate_threshold_mb_, critical_threshold_mb_);
  DCHECK_LE(0, critical_threshold_mb_);
  StartObserving();
}

SystemMemoryPressureEvaluator::~SystemMemoryPressureEvaluator() {
  StopObserving();
}

void SystemMemoryPressureEvaluator::CheckMemoryPressureSoon() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, BindOnce(&SystemMemoryPressureEvaluator::CheckMemoryPressure,
                          weak_ptr_factory_.GetWeakPtr()));
}

void SystemMemoryPressureEvaluator::InferThresholds() {


  bool high_memory = true;
  MEMORYSTATUSEX mem_status = {};
  if (GetSystemMemoryStatus(&mem_status)) {
    static const DWORDLONG kLargeMemoryThresholdBytes =
        static_cast<DWORDLONG>(kLargeMemoryThresholdMb) * kMBBytes;
    high_memory = mem_status.ullTotalPhys >= kLargeMemoryThresholdBytes;
  }

  if (high_memory) {
    moderate_threshold_mb_ = kLargeMemoryDefaultModerateThresholdMb;
    critical_threshold_mb_ = kLargeMemoryDefaultCriticalThresholdMb;
  } else {
    moderate_threshold_mb_ = kSmallMemoryDefaultModerateThresholdMb;
    critical_threshold_mb_ = kSmallMemoryDefaultCriticalThresholdMb;
  }
}

void SystemMemoryPressureEvaluator::StartObserving() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  timer_.Start(
      FROM_HERE, base::MemoryPressureMonitor::kUMAMemoryPressureLevelPeriod,
      BindRepeating(&SystemMemoryPressureEvaluator::CheckMemoryPressure,
                    weak_ptr_factory_.GetWeakPtr()));
}

void SystemMemoryPressureEvaluator::StopObserving() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  timer_.Stop();
  weak_ptr_factory_.InvalidateWeakPtrs();
}

void SystemMemoryPressureEvaluator::CheckMemoryPressure() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  MemoryPressureLevel old_vote = current_vote();
  SetCurrentVote(CalculateCurrentPressureLevel());


  bool notify = false;
  switch (current_vote()) {
    case base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE:
      break;

    case base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_MODERATE:
      if (old_vote != current_vote()) {

        moderate_pressure_repeat_count_ = 0;
        notify = true;
      } else {


        const int kModeratePressureCooldownCycles =
            kModeratePressureCooldownMs /
            base::MemoryPressureMonitor::kUMAMemoryPressureLevelPeriod
                .InMilliseconds();
        if (++moderate_pressure_repeat_count_ ==
            kModeratePressureCooldownCycles) {
          moderate_pressure_repeat_count_ = 0;
          notify = true;
        }
      }
      break;

    case base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_CRITICAL:

      notify = true;
      break;
  }

  SendCurrentVote(notify);
}

base::MemoryPressureListener::MemoryPressureLevel
SystemMemoryPressureEvaluator::CalculateCurrentPressureLevel() {
  MEMORYSTATUSEX mem_status = {};
  if (!GetSystemMemoryStatus(&mem_status))
    return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE;

  int phys_free = static_cast<int>(mem_status.ullAvailPhys / kMBBytes);








  if (phys_free <= critical_threshold_mb_)
    return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_CRITICAL;

  if (phys_free <= moderate_threshold_mb_)
    return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_MODERATE;

  return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE;
}

bool SystemMemoryPressureEvaluator::GetSystemMemoryStatus(
    MEMORYSTATUSEX* mem_status) {
  DCHECK(mem_status != nullptr);
  mem_status->dwLength = sizeof(*mem_status);
  if (!::GlobalMemoryStatusEx(mem_status))
    return false;
  return true;
}

}  // namespace win
}  // namespace util
