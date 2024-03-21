// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// various forms, including ASCII graphical, HTML, and numerically (as a
// vector of numbers corresponding to each of the aggregating buckets).

// as integral number of milliseconds), or arbitrary integral units.

// the minimum for a declared range is 1 (instead of 0), while the maximum is
// (HistogramBase::kSampleType_MAX - 1). However, there will always be underflow
// and overflow buckets added automatically, so a 0 bucket will always exist
// even when a minimum value of 1 is specified.

// data, so it is safe to record to the same histogram from multiple locations
// in the code. It is a runtime error if all uses of the same histogram do not
// agree exactly in type, bucket size and range.

// always be larger (not equal) than minimal range. Zero and
// HistogramBase::kSampleType_MAX are implicitly added as first and last ranges,
// so the smallest legal bucket_count is 3. However CustomHistogram can have
// bucket count as 2 (when you give a custom ranges vector containing only 1
// range).
// For these 3 kinds of histograms, the max bucket count is always
// (Histogram::kBucketCount_MAX - 1).

// might contain (sequentially) the count of values in the following intervals:
// [0,1), [1,2), [2,4), [4,8), [8,16), [16,32), [32,64), [64,infinity)
// That bucket allocation would actually result from construction of a histogram
// for values between 1 and 64, with 8 buckets, such as:
// Histogram count("some name", 1, 64, 8);
// Note that the underflow bucket [0,1) and the overflow bucket [64,infinity)
// are also counted by the constructor in the user supplied "bucket_count"
// argument.
// The above example has an exponential ratio of 2 (doubling the bucket width
// in each consecutive bucket).  The Histogram class automatically calculates
// the smallest ratio that it can use to construct the number of buckets
// selected in the constructor.  An another example, if you had 50 buckets,
// and millisecond time values from 1 to 10000, then the ratio between
// consecutive bucket widths will be approximately somewhere around the 50th
// root of 10000.  This approach provides very fine grain (narrow) buckets
// at the low end of the histogram scale, but allows the histogram to cover a
// gigantic range with the addition of very few buckets.

// base/metrics/histogram_macros.h. Note: Callers should include that header
// directly if they only access the histogram APIs through macros.
//
// Macros use a pattern involving a function static variable, that is a pointer
// to a histogram.  This static is explicitly initialized on any thread
// that detects a uninitialized (NULL) pointer.  The potentially racy
// initialization is not a problem as it is always set to point to the same
// value (i.e., the FactoryGet always returns the same value).  FactoryGet
// is also completely thread safe, which results in a completely thread safe,
// and relatively fast, set of counters.  To avoid races at shutdown, the static
// pointer is NOT deleted, and we leak the histograms at process termination.

#ifndef BASE_METRICS_HISTOGRAM_H_
#define BASE_METRICS_HISTOGRAM_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/containers/span.h"
#include "base/gtest_prod_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/metrics/bucket_ranges.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_samples.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"

namespace base {

class BooleanHistogram;
class CustomHistogram;
class DelayedPersistentAllocation;
class Histogram;
class HistogramTest;
class LinearHistogram;
class Pickle;
class PickleIterator;
class SampleVector;
class SampleVectorBase;

class BASE_EXPORT Histogram : public HistogramBase {
 public:

  static const uint32_t kBucketCount_MAX;

  typedef std::vector<Count> Counts;

  ~Histogram() override;










  static HistogramBase* FactoryGet(const std::string& name,
                                   Sample minimum,
                                   Sample maximum,
                                   uint32_t bucket_count,
                                   int32_t flags);
  static HistogramBase* FactoryTimeGet(const std::string& name,
                                       base::TimeDelta minimum,
                                       base::TimeDelta maximum,
                                       uint32_t bucket_count,
                                       int32_t flags);
  static HistogramBase* FactoryMicrosecondsTimeGet(const std::string& name,
                                                   base::TimeDelta minimum,
                                                   base::TimeDelta maximum,
                                                   uint32_t bucket_count,
                                                   int32_t flags);



