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

#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "gtest/gtest.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace {

void PauseAndDecreaseCounter(BlockingCounter* counter, int* done) {
  absl::SleepFor(absl::Seconds(1));
  *done = 1;
  counter->DecrementCount();
}

TEST(BlockingCounterTest, BasicFunctionality) {



  const int num_workers = 10;
  BlockingCounter counter(num_workers);

  std::vector<std::thread> workers;
  std::vector<int> done(num_workers, 0);


  workers.reserve(num_workers);
  for (int k = 0; k < num_workers; k++) {
    workers.emplace_back(
        [&counter, &done, k] { PauseAndDecreaseCounter(&counter, &done[k]); });
  }

  counter.Wait();

  for (int k = 0; k < num_workers; k++) {
    EXPECT_EQ(1, done[k]);
  }

  for (std::thread& w : workers) {
    w.join();
  }
}

TEST(BlockingCounterTest, WaitZeroInitialCount) {
  BlockingCounter counter(0);
  counter.Wait();
}

#if GTEST_HAS_DEATH_TEST
TEST(BlockingCounterTest, WaitNegativeInitialCount) {
  EXPECT_DEATH(BlockingCounter counter(-1),
               "BlockingCounter initial_count negative");
}
#endif

}  // namespace
ABSL_NAMESPACE_END
}  // namespace absl
