// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// performed by actual users in the field (i.e., in a shipped or beta product).
// All code is called exclusively on the UI thread currently.
//
// The simplest example is an experiment to see whether one of two options
// produces "better" results across our user population.  In that scenario, UMA
// data is uploaded to aggregate the test results, and this FieldTrial class
// manages the state of each such experiment (state == which option was
// pseudo-randomly selected).
//
// States are typically generated randomly, either based on a one time
// randomization (which will yield the same results, in terms of selecting
// the client for a field trial or not, for every run of the program on a
// given machine), or by a session randomization (generated each time the
// application starts up, but held constant during the duration of the
// process).

// Example:  Suppose we have an experiment involving memory, such as determining
// the impact of some pruning algorithm.
// We assume that we already have a histogram of memory usage, such as:


// instance of a FieldTrial, with code such as:

// // process teardown, courtesy of their automatic registration in
// // FieldTrialList.
// // Note: This field trial will run in Chrome instances compiled through
// //       8 July, 2015, and after that all instances will be in "StandardMem".
// scoped_refptr<base::FieldTrial> trial(
//     base::FieldTrialList::FactoryGetFieldTrial(
//         "MemoryExperiment", 1000, "StandardMem",
//         base::FieldTrial::ONE_TIME_RANDOMIZED, nullptr));
//
// const int high_mem_group =
//     trial->AppendGroup("HighMem", 20);  // 2% in HighMem group.
// const int low_mem_group =
//     trial->AppendGroup("LowMem", 20);   // 2% in LowMem group.
// // Take action depending of which group we randomly land in.
// if (trial->group() == high_mem_group)
//   SetPruningAlgorithm(kType1);  // Sample setting of browser state.
// else if (trial->group() == low_mem_group)
//   SetPruningAlgorithm(kType2);  // Sample alternate setting.


#ifndef BASE_METRICS_FIELD_TRIAL_H_
#define BASE_METRICS_FIELD_TRIAL_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/read_only_shared_memory_region.h"
#include "base/memory/ref_counted.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/metrics/persistent_memory_allocator.h"
#include "base/observer_list_threadsafe.h"
#include "base/pickle.h"
#include "base/process/launch.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/lock.h"
#include "build/build_config.h"

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include "base/mac/mach_port_rendezvous.h"
#endif

namespace base {

class FieldTrialList;

class BASE_EXPORT FieldTrial : public RefCounted<FieldTrial> {
 public:
  typedef int Probability;  // Probability type for being selected in a trial.

  enum RandomizationType {



    ONE_TIME_RANDOMIZED,


    SESSION_RANDOMIZED,
  };


  class BASE_EXPORT EntropyProvider {
   public:
    virtual ~EntropyProvider();





    virtual double GetEntropyForTrial(const std::string& trial_name,
                                      uint32_t randomization_seed) const = 0;
  };

  struct ActiveGroup {
    std::string trial_name;
    std::string group_name;
  };




  struct BASE_EXPORT State {
    const std::string* trial_name = nullptr;
    const std::string* group_name = nullptr;
    bool activated = false;

    State();
    State(const State& other);
    ~State();
  };



  struct BASE_EXPORT FieldTrialEntry {

    static constexpr uint32_t kPersistentTypeId = 0xABA17E13 + 2;

    static constexpr size_t kExpectedInstanceSize = 8;





    subtle::Atomic32 activated;

    uint32_t pickle_size;



    bool GetTrialAndGroupName(StringPiece* trial_name,
                              StringPiece* group_name) const;



    bool GetParams(std::map<std::string, std::string>* params) const;

   private:

    PickleIterator GetPickleIterator() const;


    bool ReadStringPair(PickleIterator* iter,
                        StringPiece* trial_name,
                        StringPiece* group_name) const;
  };

  typedef std::vector<ActiveGroup> ActiveGroups;


  static const int kNotFinalized;







  void Disable();




  int AppendGroup(const std::string& name, Probability group_probability);

  const std::string& trial_name() const { return trial_name_; }





  int group();


  const std::string& group_name();



  const std::string& GetGroupNameWithoutActivation();








  void SetForced();

  static void EnableBenchmarking();










  static FieldTrial* CreateSimulatedFieldTrial(
      const std::string& trial_name,
      Probability total_probability,
      const std::string& default_group_name,
      double entropy_value);

