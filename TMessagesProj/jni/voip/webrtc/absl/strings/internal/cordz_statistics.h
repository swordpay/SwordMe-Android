// Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_STRINGS_INTERNAL_CORDZ_STATISTICS_H_
#define ABSL_STRINGS_INTERNAL_CORDZ_STATISTICS_H_

#include <cstdint>

#include "absl/base/config.h"
#include "absl/strings/internal/cordz_update_tracker.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

struct CordzStatistics {
  using MethodIdentifier = CordzUpdateTracker::MethodIdentifier;

  struct NodeCounts {
    size_t flat = 0;       // #flats
    size_t flat_64 = 0;    // #flats up to 64 bytes
    size_t flat_128 = 0;   // #flats up to 128 bytes
    size_t flat_256 = 0;   // #flats up to 256 bytes
    size_t flat_512 = 0;   // #flats up to 512 bytes
    size_t flat_1k = 0;    // #flats up to 1K bytes
    size_t external = 0;   // #external reps
    size_t substring = 0;  // #substring reps
    size_t concat = 0;     // #concat reps
    size_t ring = 0;       // #ring buffer reps
    size_t btree = 0;      // #btree reps
    size_t crc = 0;        // #crc reps
  };

  size_t size = 0;



  size_t estimated_memory_usage = 0;







  size_t estimated_fair_share_memory_usage = 0;





  size_t node_count = 0;

  NodeCounts node_counts;

  MethodIdentifier method = MethodIdentifier::kUnknown;

  MethodIdentifier parent_method = MethodIdentifier::kUnknown;

  CordzUpdateTracker update_tracker;
};

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORDZ_STATISTICS_H_