  static HistogramBase* FactoryGet(const char* name,
                                   Sample minimum,
                                   Sample maximum,
                                   uint32_t bucket_count,
                                   int32_t flags);
  static HistogramBase* FactoryTimeGet(const char* name,
                                       base::TimeDelta minimum,
                                       base::TimeDelta maximum,
                                       uint32_t bucket_count,
                                       int32_t flags);
  static HistogramBase* FactoryMicrosecondsTimeGet(const char* name,
                                                   base::TimeDelta minimum,
                                                   base::TimeDelta maximum,
                                                   uint32_t bucket_count,
                                                   int32_t flags);

  static std::unique_ptr<HistogramBase> PersistentCreate(
      const char* name,
      Sample minimum,
      Sample maximum,
      const BucketRanges* ranges,
      const DelayedPersistentAllocation& counts,
      const DelayedPersistentAllocation& logged_counts,
      HistogramSamples::Metadata* meta,
      HistogramSamples::Metadata* logged_meta);

  static void InitializeBucketRanges(Sample minimum,
                                     Sample maximum,
                                     BucketRanges* ranges);







  static const int kCommonRaceBasedCountMismatch;






  uint32_t FindCorruption(const HistogramSamples& samples) const override;



  const BucketRanges* bucket_ranges() const;
  Sample declared_min() const;
  Sample declared_max() const;
  virtual Sample ranges(uint32_t i) const;
  virtual uint32_t bucket_count() const;







  static bool InspectConstructionArguments(StringPiece name,
                                           Sample* minimum,
                                           Sample* maximum,
                                           uint32_t* bucket_count);

  uint64_t name_hash() const override;
  HistogramType GetHistogramType() const override;
  bool HasConstructionArguments(Sample expected_minimum,
                                Sample expected_maximum,
                                uint32_t expected_bucket_count) const override;
  void Add(Sample value) override;
  void AddCount(Sample value, int count) override;
  std::unique_ptr<HistogramSamples> SnapshotSamples() const override;
  std::unique_ptr<HistogramSamples> SnapshotDelta() override;
  std::unique_ptr<HistogramSamples> SnapshotFinalDelta() const override;
  void AddSamples(const HistogramSamples& samples) override;
  bool AddSamplesFromPickle(base::PickleIterator* iter) override;
  void WriteHTMLGraph(std::string* output) const override;
  void WriteAscii(std::string* output) const override;


  void ValidateHistogramContents() const override;

 protected:





  class Factory;


  Histogram(const char* name,
            Sample minimum,
            Sample maximum,
            const BucketRanges* ranges);






  Histogram(const char* name,
            Sample minimum,
            Sample maximum,
            const BucketRanges* ranges,
            const DelayedPersistentAllocation& counts,
            const DelayedPersistentAllocation& logged_counts,
            HistogramSamples::Metadata* meta,
            HistogramSamples::Metadata* logged_meta);

  void SerializeInfoImpl(base::Pickle* pickle) const override;

  virtual bool PrintEmptyBucket(uint32_t index) const;

  virtual double GetBucketSize(Count current, uint32_t i) const;



  virtual const std::string GetAsciiBucketRange(uint32_t it) const;

 private:

  friend class HistogramTest;
  FRIEND_TEST_ALL_PREFIXES(HistogramTest, BoundsTest);
  FRIEND_TEST_ALL_PREFIXES(HistogramTest, BucketPlacementTest);
  FRIEND_TEST_ALL_PREFIXES(HistogramTest, CorruptSampleCounts);

  friend class StatisticsRecorder;  // To allow it to delete duplicates.
  friend class StatisticsRecorderTest;

  friend BASE_EXPORT HistogramBase* DeserializeHistogramInfo(
      base::PickleIterator* iter);
  static HistogramBase* DeserializeInfoImpl(base::PickleIterator* iter);



  std::unique_ptr<SampleVector> SnapshotAllSamples() const;

  std::unique_ptr<SampleVector> SnapshotUnloggedSamples() const;



