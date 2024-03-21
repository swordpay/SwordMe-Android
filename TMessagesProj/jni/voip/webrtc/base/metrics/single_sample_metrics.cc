// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/single_sample_metrics.h"

#include "base/memory/ptr_util.h"
#include "base/metrics/histogram.h"

namespace base {

static SingleSampleMetricsFactory* g_factory = nullptr;

SingleSampleMetricsFactory* SingleSampleMetricsFactory::Get() {
  if (!g_factory)
    g_factory = new DefaultSingleSampleMetricsFactory();

  return g_factory;
}

void SingleSampleMetricsFactory::SetFactory(
    std::unique_ptr<SingleSampleMetricsFactory> factory) {
  DCHECK(!g_factory);
  g_factory = factory.release();
}

void SingleSampleMetricsFactory::DeleteFactoryForTesting() {
  DCHECK(g_factory);
  delete g_factory;
  g_factory = nullptr;
}

std::unique_ptr<SingleSampleMetric>
DefaultSingleSampleMetricsFactory::CreateCustomCountsMetric(
    const std::string& histogram_name,
    HistogramBase::Sample min,
    HistogramBase::Sample max,
    uint32_t bucket_count) {
  return std::make_unique<DefaultSingleSampleMetric>(
      histogram_name, min, max, bucket_count,
      HistogramBase::kUmaTargetedHistogramFlag);
}

DefaultSingleSampleMetric::DefaultSingleSampleMetric(
    const std::string& histogram_name,
    HistogramBase::Sample min,
    HistogramBase::Sample max,
    uint32_t bucket_count,
    int32_t flags)
    : histogram_(Histogram::FactoryGet(histogram_name,
                                       min,
                                       max,
                                       bucket_count,
                                       flags)) {




  DCHECK(histogram_);
}

DefaultSingleSampleMetric::~DefaultSingleSampleMetric() {

  if (sample_ < 0 || !histogram_)
    return;
  histogram_->Add(sample_);
}

void DefaultSingleSampleMetric::SetSample(HistogramBase::Sample sample) {
  DCHECK_GE(sample, 0);
  sample_ = sample;
}

}  // namespace base
