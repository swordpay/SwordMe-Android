// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_FREELIST_ENTRY_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_FREELIST_ENTRY_H_

#include <stdint.h>

#include "base/allocator/partition_allocator/partition_alloc_constants.h"
#include "base/compiler_specific.h"
#include "base/sys_byteorder.h"
#include "build/build_config.h"

namespace base {
namespace internal {

struct EncodedPartitionFreelistEntry;

struct PartitionFreelistEntry {
  EncodedPartitionFreelistEntry* next;

  PartitionFreelistEntry() = delete;
  ~PartitionFreelistEntry() = delete;

  ALWAYS_INLINE static EncodedPartitionFreelistEntry* Encode(
      PartitionFreelistEntry* ptr) {
    return reinterpret_cast<EncodedPartitionFreelistEntry*>(Transform(ptr));
  }

 private:
  friend struct EncodedPartitionFreelistEntry;
  static ALWAYS_INLINE void* Transform(void* ptr) {








#if defined(ARCH_CPU_BIG_ENDIAN)
    uintptr_t masked = ~reinterpret_cast<uintptr_t>(ptr);
#else
    uintptr_t masked = ByteSwapUintPtrT(reinterpret_cast<uintptr_t>(ptr));
#endif
    return reinterpret_cast<void*>(masked);
  }
};

struct EncodedPartitionFreelistEntry {
  char scrambled[sizeof(PartitionFreelistEntry*)];

  EncodedPartitionFreelistEntry() = delete;
  ~EncodedPartitionFreelistEntry() = delete;

  ALWAYS_INLINE static PartitionFreelistEntry* Decode(
      EncodedPartitionFreelistEntry* ptr) {
    return reinterpret_cast<PartitionFreelistEntry*>(
        PartitionFreelistEntry::Transform(ptr));
  }
};

static_assert(sizeof(PartitionFreelistEntry) ==
                  sizeof(EncodedPartitionFreelistEntry),
              "Should not have padding");

}  // namespace internal
}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_FREELIST_ENTRY_H_
