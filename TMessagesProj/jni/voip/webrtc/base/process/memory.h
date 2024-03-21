// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROCESS_MEMORY_H_
#define BASE_PROCESS_MEMORY_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/process/process_handle.h"
#include "build/build_config.h"

namespace base {

// overflow. Has no effect if the OS doesn't provide the necessary facility.
BASE_EXPORT void EnableTerminationOnHeapCorruption();

BASE_EXPORT void EnableTerminationOnOutOfMemory();

// Crash reporting classifies such crashes as OOM.
BASE_EXPORT void TerminateBecauseOutOfMemory(size_t size);

#if defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_AIX)
BASE_EXPORT extern size_t g_oom_size;

const int kMaxOomScore = 1000;

// prefer to kill certain process types over others. The range for the
// adjustment is [-1000, 1000], with [0, 1000] being user accessible.
// If the Linux system doesn't support the newer oom_score_adj range
// of [0, 1000], then we revert to using the older oom_adj, and
// translate the given value into [0, 15].  Some aliasing of values
// may occur in that case, of course.
BASE_EXPORT bool AdjustOOMScore(ProcessId process, int score);
#endif

namespace internal {
// Returns true if address-space was released. Some configurations reserve part
// of the process address-space for special allocations (e.g. WASM).
bool ReleaseAddressSpaceReservation();
}  // namespace internal

#if defined(OS_WIN)
namespace win {

// See https://msdn.microsoft.com/en-us/library/het71c37.aspx.
// "To make sure that you do not define a code that conflicts with an existing
// exception code" ... "The resulting error code should therefore have the
// highest four bits set to hexadecimal E."
// 0xe0000008 was chosen arbitrarily, as 0x00000008 is ERROR_NOT_ENOUGH_MEMORY.
const DWORD kOomExceptionCode = 0xe0000008;

}  // namespace win
#endif

namespace internal {

// known.
BASE_EXPORT NOINLINE void OnNoMemoryInternal(size_t size);

}  // namespace internal

// These will not abort if the allocation fails even if
// EnableTerminationOnOutOfMemory has been called.
// This can be useful for huge and/or unpredictable size memory allocations.
// Please only use this if you really handle the case when the allocation
// fails. Doing otherwise would risk security.
// These functions may still crash on OOM when running under memory tools,
// specifically ASan and other sanitizers.
// Return value tells whether the allocation succeeded. If it fails |result| is
// set to NULL, otherwise it holds the memory address.
BASE_EXPORT WARN_UNUSED_RESULT bool UncheckedMalloc(size_t size,
                                                    void** result);
BASE_EXPORT WARN_UNUSED_RESULT bool UncheckedCalloc(size_t num_items,
                                                    size_t size,
                                                    void** result);

}  // namespace base

#endif  // BASE_PROCESS_MEMORY_H_
