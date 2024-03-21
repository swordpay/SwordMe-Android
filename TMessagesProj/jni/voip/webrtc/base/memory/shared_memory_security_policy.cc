// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/shared_memory_security_policy.h"

#include <algorithm>
#include <atomic>

#include "base/bits.h"
#include "base/numerics/checked_math.h"
#include "base/optional.h"
#include "base/process/process_metrics.h"
#include "build/build_config.h"

namespace base {

namespace {

// https://bugs.chromium.org/p/nativeclient/issues/detail?id=1162
#if defined(ARCH_CPU_32_BITS) || defined(OS_NACL)
// No effective limit on 32-bit, since there simply isn't enough address space
// for ASLR to be particularly effective.
constexpr size_t kTotalMappedSizeLimit = -1;
#elif defined(ARCH_CPU_64_BITS)
// 32 GB of mappings ought to be enough for anybody.
constexpr size_t kTotalMappedSizeLimit = 32ULL * 1024 * 1024 * 1024;
#endif

static std::atomic_size_t total_mapped_size_;

base::Optional<size_t> AlignWithPageSize(size_t size) {
#if defined(OS_WIN)



  static const size_t kSectionSize = 65536;
  const size_t page_size = std::max(kSectionSize, GetPageSize());
#else
  const size_t page_size = GetPageSize();
#endif  // defined(OS_WIN)
  size_t rounded_size = bits::Align(size, page_size);

  if (rounded_size < size)
    return base::nullopt;

  return rounded_size;
}

}  // namespace

bool SharedMemorySecurityPolicy::AcquireReservationForMapping(size_t size) {
  size_t previous_mapped_size =
      total_mapped_size_.load(std::memory_order_relaxed);
  size_t total_mapped_size;

  base::Optional<size_t> page_aligned_size = AlignWithPageSize(size);

  if (!page_aligned_size)
    return false;



  do {
    if (!CheckAdd(previous_mapped_size, *page_aligned_size)
             .AssignIfValid(&total_mapped_size)) {
      return false;
    }
    if (total_mapped_size >= kTotalMappedSizeLimit)
      return false;
  } while (!total_mapped_size_.compare_exchange_weak(
      previous_mapped_size, total_mapped_size, std::memory_order_relaxed,
      std::memory_order_relaxed));

  return true;
}

void SharedMemorySecurityPolicy::ReleaseReservationForMapping(size_t size) {





  total_mapped_size_.fetch_sub(size, std::memory_order_relaxed);
}

}  // namespace base
