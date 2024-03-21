// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_FIELD_TRIAL_PARAM_ASSOCIATOR_H_
#define BASE_METRICS_FIELD_TRIAL_PARAM_ASSOCIATOR_H_

#include <map>
#include <string>
#include <utility>

#include "base/base_export.h"
#include "base/memory/singleton.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/field_trial_params.h"
#include "base/synchronization/lock.h"

namespace base {

// is thread-safe.
class BASE_EXPORT FieldTrialParamAssociator {
 public:
  FieldTrialParamAssociator();
  ~FieldTrialParamAssociator();

  static FieldTrialParamAssociator* GetInstance();

  bool AssociateFieldTrialParams(const std::string& trial_name,
                                 const std::string& group_name,
                                 const FieldTrialParams& params);


  bool GetFieldTrialParams(const std::string& trial_name,
                           FieldTrialParams* params);




  bool GetFieldTrialParamsWithoutFallback(const std::string& trial_name,
                                          const std::string& group_name,
                                          FieldTrialParams* params);


  void ClearAllParamsForTesting();


  void ClearParamsForTesting(const std::string& trial_name,
                             const std::string& group_name);

  void ClearAllCachedParamsForTesting();

 private:
  friend struct DefaultSingletonTraits<FieldTrialParamAssociator>;

  typedef std::pair<std::string, std::string> FieldTrialKey;

  Lock lock_;
  std::map<FieldTrialKey, FieldTrialParams> field_trial_params_;

  DISALLOW_COPY_AND_ASSIGN(FieldTrialParamAssociator);
};

}  // namespace base

#endif  // BASE_METRICS_FIELD_TRIAL_PARAM_ASSOCIATOR_H_
