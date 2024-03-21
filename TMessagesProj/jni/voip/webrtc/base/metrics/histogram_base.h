// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_HISTOGRAM_BASE_H_
#define BASE_METRICS_HISTOGRAM_BASE_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"

namespace base {

class DictionaryValue;
class HistogramBase;
class HistogramSamples;
class ListValue;
class Pickle;
class PickleIterator;

// This enum is used to facilitate deserialization of histograms from other
// processes into the browser. If you create another class that inherits from
// HistogramBase, add new histogram types and names below.

enum HistogramType {
  HISTOGRAM,
  LINEAR_HISTOGRAM,
  BOOLEAN_HISTOGRAM,
  CUSTOM_HISTOGRAM,
  SPARSE_HISTOGRAM,
  DUMMY_HISTOGRAM,
};

// a JSON.
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.base.metrics
enum JSONVerbosityLevel {

  JSON_VERBOSITY_LEVEL_FULL,

  JSON_VERBOSITY_LEVEL_OMIT_BUCKETS,
};

std::string HistogramTypeToString(HistogramType type);

// variations are being created. It has to be in the main .h file so it is
// visible to files that define the various histogram types.
enum HistogramReport {


  HISTOGRAM_REPORT_CREATED = 0,


  HISTOGRAM_REPORT_HISTOGRAM_CREATED = 1,


  HISTOGRAM_REPORT_HISTOGRAM_LOOKUP = 2,


  HISTOGRAM_REPORT_TYPE_LOGARITHMIC = 3,
  HISTOGRAM_REPORT_TYPE_LINEAR = 4,
  HISTOGRAM_REPORT_TYPE_BOOLEAN = 5,
  HISTOGRAM_REPORT_TYPE_CUSTOM = 6,
  HISTOGRAM_REPORT_TYPE_SPARSE = 7,

  HISTOGRAM_REPORT_FLAG_UMA_TARGETED = 8,
  HISTOGRAM_REPORT_FLAG_UMA_STABILITY = 9,
  HISTOGRAM_REPORT_FLAG_PERSISTENT = 10,

  HISTOGRAM_REPORT_MAX = 11
};

// Returns NULL if the pickled data has problems.
BASE_EXPORT HistogramBase* DeserializeHistogramInfo(base::PickleIterator* iter);


class BASE_EXPORT HistogramBase {
 public:
  typedef int32_t Sample;                // Used for samples.
  typedef subtle::Atomic32 AtomicCount;  // Used to count samples.
  typedef int32_t Count;  // Used to manipulate counts in temporaries.

  static const Sample kSampleType_MAX;  // INT_MAX

  enum Flags {
    kNoFlags = 0x0,

    kUmaTargetedHistogramFlag = 0x1,



    kUmaStabilityHistogramFlag = kUmaTargetedHistogramFlag | 0x2,





    kIPCSerializationSourceFlag = 0x10,




    kCallbackExists = 0x20,





    kIsPersistent = 0x40,
  };

  enum Inconsistency : uint32_t {
    NO_INCONSISTENCIES = 0x0,
    RANGE_CHECKSUM_ERROR = 0x1,
    BUCKET_ORDER_ERROR = 0x2,
    COUNT_HIGH_ERROR = 0x4,
    COUNT_LOW_ERROR = 0x8,

    NEVER_EXCEEDED_VALUE = 0x10,
  };


  explicit HistogramBase(const char* name);
  virtual ~HistogramBase();

  const char* histogram_name() const { return histogram_name_; }



  virtual void CheckName(const StringPiece& name) const;

  virtual uint64_t name_hash() const = 0;

  int32_t flags() const { return subtle::NoBarrier_Load(&flags_); }
  void SetFlags(int32_t flags);
  void ClearFlags(int32_t flags);

  virtual HistogramType GetHistogramType() const = 0;



  virtual bool HasConstructionArguments(
      Sample expected_minimum,
      Sample expected_maximum,
      uint32_t expected_bucket_count) const = 0;

  virtual void Add(Sample value) = 0;




  virtual void AddCount(Sample value, int count) = 0;





  void AddScaled(Sample value, int count, int scale);
  void AddKilo(Sample value, int count);  // scale=1000
  void AddKiB(Sample value, int count);   // scale=1024

  void AddTime(const TimeDelta& time) { AddTimeMillisecondsGranularity(time); }
  void AddTimeMillisecondsGranularity(const TimeDelta& time);


  void AddTimeMicrosecondsGranularity(const TimeDelta& time);
  void AddBoolean(bool value);

  virtual void AddSamples(const HistogramSamples& samples) = 0;
  virtual bool AddSamplesFromPickle(base::PickleIterator* iter) = 0;



  void SerializeInfo(base::Pickle* pickle) const;


  virtual uint32_t FindCorruption(const HistogramSamples& samples) const;





  virtual std::unique_ptr<HistogramSamples> SnapshotSamples() const = 0;



  virtual std::unique_ptr<HistogramSamples> SnapshotDelta() = 0;







  virtual std::unique_ptr<HistogramSamples> SnapshotFinalDelta() const = 0;

  virtual void WriteHTMLGraph(std::string* output) const = 0;
  virtual void WriteAscii(std::string* output) const = 0;

  virtual void ValidateHistogramContents() const;




  void WriteJSON(std::string* output, JSONVerbosityLevel verbosity_level) const;

 protected:
  enum ReportActivity { HISTOGRAM_CREATED, HISTOGRAM_LOOKUP };

  virtual void SerializeInfoImpl(base::Pickle* pickle) const = 0;

  virtual void GetParameters(DictionaryValue* params) const = 0;



  virtual void GetCountAndBucketData(Count* count,
                                     int64_t* sum,
                                     ListValue* buckets) const = 0;

  void WriteAsciiBucketGraph(double current_size,
                             double max_size,
                             std::string* output) const;

  const std::string GetSimpleAsciiBucketRange(Sample sample) const;


  void WriteAsciiBucketValue(Count current,
                             double scaled_sum,
                             std::string* output) const;


  void FindAndRunCallback(Sample sample) const;


  static const char* GetPermanentName(const std::string& name);

 private:
  friend class HistogramBaseTest;








  const char* const histogram_name_;

  AtomicCount flags_;

  DISALLOW_COPY_AND_ASSIGN(HistogramBase);
};

}  // namespace base

#endif  // BASE_METRICS_HISTOGRAM_BASE_H_
