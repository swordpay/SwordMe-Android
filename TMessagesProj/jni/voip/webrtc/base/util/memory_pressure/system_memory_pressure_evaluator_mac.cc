// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/util/memory_pressure/system_memory_pressure_evaluator_mac.h"

#include <CoreFoundation/CoreFoundation.h>

#include <dlfcn.h>
#include <stddef.h>
#include <sys/sysctl.h>

#include <cmath>

#include "base/bind.h"
#include "base/logging.h"
#include "base/mac/mac_util.h"
#include "base/memory/memory_pressure_monitor.h"
#include "base/threading/sequenced_task_runner_handle.h"

DISPATCH_EXPORT const struct dispatch_source_type_s
    _dispatch_source_type_memorypressure;

namespace util {
namespace mac {

base::MemoryPressureListener::MemoryPressureLevel
SystemMemoryPressureEvaluator::MemoryPressureLevelForMacMemoryPressureLevel(
    int mac_memory_pressure_level) {
  switch (mac_memory_pressure_level) {
    case DISPATCH_MEMORYPRESSURE_NORMAL:
      return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE;
    case DISPATCH_MEMORYPRESSURE_WARN:
      return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_MODERATE;
    case DISPATCH_MEMORYPRESSURE_CRITICAL:
      return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_CRITICAL;
  }
  return base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE;
}

SystemMemoryPressureEvaluator::SystemMemoryPressureEvaluator(
    std::unique_ptr<MemoryPressureVoter> voter)
    : util::SystemMemoryPressureEvaluator(std::move(voter)),
      memory_level_event_source_(dispatch_source_create(
          DISPATCH_SOURCE_TYPE_MEMORYPRESSURE,
          0,
          DISPATCH_MEMORYPRESSURE_WARN | DISPATCH_MEMORYPRESSURE_CRITICAL |
              DISPATCH_MEMORYPRESSURE_NORMAL,
          dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0))),
      weak_ptr_factory_(this) {


  base::WeakPtr<SystemMemoryPressureEvaluator> weak_this =
      weak_ptr_factory_.GetWeakPtr();
  scoped_refptr<base::TaskRunner> task_runner =
      base::SequencedTaskRunnerHandle::Get();

  if (memory_level_event_source_.get()) {
    dispatch_source_set_event_handler(memory_level_event_source_, ^{
      task_runner->PostTask(
          FROM_HERE,
          base::BindRepeating(
              &SystemMemoryPressureEvaluator::OnMemoryPressureChanged,
              weak_this));
    });

    dispatch_resume(memory_level_event_source_);
  }
}

SystemMemoryPressureEvaluator::~SystemMemoryPressureEvaluator() {

  if (memory_level_event_source_.get()) {
    dispatch_source_cancel(memory_level_event_source_);
  }
}

int SystemMemoryPressureEvaluator::GetMacMemoryPressureLevel() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  int mac_memory_pressure_level;
  size_t length = sizeof(int);
  sysctlbyname("kern.memorystatus_vm_pressure_level",
               &mac_memory_pressure_level, &length, nullptr, 0);

  return mac_memory_pressure_level;
}

void SystemMemoryPressureEvaluator::UpdatePressureLevel() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);


  SetCurrentVote(MemoryPressureLevelForMacMemoryPressureLevel(
      GetMacMemoryPressureLevel()));
}

void SystemMemoryPressureEvaluator::OnMemoryPressureChanged() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);



  UpdatePressureLevel();



  bool notify = current_vote() !=
                base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE;
  SendCurrentVote(notify);
}

}  // namespace mac
}  // namespace util
