// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Defines some functions that intentionally do an invalid memory access in
// order to trigger an AddressSanitizer (ASan) error report.

#ifndef BASE_DEBUG_ASAN_INVALID_ACCESS_H_
#define BASE_DEBUG_ASAN_INVALID_ACCESS_H_

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/sanitizer_buildflags.h"
#include "build/build_config.h"

namespace base {
namespace debug {

#if defined(ADDRESS_SANITIZER) || BUILDFLAG(IS_HWASAN)

BASE_EXPORT NOINLINE void AsanHeapOverflow();

BASE_EXPORT NOINLINE void AsanHeapUnderflow();

BASE_EXPORT NOINLINE void AsanHeapUseAfterFree();

// Windows.
#if defined(OS_WIN)
// Corrupts a memory block and makes sure that the corruption gets detected when
// we try to free this block.
BASE_EXPORT NOINLINE void AsanCorruptHeapBlock();

// crash occur.
BASE_EXPORT NOINLINE void AsanCorruptHeap();

#endif  // OS_WIN
#endif  // ADDRESS_SANITIZER

}  // namespace debug
}  // namespace base

#endif  // BASE_DEBUG_ASAN_INVALID_ACCESS_H_
