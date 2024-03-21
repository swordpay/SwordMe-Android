// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_ORDERFILE_ORDERFILE_INSTRUMENTATION_H_
#define BASE_ANDROID_ORDERFILE_ORDERFILE_INSTRUMENTATION_H_

#include <cstdint>
#include <vector>

#include "base/android/orderfile/orderfile_buildflags.h"

namespace base {
namespace android {
namespace orderfile {
#if BUILDFLAG(DEVTOOLS_INSTRUMENTATION_DUMPING)
constexpr int kPhases = 2;
#else
constexpr int kPhases = 1;
#endif  // BUILDFLAG(DEVTOOLS_INSTRUMENTATION_DUMPING)

constexpr size_t kStartOfTextForTesting = 1000;
constexpr size_t kEndOfTextForTesting = kStartOfTextForTesting + 1000 * 1000;

bool Disable();

// the data to disk, and returns |true|. |pid| is the current process pid, and
// |start_ns_since_epoch| the process start timestamp.
bool SwitchToNextPhaseOrDump(int pid, uint64_t start_ns_since_epoch);

void StartDelayedDump();

// with the given tag. Will disable instrumentation. Instrumentation must be
// disabled before this is called.
void Dump(const std::string& tag);

void RecordAddressForTesting(size_t address);

// Only for testing.
void RecordAddressForTesting(size_t callee_address, size_t caller_address);

void ResetForTesting();

std::vector<size_t> GetOrderedOffsetsForTesting();
}  // namespace orderfile
}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_ORDERFILE_ORDERFILE_INSTRUMENTATION_H_
