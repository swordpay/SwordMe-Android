// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_H_
#define BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_H_

#include "base/memory/memory_pressure_listener.h"
#include "base/util/memory_pressure/memory_pressure_voter.h"
#include "base/util/memory_pressure/multi_source_memory_pressure_monitor.h"

namespace util {

// MemoryPressureVoters to cast their vote on the overall MemoryPressureLevel.
class SystemMemoryPressureEvaluator {
 public:


  static std::unique_ptr<SystemMemoryPressureEvaluator>
  CreateDefaultSystemEvaluator(MultiSourceMemoryPressureMonitor* monitor);

  virtual ~SystemMemoryPressureEvaluator();

  base::MemoryPressureListener::MemoryPressureLevel current_vote() const {
    return current_vote_;
  }

 protected:
  explicit SystemMemoryPressureEvaluator(
      std::unique_ptr<MemoryPressureVoter> voter);


  void SetCurrentVote(base::MemoryPressureListener::MemoryPressureLevel level);



  void SendCurrentVote(bool notify) const;

 private:
  base::MemoryPressureListener::MemoryPressureLevel current_vote_;


  std::unique_ptr<MemoryPressureVoter> voter_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(SystemMemoryPressureEvaluator);
};

}  // namespace util

#endif  // BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_H_
