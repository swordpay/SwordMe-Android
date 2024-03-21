// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/allocator/partition_allocator/partition_page.h"

#include "base/allocator/partition_allocator/partition_direct_map_extent.h"
#include "base/allocator/partition_allocator/partition_root_base.h"
#include "base/logging.h"

namespace base {
namespace internal {

namespace {

ALWAYS_INLINE void PartitionDirectUnmap(PartitionPage* page) {
  PartitionRootBase* root = PartitionRootBase::FromPage(page);
  const PartitionDirectMapExtent* extent =
      PartitionDirectMapExtent::FromPage(page);
  size_t unmap_size = extent->map_size;

  if (extent->prev_extent) {
    DCHECK(extent->prev_extent->next_extent == extent);
    extent->prev_extent->next_extent = extent->next_extent;
  } else {
    root->direct_map_list = extent->next_extent;
  }
  if (extent->next_extent) {
    DCHECK(extent->next_extent->prev_extent == extent);
    extent->next_extent->prev_extent = extent->prev_extent;
  }


  unmap_size += kPartitionPageSize + kSystemPageSize;

  size_t uncommitted_page_size = page->bucket->slot_size + kSystemPageSize;
  root->DecreaseCommittedPages(uncommitted_page_size);
  DCHECK(root->total_size_of_direct_mapped_pages >= uncommitted_page_size);
  root->total_size_of_direct_mapped_pages -= uncommitted_page_size;

  DCHECK(!(unmap_size & kPageAllocationGranularityOffsetMask));

  char* ptr = reinterpret_cast<char*>(PartitionPage::ToPointer(page));


  ptr -= kPartitionPageSize;

  FreePages(ptr, unmap_size);
}

ALWAYS_INLINE void PartitionRegisterEmptyPage(PartitionPage* page) {
  DCHECK(page->is_empty());
  PartitionRootBase* root = PartitionRootBase::FromPage(page);

  if (page->empty_cache_index != -1) {
    DCHECK(page->empty_cache_index >= 0);
    DCHECK(static_cast<unsigned>(page->empty_cache_index) < kMaxFreeableSpans);
    DCHECK(root->global_empty_page_ring[page->empty_cache_index] == page);
    root->global_empty_page_ring[page->empty_cache_index] = nullptr;
  }

  int16_t current_index = root->global_empty_page_ring_index;
  PartitionPage* page_to_decommit = root->global_empty_page_ring[current_index];


  if (page_to_decommit)
    page_to_decommit->DecommitIfPossible(root);




  root->global_empty_page_ring[current_index] = page;
  page->empty_cache_index = current_index;
  ++current_index;
  if (current_index == kMaxFreeableSpans)
    current_index = 0;
  root->global_empty_page_ring_index = current_index;
}

}  // namespace

PartitionPage PartitionPage::sentinel_page_;

PartitionPage* PartitionPage::get_sentinel_page() {
  return &sentinel_page_;
}

void PartitionPage::FreeSlowPath() {
  DCHECK(this != get_sentinel_page());
  if (LIKELY(num_allocated_slots == 0)) {

    if (UNLIKELY(bucket->is_direct_mapped())) {
      PartitionDirectUnmap(this);
      return;
    }


    if (LIKELY(this == bucket->active_pages_head))
      bucket->SetNewActivePage();
    DCHECK(bucket->active_pages_head != this);

    set_raw_size(0);
    DCHECK(!get_raw_size());

    PartitionRegisterEmptyPage(this);
  } else {
    DCHECK(!bucket->is_direct_mapped());


    DCHECK(num_allocated_slots < 0);


    CHECK(num_allocated_slots != -1);
    num_allocated_slots = -num_allocated_slots - 2;
    DCHECK(num_allocated_slots == bucket->get_slots_per_span() - 1);




    DCHECK(!next_page);
    if (LIKELY(bucket->active_pages_head != get_sentinel_page()))
      next_page = bucket->active_pages_head;
    bucket->active_pages_head = this;
    --bucket->num_full_pages;


    if (UNLIKELY(num_allocated_slots == 0))
      FreeSlowPath();
  }
}

void PartitionPage::Decommit(PartitionRootBase* root) {
  DCHECK(is_empty());
  DCHECK(!bucket->is_direct_mapped());
  void* addr = PartitionPage::ToPointer(this);
  root->DecommitSystemPages(addr, bucket->get_bytes_per_span());






  freelist_head = nullptr;
  num_unprovisioned_slots = 0;
  DCHECK(is_decommitted());
}

void PartitionPage::DecommitIfPossible(PartitionRootBase* root) {
  DCHECK(empty_cache_index >= 0);
  DCHECK(static_cast<unsigned>(empty_cache_index) < kMaxFreeableSpans);
  DCHECK(this == root->global_empty_page_ring[empty_cache_index]);
  empty_cache_index = -1;
  if (is_empty())
    Decommit(root);
}

}  // namespace internal
}  // namespace base