  void WriteAsciiBody(const SampleVector& snapshot,
                      bool graph_it,
                      const std::string& newline,
                      std::string* output) const;

  double GetPeakBucketSize(const SampleVectorBase& samples) const;

  void WriteAsciiHeader(const SampleVectorBase& samples,
                        std::string* output) const;


  void WriteAsciiBucketContext(const int64_t past,
                               const Count current,
                               const int64_t remaining,
                               const uint32_t i,
                               std::string* output) const;

  void GetParameters(DictionaryValue* params) const override;

  void GetCountAndBucketData(Count* count,
                             int64_t* sum,
                             ListValue* buckets) const override;

  std::unique_ptr<SampleVectorBase> unlogged_samples_;

  std::unique_ptr<SampleVectorBase> logged_samples_;

#if DCHECK_IS_ON()  // Don't waste memory if it won't be used.


  mutable bool final_delta_created_ = false;
#endif

  DISALLOW_COPY_AND_ASSIGN(Histogram);
};


// buckets.
class BASE_EXPORT LinearHistogram : public Histogram {
 public:
  ~LinearHistogram() override;

  /* minimum should start from 1. 0 is as minimum is invalid. 0 is an implicit
     default underflow bucket. */
  static HistogramBase* FactoryGet(const std::string& name,
                                   Sample minimum,
                                   Sample maximum,
                                   uint32_t bucket_count,
                                   int32_t flags);
  static HistogramBase* FactoryTimeGet(const std::string& name,
                                       TimeDelta minimum,
                                       TimeDelta maximum,
                                       uint32_t bucket_count,
                                       int32_t flags);



  static HistogramBase* FactoryGet(const char* name,
                                   Sample minimum,
                                   Sample maximum,
                                   uint32_t bucket_count,
                                   int32_t flags);
  static HistogramBase* FactoryTimeGet(const char* name,
                                       TimeDelta minimum,
                                       TimeDelta maximum,
                                       uint32_t bucket_count,
                                       int32_t flags);

  static std::unique_ptr<HistogramBase> PersistentCreate(
      const char* name,
      Sample minimum,
      Sample maximum,
      const BucketRanges* ranges,
      const DelayedPersistentAllocation& counts,
      const DelayedPersistentAllocation& logged_counts,
      HistogramSamples::Metadata* meta,
      HistogramSamples::Metadata* logged_meta);

  struct DescriptionPair {
    Sample sample;
    const char* description;  // Null means end of a list of pairs.
  };





  static HistogramBase* FactoryGetWithRangeDescription(
      const std::string& name,
      Sample minimum,
      Sample maximum,
      uint32_t bucket_count,
      int32_t flags,
      const DescriptionPair descriptions[]);

  static void InitializeBucketRanges(Sample minimum,
                                     Sample maximum,
                                     BucketRanges* ranges);

  HistogramType GetHistogramType() const override;

 protected:
  class Factory;

  LinearHistogram(const char* name,
                  Sample minimum,
                  Sample maximum,
                  const BucketRanges* ranges);

  LinearHistogram(const char* name,
                  Sample minimum,
                  Sample maximum,
                  const BucketRanges* ranges,
                  const DelayedPersistentAllocation& counts,
                  const DelayedPersistentAllocation& logged_counts,
                  HistogramSamples::Metadata* meta,
                  HistogramSamples::Metadata* logged_meta);

  double GetBucketSize(Count current, uint32_t i) const override;


  const std::string GetAsciiBucketRange(uint32_t i) const override;


  bool PrintEmptyBucket(uint32_t index) const override;

 private:
  friend BASE_EXPORT HistogramBase* DeserializeHistogramInfo(
      base::PickleIterator* iter);
  static HistogramBase* DeserializeInfoImpl(base::PickleIterator* iter);



  typedef std::map<Sample, std::string> BucketDescriptionMap;
  BucketDescriptionMap bucket_description_;

