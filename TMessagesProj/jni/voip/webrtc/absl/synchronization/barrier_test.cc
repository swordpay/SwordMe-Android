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

#include "absl/synchronization/barrier.h"

#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "gtest/gtest.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"


TEST(Barrier, SanityTest) {
  constexpr int kNumThreads = 10;
  absl::Barrier* barrier = new absl::Barrier(kNumThreads);

  absl::Mutex mutex;
  int counter = 0;  // Guarded by mutex.

  auto thread_func = [&] {
    if (barrier->Block()) {


      delete barrier;
    }

    absl::MutexLock lock(&mutex);
    ++counter;
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < kNumThreads - 1; ++i) {
    threads.push_back(std::thread(thread_func));
  }





  absl::SleepFor(absl::Seconds(1));


  {
    absl::MutexLock lock(&mutex);
    EXPECT_EQ(counter, 0);
  }

  threads.push_back(std::thread(thread_func));

  for (auto& thread : threads) {
    thread.join();
  }

  absl::MutexLock lock(&mutex);
  EXPECT_EQ(counter, kNumThreads);
}
