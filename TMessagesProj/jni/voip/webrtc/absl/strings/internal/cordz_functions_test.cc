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

#include "absl/strings/internal/cordz_functions.h"

#include <thread>  // NOLINT we need real clean new threads

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/base/config.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {
namespace {

using ::testing::Eq;
using ::testing::Ge;
using ::testing::Le;

TEST(CordzFunctionsTest, SampleRate) {
  int32_t orig_sample_rate = get_cordz_mean_interval();
  int32_t expected_sample_rate = 123;
  set_cordz_mean_interval(expected_sample_rate);
  EXPECT_THAT(get_cordz_mean_interval(), Eq(expected_sample_rate));
  set_cordz_mean_interval(orig_sample_rate);
}

// should_profile will return false when cordz is diabled, so we might want to
// avoid those tests.
#ifdef ABSL_INTERNAL_CORDZ_ENABLED

TEST(CordzFunctionsTest, ShouldProfileDisable) {
  int32_t orig_sample_rate = get_cordz_mean_interval();

  set_cordz_mean_interval(0);
  cordz_set_next_sample_for_testing(0);
  EXPECT_FALSE(cordz_should_profile());

  EXPECT_THAT(cordz_next_sample, Eq(1 << 16));

  set_cordz_mean_interval(orig_sample_rate);
}

TEST(CordzFunctionsTest, ShouldProfileAlways) {
  int32_t orig_sample_rate = get_cordz_mean_interval();

  set_cordz_mean_interval(1);
  cordz_set_next_sample_for_testing(1);
  EXPECT_TRUE(cordz_should_profile());
  EXPECT_THAT(cordz_next_sample, Le(1));

  set_cordz_mean_interval(orig_sample_rate);
}

TEST(CordzFunctionsTest, DoesNotAlwaysSampleFirstCord) {


  set_cordz_mean_interval(10000);
  int tries = 0;
  bool sampled = false;
  do {
    ++tries;
    ASSERT_THAT(tries, Le(1000));
    std::thread thread([&sampled] {
      sampled = cordz_should_profile();
    });
    thread.join();
  } while (sampled);
}

TEST(CordzFunctionsTest, ShouldProfileRate) {
  static constexpr int kDesiredMeanInterval = 1000;
  static constexpr int kSamples = 10000;
  int32_t orig_sample_rate = get_cordz_mean_interval();

  set_cordz_mean_interval(kDesiredMeanInterval);

  int64_t sum_of_intervals = 0;
  for (int i = 0; i < kSamples; i++) {


    cordz_set_next_sample_for_testing(0);
    cordz_should_profile();
    sum_of_intervals += cordz_next_sample;
  }


























  EXPECT_THAT(sum_of_intervals, Ge(9396115));
  EXPECT_THAT(sum_of_intervals, Le(10618100));

  set_cordz_mean_interval(orig_sample_rate);
}

#else  // ABSL_INTERNAL_CORDZ_ENABLED

TEST(CordzFunctionsTest, ShouldProfileDisabled) {
  int32_t orig_sample_rate = get_cordz_mean_interval();

  set_cordz_mean_interval(1);
  cordz_set_next_sample_for_testing(0);
  EXPECT_FALSE(cordz_should_profile());

  set_cordz_mean_interval(orig_sample_rate);
}

#endif  // ABSL_INTERNAL_CORDZ_ENABLED

}  // namespace
}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl
