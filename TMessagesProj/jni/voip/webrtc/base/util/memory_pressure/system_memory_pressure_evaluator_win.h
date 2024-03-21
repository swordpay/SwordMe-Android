// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_WIN_H_
#define BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_WIN_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/memory_pressure_listener.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/timer/timer.h"
#include "base/util/memory_pressure/memory_pressure_voter.h"
#include "base/util/memory_pressure/system_memory_pressure_evaluator.h"

typedef struct _MEMORYSTATUSEX MEMORYSTATUSEX;

namespace util {
namespace win {

// polls at a low frequency, and applies internal hysteresis.
class SystemMemoryPressureEvaluator
    : public util::SystemMemoryPressureEvaluator {
 public:
  using MemoryPressureLevel = base::MemoryPressureListener::MemoryPressureLevel;



  static const int kModeratePressureCooldownMs;



  static const int kLargeMemoryThresholdMb;

  static const int kSmallMemoryDefaultModerateThresholdMb;
  static const int kSmallMemoryDefaultCriticalThresholdMb;

  static const int kLargeMemoryDefaultModerateThresholdMb;
  static const int kLargeMemoryDefaultCriticalThresholdMb;


  explicit SystemMemoryPressureEvaluator(
      std::unique_ptr<MemoryPressureVoter> voter);



  SystemMemoryPressureEvaluator(int moderate_threshold_mb,
                                int critical_threshold_mb,
                                std::unique_ptr<MemoryPressureVoter> voter);

  ~SystemMemoryPressureEvaluator() override;


  void CheckMemoryPressureSoon();

  int moderate_threshold_mb() const { return moderate_threshold_mb_; }

  int critical_threshold_mb() const { return critical_threshold_mb_; }

 protected:



  void InferThresholds();


  void StartObserving();



  void StopObserving();





  void CheckMemoryPressure();



  MemoryPressureLevel CalculateCurrentPressureLevel();



  virtual bool GetSystemMemoryStatus(MEMORYSTATUSEX* mem_status);

 private:


  int moderate_threshold_mb_;
  int critical_threshold_mb_;

  base::RepeatingTimer timer_;




  int moderate_pressure_repeat_count_;

  SEQUENCE_CHECKER(sequence_checker_);


  base::WeakPtrFactory<SystemMemoryPressureEvaluator> weak_ptr_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(SystemMemoryPressureEvaluator);
};

}  // namespace win
}  // namespace util

#endif  // BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_WIN_H_
