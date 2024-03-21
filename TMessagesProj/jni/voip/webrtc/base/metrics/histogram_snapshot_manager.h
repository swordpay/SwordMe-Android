// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_HISTOGRAM_SNAPSHOT_MANAGER_H_
#define BASE_METRICS_HISTOGRAM_SNAPSHOT_MANAGER_H_

#include <stdint.h>

#include <atomic>
#include <map>
#include <string>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/metrics/histogram_base.h"

namespace base {

class HistogramSamples;
class HistogramFlattener;

// histograms for recording either to disk or for transmission (such as from
// renderer to browser, or from browser to UMA upload). Since histograms can sit
// in memory for an extended period of time, and are vulnerable to memory
// corruption, this class also validates as much redundancy as it can before
// calling for the marginal change (a.k.a., delta) in a histogram to be
// recorded.
class BASE_EXPORT HistogramSnapshotManager final {
 public:
  explicit HistogramSnapshotManager(HistogramFlattener* histogram_flattener);
  ~HistogramSnapshotManager();






  void PrepareDeltas(const std::vector<HistogramBase*>& histograms,
                     HistogramBase::Flags flags_to_set,
                     HistogramBase::Flags required_flags);





  void PrepareDelta(HistogramBase* histogram);
  void PrepareFinalDelta(const HistogramBase* histogram);

 private:
  FRIEND_TEST_ALL_PREFIXES(HistogramSnapshotManagerTest, CheckMerge);



  struct SampleInfo {


    uint32_t inconsistencies = 0;
  };


  void PrepareSamples(const HistogramBase* histogram,
                      std::unique_ptr<HistogramSamples> samples);


  HistogramFlattener* const histogram_flattener_;  // Weak.


  std::map<uint64_t, SampleInfo> known_histograms_;




  std::atomic<bool> is_active_;

  DISALLOW_COPY_AND_ASSIGN(HistogramSnapshotManager);
};

}  // namespace base

#endif  // BASE_METRICS_HISTOGRAM_SNAPSHOT_MANAGER_H_
