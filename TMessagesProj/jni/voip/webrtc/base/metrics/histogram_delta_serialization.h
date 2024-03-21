// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_HISTOGRAM_DELTA_SERIALIZATION_H_
#define BASE_METRICS_HISTOGRAM_DELTA_SERIALIZATION_H_

#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/metrics/histogram_flattener.h"
#include "base/metrics/histogram_snapshot_manager.h"
#include "base/threading/thread_checker.h"

namespace base {

class HistogramBase;

class BASE_EXPORT HistogramDeltaSerialization : public HistogramFlattener {
 public:

  explicit HistogramDeltaSerialization(const std::string& caller_name);
  ~HistogramDeltaSerialization() override;






  void PrepareAndSerializeDeltas(std::vector<std::string>* serialized_deltas,
                                 bool include_persistent);


  static void DeserializeAndAddSamples(
      const std::vector<std::string>& serialized_deltas);

 private:

  void RecordDelta(const HistogramBase& histogram,
                   const HistogramSamples& snapshot) override;

  ThreadChecker thread_checker_;

  HistogramSnapshotManager histogram_snapshot_manager_;

  std::vector<std::string>* serialized_deltas_;

  DISALLOW_COPY_AND_ASSIGN(HistogramDeltaSerialization);
};

}  // namespace base

#endif  // BASE_METRICS_HISTOGRAM_DELTA_SERIALIZATION_H_
