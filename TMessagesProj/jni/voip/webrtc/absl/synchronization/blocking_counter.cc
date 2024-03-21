// Copyright 2017 The Abseil Authors.
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

#include "absl/synchronization/blocking_counter.h"

#include <atomic>

#include "absl/base/internal/raw_logging.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

namespace {

bool IsDone(void *arg) { return *reinterpret_cast<bool *>(arg); }

}  // namespace

BlockingCounter::BlockingCounter(int initial_count)
    : count_(initial_count),
      num_waiting_(0),
      done_{initial_count == 0 ? true : false} {
  ABSL_RAW_CHECK(initial_count >= 0, "BlockingCounter initial_count negative");
}

bool BlockingCounter::DecrementCount() {
  int count = count_.fetch_sub(1, std::memory_order_acq_rel) - 1;
  ABSL_RAW_CHECK(count >= 0,
                 "BlockingCounter::DecrementCount() called too many times");
  if (count == 0) {
    MutexLock l(&lock_);
    done_ = true;
    return true;
  }
  return false;
}

void BlockingCounter::Wait() {
  MutexLock l(&this->lock_);


  ABSL_RAW_CHECK(num_waiting_ == 0, "multiple threads called Wait()");
  num_waiting_++;

  this->lock_.Await(Condition(IsDone, &this->done_));




}

ABSL_NAMESPACE_END
}  // namespace absl
