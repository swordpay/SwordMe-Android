// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/memory.h"

#if defined(OS_WIN)
#include <windows.h>
#endif  // defined(OS_WIN)

#include "base/debug/alias.h"
#include "base/logging.h"
#include "base/partition_alloc_buildflags.h"
#if BUILDFLAG(USE_PARTITION_ALLOC)
#include "base/allocator/partition_allocator/page_allocator.h"
#endif
#include "build/build_config.h"

namespace base {

namespace internal {

NOINLINE void OnNoMemoryInternal(size_t size) {
#if defined(OS_WIN)




  ULONG_PTR exception_args[] = {size};
  ::RaiseException(base::win::kOomExceptionCode, EXCEPTION_NONCONTINUABLE,
                   base::size(exception_args), exception_args);

  _exit(win::kOomExceptionCode);
#else
  size_t tmp_size = size;
  base::debug::Alias(&tmp_size);
  LOG(FATAL) << "Out of memory. size=" << tmp_size;
#endif  // defined(OS_WIN)
}

}  // namespace internal

#if !defined(OS_WIN)

namespace {

// out-of-memory crash.
NOINLINE void OnNoMemory(size_t size) {
  internal::OnNoMemoryInternal(size);
}

}  // namespace

void TerminateBecauseOutOfMemory(size_t size) {
  OnNoMemory(size);
}

#endif  // !defined(OS_WIN)

#if !defined(OS_MACOSX)

bool UncheckedCalloc(size_t num_items, size_t size, void** result) {
  const size_t alloc_size = num_items * size;

  if (size && ((alloc_size / size) != num_items)) {
    *result = nullptr;
    return false;
  }

  if (!UncheckedMalloc(alloc_size, result))
    return false;

  memset(*result, 0, alloc_size);
  return true;
}

#endif  // defined(OS_MACOSX)

namespace internal {
bool ReleaseAddressSpaceReservation() {
#if BUILDFLAG(USE_PARTITION_ALLOC)
  return ReleaseReservation();
#else
  return false;
#endif
}
}  // namespace internal

}  // namespace base