 private:

  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, Registration);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, AbsoluteProbabilities);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, RemainingProbability);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, FiftyFiftyProbability);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, MiddleProbabilities);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, OneWinner);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, DisableProbability);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, ActiveGroups);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, AllGroups);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, ActiveGroupsNotFinalized);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, Save);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, SaveAll);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, DuplicateRestore);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, SetForcedTurnFeatureOff);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, SetForcedTurnFeatureOn);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, SetForcedChangeDefault_Default);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, SetForcedChangeDefault_NonDefault);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, FloatBoundariesGiveEqualGroupSizes);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialTest, DoesNotSurpassTotalProbability);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest,
                           DoNotAddSimulatedFieldTrialsToAllocator);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest, ClearParamsFromSharedMemory);

  friend class base::FieldTrialList;

  friend class RefCounted<FieldTrial>;

  using FieldTrialRef = PersistentMemoryAllocator::Reference;



  static const int kDefaultGroupNumber;


  FieldTrial(const std::string& trial_name,
             Probability total_probability,
             const std::string& default_group_name,
             double entropy_value);
  virtual ~FieldTrial();

  std::string default_group_name() const { return default_group_name_; }


  void SetTrialRegistered();

  void SetGroupChoice(const std::string& group_name, int number);



  void FinalizeGroupChoice();


  void FinalizeGroupChoiceImpl(bool is_locked);






  bool GetActiveGroup(ActiveGroup* active_group) const;






  bool GetStateWhileLocked(State* field_trial_state, bool include_disabled);

  std::string group_name_internal() const { return group_name_; }

  const std::string trial_name_;


  const Probability divisor_;

  const std::string default_group_name_;



  Probability random_;

  Probability accumulated_group_probability_;

  int next_group_number_;


  int group_;


  std::string group_name_;


  bool enable_field_trial_;


  bool forced_;

  bool group_reported_;


  bool trial_registered_;

  FieldTrialRef ref_;


  static bool enable_benchmarking_;

  DISALLOW_COPY_AND_ASSIGN(FieldTrial);
};

// Class with a list of all active field trials.  A trial is active if it has
// been registered, which includes evaluating its state based on its probaility.
// Only one instance of this class exists and outside of testing, will live for
// the entire life time of the process.
class BASE_EXPORT FieldTrialList {
 public:
  using FieldTrialAllocator = PersistentMemoryAllocator;


  typedef std::string (*EscapeDataFunc)(const std::string& input);

  class BASE_EXPORT Observer {
   public:

    virtual void OnFieldTrialGroupFinalized(const std::string& trial_name,
                                            const std::string& group_name) = 0;

   protected:
    virtual ~Observer();
  };






  explicit FieldTrialList(
      std::unique_ptr<const FieldTrial::EntropyProvider> entropy_provider);

  ~FieldTrialList();
















  static FieldTrial* FactoryGetFieldTrial(
      const std::string& trial_name,
      FieldTrial::Probability total_probability,
      const std::string& default_group_name,
      FieldTrial::RandomizationType randomization_type,
      int* default_group_number);











  static FieldTrial* FactoryGetFieldTrialWithRandomizationSeed(
      const std::string& trial_name,
      FieldTrial::Probability total_probability,
      const std::string& default_group_name,
      FieldTrial::RandomizationType randomization_type,
      uint32_t randomization_seed,
      int* default_group_number,
      const FieldTrial::EntropyProvider* override_entropy_provider);


  static FieldTrial* Find(const std::string& trial_name);


  static int FindValue(const std::string& trial_name);






  static std::string FindFullName(const std::string& trial_name);

  static bool TrialExists(const std::string& trial_name);

  static bool IsTrialActive(const std::string& trial_name);








  static void StatesToString(std::string* output);








  static void AllStatesToString(std::string* output, bool include_disabled);







  static std::string AllParamsToString(bool include_disabled,
                                       EscapeDataFunc encode_data_func);




  static void GetActiveFieldTrialGroups(
      FieldTrial::ActiveGroups* active_groups);

  static void GetActiveFieldTrialGroupsFromString(
      const std::string& trials_string,
      FieldTrial::ActiveGroups* active_groups);




  static void GetInitiallyActiveFieldTrials(
      const CommandLine& command_line,
      FieldTrial::ActiveGroups* active_groups);









  static bool CreateTrialsFromString(
      const std::string& trials_string,
      const std::set<std::string>& ignored_trial_names);








  static void CreateTrialsFromCommandLine(const CommandLine& cmd_line,
                                          const char* field_trial_handle_switch,
                                          int fd_key);


  static void CreateFeaturesFromCommandLine(const CommandLine& command_line,
                                            const char* enable_features_switch,
                                            const char* disable_features_switch,
                                            FeatureList* feature_list);

#if defined(OS_WIN)



  static void AppendFieldTrialHandleIfNeeded(HandlesToInheritVector* handles);
#elif defined(OS_FUCHSIA)

#elif defined(OS_MACOSX) && !defined(OS_IOS)


  static void InsertFieldTrialHandleIfNeeded(
      MachPortsForRendezvous* rendezvous_ports);
#elif defined(OS_POSIX) && !defined(OS_NACL)




