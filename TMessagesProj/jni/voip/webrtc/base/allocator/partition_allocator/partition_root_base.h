// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ROOT_BASE_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ROOT_BASE_H_

#include "base/allocator/partition_allocator/page_allocator.h"
#include "base/allocator/partition_allocator/partition_alloc_constants.h"
#include "base/allocator/partition_allocator/partition_bucket.h"
#include "base/allocator/partition_allocator/partition_direct_map_extent.h"
#include "base/allocator/partition_allocator/partition_page.h"
#include "base/logging.h"
#include "build/build_config.h"

namespace base {

typedef void (*OomFunction)(size_t);

namespace internal {

struct PartitionPage;
struct PartitionRootBase;

// next extent (if there is one) to the very start of a superpage's metadata
// area.
struct PartitionSuperPageExtentEntry {
  PartitionRootBase* root;
  char* super_page_base;
  char* super_pages_end;
  PartitionSuperPageExtentEntry* next;
};
static_assert(
    sizeof(PartitionSuperPageExtentEntry) <= kPageMetadataSize,
    "PartitionSuperPageExtentEntry must be able to fit in a metadata slot");

struct BASE_EXPORT PartitionRootBase {
  PartitionRootBase();
  virtual ~PartitionRootBase();
  size_t total_size_of_committed_pages = 0;
  size_t total_size_of_super_pages = 0;
  size_t total_size_of_direct_mapped_pages = 0;



  unsigned num_buckets = 0;
  unsigned max_allocation = 0;
  bool initialized = false;
  char* next_super_page = nullptr;
  char* next_partition_page = nullptr;
  char* next_partition_page_end = nullptr;
  PartitionSuperPageExtentEntry* current_extent = nullptr;
  PartitionSuperPageExtentEntry* first_extent = nullptr;
  PartitionDirectMapExtent* direct_map_list = nullptr;
  PartitionPage* global_empty_page_ring[kMaxFreeableSpans] = {};
  int16_t global_empty_page_ring_index = 0;
  uintptr_t inverted_self = 0;











  ALWAYS_INLINE void* AllocFromBucket(PartitionBucket* bucket,
                                      int flags,
                                      size_t size);

  ALWAYS_INLINE static bool IsValidPage(PartitionPage* page);
  ALWAYS_INLINE static PartitionRootBase* FromPage(PartitionPage* page);

  static OomFunction g_oom_handling_function;
  NOINLINE void OutOfMemory(size_t size);

  ALWAYS_INLINE void IncreaseCommittedPages(size_t len);
  ALWAYS_INLINE void DecreaseCommittedPages(size_t len);
  ALWAYS_INLINE void DecommitSystemPages(void* address, size_t length);
  ALWAYS_INLINE void RecommitSystemPages(void* address, size_t length);


  virtual void PurgeMemory(int flags) = 0;
  void DecommitEmptyPages();
};

ALWAYS_INLINE void* PartitionRootBase::AllocFromBucket(PartitionBucket* bucket,
                                                       int flags,
                                                       size_t size) {
  bool zero_fill = flags & PartitionAllocZeroFill;
  bool is_already_zeroed = false;

  PartitionPage* page = bucket->active_pages_head;

  DCHECK(page->num_allocated_slots >= 0);
  void* ret = page->freelist_head;
  if (LIKELY(ret != 0)) {


    DCHECK(PartitionRootBase::IsValidPage(page));


    DCHECK(page->get_raw_size() == 0);
    internal::PartitionFreelistEntry* new_head =
        internal::EncodedPartitionFreelistEntry::Decode(
            page->freelist_head->next);
    page->freelist_head = new_head;
    page->num_allocated_slots++;
  } else {
    ret = bucket->SlowPathAlloc(this, flags, size, &is_already_zeroed);

    DCHECK(!ret ||
           PartitionRootBase::IsValidPage(PartitionPage::FromPointer(ret)));
  }

#if DCHECK_IS_ON()
  if (!ret) {
    return nullptr;
  }

  page = PartitionPage::FromPointer(ret);


  size_t new_slot_size = page->bucket->slot_size;
  size_t raw_size = page->get_raw_size();
  if (raw_size) {
    DCHECK(raw_size == size);
    new_slot_size = raw_size;
  }
  size_t no_cookie_size = PartitionCookieSizeAdjustSubtract(new_slot_size);
  char* char_ret = static_cast<char*>(ret);

  ret = char_ret + kCookieSize;

  PartitionCookieWriteValue(char_ret);
  if (!zero_fill) {
    memset(ret, kUninitializedByte, no_cookie_size);
  } else if (!is_already_zeroed) {
    memset(ret, 0, no_cookie_size);
  }
  PartitionCookieWriteValue(char_ret + kCookieSize + no_cookie_size);
#else
  if (ret && zero_fill && !is_already_zeroed) {
    memset(ret, 0, size);
  }
#endif

  return ret;
}

ALWAYS_INLINE bool PartitionRootBase::IsValidPage(PartitionPage* page) {
  PartitionRootBase* root = PartitionRootBase::FromPage(page);
  return root->inverted_self == ~reinterpret_cast<uintptr_t>(root);
}

ALWAYS_INLINE PartitionRootBase* PartitionRootBase::FromPage(
    PartitionPage* page) {
  PartitionSuperPageExtentEntry* extent_entry =
      reinterpret_cast<PartitionSuperPageExtentEntry*>(
          reinterpret_cast<uintptr_t>(page) & kSystemPageBaseMask);
  return extent_entry->root;
}

ALWAYS_INLINE void PartitionRootBase::IncreaseCommittedPages(size_t len) {
  total_size_of_committed_pages += len;
  DCHECK(total_size_of_committed_pages <=
         total_size_of_super_pages + total_size_of_direct_mapped_pages);
}

ALWAYS_INLINE void PartitionRootBase::DecreaseCommittedPages(size_t len) {
  total_size_of_committed_pages -= len;
  DCHECK(total_size_of_committed_pages <=
         total_size_of_super_pages + total_size_of_direct_mapped_pages);
}

ALWAYS_INLINE void PartitionRootBase::DecommitSystemPages(void* address,
                                                          size_t length) {
  ::base::DecommitSystemPages(address, length);
  DecreaseCommittedPages(length);
}

ALWAYS_INLINE void PartitionRootBase::RecommitSystemPages(void* address,
                                                          size_t length) {
  CHECK(::base::RecommitSystemPages(address, length, PageReadWrite));
  IncreaseCommittedPages(length);
}

}  // namespace internal
}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ROOT_BASE_H_
