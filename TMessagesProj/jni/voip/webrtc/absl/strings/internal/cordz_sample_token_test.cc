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

#include "absl/strings/internal/cordz_sample_token.h"

#include <memory>
#include <type_traits>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/memory/memory.h"
#include "absl/random/random.h"
#include "absl/strings/cordz_test_helpers.h"
#include "absl/strings/internal/cord_rep_flat.h"
#include "absl/strings/internal/cordz_handle.h"
#include "absl/strings/internal/cordz_info.h"
#include "absl/synchronization/internal/thread_pool.h"
#include "absl/synchronization/notification.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Ne;

auto constexpr kTrackCordMethod = CordzUpdateTracker::kConstructorString;

TEST(CordzSampleTokenTest, IteratorTraits) {
  static_assert(std::is_copy_constructible<CordzSampleToken::Iterator>::value,
                "");
  static_assert(std::is_copy_assignable<CordzSampleToken::Iterator>::value, "");
  static_assert(std::is_move_constructible<CordzSampleToken::Iterator>::value,
                "");
  static_assert(std::is_move_assignable<CordzSampleToken::Iterator>::value, "");
  static_assert(
      std::is_same<
          std::iterator_traits<CordzSampleToken::Iterator>::iterator_category,
          std::input_iterator_tag>::value,
      "");
  static_assert(
      std::is_same<std::iterator_traits<CordzSampleToken::Iterator>::value_type,
                   const CordzInfo&>::value,
      "");
  static_assert(
      std::is_same<
          std::iterator_traits<CordzSampleToken::Iterator>::difference_type,
          ptrdiff_t>::value,
      "");
  static_assert(
      std::is_same<std::iterator_traits<CordzSampleToken::Iterator>::pointer,
                   const CordzInfo*>::value,
      "");
  static_assert(
      std::is_same<std::iterator_traits<CordzSampleToken::Iterator>::reference,
                   const CordzInfo&>::value,
      "");
}

TEST(CordzSampleTokenTest, IteratorEmpty) {
  CordzSampleToken token;
  EXPECT_THAT(token.begin(), Eq(token.end()));
}

TEST(CordzSampleTokenTest, Iterator) {
  TestCordData cord1, cord2, cord3;
  CordzInfo::TrackCord(cord1.data, kTrackCordMethod);
  CordzInfo* info1 = cord1.data.cordz_info();
  CordzInfo::TrackCord(cord2.data, kTrackCordMethod);
  CordzInfo* info2 = cord2.data.cordz_info();
  CordzInfo::TrackCord(cord3.data, kTrackCordMethod);
  CordzInfo* info3 = cord3.data.cordz_info();

  CordzSampleToken token;
  std::vector<const CordzInfo*> found;
  for (const CordzInfo& cord_info : token) {
    found.push_back(&cord_info);
  }

  EXPECT_THAT(found, ElementsAre(info3, info2, info1));

  info1->Untrack();
  info2->Untrack();
  info3->Untrack();
}

TEST(CordzSampleTokenTest, IteratorEquality) {
  TestCordData cord1;
  TestCordData cord2;
  TestCordData cord3;
  CordzInfo::TrackCord(cord1.data, kTrackCordMethod);
  CordzInfo* info1 = cord1.data.cordz_info();

  CordzSampleToken token1;

  CordzSampleToken::Iterator lhs = token1.begin();

  CordzInfo::TrackCord(cord2.data, kTrackCordMethod);
  CordzInfo* info2 = cord2.data.cordz_info();

  CordzSampleToken token2;

  CordzSampleToken::Iterator rhs = token2.begin();

  CordzInfo::TrackCord(cord3.data, kTrackCordMethod);
  CordzInfo* info3 = cord3.data.cordz_info();

  EXPECT_THAT(lhs, Ne(rhs));

  rhs++;


  EXPECT_THAT(lhs, Ne(rhs));

  lhs++;
  rhs++;

  EXPECT_THAT(lhs, Eq(rhs));

  info1->Untrack();
  info2->Untrack();
  info3->Untrack();
}

TEST(CordzSampleTokenTest, MultiThreaded) {
  Notification stop;
  static constexpr int kNumThreads = 4;
  static constexpr int kNumCords = 3;
  static constexpr int kNumTokens = 3;
  absl::synchronization_internal::ThreadPool pool(kNumThreads);

  for (int i = 0; i < kNumThreads; ++i) {
    pool.Schedule([&stop]() {
      absl::BitGen gen;
      TestCordData cords[kNumCords];
      std::unique_ptr<CordzSampleToken> tokens[kNumTokens];

      while (!stop.HasBeenNotified()) {






        int index = absl::Uniform(gen, 0, kNumCords);
        if (absl::Bernoulli(gen, 0.5)) {
          TestCordData& cord = cords[index];

          if (cord.data.is_profiled()) {

            cord.data.cordz_info()->Untrack();
            cord.data.clear_cordz_info();
          } else {

            CordzInfo::TrackCord(cord.data, kTrackCordMethod);
          }
        } else {
          std::unique_ptr<CordzSampleToken>& token = tokens[index];
          if (token) {
            if (absl::Bernoulli(gen, 0.5)) {

              for (const CordzInfo& info : *token) {

                EXPECT_THAT(info.Next(*token), Ne(&info));
              }
            } else {

              token = nullptr;
            }
          } else {

            token = absl::make_unique<CordzSampleToken>();
          }
        }
      }
      for (TestCordData& cord : cords) {
        CordzInfo::MaybeUntrackCord(cord.data.cordz_info());
      }
    });
  }


  absl::SleepFor(absl::Seconds(3));
  stop.Notify();
}

}  // namespace
}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl
