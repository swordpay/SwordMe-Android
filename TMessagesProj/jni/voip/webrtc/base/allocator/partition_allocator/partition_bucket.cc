// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/allocator/partition_allocator/partition_bucket.h"

#include "base/allocator/partition_allocator/oom.h"
#include "base/allocator/partition_allocator/page_allocator.h"
#include "base/allocator/partition_allocator/partition_alloc_constants.h"
#include "base/allocator/partition_allocator/partition_direct_map_extent.h"
#include "base/allocator/partition_allocator/partition_oom.h"
#include "base/allocator/partition_allocator/partition_page.h"
#include "base/allocator/partition_allocator/partition_root_base.h"
#include "base/logging.h"
#include "build/build_config.h"

namespace base {
namespace internal {

namespace {

ALWAYS_INLINE PartitionPage* PartitionDirectMap(PartitionRootBase* root,
                                                int flags,
                                                size_t raw_size) {
  size_t size = PartitionBucket::get_direct_map_size(raw_size);







  size_t map_size = size + kPartitionPageSize;
#if !defined(ARCH_CPU_64_BITS)
  map_size += kSystemPageSize;
#endif

  map_size += kPageAllocationGranularityOffsetMask;
  map_size &= kPageAllocationGranularityBaseMask;

  char* ptr = reinterpret_cast<char*>(AllocPages(nullptr, map_size,
                                                 kSuperPageSize, PageReadWrite,
                                                 PageTag::kPartitionAlloc));
  if (UNLIKELY(!ptr))
    return nullptr;

  size_t committed_page_size = size + kSystemPageSize;
  root->total_size_of_direct_mapped_pages += committed_page_size;
  root->IncreaseCommittedPages(committed_page_size);

  char* slot = ptr + kPartitionPageSize;
  SetSystemPagesAccess(ptr + (kSystemPageSize * 2),
                       kPartitionPageSize - (kSystemPageSize * 2),
                       PageInaccessible);
#if !defined(ARCH_CPU_64_BITS)
  SetSystemPagesAccess(ptr, kSystemPageSize, PageInaccessible);
  SetSystemPagesAccess(slot + size, kSystemPageSize, PageInaccessible);
#endif

  PartitionSuperPageExtentEntry* extent =
      reinterpret_cast<PartitionSuperPageExtentEntry*>(
          PartitionSuperPageToMetadataArea(ptr));
  extent->root = root;


  DCHECK(!extent->super_page_base);
  DCHECK(!extent->super_pages_end);
  DCHECK(!extent->next);
  PartitionPage* page = PartitionPage::FromPointerNoAlignmentCheck(slot);
  PartitionBucket* bucket = reinterpret_cast<PartitionBucket*>(
      reinterpret_cast<char*>(page) + (kPageMetadataSize * 2));
  DCHECK(!page->next_page);
  DCHECK(!page->num_allocated_slots);
  DCHECK(!page->num_unprovisioned_slots);
  DCHECK(!page->page_offset);
  DCHECK(!page->empty_cache_index);
  page->bucket = bucket;
  page->freelist_head = reinterpret_cast<PartitionFreelistEntry*>(slot);
  PartitionFreelistEntry* next_entry =
      reinterpret_cast<PartitionFreelistEntry*>(slot);
  next_entry->next = PartitionFreelistEntry::Encode(nullptr);

  DCHECK(!bucket->active_pages_head);
  DCHECK(!bucket->empty_pages_head);
  DCHECK(!bucket->decommitted_pages_head);
  DCHECK(!bucket->num_system_pages_per_slot_span);
  DCHECK(!bucket->num_full_pages);
  bucket->slot_size = size;

  PartitionDirectMapExtent* map_extent =
      PartitionDirectMapExtent::FromPage(page);
  map_extent->map_size = map_size - kPartitionPageSize - kSystemPageSize;
  map_extent->bucket = bucket;

  map_extent->next_extent = root->direct_map_list;
  if (map_extent->next_extent)
    map_extent->next_extent->prev_extent = map_extent;
  map_extent->prev_extent = nullptr;
  root->direct_map_list = map_extent;

  return page;
}

}  // namespace

PartitionBucket PartitionBucket::sentinel_bucket_;

PartitionBucket* PartitionBucket::get_sentinel_bucket() {
  return &sentinel_bucket_;
}

// get_pages_per_slot_span() which rounds the value from this up to a
// multiple of kNumSystemPagesPerPartitionPage (aka 4) anyways.
// http://crbug.com/776537
//
// TODO(ajwong): The waste calculation seems wrong. The PTE usage should cover
// both used and unsed pages.
// http://crbug.com/776537
uint8_t PartitionBucket::get_system_pages_per_slot_span() {










  double best_waste_ratio = 1.0f;
  uint16_t best_pages = 0;
  if (slot_size > kMaxSystemPagesPerSlotSpan * kSystemPageSize) {


    DCHECK(!(slot_size % kSystemPageSize));
    best_pages = static_cast<uint16_t>(slot_size / kSystemPageSize);



    CHECK(best_pages < (1 << 8));
    return static_cast<uint8_t>(best_pages);
  }
  DCHECK(slot_size <= kMaxSystemPagesPerSlotSpan * kSystemPageSize);
  for (uint16_t i = kNumSystemPagesPerPartitionPage - 1;
       i <= kMaxSystemPagesPerSlotSpan; ++i) {
    size_t page_size = kSystemPageSize * i;
    size_t num_slots = page_size / slot_size;
    size_t waste = page_size - (num_slots * slot_size);







    size_t num_remainder_pages = i & (kNumSystemPagesPerPartitionPage - 1);
    size_t num_unfaulted_pages =
        num_remainder_pages
            ? (kNumSystemPagesPerPartitionPage - num_remainder_pages)
            : 0;
    waste += sizeof(void*) * num_unfaulted_pages;
    double waste_ratio =
        static_cast<double>(waste) / static_cast<double>(page_size);
    if (waste_ratio < best_waste_ratio) {
      best_waste_ratio = waste_ratio;
      best_pages = i;
    }
  }
  DCHECK(best_pages > 0);
  CHECK(best_pages <= kMaxSystemPagesPerSlotSpan);
  return static_cast<uint8_t>(best_pages);
}

void PartitionBucket::Init(uint32_t new_slot_size) {
  slot_size = new_slot_size;
  active_pages_head = PartitionPage::get_sentinel_page();
  empty_pages_head = nullptr;
  decommitted_pages_head = nullptr;
  num_full_pages = 0;
  num_system_pages_per_slot_span = get_system_pages_per_slot_span();
}

NOINLINE void PartitionBucket::OnFull() {
  OOM_CRASH(0);
}

ALWAYS_INLINE void* PartitionBucket::AllocNewSlotSpan(
    PartitionRootBase* root,
    int flags,
    uint16_t num_partition_pages) {
  DCHECK(!(reinterpret_cast<uintptr_t>(root->next_partition_page) %
           kPartitionPageSize));
  DCHECK(!(reinterpret_cast<uintptr_t>(root->next_partition_page_end) %
           kPartitionPageSize));
  DCHECK(num_partition_pages <= kNumPartitionPagesPerSuperPage);
  size_t total_size = kPartitionPageSize * num_partition_pages;
  size_t num_partition_pages_left =
      (root->next_partition_page_end - root->next_partition_page) >>
      kPartitionPageShift;
  if (LIKELY(num_partition_pages_left >= num_partition_pages)) {


    char* ret = root->next_partition_page;


    SetSystemPagesAccess(ret, total_size, PageReadWrite);

    root->next_partition_page += total_size;
    root->IncreaseCommittedPages(total_size);
    return ret;
  }




  char* requested_address = root->next_super_page;
  char* super_page = reinterpret_cast<char*>(
      AllocPages(requested_address, kSuperPageSize, kSuperPageSize,
                 PageReadWrite, PageTag::kPartitionAlloc));
  if (UNLIKELY(!super_page))
    return nullptr;

  root->total_size_of_super_pages += kSuperPageSize;
  root->IncreaseCommittedPages(total_size);





  root->next_super_page = super_page + kSuperPageSize;
  char* ret = super_page + kPartitionPageSize;
  root->next_partition_page = ret + total_size;
  root->next_partition_page_end = root->next_super_page - kPartitionPageSize;




  SetSystemPagesAccess(super_page, kSystemPageSize, PageInaccessible);
  SetSystemPagesAccess(super_page + (kSystemPageSize * 2),
                       kPartitionPageSize - (kSystemPageSize * 2),
                       PageInaccessible);









  SetSystemPagesAccess(super_page + kPartitionPageSize + total_size,
                       (kSuperPageSize - kPartitionPageSize - total_size),
                       PageInaccessible);






  if (requested_address && requested_address != super_page)
    root->next_super_page = nullptr;


  PartitionSuperPageExtentEntry* latest_extent =
      reinterpret_cast<PartitionSuperPageExtentEntry*>(
          PartitionSuperPageToMetadataArea(super_page));


  latest_extent->root = root;



  latest_extent->super_page_base = nullptr;
  latest_extent->super_pages_end = nullptr;
  latest_extent->next = nullptr;

  PartitionSuperPageExtentEntry* current_extent = root->current_extent;
  bool is_new_extent = (super_page != requested_address);
  if (UNLIKELY(is_new_extent)) {
    if (UNLIKELY(!current_extent)) {
      DCHECK(!root->first_extent);
      root->first_extent = latest_extent;
    } else {
      DCHECK(current_extent->super_page_base);
      current_extent->next = latest_extent;
    }
    root->current_extent = latest_extent;
    latest_extent->super_page_base = super_page;
    latest_extent->super_pages_end = super_page + kSuperPageSize;
  } else {


    DCHECK(current_extent->super_pages_end);
    current_extent->super_pages_end += kSuperPageSize;
    DCHECK(ret >= current_extent->super_page_base &&
           ret < current_extent->super_pages_end);
  }
  return ret;
}

ALWAYS_INLINE uint16_t PartitionBucket::get_pages_per_slot_span() {

  return (num_system_pages_per_slot_span +
          (kNumSystemPagesPerPartitionPage - 1)) /
         kNumSystemPagesPerPartitionPage;
}

ALWAYS_INLINE void PartitionBucket::InitializeSlotSpan(PartitionPage* page) {

  page->bucket = this;
  page->empty_cache_index = -1;

  page->Reset();



  if (page->num_unprovisioned_slots == 1)
    return;

  uint16_t num_partition_pages = get_pages_per_slot_span();
  char* page_char_ptr = reinterpret_cast<char*>(page);
  for (uint16_t i = 1; i < num_partition_pages; ++i) {
    page_char_ptr += kPageMetadataSize;
    PartitionPage* secondary_page =
        reinterpret_cast<PartitionPage*>(page_char_ptr);
    secondary_page->page_offset = i;
  }
}

ALWAYS_INLINE char* PartitionBucket::AllocAndFillFreelist(PartitionPage* page) {
  DCHECK(page != PartitionPage::get_sentinel_page());
  uint16_t num_slots = page->num_unprovisioned_slots;
  DCHECK(num_slots);



  DCHECK(num_slots + page->num_allocated_slots == get_slots_per_span());

  DCHECK(!page->freelist_head);
  DCHECK(page->num_allocated_slots >= 0);

  size_t size = slot_size;
  char* base = reinterpret_cast<char*>(PartitionPage::ToPointer(page));
  char* return_object = base + (size * page->num_allocated_slots);
  char* first_freelist_pointer = return_object + size;
  char* first_freelist_pointer_extent =
      first_freelist_pointer + sizeof(PartitionFreelistEntry*);



  char* sub_page_limit = reinterpret_cast<char*>(
      RoundUpToSystemPage(reinterpret_cast<size_t>(first_freelist_pointer)));
  char* slots_limit = return_object + (size * num_slots);
  char* freelist_limit = sub_page_limit;
  if (UNLIKELY(slots_limit < freelist_limit))
    freelist_limit = slots_limit;

  uint16_t num_new_freelist_entries = 0;
  if (LIKELY(first_freelist_pointer_extent <= freelist_limit)) {




    num_new_freelist_entries = 1;

    num_new_freelist_entries += static_cast<uint16_t>(
        (freelist_limit - first_freelist_pointer_extent) / size);
  }



  DCHECK(num_new_freelist_entries + 1 <= num_slots);
  num_slots -= (num_new_freelist_entries + 1);
  page->num_unprovisioned_slots = num_slots;
  page->num_allocated_slots++;

  if (LIKELY(num_new_freelist_entries)) {
    char* freelist_pointer = first_freelist_pointer;
    PartitionFreelistEntry* entry =
        reinterpret_cast<PartitionFreelistEntry*>(freelist_pointer);
    page->freelist_head = entry;
    while (--num_new_freelist_entries) {
      freelist_pointer += size;
      PartitionFreelistEntry* next_entry =
          reinterpret_cast<PartitionFreelistEntry*>(freelist_pointer);
      entry->next = PartitionFreelistEntry::Encode(next_entry);
      entry = next_entry;
    }
    entry->next = PartitionFreelistEntry::Encode(nullptr);
  } else {
    page->freelist_head = nullptr;
  }
  return return_object;
}

bool PartitionBucket::SetNewActivePage() {
  PartitionPage* page = active_pages_head;
  if (page == PartitionPage::get_sentinel_page())
    return false;

  PartitionPage* next_page;

  for (; page; page = next_page) {
    next_page = page->next_page;
    DCHECK(page->bucket == this);
    DCHECK(page != empty_pages_head);
    DCHECK(page != decommitted_pages_head);

    if (LIKELY(page->is_active())) {


      active_pages_head = page;
      return true;
    }

    if (LIKELY(page->is_empty())) {
      page->next_page = empty_pages_head;
      empty_pages_head = page;
    } else if (LIKELY(page->is_decommitted())) {
      page->next_page = decommitted_pages_head;
      decommitted_pages_head = page;
    } else {
      DCHECK(page->is_full());



      page->num_allocated_slots = -page->num_allocated_slots;
      ++num_full_pages;


      if (UNLIKELY(!num_full_pages))
        OnFull();

      page->next_page = nullptr;
    }
  }

  active_pages_head = PartitionPage::get_sentinel_page();
  return false;
}

void* PartitionBucket::SlowPathAlloc(PartitionRootBase* root,
                                     int flags,
                                     size_t size,
                                     bool* is_already_zeroed) {

  DCHECK(!active_pages_head->freelist_head);

  PartitionPage* new_page = nullptr;
  *is_already_zeroed = false;









  bool return_null = flags & PartitionAllocReturnNull;
  if (UNLIKELY(is_direct_mapped())) {
    DCHECK(size > kGenericMaxBucketed);
    DCHECK(this == get_sentinel_bucket());
    DCHECK(active_pages_head == PartitionPage::get_sentinel_page());
    if (size > kGenericMaxDirectMapped) {
      if (return_null)
        return nullptr;
      PartitionExcessiveAllocationSize(size);
    }
    new_page = PartitionDirectMap(root, flags, size);
    *is_already_zeroed = true;
  } else if (LIKELY(SetNewActivePage())) {

    new_page = active_pages_head;
    DCHECK(new_page->is_active());
  } else if (LIKELY(empty_pages_head != nullptr) ||
             LIKELY(decommitted_pages_head != nullptr)) {



    while (LIKELY((new_page = empty_pages_head) != nullptr)) {
      DCHECK(new_page->bucket == this);
      DCHECK(new_page->is_empty() || new_page->is_decommitted());
      empty_pages_head = new_page->next_page;

      if (new_page->freelist_head) {
        new_page->next_page = nullptr;
        break;
      }
      DCHECK(new_page->is_decommitted());
      new_page->next_page = decommitted_pages_head;
      decommitted_pages_head = new_page;
    }
    if (UNLIKELY(!new_page) && LIKELY(decommitted_pages_head != nullptr)) {
      new_page = decommitted_pages_head;
      DCHECK(new_page->bucket == this);
      DCHECK(new_page->is_decommitted());
      decommitted_pages_head = new_page->next_page;
      void* addr = PartitionPage::ToPointer(new_page);
      root->RecommitSystemPages(addr, new_page->bucket->get_bytes_per_span());
      new_page->Reset();



    }
    DCHECK(new_page);
  } else {

    uint16_t num_partition_pages = get_pages_per_slot_span();
    void* raw_pages = AllocNewSlotSpan(root, flags, num_partition_pages);
    if (LIKELY(raw_pages != nullptr)) {
      new_page = PartitionPage::FromPointerNoAlignmentCheck(raw_pages);
      InitializeSlotSpan(new_page);



    }
  }

  if (UNLIKELY(!new_page)) {
    DCHECK(active_pages_head == PartitionPage::get_sentinel_page());
    if (return_null)
      return nullptr;
    root->OutOfMemory(size);
  }



  PartitionBucket* bucket = new_page->bucket;
  DCHECK(bucket != get_sentinel_bucket());
  bucket->active_pages_head = new_page;
  new_page->set_raw_size(size);


  if (LIKELY(new_page->freelist_head != nullptr)) {
    PartitionFreelistEntry* entry = new_page->freelist_head;
    PartitionFreelistEntry* new_head =
        EncodedPartitionFreelistEntry::Decode(entry->next);
    new_page->freelist_head = new_head;
    new_page->num_allocated_slots++;
    return entry;
  }

  DCHECK(new_page->num_unprovisioned_slots);
  return AllocAndFillFreelist(new_page);
}

}  // namespace internal
}  // namespace base
