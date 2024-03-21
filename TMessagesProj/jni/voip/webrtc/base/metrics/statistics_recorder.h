// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Histograms in the system. It provides a general place for
// Histograms/BucketRanges to register, and supports a global API for accessing
// (i.e., dumping, or graphing) the data.

#ifndef BASE_METRICS_STATISTICS_RECORDER_H_
#define BASE_METRICS_STATISTICS_RECORDER_H_

#include <stdint.h>

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/gtest_prod_util.h"
#include "base/lazy_instance.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/record_histogram_checker.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/lock.h"

namespace base {

class BucketRanges;
class HistogramSnapshotManager;

//
// All the public methods are static and act on a global recorder. This global
// recorder is internally synchronized and all the static methods are thread
// safe.
//
// StatisticsRecorder doesn't have any public constructor. For testing purpose,
// you can create a temporary recorder using the factory method
// CreateTemporaryForTesting(). This temporary recorder becomes the global one
// until deleted. When this temporary recorder is deleted, it restores the
// previous global one.
class BASE_EXPORT StatisticsRecorder {
 public:


  class HistogramProvider {
   public:

    virtual void MergeHistogramDeltas() = 0;
  };

  typedef std::vector<HistogramBase*> Histograms;









  ~StatisticsRecorder();





  static void RegisterHistogramProvider(
      const WeakPtr<HistogramProvider>& provider);






  static HistogramBase* RegisterOrDeleteDuplicate(HistogramBase* histogram);






  static const BucketRanges* RegisterOrDeleteDuplicateRanges(
      const BucketRanges* ranges);





  static void WriteHTMLGraph(const std::string& query, std::string* output);
  static void WriteGraph(const std::string& query, std::string* output);




  static std::string ToJSON(JSONVerbosityLevel verbosity_level);







  static Histograms GetHistograms();




  static std::vector<const BucketRanges*> GetBucketRanges();




  static HistogramBase* FindHistogram(base::StringPiece name);



  static void ImportProvidedHistograms();





  static void PrepareDeltas(bool include_persistent,
                            HistogramBase::Flags flags_to_set,
                            HistogramBase::Flags required_flags,
                            HistogramSnapshotManager* snapshot_manager);

  using OnSampleCallback = base::RepeatingCallback<void(HistogramBase::Sample)>;





  static bool SetCallback(const std::string& histogram_name,
                          OnSampleCallback callback);



  static void ClearCallback(const std::string& histogram_name);




  static OnSampleCallback FindCallback(const std::string& histogram_name);



  static size_t GetHistogramCount();






  static void InitLogOnShutdown();





  static void ForgetHistogramForTesting(base::StringPiece name);








  static std::unique_ptr<StatisticsRecorder> CreateTemporaryForTesting()
      WARN_UNUSED_RESULT;





  static void SetRecordChecker(
      std::unique_ptr<RecordHistogramChecker> record_checker);





  static bool ShouldRecordHistogram(uint64_t histogram_hash);

  static Histograms Sort(Histograms histograms);


  static Histograms WithName(Histograms histograms, const std::string& query);

  static Histograms NonPersistent(Histograms histograms);

  using GlobalSampleCallback = void (*)(const char* /*=histogram_name*/,
                                        uint64_t /*=name_hash*/,
                                        HistogramBase::Sample);



  static void SetGlobalSampleCallback(
      const GlobalSampleCallback& global_sample_callback);


  static GlobalSampleCallback global_sample_callback() {
    return global_sample_callback_.load(std::memory_order_relaxed);
  }



  static bool have_active_callbacks() {
    return have_active_callbacks_.load(std::memory_order_relaxed);
  }

 private:
  typedef std::vector<WeakPtr<HistogramProvider>> HistogramProviders;

  typedef std::unordered_map<StringPiece, HistogramBase*, StringPieceHash>
      HistogramMap;


  typedef std::unordered_map<std::string, OnSampleCallback> CallbackMap;

  struct BucketRangesHash {
    size_t operator()(const BucketRanges* a) const;
  };

  struct BucketRangesEqual {
    bool operator()(const BucketRanges* a, const BucketRanges* b) const;
  };

  typedef std::
      unordered_set<const BucketRanges*, BucketRangesHash, BucketRangesEqual>
          RangesMap;

  friend class StatisticsRecorderTest;
  FRIEND_TEST_ALL_PREFIXES(StatisticsRecorderTest, IterationTest);




  static void EnsureGlobalRecorderWhileLocked();



  static HistogramProviders GetHistogramProviders();



  static void ImportGlobalPersistentHistograms();




  StatisticsRecorder();




  static void InitLogOnShutdownWhileLocked();

  HistogramMap histograms_;
  CallbackMap callbacks_;
  RangesMap ranges_;
  HistogramProviders providers_;
  std::unique_ptr<RecordHistogramChecker> record_checker_;

  StatisticsRecorder* previous_ = nullptr;

  static LazyInstance<Lock>::Leaky lock_;



  static StatisticsRecorder* top_;


  static bool is_vlog_initialized_;

  static std::atomic<bool> have_active_callbacks_;


  static std::atomic<GlobalSampleCallback> global_sample_callback_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsRecorder);
};

}  // namespace base

#endif  // BASE_METRICS_STATISTICS_RECORDER_H_
