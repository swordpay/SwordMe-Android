// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_CHROMEOS_H_
#define BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_CHROMEOS_H_

#include <vector>

#include "base/base_export.h"
#include "base/feature_list.h"
#include "base/files/scoped_file.h"
#include "base/macros.h"
#include "base/memory/memory_pressure_listener.h"
#include "base/memory/weak_ptr.h"
#include "base/process/process_metrics.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "base/util/memory_pressure/memory_pressure_voter.h"
#include "base/util/memory_pressure/system_memory_pressure_evaluator.h"

namespace util {
namespace chromeos {

extern const base::Feature kCrOSUserSpaceLowMemoryNotification;

// SystemMemoryPressureEvaluator
//
// A class to handle the observation of our free memory. It notifies the
// MemoryPressureListener of memory fill level changes, so that it can take
// action to reduce memory resources accordingly.
class SystemMemoryPressureEvaluator
    : public util::SystemMemoryPressureEvaluator {
 public:






  explicit SystemMemoryPressureEvaluator(
      std::unique_ptr<MemoryPressureVoter> voter);
  ~SystemMemoryPressureEvaluator() override;




  static std::vector<int> GetMarginFileParts();

  uint64_t GetAvailableMemoryKB();


  static bool SupportsKernelNotifications();



  void ScheduleEarlyCheck();

  int ModeratePressureThresholdMBForTesting() const {
    return moderate_pressure_threshold_mb_;
  }

  int CriticalPressureThresholdMBForTesting() const {
    return critical_pressure_threshold_mb_;
  }


  void UpdateMemoryParameters();

  static SystemMemoryPressureEvaluator* Get();

 protected:

  SystemMemoryPressureEvaluator(
      const std::string& margin_file,
      const std::string& available_file,
      base::RepeatingCallback<bool(int)> kernel_waiting_callback,
      bool disable_timer_for_testing,
      bool is_user_space_notify,
      std::unique_ptr<MemoryPressureVoter> voter);

  static std::vector<int> GetMarginFileParts(const std::string& margin_file);

  static uint64_t CalculateReservedFreeKB(const std::string& zoneinfo);

  static uint64_t GetReservedMemoryKB();

  static uint64_t CalculateAvailableMemoryUserSpaceKB(
      const base::SystemMemoryInfoKB& info,
      uint64_t reserved_free,
      uint64_t min_filelist,
      uint64_t ram_swap_weight);

  void CheckMemoryPressure();

 private:
  void HandleKernelNotification(bool result);
  void ScheduleWaitForKernelNotification();
  void CheckMemoryPressureAndRecordStatistics();
  int moderate_pressure_threshold_mb_ = 0;
  int critical_pressure_threshold_mb_ = 0;


  base::TimeTicks last_moderate_notification_;


  base::TimeTicks last_pressure_level_report_;


  base::ScopedFD available_mem_file_;


  base::RepeatingTimer checking_timer_;



  base::RepeatingCallback<bool()> kernel_waiting_callback_;

  const bool is_user_space_notify_;




  uint64_t reserved_free_;
  uint64_t min_filelist_;
  uint64_t ram_swap_weight_;

  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<SystemMemoryPressureEvaluator> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(SystemMemoryPressureEvaluator);
};

}  // namespace chromeos
}  // namespace util
#endif  // BASE_UTIL_MEMORY_PRESSURE_SYSTEM_MEMORY_PRESSURE_EVALUATOR_CHROMEOS_H_
