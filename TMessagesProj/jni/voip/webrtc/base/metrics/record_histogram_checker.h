// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_RECORD_HISTOGRAM_CHECKER_H_
#define BASE_METRICS_RECORD_HISTOGRAM_CHECKER_H_

#include <stdint.h>

#include "base/base_export.h"

namespace base {

// the given histogram should be recorded.
class BASE_EXPORT RecordHistogramChecker {
 public:
  virtual ~RecordHistogramChecker() = default;


  virtual bool ShouldRecord(uint64_t histogram_hash) const = 0;
};

}  // namespace base

#endif  // BASE_METRICS_RECORD_HISTOGRAM_CHECKER_H_
