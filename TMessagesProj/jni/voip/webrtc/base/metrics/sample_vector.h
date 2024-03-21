// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Histogram based classes to store samples.

#ifndef BASE_METRICS_SAMPLE_VECTOR_H_
#define BASE_METRICS_SAMPLE_VECTOR_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "base/atomicops.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/metrics/bucket_ranges.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_samples.h"
#include "base/metrics/persistent_memory_allocator.h"

namespace base {

class BucketRanges;

class BASE_EXPORT SampleVectorBase : public HistogramSamples {
 public:
  SampleVectorBase(uint64_t id,
                   Metadata* meta,
                   const BucketRanges* bucket_ranges);
  ~SampleVectorBase() override;

  void Accumulate(HistogramBase::Sample value,
                  HistogramBase::Count count) override;
  HistogramBase::Count GetCount(HistogramBase::Sample value) const override;
  HistogramBase::Count TotalCount() const override;
  std::unique_ptr<SampleCountIterator> Iterator() const override;

  HistogramBase::Count GetCountAtIndex(size_t bucket_index) const;

  const BucketRanges* bucket_ranges() const { return bucket_ranges_; }

 protected:
  bool AddSubtractImpl(
      SampleCountIterator* iter,
      HistogramSamples::Operator op) override;  // |op| is ADD or SUBTRACT.

  virtual size_t GetBucketIndex(HistogramBase::Sample value) const;

  void MoveSingleSampleToCounts();


  void MountCountsStorageAndMoveSingleSample();



  virtual bool MountExistingCountsStorage() const = 0;



  virtual HistogramBase::Count* CreateCountsStorageWhileLocked() = 0;

  HistogramBase::AtomicCount* counts() {
    return reinterpret_cast<HistogramBase::AtomicCount*>(
        subtle::Acquire_Load(&counts_));
  }

  const HistogramBase::AtomicCount* counts() const {
    return reinterpret_cast<HistogramBase::AtomicCount*>(
        subtle::Acquire_Load(&counts_));
  }

  void set_counts(const HistogramBase::AtomicCount* counts) const {
    subtle::Release_Store(&counts_, reinterpret_cast<uintptr_t>(counts));
  }

  size_t counts_size() const { return bucket_ranges_->bucket_count(); }

 private:
  friend class SampleVectorTest;
  FRIEND_TEST_ALL_PREFIXES(HistogramTest, CorruptSampleCounts);
  FRIEND_TEST_ALL_PREFIXES(SharedHistogramTest, CorruptSampleCounts);











  mutable subtle::AtomicWord counts_ = 0;

  const BucketRanges* const bucket_ranges_;

  DISALLOW_COPY_AND_ASSIGN(SampleVectorBase);
};

class BASE_EXPORT SampleVector : public SampleVectorBase {
 public:
  explicit SampleVector(const BucketRanges* bucket_ranges);
  SampleVector(uint64_t id, const BucketRanges* bucket_ranges);
  ~SampleVector() override;

 private:

  bool MountExistingCountsStorage() const override;
  HistogramBase::Count* CreateCountsStorageWhileLocked() override;

  mutable std::vector<HistogramBase::AtomicCount> local_counts_;

  DISALLOW_COPY_AND_ASSIGN(SampleVector);
};

class BASE_EXPORT PersistentSampleVector : public SampleVectorBase {
 public:
  PersistentSampleVector(uint64_t id,
                         const BucketRanges* bucket_ranges,
                         Metadata* meta,
                         const DelayedPersistentAllocation& counts);
  ~PersistentSampleVector() override;

 private:

  bool MountExistingCountsStorage() const override;
  HistogramBase::Count* CreateCountsStorageWhileLocked() override;

  DelayedPersistentAllocation persistent_counts_;

  DISALLOW_COPY_AND_ASSIGN(PersistentSampleVector);
};

// file but is here for easy testing.
class BASE_EXPORT SampleVectorIterator : public SampleCountIterator {
 public:
  SampleVectorIterator(const std::vector<HistogramBase::AtomicCount>* counts,
                       const BucketRanges* bucket_ranges);
  SampleVectorIterator(const HistogramBase::AtomicCount* counts,
                       size_t counts_size,
                       const BucketRanges* bucket_ranges);
  ~SampleVectorIterator() override;

  bool Done() const override;
  void Next() override;
  void Get(HistogramBase::Sample* min,
           int64_t* max,
           HistogramBase::Count* count) const override;

  bool GetBucketIndex(size_t* index) const override;

 private:
  void SkipEmptyBuckets();

  const HistogramBase::AtomicCount* counts_;
  size_t counts_size_;
  const BucketRanges* bucket_ranges_;

  size_t index_;
};

}  // namespace base

#endif  // BASE_METRICS_SAMPLE_VECTOR_H_
