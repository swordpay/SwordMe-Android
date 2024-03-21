// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_BUCKET_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_BUCKET_H_

#include <stddef.h>
#include <stdint.h>

#include "base/allocator/partition_allocator/partition_alloc_constants.h"
#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/logging.h"

namespace base {
namespace internal {

struct PartitionPage;
struct PartitionRootBase;

struct PartitionBucket {

  PartitionPage* active_pages_head;

  PartitionPage* empty_pages_head;
  PartitionPage* decommitted_pages_head;
  uint32_t slot_size;
  uint32_t num_system_pages_per_slot_span : 8;
  uint32_t num_full_pages : 24;

  void Init(uint32_t new_slot_size);







  BASE_EXPORT NOINLINE void* SlowPathAlloc(PartitionRootBase* root,
                                           int flags,
                                           size_t size,
                                           bool* is_already_zeroed);

  ALWAYS_INLINE bool is_direct_mapped() const {
    return !num_system_pages_per_slot_span;
  }
  ALWAYS_INLINE size_t get_bytes_per_span() const {


    return num_system_pages_per_slot_span * kSystemPageSize;
  }
  ALWAYS_INLINE uint16_t get_slots_per_span() const {


    return static_cast<uint16_t>(get_bytes_per_span() / slot_size);
  }

  static ALWAYS_INLINE size_t get_direct_map_size(size_t size) {



    DCHECK(size <= kGenericMaxDirectMapped);
    return (size + kSystemPageOffsetMask) & kSystemPageBaseMask;
  }

  static PartitionBucket* get_sentinel_bucket();










  bool SetNewActivePage();

 private:
  static void OutOfMemory(const PartitionRootBase* root);
  static void OutOfMemoryWithLotsOfUncommitedPages();

  static NOINLINE void OnFull();



  ALWAYS_INLINE uint16_t get_pages_per_slot_span();






  uint8_t get_system_pages_per_slot_span();



  ALWAYS_INLINE void* AllocNewSlotSpan(PartitionRootBase* root,
                                       int flags,
                                       uint16_t num_partition_pages);






  ALWAYS_INLINE void InitializeSlotSpan(PartitionPage* page);



  ALWAYS_INLINE char* AllocAndFillFreelist(PartitionPage* page);

  static PartitionBucket sentinel_bucket_;
};

}  // namespace internal
}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_BUCKET_H_
