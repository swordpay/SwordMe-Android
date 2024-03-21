// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/field_trial.h"

#include <algorithm>
#include <utility>

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/debug/activity_tracker.h"
#include "base/logging.h"
#include "base/metrics/field_trial_param_associator.h"
#include "base/metrics/histogram_macros.h"
#include "base/process/memory.h"
#include "base/process/process_handle.h"
#include "base/process/process_info.h"
#include "base/rand_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/unguessable_token.h"

#if defined(OS_POSIX) && !defined(OS_NACL)
#include "base/posix/global_descriptors.h"
#endif

namespace base {

namespace {

// instance.  This is intended for use as a command line argument, passed to a
// second process to mimic our state (i.e., provide the same group name).
const char kPersistentStringSeparator = '/';  // Currently a slash.

// command line which forces its activation.
const char kActivationMarker = '*';

const char kAllocatorName[] = "FieldTrialAllocator";

// as most people use 3 - 25 KiB for field trials (as of 11/25/2016).
// This also doesn't allocate all 128 KiB at once -- the pages only get mapped
// to physical memory when they are touched. If the size of the allocated field
// trials does get larger than 128 KiB, then we will drop some field trials in
// child processes, leading to an inconsistent view between browser and child
// processes and possibly causing crashes (see crbug.com/661617).
const size_t kFieldTrialAllocationSize = 128 << 10;  // 128 KiB

#if defined(OS_MACOSX) && !defined(OS_IOS)
constexpr MachPortsForRendezvous::key_type kFieldTrialRendezvousKey = 'fldt';
#endif

void WriteStringPair(Pickle* pickle,
                     const StringPiece& string1,
                     const StringPiece& string2) {
  pickle->WriteString(string1);
  pickle->WriteString(string2);
}

// format of the pickle looks like:
// TrialName, GroupName, ParamKey1, ParamValue1, ParamKey2, ParamValue2, ...
// If there are no parameters, then it just ends at GroupName.
void PickleFieldTrial(const FieldTrial::State& trial_state, Pickle* pickle) {
  WriteStringPair(pickle, *trial_state.trial_name, *trial_state.group_name);

  std::map<std::string, std::string> params;
  FieldTrialParamAssociator::GetInstance()->GetFieldTrialParamsWithoutFallback(
      *trial_state.trial_name, *trial_state.group_name, &params);

  for (const auto& param : params)
    WriteStringPair(pickle, param.first, param.second);
}

// groups for a given |divisor| (total probability) and |entropy_value|.
FieldTrial::Probability GetGroupBoundaryValue(
    FieldTrial::Probability divisor,
    double entropy_value) {







  const double kEpsilon = 1e-8;
  const FieldTrial::Probability result =
      static_cast<FieldTrial::Probability>(divisor * entropy_value + kEpsilon);

  return std::min(result, divisor - 1);
}

struct FieldTrialStringEntry {
  StringPiece trial_name;
  StringPiece group_name;
  bool activated = false;
};

// Returns true if the string was parsed correctly. On failure, the |entries|
// array may end up being partially filled.
bool ParseFieldTrialsString(const std::string& trials_string,
                            std::vector<FieldTrialStringEntry>* entries) {
  const StringPiece trials_string_piece(trials_string);

  size_t next_item = 0;
  while (next_item < trials_string.length()) {
    size_t name_end = trials_string.find(kPersistentStringSeparator, next_item);
    if (name_end == trials_string.npos || next_item == name_end)
      return false;
    size_t group_name_end =
        trials_string.find(kPersistentStringSeparator, name_end + 1);
    if (name_end + 1 == group_name_end)
      return false;
    if (group_name_end == trials_string.npos)
      group_name_end = trials_string.length();

    FieldTrialStringEntry entry;

    if (trials_string[next_item] == kActivationMarker) {

      if (name_end - next_item == 1)
        return false;
      next_item++;
      entry.activated = true;
    }
    entry.trial_name =
        trials_string_piece.substr(next_item, name_end - next_item);
    entry.group_name =
        trials_string_piece.substr(name_end + 1, group_name_end - name_end - 1);
    next_item = group_name_end + 1;

    entries->push_back(std::move(entry));
  }
  return true;
}

void AddFeatureAndFieldTrialFlags(const char* enable_features_switch,
                                  const char* disable_features_switch,
                                  CommandLine* cmd_line) {
  std::string enabled_features;
  std::string disabled_features;
  FeatureList::GetInstance()->GetFeatureOverrides(&enabled_features,
                                                  &disabled_features);

  if (!enabled_features.empty())
    cmd_line->AppendSwitchASCII(enable_features_switch, enabled_features);
  if (!disabled_features.empty())
    cmd_line->AppendSwitchASCII(disable_features_switch, disabled_features);

  std::string field_trial_states;
  FieldTrialList::AllStatesToString(&field_trial_states, false);
  if (!field_trial_states.empty()) {
    cmd_line->AppendSwitchASCII(switches::kForceFieldTrials,
                                field_trial_states);
  }
}

void OnOutOfMemory(size_t size) {
#if defined(OS_NACL)
  NOTREACHED();
#else
  TerminateBecauseOutOfMemory(size);
#endif
}

#if !defined(OS_NACL)
// Returns whether the operation succeeded.
bool DeserializeGUIDFromStringPieces(StringPiece first,
                                     StringPiece second,
                                     UnguessableToken* guid) {
  uint64_t high = 0;
  uint64_t low = 0;
  if (!StringToUint64(first, &high) || !StringToUint64(second, &low))
    return false;

  *guid = UnguessableToken::Deserialize(high, low);
  return true;
}
#endif  // !defined(OS_NACL)

}  // namespace

const int FieldTrial::kNotFinalized = -1;
const int FieldTrial::kDefaultGroupNumber = 0;
bool FieldTrial::enable_benchmarking_ = false;

// FieldTrial methods and members.

FieldTrial::EntropyProvider::~EntropyProvider() = default;

FieldTrial::State::State() = default;

FieldTrial::State::State(const State& other) = default;

FieldTrial::State::~State() = default;

bool FieldTrial::FieldTrialEntry::GetTrialAndGroupName(
    StringPiece* trial_name,
    StringPiece* group_name) const {
  PickleIterator iter = GetPickleIterator();
  return ReadStringPair(&iter, trial_name, group_name);
}

bool FieldTrial::FieldTrialEntry::GetParams(
    std::map<std::string, std::string>* params) const {
  PickleIterator iter = GetPickleIterator();
  StringPiece tmp;

  if (!ReadStringPair(&iter, &tmp, &tmp))
    return false;

  while (true) {
    StringPiece key;
    StringPiece value;
    if (!ReadStringPair(&iter, &key, &value))
      return key.empty();  // Non-empty is bad: got one of a pair.
    (*params)[key.as_string()] = value.as_string();
  }
}

PickleIterator FieldTrial::FieldTrialEntry::GetPickleIterator() const {
  const char* src =
      reinterpret_cast<const char*>(this) + sizeof(FieldTrialEntry);

  Pickle pickle(src, pickle_size);
  return PickleIterator(pickle);
}

bool FieldTrial::FieldTrialEntry::ReadStringPair(
    PickleIterator* iter,
    StringPiece* trial_name,
    StringPiece* group_name) const {
  if (!iter->ReadStringPiece(trial_name))
    return false;
  if (!iter->ReadStringPiece(group_name))
    return false;
  return true;
}

void FieldTrial::Disable() {
  DCHECK(!group_reported_);
  enable_field_trial_ = false;


  if (group_ != kNotFinalized) {



    if (group_name_ != default_group_name_)
      SetGroupChoice(default_group_name_, kDefaultGroupNumber);
  }
}

int FieldTrial::AppendGroup(const std::string& name,
                            Probability group_probability) {


  if (forced_) {
    DCHECK(!group_name_.empty());
    if (name == group_name_) {




      return group_;
    }
    DCHECK_NE(next_group_number_, group_);


    return next_group_number_++;
  }

  DCHECK_LE(group_probability, divisor_);
  DCHECK_GE(group_probability, 0);

  if (enable_benchmarking_ || !enable_field_trial_)
    group_probability = 0;

  accumulated_group_probability_ += group_probability;

  DCHECK_LE(accumulated_group_probability_, divisor_);
  if (group_ == kNotFinalized && accumulated_group_probability_ > random_) {

    SetGroupChoice(name, next_group_number_);
  }
  return next_group_number_++;
}

int FieldTrial::group() {
  FinalizeGroupChoice();
  if (trial_registered_)
    FieldTrialList::NotifyFieldTrialGroupSelection(this);
  return group_;
}

const std::string& FieldTrial::group_name() {

  group();
  DCHECK(!group_name_.empty());
  return group_name_;
}

const std::string& FieldTrial::GetGroupNameWithoutActivation() {
  FinalizeGroupChoice();
  return group_name_;
}

void FieldTrial::SetForced() {


  if (forced_)
    return;

  FinalizeGroupChoice();
  forced_ = true;
}

void FieldTrial::EnableBenchmarking() {
  DCHECK_EQ(0u, FieldTrialList::GetFieldTrialCount());
  enable_benchmarking_ = true;
}

FieldTrial* FieldTrial::CreateSimulatedFieldTrial(
    const std::string& trial_name,
    Probability total_probability,
    const std::string& default_group_name,
    double entropy_value) {
  return new FieldTrial(trial_name, total_probability, default_group_name,
                        entropy_value);
}

FieldTrial::FieldTrial(const std::string& trial_name,
                       const Probability total_probability,
                       const std::string& default_group_name,
                       double entropy_value)
    : trial_name_(trial_name),
      divisor_(total_probability),
      default_group_name_(default_group_name),
      random_(GetGroupBoundaryValue(total_probability, entropy_value)),
      accumulated_group_probability_(0),
      next_group_number_(kDefaultGroupNumber + 1),
      group_(kNotFinalized),
      enable_field_trial_(true),
      forced_(false),
      group_reported_(false),
      trial_registered_(false),
      ref_(FieldTrialList::FieldTrialAllocator::kReferenceNull) {
  DCHECK_GT(total_probability, 0);
  DCHECK(!trial_name_.empty());
  DCHECK(!default_group_name_.empty())
      << "Trial " << trial_name << " is missing a default group name.";
}

FieldTrial::~FieldTrial() = default;

void FieldTrial::SetTrialRegistered() {
  DCHECK_EQ(kNotFinalized, group_);
  DCHECK(!trial_registered_);
  trial_registered_ = true;
}

void FieldTrial::SetGroupChoice(const std::string& group_name, int number) {
  group_ = number;
  if (group_name.empty())
    StringAppendF(&group_name_, "%d", group_);
  else
    group_name_ = group_name;
  DVLOG(1) << "Field trial: " << trial_name_ << " Group choice:" << group_name_;
}

void FieldTrial::FinalizeGroupChoice() {
  FinalizeGroupChoiceImpl(false);
}

void FieldTrial::FinalizeGroupChoiceImpl(bool is_locked) {
  if (group_ != kNotFinalized)
    return;
  accumulated_group_probability_ = divisor_;


  DCHECK(!forced_);
  SetGroupChoice(default_group_name_, kDefaultGroupNumber);

  if (trial_registered_)
    FieldTrialList::OnGroupFinalized(is_locked, this);
}

bool FieldTrial::GetActiveGroup(ActiveGroup* active_group) const {
  if (!group_reported_ || !enable_field_trial_)
    return false;
  DCHECK_NE(group_, kNotFinalized);
  active_group->trial_name = trial_name_;
  active_group->group_name = group_name_;
  return true;
}

bool FieldTrial::GetStateWhileLocked(State* field_trial_state,
                                     bool include_disabled) {
  if (!include_disabled && !enable_field_trial_)
    return false;
  FinalizeGroupChoiceImpl(true);
  field_trial_state->trial_name = &trial_name_;
  field_trial_state->group_name = &group_name_;
  field_trial_state->activated = group_reported_;
  return true;
}

// FieldTrialList methods and members.

FieldTrialList* FieldTrialList::global_ = nullptr;

bool FieldTrialList::used_without_global_ = false;

FieldTrialList::Observer::~Observer() = default;

FieldTrialList::FieldTrialList(
    std::unique_ptr<const FieldTrial::EntropyProvider> entropy_provider)
    : entropy_provider_(std::move(entropy_provider)),
      observer_list_(new ObserverListThreadSafe<FieldTrialList::Observer>(
          ObserverListPolicy::EXISTING_ONLY)) {
  DCHECK(!global_);
  DCHECK(!used_without_global_);
  global_ = this;
}

FieldTrialList::~FieldTrialList() {
  AutoLock auto_lock(lock_);
  while (!registered_.empty()) {
    auto it = registered_.begin();
    it->second->Release();
    registered_.erase(it->first);
  }



  DCHECK_EQ(this, global_);
  global_ = nullptr;
}

FieldTrial* FieldTrialList::FactoryGetFieldTrial(
    const std::string& trial_name,
    FieldTrial::Probability total_probability,
    const std::string& default_group_name,
    FieldTrial::RandomizationType randomization_type,
    int* default_group_number) {
  return FactoryGetFieldTrialWithRandomizationSeed(
      trial_name, total_probability, default_group_name, randomization_type, 0,
      default_group_number, nullptr);
}

FieldTrial* FieldTrialList::FactoryGetFieldTrialWithRandomizationSeed(
    const std::string& trial_name,
    FieldTrial::Probability total_probability,
    const std::string& default_group_name,
    FieldTrial::RandomizationType randomization_type,
    uint32_t randomization_seed,
    int* default_group_number,
    const FieldTrial::EntropyProvider* override_entropy_provider) {
  if (default_group_number)
    *default_group_number = FieldTrial::kDefaultGroupNumber;

  FieldTrial* existing_trial = Find(trial_name);
  if (existing_trial) {
    CHECK(existing_trial->forced_);


    if (default_group_number &&
        default_group_name != existing_trial->default_group_name()) {



      if (default_group_name == existing_trial->group_name_internal()) {
        *default_group_number = existing_trial->group_;
      } else {



        const int kNonConflictingGroupNumber = -2;
        static_assert(
            kNonConflictingGroupNumber != FieldTrial::kDefaultGroupNumber,
            "The 'non-conflicting' group number conflicts");
        static_assert(kNonConflictingGroupNumber != FieldTrial::kNotFinalized,
                      "The 'non-conflicting' group number conflicts");
        *default_group_number = kNonConflictingGroupNumber;
      }
    }
    return existing_trial;
  }

  double entropy_value;
  if (randomization_type == FieldTrial::ONE_TIME_RANDOMIZED) {

    const FieldTrial::EntropyProvider* entropy_provider =
        override_entropy_provider ? override_entropy_provider
                                  : GetEntropyProviderForOneTimeRandomization();
    CHECK(entropy_provider);
    entropy_value = entropy_provider->GetEntropyForTrial(trial_name,
                                                         randomization_seed);
  } else {
    DCHECK_EQ(FieldTrial::SESSION_RANDOMIZED, randomization_type);
    DCHECK_EQ(0U, randomization_seed);
    entropy_value = RandDouble();
  }

  FieldTrial* field_trial = new FieldTrial(trial_name, total_probability,
                                           default_group_name, entropy_value);
  FieldTrialList::Register(field_trial);
  return field_trial;
}

FieldTrial* FieldTrialList::Find(const std::string& trial_name) {
  if (!global_)
    return nullptr;
  AutoLock auto_lock(global_->lock_);
  return global_->PreLockedFind(trial_name);
}

int FieldTrialList::FindValue(const std::string& trial_name) {
  FieldTrial* field_trial = Find(trial_name);
  if (field_trial)
    return field_trial->group();
  return FieldTrial::kNotFinalized;
}

std::string FieldTrialList::FindFullName(const std::string& trial_name) {
  FieldTrial* field_trial = Find(trial_name);
  if (field_trial)
    return field_trial->group_name();
  return std::string();
}

bool FieldTrialList::TrialExists(const std::string& trial_name) {
  return Find(trial_name) != nullptr;
}

bool FieldTrialList::IsTrialActive(const std::string& trial_name) {
  FieldTrial* field_trial = Find(trial_name);
  FieldTrial::ActiveGroup active_group;
  return field_trial && field_trial->GetActiveGroup(&active_group);
}

void FieldTrialList::StatesToString(std::string* output) {
  FieldTrial::ActiveGroups active_groups;
  GetActiveFieldTrialGroups(&active_groups);
  for (const auto& active_group : active_groups) {
    DCHECK_EQ(std::string::npos,
              active_group.trial_name.find(kPersistentStringSeparator));
    DCHECK_EQ(std::string::npos,
              active_group.group_name.find(kPersistentStringSeparator));
    output->append(active_group.trial_name);
    output->append(1, kPersistentStringSeparator);
    output->append(active_group.group_name);
    output->append(1, kPersistentStringSeparator);
  }
}

void FieldTrialList::AllStatesToString(std::string* output,
                                       bool include_disabled) {
  if (!global_)
    return;
  AutoLock auto_lock(global_->lock_);

  for (const auto& registered : global_->registered_) {
    FieldTrial::State trial;
    if (!registered.second->GetStateWhileLocked(&trial, include_disabled))
      continue;
    DCHECK_EQ(std::string::npos,
              trial.trial_name->find(kPersistentStringSeparator));
    DCHECK_EQ(std::string::npos,
              trial.group_name->find(kPersistentStringSeparator));
    if (trial.activated)
      output->append(1, kActivationMarker);
    output->append(*trial.trial_name);
    output->append(1, kPersistentStringSeparator);
    output->append(*trial.group_name);
    output->append(1, kPersistentStringSeparator);
  }
}

std::string FieldTrialList::AllParamsToString(bool include_disabled,
                                              EscapeDataFunc encode_data_func) {
  FieldTrialParamAssociator* params_associator =
      FieldTrialParamAssociator::GetInstance();
  std::string output;
  for (const auto& registered : GetRegisteredTrials()) {
    FieldTrial::State trial;
    if (!registered.second->GetStateWhileLocked(&trial, include_disabled))
      continue;
    DCHECK_EQ(std::string::npos,
              trial.trial_name->find(kPersistentStringSeparator));
    DCHECK_EQ(std::string::npos,
              trial.group_name->find(kPersistentStringSeparator));
    std::map<std::string, std::string> params;
    if (params_associator->GetFieldTrialParamsWithoutFallback(
            *trial.trial_name, *trial.group_name, &params)) {
      if (params.size() > 0) {

        if (!output.empty())
          output.append(1, ',');

        output.append(encode_data_func(*trial.trial_name));
        output.append(1, '.');
        output.append(encode_data_func(*trial.group_name));
        output.append(1, ':');

        std::string param_str;
        for (const auto& param : params) {

          if (!param_str.empty())
            param_str.append(1, kPersistentStringSeparator);
          param_str.append(encode_data_func(param.first));
          param_str.append(1, kPersistentStringSeparator);
          param_str.append(encode_data_func(param.second));
        }

        output.append(param_str);
      }
    }
  }
  return output;
}

void FieldTrialList::GetActiveFieldTrialGroups(
    FieldTrial::ActiveGroups* active_groups) {
  DCHECK(active_groups->empty());
  if (!global_)
    return;
  AutoLock auto_lock(global_->lock_);

  for (const auto& registered : global_->registered_) {
    FieldTrial::ActiveGroup active_group;
    if (registered.second->GetActiveGroup(&active_group))
      active_groups->push_back(active_group);
  }
}

void FieldTrialList::GetActiveFieldTrialGroupsFromString(
    const std::string& trials_string,
    FieldTrial::ActiveGroups* active_groups) {
  std::vector<FieldTrialStringEntry> entries;
  if (!ParseFieldTrialsString(trials_string, &entries))
    return;

  for (const auto& entry : entries) {
    if (entry.activated) {
      FieldTrial::ActiveGroup group;
      group.trial_name = entry.trial_name.as_string();
      group.group_name = entry.group_name.as_string();
      active_groups->push_back(group);
    }
  }
}

void FieldTrialList::GetInitiallyActiveFieldTrials(
    const CommandLine& command_line,
    FieldTrial::ActiveGroups* active_groups) {
  DCHECK(global_);
  DCHECK(global_->create_trials_from_command_line_called_);

  if (!global_->field_trial_allocator_) {
    GetActiveFieldTrialGroupsFromString(
        command_line.GetSwitchValueASCII(switches::kForceFieldTrials),
        active_groups);
    return;
  }

  FieldTrialAllocator* allocator = global_->field_trial_allocator_.get();
  FieldTrialAllocator::Iterator mem_iter(allocator);
  const FieldTrial::FieldTrialEntry* entry;
  while ((entry = mem_iter.GetNextOfObject<FieldTrial::FieldTrialEntry>()) !=
         nullptr) {
    StringPiece trial_name;
    StringPiece group_name;
    if (subtle::NoBarrier_Load(&entry->activated) &&
        entry->GetTrialAndGroupName(&trial_name, &group_name)) {
      FieldTrial::ActiveGroup group;
      group.trial_name = trial_name.as_string();
      group.group_name = group_name.as_string();
      active_groups->push_back(group);
    }
  }
}

bool FieldTrialList::CreateTrialsFromString(
    const std::string& trials_string,
    const std::set<std::string>& ignored_trial_names) {
  DCHECK(global_);
  if (trials_string.empty() || !global_)
    return true;

  std::vector<FieldTrialStringEntry> entries;
  if (!ParseFieldTrialsString(trials_string, &entries))
    return false;

  for (const auto& entry : entries) {
    const std::string trial_name = entry.trial_name.as_string();
    const std::string group_name = entry.group_name.as_string();

    if (Contains(ignored_trial_names, trial_name)) {



      LOG(WARNING) << "Field trial: " << trial_name << " cannot be forced.";
      continue;
    }

    FieldTrial* trial = CreateFieldTrial(trial_name, group_name);
    if (!trial)
      return false;
    if (entry.activated) {



      trial->group();
    }
  }
  return true;
}

void FieldTrialList::CreateTrialsFromCommandLine(
    const CommandLine& cmd_line,
    const char* field_trial_handle_switch,
    int fd_key) {
  global_->create_trials_from_command_line_called_ = true;

#if defined(OS_WIN) || defined(OS_FUCHSIA) || \
    (defined(OS_MACOSX) && !defined(OS_IOS))
  if (cmd_line.HasSwitch(field_trial_handle_switch)) {
    std::string switch_value =
        cmd_line.GetSwitchValueASCII(field_trial_handle_switch);
    bool result = CreateTrialsFromSwitchValue(switch_value);
    UMA_HISTOGRAM_BOOLEAN("ChildProcess.FieldTrials.CreateFromShmemSuccess",
                          result);
  }
#elif defined(OS_POSIX) && !defined(OS_NACL)



  if (cmd_line.HasSwitch(field_trial_handle_switch)) {
    std::string switch_value =
        cmd_line.GetSwitchValueASCII(field_trial_handle_switch);
    bool result = CreateTrialsFromDescriptor(fd_key, switch_value);
    UMA_HISTOGRAM_BOOLEAN("ChildProcess.FieldTrials.CreateFromShmemSuccess",
                          result);
    DCHECK(result);
  }
#endif

  if (cmd_line.HasSwitch(switches::kForceFieldTrials)) {
    bool result = FieldTrialList::CreateTrialsFromString(
        cmd_line.GetSwitchValueASCII(switches::kForceFieldTrials),
        std::set<std::string>());
    UMA_HISTOGRAM_BOOLEAN("ChildProcess.FieldTrials.CreateFromSwitchSuccess",
                          result);
    DCHECK(result);
  }
}

void FieldTrialList::CreateFeaturesFromCommandLine(
    const CommandLine& command_line,
    const char* enable_features_switch,
    const char* disable_features_switch,
    FeatureList* feature_list) {

  if (!global_->field_trial_allocator_.get()) {
    return feature_list->InitializeFromCommandLine(
        command_line.GetSwitchValueASCII(enable_features_switch),
        command_line.GetSwitchValueASCII(disable_features_switch));
  }

  feature_list->InitializeFromSharedMemory(
      global_->field_trial_allocator_.get());
}

#if defined(OS_WIN)
// static
void FieldTrialList::AppendFieldTrialHandleIfNeeded(
    HandlesToInheritVector* handles) {
  if (!global_)
    return;
  InstantiateFieldTrialAllocatorIfNeeded();
  if (global_->readonly_allocator_region_.IsValid())
    handles->push_back(global_->readonly_allocator_region_.GetPlatformHandle());
}
#elif defined(OS_FUCHSIA)
// TODO(fuchsia): Implement shared-memory configuration (crbug.com/752368).
#elif defined(OS_MACOSX) && !defined(OS_IOS)
// static
void FieldTrialList::InsertFieldTrialHandleIfNeeded(
    MachPortsForRendezvous* rendezvous_ports) {
  if (!global_)
    return;
  InstantiateFieldTrialAllocatorIfNeeded();
  if (global_->readonly_allocator_region_.IsValid()) {
    rendezvous_ports->emplace(
        kFieldTrialRendezvousKey,
        MachRendezvousPort(
            global_->readonly_allocator_region_.GetPlatformHandle(),
            MACH_MSG_TYPE_COPY_SEND));
  }
}
#elif defined(OS_POSIX) && !defined(OS_NACL)
// static
int FieldTrialList::GetFieldTrialDescriptor() {
  InstantiateFieldTrialAllocatorIfNeeded();
  if (!global_ || !global_->readonly_allocator_region_.IsValid())
    return -1;

#if defined(OS_ANDROID)
  return global_->readonly_allocator_region_.GetPlatformHandle();
#else
  return global_->readonly_allocator_region_.GetPlatformHandle().fd;
#endif
}
#endif

ReadOnlySharedMemoryRegion
FieldTrialList::DuplicateFieldTrialSharedMemoryForTesting() {
  if (!global_)
    return ReadOnlySharedMemoryRegion();

  return global_->readonly_allocator_region_.Duplicate();
}

void FieldTrialList::CopyFieldTrialStateToFlags(
    const char* field_trial_handle_switch,
    const char* enable_features_switch,
    const char* disable_features_switch,
    CommandLine* cmd_line) {
#if !defined(OS_FUCHSIA)  // TODO(752368): Not yet supported on Fuchsia.


  InstantiateFieldTrialAllocatorIfNeeded();
#endif  // !defined(OS_FUCHSIA)

  if (!global_ || !global_->readonly_allocator_region_.IsValid()) {
    AddFeatureAndFieldTrialFlags(enable_features_switch,
                                 disable_features_switch, cmd_line);
    return;
  }

  global_->field_trial_allocator_->UpdateTrackingHistograms();
  std::string switch_value =
      SerializeSharedMemoryRegionMetadata(global_->readonly_allocator_region_);
  cmd_line->AppendSwitchASCII(field_trial_handle_switch, switch_value);




  std::string enabled_features;
  std::string disabled_features;
  FeatureList::GetInstance()->GetCommandLineFeatureOverrides(
      &enabled_features, &disabled_features);

  if (!enabled_features.empty())
    cmd_line->AppendSwitchASCII(enable_features_switch, enabled_features);
  if (!disabled_features.empty())
    cmd_line->AppendSwitchASCII(disable_features_switch, disabled_features);
}

FieldTrial* FieldTrialList::CreateFieldTrial(
    const std::string& name,
    const std::string& group_name) {
  DCHECK(global_);
  DCHECK_GE(name.size(), 0u);
  DCHECK_GE(group_name.size(), 0u);
  if (name.empty() || group_name.empty() || !global_)
    return nullptr;

  FieldTrial* field_trial = FieldTrialList::Find(name);
  if (field_trial) {


    if (field_trial->group_name_internal() != group_name)
      return nullptr;
    return field_trial;
  }
  const int kTotalProbability = 100;
  field_trial = new FieldTrial(name, kTotalProbability, group_name, 0);
  FieldTrialList::Register(field_trial);

  field_trial->SetForced();
  return field_trial;
}

bool FieldTrialList::AddObserver(Observer* observer) {
  if (!global_)
    return false;
  global_->observer_list_->AddObserver(observer);
  return true;
}

void FieldTrialList::RemoveObserver(Observer* observer) {
  if (!global_)
    return;
  global_->observer_list_->RemoveObserver(observer);
}

void FieldTrialList::SetSynchronousObserver(Observer* observer) {
  DCHECK(!global_->synchronous_observer_);
  global_->synchronous_observer_ = observer;
}

void FieldTrialList::RemoveSynchronousObserver(Observer* observer) {
  DCHECK_EQ(global_->synchronous_observer_, observer);
  global_->synchronous_observer_ = nullptr;
}

void FieldTrialList::OnGroupFinalized(bool is_locked, FieldTrial* field_trial) {
  if (!global_)
    return;
  if (is_locked) {
    AddToAllocatorWhileLocked(global_->field_trial_allocator_.get(),
                              field_trial);
  } else {
    AutoLock auto_lock(global_->lock_);
    AddToAllocatorWhileLocked(global_->field_trial_allocator_.get(),
                              field_trial);
  }
}

void FieldTrialList::NotifyFieldTrialGroupSelection(FieldTrial* field_trial) {
  if (!global_)
    return;

  {
    AutoLock auto_lock(global_->lock_);
    if (field_trial->group_reported_)
      return;
    field_trial->group_reported_ = true;

    if (!field_trial->enable_field_trial_)
      return;

    ActivateFieldTrialEntryWhileLocked(field_trial);
  }

  if (global_->synchronous_observer_) {
    global_->synchronous_observer_->OnFieldTrialGroupFinalized(
        field_trial->trial_name(), field_trial->group_name_internal());
  }

  global_->observer_list_->NotifySynchronously(
      FROM_HERE, &FieldTrialList::Observer::OnFieldTrialGroupFinalized,
      field_trial->trial_name(), field_trial->group_name_internal());
}

size_t FieldTrialList::GetFieldTrialCount() {
  if (!global_)
    return 0;
  AutoLock auto_lock(global_->lock_);
  return global_->registered_.size();
}

bool FieldTrialList::GetParamsFromSharedMemory(
    FieldTrial* field_trial,
    std::map<std::string, std::string>* params) {
  DCHECK(global_);











  AutoLock auto_lock(global_->lock_);
  if (!global_->field_trial_allocator_)
    return false;

  if (!field_trial->ref_)
    return false;

  const FieldTrial::FieldTrialEntry* entry =
      global_->field_trial_allocator_->GetAsObject<FieldTrial::FieldTrialEntry>(
          field_trial->ref_);

  size_t allocated_size =
      global_->field_trial_allocator_->GetAllocSize(field_trial->ref_);
  size_t actual_size = sizeof(FieldTrial::FieldTrialEntry) + entry->pickle_size;
  if (allocated_size < actual_size)
    return false;

  return entry->GetParams(params);
}

void FieldTrialList::ClearParamsFromSharedMemoryForTesting() {
  if (!global_)
    return;

  AutoLock auto_lock(global_->lock_);
  if (!global_->field_trial_allocator_)
    return;



  FieldTrialAllocator* allocator = global_->field_trial_allocator_.get();
  FieldTrialAllocator::Iterator mem_iter(allocator);


  std::vector<FieldTrial::FieldTrialRef> new_refs;

  FieldTrial::FieldTrialRef prev_ref;
  while ((prev_ref = mem_iter.GetNextOfType<FieldTrial::FieldTrialEntry>()) !=
         FieldTrialAllocator::kReferenceNull) {

    const FieldTrial::FieldTrialEntry* prev_entry =
        allocator->GetAsObject<FieldTrial::FieldTrialEntry>(prev_ref);
    StringPiece trial_name;
    StringPiece group_name;
    if (!prev_entry->GetTrialAndGroupName(&trial_name, &group_name))
      continue;

    Pickle pickle;
    pickle.WriteString(trial_name);
    pickle.WriteString(group_name);
    size_t total_size = sizeof(FieldTrial::FieldTrialEntry) + pickle.size();
    FieldTrial::FieldTrialEntry* new_entry =
        allocator->New<FieldTrial::FieldTrialEntry>(total_size);
    subtle::NoBarrier_Store(&new_entry->activated,
                            subtle::NoBarrier_Load(&prev_entry->activated));
    new_entry->pickle_size = pickle.size();


    char* dst = reinterpret_cast<char*>(new_entry) +
                sizeof(FieldTrial::FieldTrialEntry);
    memcpy(dst, pickle.data(), pickle.size());


    FieldTrial::FieldTrialRef new_ref = allocator->GetAsReference(new_entry);
    FieldTrial* trial = global_->PreLockedFind(trial_name.as_string());
    trial->ref_ = new_ref;
    new_refs.push_back(new_ref);

    allocator->ChangeType(prev_ref, 0,
                          FieldTrial::FieldTrialEntry::kPersistentTypeId,
                          /*clear=*/false);
  }

  for (const auto& ref : new_refs) {
    allocator->MakeIterable(ref);
  }
}

void FieldTrialList::DumpAllFieldTrialsToPersistentAllocator(
    PersistentMemoryAllocator* allocator) {
  if (!global_)
    return;
  AutoLock auto_lock(global_->lock_);
  for (const auto& registered : global_->registered_) {
    AddToAllocatorWhileLocked(allocator, registered.second);
  }
}

std::vector<const FieldTrial::FieldTrialEntry*>
FieldTrialList::GetAllFieldTrialsFromPersistentAllocator(
    PersistentMemoryAllocator const& allocator) {
  std::vector<const FieldTrial::FieldTrialEntry*> entries;
  FieldTrialAllocator::Iterator iter(&allocator);
  const FieldTrial::FieldTrialEntry* entry;
  while ((entry = iter.GetNextOfObject<FieldTrial::FieldTrialEntry>()) !=
         nullptr) {
    entries.push_back(entry);
  }
  return entries;
}

FieldTrialList* FieldTrialList::GetInstance() {
  return global_;
}

FieldTrialList* FieldTrialList::BackupInstanceForTesting() {
  FieldTrialList* instance = global_;
  global_ = nullptr;
  return instance;
}

void FieldTrialList::RestoreInstanceForTesting(FieldTrialList* instance) {
  global_ = instance;
}

std::string FieldTrialList::SerializeSharedMemoryRegionMetadata(
    const ReadOnlySharedMemoryRegion& shm) {
  std::stringstream ss;
#if defined(OS_WIN)

  uintptr_t uintptr_handle =
      reinterpret_cast<uintptr_t>(shm.GetPlatformHandle());
  ss << uintptr_handle << ",";
#elif defined(OS_FUCHSIA)
  ss << shm.GetPlatformHandle()->get() << ",";
#elif defined(OS_MACOSX) && !defined(OS_IOS)


  ss << kFieldTrialRendezvousKey << ",";
#elif !defined(OS_POSIX)
#error Unsupported OS
#endif

  UnguessableToken guid = shm.GetGUID();
  ss << guid.GetHighForSerialization() << "," << guid.GetLowForSerialization();
  ss << "," << shm.GetSize();
  return ss.str();
}

#if defined(OS_WIN) || defined(OS_FUCHSIA) || \
    (defined(OS_MACOSX) && !defined(OS_IOS))

ReadOnlySharedMemoryRegion
FieldTrialList::DeserializeSharedMemoryRegionMetadata(
    const std::string& switch_value) {
  std::vector<StringPiece> tokens =
      SplitStringPiece(switch_value, ",", KEEP_WHITESPACE, SPLIT_WANT_ALL);

  if (tokens.size() != 4)
    return ReadOnlySharedMemoryRegion();

  int field_trial_handle = 0;
  if (!StringToInt(tokens[0], &field_trial_handle))
    return ReadOnlySharedMemoryRegion();
#if defined(OS_FUCHSIA)
  zx_handle_t handle = static_cast<zx_handle_t>(field_trial_handle);
  zx::vmo scoped_handle = zx::vmo(handle);
#elif defined(OS_WIN)
  HANDLE handle = reinterpret_cast<HANDLE>(field_trial_handle);
  if (IsCurrentProcessElevated()) {


    ProcessId parent_pid = GetParentProcessId(GetCurrentProcess());
    HANDLE parent_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, parent_pid);



    DuplicateHandle(parent_handle, handle, GetCurrentProcess(), &handle, 0,
                    FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(parent_handle);
  }
  win::ScopedHandle scoped_handle(handle);
#elif defined(OS_MACOSX) && !defined(OS_IOS)
  auto* rendezvous = MachPortRendezvousClient::GetInstance();
  if (!rendezvous)
    return ReadOnlySharedMemoryRegion();
  mac::ScopedMachSendRight scoped_handle =
      rendezvous->TakeSendRight(field_trial_handle);
  if (!scoped_handle.is_valid())
    return ReadOnlySharedMemoryRegion();
#endif

  UnguessableToken guid;
  if (!DeserializeGUIDFromStringPieces(tokens[1], tokens[2], &guid))
    return ReadOnlySharedMemoryRegion();

  int size;
  if (!StringToInt(tokens[3], &size))
    return ReadOnlySharedMemoryRegion();

  auto platform_handle = subtle::PlatformSharedMemoryRegion::Take(
      std::move(scoped_handle),
      subtle::PlatformSharedMemoryRegion::Mode::kReadOnly,
      static_cast<size_t>(size), guid);
  return ReadOnlySharedMemoryRegion::Deserialize(std::move(platform_handle));
}

#elif defined(OS_POSIX) && !defined(OS_NACL)

ReadOnlySharedMemoryRegion
FieldTrialList::DeserializeSharedMemoryRegionMetadata(
    int fd,
    const std::string& switch_value) {
  std::vector<StringPiece> tokens =
      SplitStringPiece(switch_value, ",", KEEP_WHITESPACE, SPLIT_WANT_ALL);

  if (tokens.size() != 3)
    return ReadOnlySharedMemoryRegion();

  UnguessableToken guid;
  if (!DeserializeGUIDFromStringPieces(tokens[0], tokens[1], &guid))
    return ReadOnlySharedMemoryRegion();

  int size;
  if (!StringToInt(tokens[2], &size))
    return ReadOnlySharedMemoryRegion();

  auto platform_region = subtle::PlatformSharedMemoryRegion::Take(
      ScopedFD(fd), subtle::PlatformSharedMemoryRegion::Mode::kReadOnly,
      static_cast<size_t>(size), guid);
  return ReadOnlySharedMemoryRegion::Deserialize(std::move(platform_region));
}

#endif

#if defined(OS_WIN) || defined(OS_FUCHSIA) || \
    (defined(OS_MACOSX) && !defined(OS_IOS))
// static
bool FieldTrialList::CreateTrialsFromSwitchValue(
    const std::string& switch_value) {
  ReadOnlySharedMemoryRegion shm =
      DeserializeSharedMemoryRegionMetadata(switch_value);
  if (!shm.IsValid())
    return false;
  return FieldTrialList::CreateTrialsFromSharedMemoryRegion(shm);
}
#elif defined(OS_POSIX) && !defined(OS_NACL)
// static
bool FieldTrialList::CreateTrialsFromDescriptor(
    int fd_key,
    const std::string& switch_value) {
  if (fd_key == -1)
    return false;

  int fd = GlobalDescriptors::GetInstance()->MaybeGet(fd_key);
  if (fd == -1)
    return false;

  ReadOnlySharedMemoryRegion shm =
      DeserializeSharedMemoryRegionMetadata(fd, switch_value);
  if (!shm.IsValid())
    return false;

  bool result = FieldTrialList::CreateTrialsFromSharedMemoryRegion(shm);
  DCHECK(result);
  return true;
}
#endif  // defined(OS_POSIX) && !defined(OS_NACL)

bool FieldTrialList::CreateTrialsFromSharedMemoryRegion(
    const ReadOnlySharedMemoryRegion& shm_region) {
  ReadOnlySharedMemoryMapping shm_mapping =
      shm_region.MapAt(0, kFieldTrialAllocationSize);
  if (!shm_mapping.IsValid())
    OnOutOfMemory(kFieldTrialAllocationSize);

  return FieldTrialList::CreateTrialsFromSharedMemoryMapping(
      std::move(shm_mapping));
}

bool FieldTrialList::CreateTrialsFromSharedMemoryMapping(
    ReadOnlySharedMemoryMapping shm_mapping) {
  global_->field_trial_allocator_ =
      std::make_unique<ReadOnlySharedPersistentMemoryAllocator>(
          std::move(shm_mapping), 0, kAllocatorName);
  FieldTrialAllocator* shalloc = global_->field_trial_allocator_.get();
  FieldTrialAllocator::Iterator mem_iter(shalloc);

  const FieldTrial::FieldTrialEntry* entry;
  while ((entry = mem_iter.GetNextOfObject<FieldTrial::FieldTrialEntry>()) !=
         nullptr) {
    StringPiece trial_name;
    StringPiece group_name;
    if (!entry->GetTrialAndGroupName(&trial_name, &group_name))
      return false;


    FieldTrial* trial =
        CreateFieldTrial(trial_name.as_string(), group_name.as_string());

    trial->ref_ = mem_iter.GetAsReference(entry);
    if (subtle::NoBarrier_Load(&entry->activated)) {



      trial->group();
    }
  }
  return true;
}

void FieldTrialList::InstantiateFieldTrialAllocatorIfNeeded() {
  if (!global_)
    return;

  AutoLock auto_lock(global_->lock_);

  if (global_->field_trial_allocator_ != nullptr)
    return;

  MappedReadOnlyRegion shm =
      ReadOnlySharedMemoryRegion::Create(kFieldTrialAllocationSize);

  if (!shm.IsValid())
    OnOutOfMemory(kFieldTrialAllocationSize);

  global_->field_trial_allocator_ =
      std::make_unique<WritableSharedPersistentMemoryAllocator>(
          std::move(shm.mapping), 0, kAllocatorName);
  global_->field_trial_allocator_->CreateTrackingHistograms(kAllocatorName);

  for (const auto& registered : global_->registered_) {
    AddToAllocatorWhileLocked(global_->field_trial_allocator_.get(),
                              registered.second);
  }

  FeatureList::GetInstance()->AddFeaturesToAllocator(
      global_->field_trial_allocator_.get());

#if !defined(OS_NACL)
  global_->readonly_allocator_region_ = std::move(shm.region);
#endif
}

void FieldTrialList::AddToAllocatorWhileLocked(
    PersistentMemoryAllocator* allocator,
    FieldTrial* field_trial) {

  if (allocator == nullptr)
    return;


  if (allocator->IsReadonly())
    return;

  FieldTrial::State trial_state;
  if (!field_trial->GetStateWhileLocked(&trial_state, false))
    return;


  if (field_trial->ref_)
    return;

  Pickle pickle;
  PickleFieldTrial(trial_state, &pickle);

  size_t total_size = sizeof(FieldTrial::FieldTrialEntry) + pickle.size();
  FieldTrial::FieldTrialRef ref = allocator->Allocate(
      total_size, FieldTrial::FieldTrialEntry::kPersistentTypeId);
  if (ref == FieldTrialAllocator::kReferenceNull) {
    NOTREACHED();
    return;
  }

  FieldTrial::FieldTrialEntry* entry =
      allocator->GetAsObject<FieldTrial::FieldTrialEntry>(ref);
  subtle::NoBarrier_Store(&entry->activated, trial_state.activated);
  entry->pickle_size = pickle.size();


  char* dst =
      reinterpret_cast<char*>(entry) + sizeof(FieldTrial::FieldTrialEntry);
  memcpy(dst, pickle.data(), pickle.size());

  allocator->MakeIterable(ref);
  field_trial->ref_ = ref;
}

void FieldTrialList::ActivateFieldTrialEntryWhileLocked(
    FieldTrial* field_trial) {
  FieldTrialAllocator* allocator = global_->field_trial_allocator_.get();

  if (!allocator || allocator->IsReadonly())
    return;

  FieldTrial::FieldTrialRef ref = field_trial->ref_;
  if (ref == FieldTrialAllocator::kReferenceNull) {


    AddToAllocatorWhileLocked(allocator, field_trial);
  } else {



    FieldTrial::FieldTrialEntry* entry =
        allocator->GetAsObject<FieldTrial::FieldTrialEntry>(ref);
    subtle::NoBarrier_Store(&entry->activated, 1);
  }
}

const FieldTrial::EntropyProvider*
    FieldTrialList::GetEntropyProviderForOneTimeRandomization() {
  if (!global_) {
    used_without_global_ = true;
    return nullptr;
  }

  return global_->entropy_provider_.get();
}

FieldTrial* FieldTrialList::PreLockedFind(const std::string& name) {
  auto it = registered_.find(name);
  if (registered_.end() == it)
    return nullptr;
  return it->second;
}

void FieldTrialList::Register(FieldTrial* trial) {
  if (!global_) {
    used_without_global_ = true;
    return;
  }
  AutoLock auto_lock(global_->lock_);
  CHECK(!global_->PreLockedFind(trial->trial_name())) << trial->trial_name();
  trial->AddRef();
  trial->SetTrialRegistered();
  global_->registered_[trial->trial_name()] = trial;
}

FieldTrialList::RegistrationMap FieldTrialList::GetRegisteredTrials() {
  RegistrationMap output;
  if (global_) {
    AutoLock auto_lock(global_->lock_);
    output = global_->registered_;
  }
  return output;
}

}  // namespace base