  static int GetFieldTrialDescriptor();
#endif
  static ReadOnlySharedMemoryRegion DuplicateFieldTrialSharedMemoryForTesting();





  static void CopyFieldTrialStateToFlags(const char* field_trial_handle_switch,
                                         const char* enable_features_switch,
                                         const char* disable_features_switch,
                                         CommandLine* cmd_line);






  static FieldTrial* CreateFieldTrial(const std::string& name,
                                      const std::string& group_name);




  static bool AddObserver(Observer* observer);

  static void RemoveObserver(Observer* observer);









  static void SetSynchronousObserver(Observer* observer);

  static void RemoveSynchronousObserver(Observer* observer);


  static void OnGroupFinalized(bool is_locked, FieldTrial* field_trial);

  static void NotifyFieldTrialGroupSelection(FieldTrial* field_trial);

  static size_t GetFieldTrialCount();



  static bool GetParamsFromSharedMemory(
      FieldTrial* field_trial,
      std::map<std::string, std::string>* params);

  static void ClearParamsFromSharedMemoryForTesting();


  static void DumpAllFieldTrialsToPersistentAllocator(
      PersistentMemoryAllocator* allocator);



  static std::vector<const FieldTrial::FieldTrialEntry*>
  GetAllFieldTrialsFromPersistentAllocator(
      PersistentMemoryAllocator const& allocator);



  static FieldTrialList* GetInstance();

  static FieldTrialList* BackupInstanceForTesting();

  static void RestoreInstanceForTesting(FieldTrialList* instance);

 private:

  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest, InstantiateAllocator);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest, AddTrialsToAllocator);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest,
                           DoNotAddSimulatedFieldTrialsToAllocator);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest, AssociateFieldTrialParams);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest, ClearParamsFromSharedMemory);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest,
                           SerializeSharedMemoryRegionMetadata);
  friend int SerializeSharedMemoryRegionMetadata(void);
  FRIEND_TEST_ALL_PREFIXES(FieldTrialListTest, CheckReadOnlySharedMemoryRegion);




  static std::string SerializeSharedMemoryRegionMetadata(
      const ReadOnlySharedMemoryRegion& shm);
#if defined(OS_WIN) || defined(OS_FUCHSIA) || \
    (defined(OS_MACOSX) && !defined(OS_IOS))
  static ReadOnlySharedMemoryRegion DeserializeSharedMemoryRegionMetadata(
      const std::string& switch_value);
#elif defined(OS_POSIX) && !defined(OS_NACL)
  static ReadOnlySharedMemoryRegion DeserializeSharedMemoryRegionMetadata(
      int fd,
      const std::string& switch_value);
#endif

#if defined(OS_WIN) || defined(OS_FUCHSIA) || \
    (defined(OS_MACOSX) && !defined(OS_IOS))




  static bool CreateTrialsFromSwitchValue(const std::string& switch_value);
#elif defined(OS_POSIX) && !defined(OS_NACL)




  static bool CreateTrialsFromDescriptor(int fd_key,
                                         const std::string& switch_value);
#endif



  static bool CreateTrialsFromSharedMemoryRegion(
      const ReadOnlySharedMemoryRegion& shm_region);





  static bool CreateTrialsFromSharedMemoryMapping(
      ReadOnlySharedMemoryMapping shm_mapping);



  static void InstantiateFieldTrialAllocatorIfNeeded();


  static void AddToAllocatorWhileLocked(PersistentMemoryAllocator* allocator,
                                        FieldTrial* field_trial);

  static void ActivateFieldTrialEntryWhileLocked(FieldTrial* field_trial);

  typedef std::map<std::string, FieldTrial*> RegistrationMap;


  static const FieldTrial::EntropyProvider*
      GetEntropyProviderForOneTimeRandomization();

  FieldTrial* PreLockedFind(const std::string& name);



  static void Register(FieldTrial* trial);

  static RegistrationMap GetRegisteredTrials();

  static FieldTrialList* global_;  // The singleton of this class.




  static bool used_without_global_;

  Lock lock_;
  RegistrationMap registered_;

  std::map<std::string, std::string> seen_states_;


  std::unique_ptr<const FieldTrial::EntropyProvider> entropy_provider_;

  scoped_refptr<ObserverListThreadSafe<Observer> > observer_list_;

  Observer* synchronous_observer_ = nullptr;




  std::unique_ptr<FieldTrialAllocator> field_trial_allocator_ = nullptr;



  ReadOnlySharedMemoryRegion readonly_allocator_region_;

  bool create_trials_from_command_line_called_ = false;

  DISALLOW_COPY_AND_ASSIGN(FieldTrialList);
};

}  // namespace base

#endif  // BASE_METRICS_FIELD_TRIAL_H_
