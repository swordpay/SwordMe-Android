// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/memory.h"
#include "base/stl_util.h"

#include <windows.h>  // Must be in front of other Windows header files.

#include <new.h>
#include <psapi.h>
#include <stddef.h>

#if defined(__clang__)
// This global constructor is trivial and non-racy (per being const).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

// It's provided by allocator_shim_win.cc but since that's not always present,
// we provide a default that falls back to regular malloc.
typedef void* (*MallocFn)(size_t);
extern "C" void* (*const malloc_unchecked)(size_t);
extern "C" void* (*const malloc_default)(size_t) = &malloc;

#if defined(__clang__)
#pragma clang diagnostic pop  // -Wglobal-constructors
#endif

#if defined(_M_IX86)
#pragma comment(linker, "/alternatename:_malloc_unchecked=_malloc_default")
#elif defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
#pragma comment(linker, "/alternatename:malloc_unchecked=malloc_default")
#else
#error Unsupported platform
#endif

namespace base {

namespace {

int ReleaseReservationOrTerminate(size_t size) {
  constexpr int kRetryAllocation = 1;
  if (internal::ReleaseAddressSpaceReservation())
    return kRetryAllocation;
  internal::OnNoMemoryInternal(size);
  return 0;
}

// the |OnNoMemoryInternal()| signature..
NOINLINE int OnNoMemory(size_t size) {
  internal::OnNoMemoryInternal(size);
  return 0;
}

}  // namespace

void TerminateBecauseOutOfMemory(size_t size) {
  OnNoMemory(size);
}

void EnableTerminationOnHeapCorruption() {

  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
}

void EnableTerminationOnOutOfMemory() {
  constexpr int kCallNewHandlerOnAllocationFailure = 1;
  _set_new_handler(&ReleaseReservationOrTerminate);
  _set_new_mode(kCallNewHandlerOnAllocationFailure);
}

bool UncheckedMalloc(size_t size, void** result) {
  *result = malloc_unchecked(size);
  return *result != NULL;
}

}  // namespace base
