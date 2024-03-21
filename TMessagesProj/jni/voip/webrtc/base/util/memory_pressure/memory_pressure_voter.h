// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_UTIL_MEMORY_PRESSURE_MEMORY_PRESSURE_VOTER_H_
#define BASE_UTIL_MEMORY_PRESSURE_MEMORY_PRESSURE_VOTER_H_

#include <array>

#include "base/callback.h"
#include "base/memory/memory_pressure_listener.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/sequence_checker.h"

namespace util {

// a MemoryPressureAggregator when the pressure they observe changes, or they
// want to trigger a (re-)notification of clients of the current level.
// Voters must be used only from the same sequence as the Aggregator to which
// they are attached.
class MemoryPressureVoter {
 public:
  virtual ~MemoryPressureVoter() {}

  virtual void SetVote(base::MemoryPressureListener::MemoryPressureLevel level,
                       bool notify_listeners) = 0;
};

// pressure level for the MultiSourceMemoryPressureMonitor, which will own
// and outlive the aggregator. The pressure level is calculated as the most
// critical of all votes collected. This class is not thread safe and should be
// used from a single sequence.
class MemoryPressureVoteAggregator {
 public:
  class Delegate;

  using MemoryPressureLevel = base::MemoryPressureListener::MemoryPressureLevel;

  explicit MemoryPressureVoteAggregator(Delegate* delegate);
  ~MemoryPressureVoteAggregator();


  std::unique_ptr<MemoryPressureVoter> CreateVoter();

  void OnVoteForTesting(base::Optional<MemoryPressureLevel> old_vote,
                        base::Optional<MemoryPressureLevel> new_vote);

  void NotifyListenersForTesting();

  base::MemoryPressureListener::MemoryPressureLevel EvaluateVotesForTesting();
  void SetVotesForTesting(size_t none_votes,
                          size_t moderate_votes,
                          size_t critical_votes);

 private:
  friend class MemoryPressureVoterImpl;




  void OnVote(base::Optional<MemoryPressureLevel> old_vote,
              base::Optional<MemoryPressureLevel> new_vote);




  void NotifyListeners();


  MemoryPressureLevel EvaluateVotes() const;

  MemoryPressureLevel current_pressure_level_ =
      base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_NONE;

  Delegate* const delegate_;







  std::array<size_t,
             base::MemoryPressureListener::MEMORY_PRESSURE_LEVEL_CRITICAL + 1>
      votes_ = {};

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(MemoryPressureVoteAggregator);
};

// vote aggregation.
class MemoryPressureVoteAggregator::Delegate {
 public:
  Delegate() = default;
  virtual ~Delegate() = default;

  virtual void OnMemoryPressureLevelChanged(
      base::MemoryPressureListener::MemoryPressureLevel level) = 0;


  virtual void OnNotifyListenersRequested() = 0;
};

}  // namespace util

#endif  // BASE_UTIL_MEMORY_PRESSURE_MEMORY_PRESSURE_VOTER_H_