  DISALLOW_COPY_AND_ASSIGN(LinearHistogram);
};


// counts down by some factor. Remainder values are kept locally but lost when
// uploaded or serialized. The integral counts are rounded up/down so should
// average to the correct value when many reports are added.
//
// This is most useful when adding many counts at once via AddCount() that can
// cause overflows of the 31-bit counters, usually with an enum as the value.
class BASE_EXPORT ScaledLinearHistogram {
  using AtomicCount = Histogram::AtomicCount;
  using Sample = Histogram::Sample;

 public:


  ScaledLinearHistogram(const char* name,
                        Sample minimum,
                        Sample maximum,
                        uint32_t bucket_count,
                        int32_t scale,
                        int32_t flags);

  ~ScaledLinearHistogram();



  void AddScaledCount(Sample value, int count);

  int32_t scale() const { return scale_; }
  LinearHistogram* histogram() { return histogram_; }

 private:


  LinearHistogram* const histogram_;

  const int32_t scale_;



  std::vector<AtomicCount> remainders_;

  DISALLOW_COPY_AND_ASSIGN(ScaledLinearHistogram);
};


class BASE_EXPORT BooleanHistogram : public LinearHistogram {
 public:
  static HistogramBase* FactoryGet(const std::string& name, int32_t flags);



  static HistogramBase* FactoryGet(const char* name, int32_t flags);

  static std::unique_ptr<HistogramBase> PersistentCreate(
      const char* name,
      const BucketRanges* ranges,
      const DelayedPersistentAllocation& counts,
      const DelayedPersistentAllocation& logged_counts,
      HistogramSamples::Metadata* meta,
      HistogramSamples::Metadata* logged_meta);

  HistogramType GetHistogramType() const override;

 protected:
  class Factory;

 private:
  BooleanHistogram(const char* name, const BucketRanges* ranges);
  BooleanHistogram(const char* name,
                   const BucketRanges* ranges,
                   const DelayedPersistentAllocation& counts,
                   const DelayedPersistentAllocation& logged_counts,
                   HistogramSamples::Metadata* meta,
                   HistogramSamples::Metadata* logged_meta);

  friend BASE_EXPORT HistogramBase* DeserializeHistogramInfo(
      base::PickleIterator* iter);
  static HistogramBase* DeserializeInfoImpl(base::PickleIterator* iter);

  DISALLOW_COPY_AND_ASSIGN(BooleanHistogram);
};


class BASE_EXPORT CustomHistogram : public Histogram {
 public:




  static HistogramBase* FactoryGet(const std::string& name,
                                   const std::vector<Sample>& custom_ranges,
                                   int32_t flags);



  static HistogramBase* FactoryGet(const char* name,
                                   const std::vector<Sample>& custom_ranges,
                                   int32_t flags);

  static std::unique_ptr<HistogramBase> PersistentCreate(
      const char* name,
      const BucketRanges* ranges,
      const DelayedPersistentAllocation& counts,
      const DelayedPersistentAllocation& logged_counts,
      HistogramSamples::Metadata* meta,
      HistogramSamples::Metadata* logged_meta);

  HistogramType GetHistogramType() const override;





  static std::vector<Sample> ArrayToCustomEnumRanges(
      base::span<const Sample> values);

 protected:
  class Factory;

  CustomHistogram(const char* name, const BucketRanges* ranges);

  CustomHistogram(const char* name,
                  const BucketRanges* ranges,
                  const DelayedPersistentAllocation& counts,
                  const DelayedPersistentAllocation& logged_counts,
                  HistogramSamples::Metadata* meta,
                  HistogramSamples::Metadata* logged_meta);

  void SerializeInfoImpl(base::Pickle* pickle) const override;

  double GetBucketSize(Count current, uint32_t i) const override;

 private:
  friend BASE_EXPORT HistogramBase* DeserializeHistogramInfo(
      base::PickleIterator* iter);
  static HistogramBase* DeserializeInfoImpl(base::PickleIterator* iter);

  static bool ValidateCustomRanges(const std::vector<Sample>& custom_ranges);

  DISALLOW_COPY_AND_ASSIGN(CustomHistogram);
};

}  // namespace base

#endif  // BASE_METRICS_HISTOGRAM_H_
