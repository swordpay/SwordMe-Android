// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_HISTOGRAM_SAMPLES_H_
#define BASE_METRICS_HISTOGRAM_SAMPLES_H_

#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <memory>

#include "base/atomicops.h"
#include "base/macros.h"
#include "base/metrics/histogram_base.h"

namespace base {

class Pickle;
class PickleIterator;
class SampleCountIterator;

// elements must be of a fixed width to ensure 32/64-bit interoperability.
// If this structure changes, bump the version number for kTypeIdHistogram
// in persistent_histogram_allocator.cc.
//
// Note that though these samples are individually consistent (through the use
// of atomic operations on the counts), there is only "eventual consistency"
// overall when multiple threads are accessing this data. That means that the
// sum, redundant-count, etc. could be momentarily out-of-sync with the stored
// counts but will settle to a consistent "steady state" once all threads have
// exited this code.
class BASE_EXPORT HistogramSamples {
 public:




  struct SingleSample {
    uint16_t bucket;
    uint16_t count;
  };



  union BASE_EXPORT AtomicSingleSample {
    AtomicSingleSample() : as_atomic(0) {}
    AtomicSingleSample(subtle::Atomic32 rhs) : as_atomic(rhs) {}



    SingleSample Load() const;



    SingleSample Extract(bool disable);



    bool Accumulate(size_t bucket, HistogramBase::Count count);


    bool IsDisabled() const;

   private:

    SingleSample as_parts;




    subtle::Atomic32 as_atomic;
  };



  struct Metadata {

    static constexpr size_t kExpectedInstanceSize = 24;




    uint64_t id;





#ifdef ARCH_CPU_64_BITS
    subtle::Atomic64 sum;
#else


    int64_t sum;
#endif







    HistogramBase::AtomicCount redundant_count;



    AtomicSingleSample single_sample;  // 32 bits
  };



  struct BASE_EXPORT LocalMetadata : Metadata {
    LocalMetadata();
  };

  HistogramSamples(uint64_t id, Metadata* meta);
  virtual ~HistogramSamples();

  virtual void Accumulate(HistogramBase::Sample value,
                          HistogramBase::Count count) = 0;
  virtual HistogramBase::Count GetCount(HistogramBase::Sample value) const = 0;
  virtual HistogramBase::Count TotalCount() const = 0;

  virtual void Add(const HistogramSamples& other);

  virtual bool AddFromPickle(PickleIterator* iter);

  virtual void Subtract(const HistogramSamples& other);

  virtual std::unique_ptr<SampleCountIterator> Iterator() const = 0;
  virtual void Serialize(Pickle* pickle) const;

  uint64_t id() const { return meta_->id; }
  int64_t sum() const {
#ifdef ARCH_CPU_64_BITS
    return subtle::NoBarrier_Load(&meta_->sum);
#else
    return meta_->sum;
#endif
  }
  HistogramBase::Count redundant_count() const {
    return subtle::NoBarrier_Load(&meta_->redundant_count);
  }

 protected:
  enum NegativeSampleReason {
    SAMPLES_HAVE_LOGGED_BUT_NOT_SAMPLE,
    SAMPLES_SAMPLE_LESS_THAN_LOGGED,
    SAMPLES_ADDED_NEGATIVE_COUNT,
    SAMPLES_ADD_WENT_NEGATIVE,
    SAMPLES_ADD_OVERFLOW,
    SAMPLES_ACCUMULATE_NEGATIVE_COUNT,
    SAMPLES_ACCUMULATE_WENT_NEGATIVE,
    DEPRECATED_SAMPLES_ACCUMULATE_OVERFLOW,
    SAMPLES_ACCUMULATE_OVERFLOW,
    MAX_NEGATIVE_SAMPLE_REASONS
  };

  enum Operator { ADD, SUBTRACT };
  virtual bool AddSubtractImpl(SampleCountIterator* iter, Operator op) = 0;



  bool AccumulateSingleSample(HistogramBase::Sample value,
                              HistogramBase::Count count,
                              size_t bucket);

  void IncreaseSumAndCount(int64_t sum, HistogramBase::Count count);

  void RecordNegativeSample(NegativeSampleReason reason,
                            HistogramBase::Count increment);

  AtomicSingleSample& single_sample() { return meta_->single_sample; }
  const AtomicSingleSample& single_sample() const {
    return meta_->single_sample;
  }

  Metadata* meta() { return meta_; }

 private:



  Metadata* meta_;

  DISALLOW_COPY_AND_ASSIGN(HistogramSamples);
};

class BASE_EXPORT SampleCountIterator {
 public:
  virtual ~SampleCountIterator();

  virtual bool Done() const = 0;
  virtual void Next() = 0;






  virtual void Get(HistogramBase::Sample* min,
                   int64_t* max,
                   HistogramBase::Count* count) const = 0;
  static_assert(std::numeric_limits<HistogramBase::Sample>::max() <
                    std::numeric_limits<int64_t>::max(),
                "Get() |max| must be able to hold Histogram::Sample max + 1");



  virtual bool GetBucketIndex(size_t* index) const;
};

class BASE_EXPORT SingleSampleIterator : public SampleCountIterator {
 public:
  SingleSampleIterator(HistogramBase::Sample min,
                       int64_t max,
                       HistogramBase::Count count);
  SingleSampleIterator(HistogramBase::Sample min,
                       int64_t max,
                       HistogramBase::Count count,
                       size_t bucket_index);
  ~SingleSampleIterator() override;

  bool Done() const override;
  void Next() override;
  void Get(HistogramBase::Sample* min,
           int64_t* max,
           HistogramBase::Count* count) const override;

  bool GetBucketIndex(size_t* index) const override;

 private:

  const HistogramBase::Sample min_;
  const int64_t max_;
  const size_t bucket_index_;
  HistogramBase::Count count_;
};

}  // namespace base

#endif  // BASE_METRICS_HISTOGRAM_SAMPLES_H_
