// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// SparseHistogram class to store samples.

#ifndef BASE_METRICS_SAMPLE_MAP_H_
#define BASE_METRICS_SAMPLE_MAP_H_

#include <stdint.h>

#include <map>
#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_samples.h"

namespace base {

// data structures. Changes here likely need to be duplicated there.
class BASE_EXPORT SampleMap : public HistogramSamples {
 public:
  SampleMap();
  explicit SampleMap(uint64_t id);
  ~SampleMap() override;

  void Accumulate(HistogramBase::Sample value,
                  HistogramBase::Count count) override;
  HistogramBase::Count GetCount(HistogramBase::Sample value) const override;
  HistogramBase::Count TotalCount() const override;
  std::unique_ptr<SampleCountIterator> Iterator() const override;

 protected:

  bool AddSubtractImpl(SampleCountIterator* iter, Operator op) override;

 private:
  std::map<HistogramBase::Sample, HistogramBase::Count> sample_counts_;

  DISALLOW_COPY_AND_ASSIGN(SampleMap);
};

}  // namespace base

#endif  // BASE_METRICS_SAMPLE_MAP_H_
