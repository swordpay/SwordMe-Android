// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FEATURE_LIST_H_
#define BASE_FEATURE_LIST_H_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/metrics/persistent_memory_allocator.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/lock.h"

namespace base {

class FieldTrial;
class FieldTrialList;

// NOTE: The actual runtime state may be different, due to a field trial or a
// command line switch.
enum FeatureState {
  FEATURE_DISABLED_BY_DEFAULT,
  FEATURE_ENABLED_BY_DEFAULT,
};

// comment below for more details. There must only ever be one struct instance
// for a given feature name - generally defined as a constant global variable or
// file static. It should never be used as a constexpr as it breaks
// pointer-based identity lookup.
struct BASE_EXPORT Feature {




  const char* const name;



  const FeatureState default_state;
};

#if defined(DCHECK_IS_CONFIGURABLE)
// DCHECKs have been built-in, and are configurable at run-time to be fatal, or
// not, via a DcheckIsFatal feature. We define the Feature here since it is
// checked in FeatureList::SetInstance(). See https://crbug.com/596231.
extern BASE_EXPORT const Feature kDCheckIsFatalFeature;
#endif  // defined(DCHECK_IS_CONFIGURABLE)

// off. It provides an authoritative answer, taking into account command-line
// overrides and experimental control.
//
// The basic use case is for any feature that can be toggled (e.g. through
// command-line or an experiment) to have a defined Feature struct, e.g.:
//
//   const base::Feature kMyGreatFeature {
//     "MyGreatFeature", base::FEATURE_ENABLED_BY_DEFAULT
//   };
//
// Then, client code that wishes to query the state of the feature would check:
//
//   if (base::FeatureList::IsEnabled(kMyGreatFeature)) {
//     // Feature code goes here.
//   }
//
// Behind the scenes, the above call would take into account any command-line
// flags to enable or disable the feature, any experiments that may control it
// and finally its default state (in that order of priority), to determine
// whether the feature is on.
//
// Features can be explicitly forced on or off by specifying a list of comma-
// separated feature names via the following command-line flags:
//
//   --enable-features=Feature5,Feature7
//   --disable-features=Feature1,Feature2,Feature3
//
// To enable/disable features in a test, do NOT append --enable-features or
// --disable-features to the command-line directly. Instead, use
// ScopedFeatureList. See base/test/scoped_feature_list.h for details.
//
// After initialization (which should be done single-threaded), the FeatureList
// API is thread safe.
//
// Note: This class is a singleton, but does not use base/memory/singleton.h in
// order to have control over its initialization sequence. Specifically, the
// intended use is to create an instance of this class and fully initialize it,
// before setting it as the singleton for a process, via SetInstance().
class BASE_EXPORT FeatureList {
 public:
  FeatureList();
  ~FeatureList();


  class BASE_EXPORT ScopedDisallowOverrides {
   public:
    explicit ScopedDisallowOverrides(const char* reason);
    ~ScopedDisallowOverrides();

   private:
#if DCHECK_IS_ON()
    const char* const previous_reason_;
#endif

    DISALLOW_COPY_AND_ASSIGN(ScopedDisallowOverrides);
  };

  enum OverrideState {
    OVERRIDE_USE_DEFAULT,
    OVERRIDE_DISABLE_FEATURE,
    OVERRIDE_ENABLE_FEATURE,
  };


  using FeatureOverrideInfo =
      std::pair<const std::reference_wrapper<const Feature>, OverrideState>;










  void InitializeFromCommandLine(const std::string& enable_features,
                                 const std::string& disable_features);



  void InitializeFromSharedMemory(PersistentMemoryAllocator* allocator);




  bool IsFeatureOverriddenFromCommandLine(const std::string& feature_name,
                                          OverrideState state) const;





  void AssociateReportingFieldTrial(const std::string& feature_name,
                                    OverrideState for_overridden_state,
                                    FieldTrial* field_trial);







  void RegisterFieldTrialOverride(const std::string& feature_name,
                                  OverrideState override_state,
                                  FieldTrial* field_trial);






  void RegisterExtraFeatureOverrides(
      const std::vector<FeatureOverrideInfo>& extra_overrides);

  void AddFeaturesToAllocator(PersistentMemoryAllocator* allocator);








  void GetFeatureOverrides(std::string* enable_overrides,
                           std::string* disable_overrides);


  void GetCommandLineFeatureOverrides(std::string* enable_overrides,
                                      std::string* disable_overrides);




  static bool IsEnabled(const Feature& feature);


  static FieldTrial* GetFieldTrial(const Feature& feature);


  static std::vector<base::StringPiece> SplitFeatureListString(
      base::StringPiece input);





  static bool InitializeInstance(const std::string& enable_features,
                                 const std::string& disable_features);



  static bool InitializeInstance(
      const std::string& enable_features,
      const std::string& disable_features,
      const std::vector<FeatureOverrideInfo>& extra_overrides);


  static FeatureList* GetInstance();




  static void SetInstance(std::unique_ptr<FeatureList> instance);




  static std::unique_ptr<FeatureList> ClearInstanceForTesting();



  static void RestoreInstanceForTesting(std::unique_ptr<FeatureList> instance);

 private:
  FRIEND_TEST_ALL_PREFIXES(FeatureListTest, CheckFeatureIdentity);
  FRIEND_TEST_ALL_PREFIXES(FeatureListTest,
                           StoreAndRetrieveFeaturesFromSharedMemory);
  FRIEND_TEST_ALL_PREFIXES(FeatureListTest,
                           StoreAndRetrieveAssociatedFeaturesFromSharedMemory);

  struct OverrideEntry {

    const OverrideState overridden_state;



    base::FieldTrial* field_trial;




    const bool overridden_by_field_trial;




    OverrideEntry(OverrideState overridden_state, FieldTrial* field_trial);
  };



  void FinalizeInitialization();



  bool IsFeatureEnabled(const Feature& feature);




  base::FieldTrial* GetAssociatedFieldTrial(const Feature& feature);




  void RegisterOverridesFromCommandLine(const std::string& feature_list,
                                        OverrideState overridden_state);







  void RegisterOverride(StringPiece feature_name,
                        OverrideState overridden_state,
                        FieldTrial* field_trial);



  void GetFeatureOverridesImpl(std::string* enable_overrides,
                               std::string* disable_overrides,
                               bool command_line_only);





  bool CheckFeatureIdentity(const Feature& feature);


  std::map<std::string, OverrideEntry, std::less<>> overrides_;



  Lock feature_identity_tracker_lock_;
  std::map<std::string, const Feature*> feature_identity_tracker_;




  base::FieldTrialList* field_trial_list_ = nullptr;


  bool initialized_ = false;

  bool initialized_from_command_line_ = false;

  DISALLOW_COPY_AND_ASSIGN(FeatureList);
};

}  // namespace base

#endif  // BASE_FEATURE_LIST_H_
