// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/asan_invalid_access.h"

#include <stddef.h>

#include <memory>

#include "base/debug/alias.h"
#include "base/logging.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace base {
namespace debug {

namespace {

#if defined(OS_WIN) && defined(ADDRESS_SANITIZER)
// Corrupt a memory block and make sure that the corruption gets detected either
// when we free it or when another crash happens (if |induce_crash| is set to
// true).
NOINLINE void CorruptMemoryBlock(bool induce_crash) {


  static const int kArraySize = 5;
  LONG* array = new LONG[kArraySize];



  auto InterlockedIncrementFn =
      reinterpret_cast<LONG (*)(LONG volatile * addend)>(
          GetProcAddress(GetModuleHandle(L"kernel32"), "InterlockedIncrement"));
  CHECK(InterlockedIncrementFn);

  LONG volatile dummy = InterlockedIncrementFn(array - 1);
  base::debug::Alias(const_cast<LONG*>(&dummy));

  if (induce_crash)
    CHECK(false);
  delete[] array;
}
#endif  // OS_WIN && ADDRESS_SANITIZER

}  // namespace

#if defined(ADDRESS_SANITIZER) || BUILDFLAG(IS_HWASAN)
// NOTE(sebmarchand): We intentionally perform some invalid heap access here in
//     order to trigger an AddressSanitizer (ASan) error report.

// 4 so that off-by-one overflows are detected by HWASan, which has a shadow
// granularity of 16 bytes.
static const size_t kArraySize = 4;

void AsanHeapOverflow() {

  std::unique_ptr<volatile int[]> array(
      const_cast<volatile int*>(new int[kArraySize]));
  int dummy = array[kArraySize];
  base::debug::Alias(&dummy);
}

void AsanHeapUnderflow() {

  std::unique_ptr<volatile int[]> array(
      const_cast<volatile int*>(new int[kArraySize]));



  volatile int* underflow_address = &array[0] - 1;
  int dummy = *underflow_address;
  base::debug::Alias(&dummy);
}

void AsanHeapUseAfterFree() {

  std::unique_ptr<volatile int[]> array(
      const_cast<volatile int*>(new int[kArraySize]));
  volatile int* dangling = array.get();
  array.reset();
  int dummy = dangling[kArraySize / 2];
  base::debug::Alias(&dummy);
}

#if defined(OS_WIN)
void AsanCorruptHeapBlock() {
  CorruptMemoryBlock(false);
}

void AsanCorruptHeap() {
  CorruptMemoryBlock(true);
}
#endif  // OS_WIN
#endif  // ADDRESS_SANITIZER

}  // namespace debug
}  // namespace base
