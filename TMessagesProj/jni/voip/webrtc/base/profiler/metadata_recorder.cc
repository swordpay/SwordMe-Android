// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/metadata_recorder.h"

#include "base/metrics/histogram_macros.h"

namespace base {

MetadataRecorder::ItemInternal::ItemInternal() = default;

MetadataRecorder::ItemInternal::~ItemInternal() = default;

MetadataRecorder::MetadataRecorder() {

  DCHECK(items_[0].is_active.is_lock_free());
  DCHECK(items_[0].value.is_lock_free());
}

MetadataRecorder::~MetadataRecorder() = default;

void MetadataRecorder::Set(uint64_t name_hash,
                           Optional<int64_t> key,
                           int64_t value) {
  AutoLock lock(write_lock_);






  size_t item_slots_used = item_slots_used_.load(std::memory_order_relaxed);
  for (size_t i = 0; i < item_slots_used; ++i) {
    auto& item = items_[i];
    if (item.name_hash == name_hash && item.key == key) {
      item.value.store(value, std::memory_order_relaxed);

      const bool was_active =
          item.is_active.exchange(true, std::memory_order_release);
      if (!was_active)
        inactive_item_count_--;

      UMA_HISTOGRAM_COUNTS_10000("StackSamplingProfiler.MetadataSlotsUsed",
                                 item_slots_used);

      return;
    }
  }

  item_slots_used = TryReclaimInactiveSlots(item_slots_used);

  UMA_HISTOGRAM_COUNTS_10000("StackSamplingProfiler.MetadataSlotsUsed",
                             item_slots_used + 1);

  if (item_slots_used == items_.size()) {



    return;
  }



  auto& item = items_[item_slots_used];
  item.name_hash = name_hash;
  item.key = key;
  item.value.store(value, std::memory_order_relaxed);
  item.is_active.store(true, std::memory_order_release);
  item_slots_used_.fetch_add(1, std::memory_order_release);
}

void MetadataRecorder::Remove(uint64_t name_hash, Optional<int64_t> key) {
  AutoLock lock(write_lock_);

  size_t item_slots_used = item_slots_used_.load(std::memory_order_relaxed);
  for (size_t i = 0; i < item_slots_used; ++i) {
    auto& item = items_[i];
    if (item.name_hash == name_hash && item.key == key) {

      const bool was_active =
          item.is_active.exchange(false, std::memory_order_relaxed);
      if (was_active)
        inactive_item_count_++;

      return;
    }
  }
}

MetadataRecorder::ScopedGetItems::ScopedGetItems(
    MetadataRecorder* metadata_recorder)
    : metadata_recorder_(metadata_recorder),
      auto_lock_(&metadata_recorder->read_lock_) {}

MetadataRecorder::ScopedGetItems::~ScopedGetItems() {}

// doesn't understand that the lock is acquired in the constructor initializer
// list and can therefore be safely released here.
size_t MetadataRecorder::ScopedGetItems::GetItems(
    ProfileBuilder::MetadataItemArray* const items) NO_THREAD_SAFETY_ANALYSIS {
  size_t item_count = metadata_recorder_->GetItems(items);
  auto_lock_.Release();
  return item_count;
}

std::unique_ptr<ProfileBuilder::MetadataProvider>
MetadataRecorder::CreateMetadataProvider() {
  return std::make_unique<MetadataRecorder::ScopedGetItems>(this);
}

size_t MetadataRecorder::GetItems(
    ProfileBuilder::MetadataItemArray* const items) const {
  read_lock_.AssertAcquired();











  size_t item_slots_used = item_slots_used_.load(std::memory_order_acquire);
  size_t write_index = 0;
  for (size_t read_index = 0; read_index < item_slots_used; ++read_index) {
    const auto& item = items_[read_index];


    if (item.is_active.load(std::memory_order_acquire)) {
      (*items)[write_index++] = ProfileBuilder::MetadataItem{
          item.name_hash, item.key, item.value.load(std::memory_order_relaxed)};
    }
  }

  return write_index;
}

size_t MetadataRecorder::TryReclaimInactiveSlots(size_t item_slots_used) {
  const size_t remaining_slots =
      ProfileBuilder::MAX_METADATA_COUNT - item_slots_used;

  if (inactive_item_count_ == 0 || inactive_item_count_ < remaining_slots) {





    return item_slots_used;
  }

  if (read_lock_.Try()) {


    item_slots_used = ReclaimInactiveSlots(item_slots_used);
    read_lock_.Release();
  }

  return item_slots_used;
}

size_t MetadataRecorder::ReclaimInactiveSlots(size_t item_slots_used) {






  size_t first_inactive_item_idx = 0;
  size_t last_active_item_idx = item_slots_used - 1;
  while (first_inactive_item_idx < last_active_item_idx) {
    ItemInternal& inactive_item = items_[first_inactive_item_idx];
    ItemInternal& active_item = items_[last_active_item_idx];

    if (inactive_item.is_active.load(std::memory_order_relaxed)) {

      ++first_inactive_item_idx;
      continue;
    }

    if (!active_item.is_active.load(std::memory_order_relaxed)) {


      --last_active_item_idx;
      item_slots_used--;
      continue;
    }

    inactive_item.name_hash = active_item.name_hash;
    inactive_item.value.store(active_item.value.load(std::memory_order_relaxed),
                              std::memory_order_relaxed);
    inactive_item.is_active.store(true, std::memory_order_relaxed);

    ++first_inactive_item_idx;
    --last_active_item_idx;
    item_slots_used--;
  }

  item_slots_used_.store(item_slots_used, std::memory_order_relaxed);
  return item_slots_used;
}
}  // namespace base
