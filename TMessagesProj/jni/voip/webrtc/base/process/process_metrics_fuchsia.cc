// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process_metrics.h"

#include <lib/fdio/limits.h>

namespace base {

size_t GetMaxFds() {
  return FDIO_MAX_FD;
}

size_t GetHandleLimit() {


  return 256 * 1024u;
}

size_t GetSystemCommitCharge() {

  return 0;
}

std::unique_ptr<ProcessMetrics> ProcessMetrics::CreateProcessMetrics(
    ProcessHandle process) {

  return nullptr;
}

TimeDelta ProcessMetrics::GetCumulativeCPUUsage() {

  return TimeDelta();
}

bool GetSystemMemoryInfo(SystemMemoryInfoKB* meminfo) {

  return false;
}

}  // namespace base
