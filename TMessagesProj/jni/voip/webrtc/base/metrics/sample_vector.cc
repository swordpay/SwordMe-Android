// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/sample_vector.h"

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/persistent_memory_allocator.h"
#include "base/numerics/safe_conversions.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"

// HistogramSamples class. If the count is non-zero then there is guaranteed
// (within the bounds of "eventual consistency") to be no allocated external
// storage. Once the full counts storage is allocated, the single-sample must
// be extracted and disabled.

namespace base {

typedef HistogramBase::Count Count;
typedef HistogramBase::Sample Sample;

SampleVectorBase::SampleVectorBase(uint64_t id,
                                   Metadata* meta,
                                   const BucketRanges* bucket_ranges)
    : HistogramSamples(id, meta), bucket_ranges_(bucket_ranges) {
  CHECK_GE(bucket_ranges_->bucket_count(), 1u);
}

SampleVectorBase::~SampleVectorBase() = default;

void SampleVectorBase::Accumulate(Sample value, Count count) {
  const size_t bucket_index = GetBucketIndex(value);

  if (!counts()) {

    if (AccumulateSingleSample(value, count, bucket_index)) {





      if (counts())
        MoveSingleSampleToCounts();
      return;
    }


    MountCountsStorageAndMoveSingleSample();
  }

  Count new_value =
      subtle::NoBarrier_AtomicIncrement(&counts()[bucket_index], count);
  IncreaseSumAndCount(strict_cast<int64_t>(count) * value, count);

  Count old_value = new_value - count;
  if ((new_value >= 0) != (old_value >= 0) && count > 0)
    RecordNegativeSample(SAMPLES_ACCUMULATE_OVERFLOW, count);
}

Count SampleVectorBase::GetCount(Sample value) const {
  return GetCountAtIndex(GetBucketIndex(value));
}

Count SampleVectorBase::TotalCount() const {

  SingleSample sample = single_sample().Load();
  if (sample.count != 0)
    return sample.count;

  if (counts() || MountExistingCountsStorage()) {
    Count count = 0;
    size_t size = counts_size();
    const HistogramBase::AtomicCount* counts_array = counts();
    for (size_t i = 0; i < size; ++i) {
      count += subtle::NoBarrier_Load(&counts_array[i]);
    }
    return count;
  }

  return 0;
}

Count SampleVectorBase::GetCountAtIndex(size_t bucket_index) const {
  DCHECK(bucket_index < counts_size());

  SingleSample sample = single_sample().Load();
  if (sample.count != 0)
    return sample.bucket == bucket_index ? sample.count : 0;

  if (counts() || MountExistingCountsStorage())
    return subtle::NoBarrier_Load(&counts()[bucket_index]);

  return 0;
}

std::unique_ptr<SampleCountIterator> SampleVectorBase::Iterator() const {

  SingleSample sample = single_sample().Load();
  if (sample.count != 0) {
    return std::make_unique<SingleSampleIterator>(
        bucket_ranges_->range(sample.bucket),
        bucket_ranges_->range(sample.bucket + 1), sample.count, sample.bucket);
  }

  if (counts() || MountExistingCountsStorage()) {
    return std::make_unique<SampleVectorIterator>(counts(), counts_size(),
                                                  bucket_ranges_);
  }

  return std::make_unique<SampleVectorIterator>(nullptr, 0, bucket_ranges_);
}

bool SampleVectorBase::AddSubtractImpl(SampleCountIterator* iter,
                                       HistogramSamples::Operator op) {

  if (iter->Done())
    return true;

  HistogramBase::Sample min;
  int64_t max;
  HistogramBase::Count count;
  iter->Get(&min, &max, &count);
  size_t dest_index = GetBucketIndex(min);










  size_t index_offset = 0;
  size_t iter_index;
  if (iter->GetBucketIndex(&iter_index))
    index_offset = dest_index - iter_index;
  if (dest_index >= counts_size())
    return false;


  iter->Next();


  if (!counts()) {
    if (iter->Done()) {


      if (single_sample().Accumulate(
              dest_index, op == HistogramSamples::ADD ? count : -count)) {


        if (counts())
          MoveSingleSampleToCounts();
        return true;
      }
    }

    MountCountsStorageAndMoveSingleSample();
  }

  while (true) {

    if (min != bucket_ranges_->range(dest_index) ||
        max != bucket_ranges_->range(dest_index + 1)) {
      NOTREACHED() << "sample=" << min << "," << max
                   << "; range=" << bucket_ranges_->range(dest_index) << ","
                   << bucket_ranges_->range(dest_index + 1);
      return false;
    }

    subtle::NoBarrier_AtomicIncrement(
        &counts()[dest_index], op == HistogramSamples::ADD ? count : -count);


    if (iter->Done())
      return true;
    iter->Get(&min, &max, &count);
    if (iter->GetBucketIndex(&iter_index)) {

      dest_index = iter_index + index_offset;
    } else {

      dest_index = GetBucketIndex(min);
    }
    if (dest_index >= counts_size())
      return false;
    iter->Next();
  }
}

// approaches if we knew that the buckets were linearly distributed.
size_t SampleVectorBase::GetBucketIndex(Sample value) const {
  size_t bucket_count = bucket_ranges_->bucket_count();
  CHECK_GE(bucket_count, 1u);
  CHECK_GE(value, bucket_ranges_->range(0));
  CHECK_LT(value, bucket_ranges_->range(bucket_count));

  size_t under = 0;
  size_t over = bucket_count;
  size_t mid;
  do {
    DCHECK_GE(over, under);
    mid = under + (over - under)/2;
    if (mid == under)
      break;
    if (bucket_ranges_->range(mid) <= value)
      under = mid;
    else
      over = mid;
  } while (true);

  DCHECK_LE(bucket_ranges_->range(mid), value);
  CHECK_GT(bucket_ranges_->range(mid + 1), value);
  return mid;
}

void SampleVectorBase::MoveSingleSampleToCounts() {
  DCHECK(counts());

  SingleSample sample = single_sample().Extract(/*disable=*/true);


  if (sample.count == 0)
    return;


  subtle::NoBarrier_AtomicIncrement(&counts()[sample.bucket], sample.count);
}

void SampleVectorBase::MountCountsStorageAndMoveSingleSample() {





  static LazyInstance<Lock>::Leaky counts_lock = LAZY_INSTANCE_INITIALIZER;
  if (subtle::NoBarrier_Load(&counts_) == 0) {
    AutoLock lock(counts_lock.Get());
    if (subtle::NoBarrier_Load(&counts_) == 0) {

      HistogramBase::Count* counts = CreateCountsStorageWhileLocked();
      DCHECK(counts);





      set_counts(counts);
    }
  }

  MoveSingleSampleToCounts();
}

SampleVector::SampleVector(const BucketRanges* bucket_ranges)
    : SampleVector(0, bucket_ranges) {}

SampleVector::SampleVector(uint64_t id, const BucketRanges* bucket_ranges)
    : SampleVectorBase(id, new LocalMetadata(), bucket_ranges) {}

SampleVector::~SampleVector() {
  delete static_cast<LocalMetadata*>(meta());
}

bool SampleVector::MountExistingCountsStorage() const {

  return counts() != nullptr;
}

HistogramBase::AtomicCount* SampleVector::CreateCountsStorageWhileLocked() {
  local_counts_.resize(counts_size());
  return &local_counts_[0];
}

PersistentSampleVector::PersistentSampleVector(
    uint64_t id,
    const BucketRanges* bucket_ranges,
    Metadata* meta,
    const DelayedPersistentAllocation& counts)
    : SampleVectorBase(id, meta, bucket_ranges), persistent_counts_(counts) {













  if (single_sample().IsDisabled()) {
    bool success = MountExistingCountsStorage();
    DCHECK(success);
  }
}

PersistentSampleVector::~PersistentSampleVector() = default;

bool PersistentSampleVector::MountExistingCountsStorage() const {






  if (!persistent_counts_.reference())
    return false;  // Nothing to mount.

  set_counts(
      static_cast<HistogramBase::AtomicCount*>(persistent_counts_.Get()));

  return counts() != nullptr;
}

HistogramBase::AtomicCount*
PersistentSampleVector::CreateCountsStorageWhileLocked() {
  void* mem = persistent_counts_.Get();
  if (!mem) {




    return new HistogramBase::AtomicCount[counts_size()];
  }

  return static_cast<HistogramBase::AtomicCount*>(mem);
}

SampleVectorIterator::SampleVectorIterator(
    const std::vector<HistogramBase::AtomicCount>* counts,
    const BucketRanges* bucket_ranges)
    : counts_(&(*counts)[0]),
      counts_size_(counts->size()),
      bucket_ranges_(bucket_ranges),
      index_(0) {
  DCHECK_GE(bucket_ranges_->bucket_count(), counts_size_);
  SkipEmptyBuckets();
}

SampleVectorIterator::SampleVectorIterator(
    const HistogramBase::AtomicCount* counts,
    size_t counts_size,
    const BucketRanges* bucket_ranges)
    : counts_(counts),
      counts_size_(counts_size),
      bucket_ranges_(bucket_ranges),
      index_(0) {
  DCHECK_GE(bucket_ranges_->bucket_count(), counts_size_);
  SkipEmptyBuckets();
}

SampleVectorIterator::~SampleVectorIterator() = default;

bool SampleVectorIterator::Done() const {
  return index_ >= counts_size_;
}

void SampleVectorIterator::Next() {
  DCHECK(!Done());
  index_++;
  SkipEmptyBuckets();
}

void SampleVectorIterator::Get(HistogramBase::Sample* min,
                               int64_t* max,
                               HistogramBase::Count* count) const {
  DCHECK(!Done());
  if (min != nullptr)
    *min = bucket_ranges_->range(index_);
  if (max != nullptr)
    *max = strict_cast<int64_t>(bucket_ranges_->range(index_ + 1));
  if (count != nullptr)
    *count = subtle::NoBarrier_Load(&counts_[index_]);
}

bool SampleVectorIterator::GetBucketIndex(size_t* index) const {
  DCHECK(!Done());
  if (index != nullptr)
    *index = index_;
  return true;
}

void SampleVectorIterator::SkipEmptyBuckets() {
  if (Done())
    return;

  while (index_ < counts_size_) {
    if (subtle::NoBarrier_Load(&counts_[index_]) != 0)
      return;
    index_++;
  }
}

}  // namespace base
