// Copyright 2018 The Abseil Authors.
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
//
// -----------------------------------------------------------------------------
// mocking_bit_gen.h
// -----------------------------------------------------------------------------
//
// This file includes an `absl::MockingBitGen` class to use as a mock within the
// Googletest testing framework. Such a mock is useful to provide deterministic
// values as return values within (otherwise random) Abseil distribution
// functions. Such determinism within a mock is useful within testing frameworks
// to test otherwise indeterminate APIs.
//
// More information about the Googletest testing framework is available at
// https://github.com/google/googletest

#ifndef ABSL_RANDOM_MOCKING_BIT_GEN_H_
#define ABSL_RANDOM_MOCKING_BIT_GEN_H_

#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/base/internal/fast_type_id.h"
#include "absl/container/flat_hash_map.h"
#include "absl/meta/type_traits.h"
#include "absl/random/distributions.h"
#include "absl/random/internal/distribution_caller.h"
#include "absl/random/random.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "absl/utility/utility.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

namespace random_internal {
template <typename>
struct DistributionCaller;
class MockHelpers;

}  // namespace random_internal
class BitGenRef;

//
// `absl::MockingBitGen` is a mock Uniform Random Bit Generator (URBG) class
// which can act in place of an `absl::BitGen` URBG within tests using the
// Googletest testing framework.
//
// Usage:
//
// Use an `absl::MockingBitGen` along with a mock distribution object (within
// mock_distributions.h) inside Googletest constructs such as ON_CALL(),
// EXPECT_TRUE(), etc. to produce deterministic results conforming to the
// distribution's API contract.
//
// Example:
//
//  // Mock a call to an `absl::Bernoulli` distribution using Googletest
//   absl::MockingBitGen bitgen;
//
//   ON_CALL(absl::MockBernoulli(), Call(bitgen, 0.5))
//       .WillByDefault(testing::Return(true));
//   EXPECT_TRUE(absl::Bernoulli(bitgen, 0.5));
//
//  // Mock a call to an `absl::Uniform` distribution within Googletest
//  absl::MockingBitGen bitgen;
//
//   ON_CALL(absl::MockUniform<int>(), Call(bitgen, testing::_, testing::_))
//       .WillByDefault([] (int low, int high) {
//           return low + (high - low) / 2;
//       });
//
//   EXPECT_EQ(absl::Uniform<int>(gen, 0, 10), 5);
//   EXPECT_EQ(absl::Uniform<int>(gen, 30, 40), 35);
//
// At this time, only mock distributions supplied within the Abseil random
// library are officially supported.
//
// EXPECT_CALL and ON_CALL need to be made within the same DLL component as
// the call to absl::Uniform and related methods, otherwise mocking will fail
// since the  underlying implementation creates a type-specific pointer which
// will be distinct across different DLL boundaries.
//
class MockingBitGen {
 public:
  MockingBitGen() = default;
  ~MockingBitGen() = default;

  using result_type = absl::BitGen::result_type;

  static constexpr result_type(min)() { return (absl::BitGen::min)(); }
  static constexpr result_type(max)() { return (absl::BitGen::max)(); }
  result_type operator()() { return gen_(); }

 private:


  template <typename ResultT, typename... Args>
  static auto GetMockFnType(ResultT, std::tuple<Args...>)
      -> ::testing::MockFunction<ResultT(Args...)>;





  template <typename MockFnType, typename ResultT, typename Tuple>
  struct MockFnCaller;

  template <typename MockFnType, typename ResultT, typename... Args>
  struct MockFnCaller<MockFnType, ResultT, std::tuple<Args...>> {
    MockFnType* fn;
    inline ResultT operator()(Args... args) {
      return fn->Call(std::move(args)...);
    }
  };



  class FunctionHolder {
   public:
    virtual ~FunctionHolder() = default;


    virtual void Apply(/*ArgTupleT*/ void* args_tuple,
                       /*ResultT*/ void* result) = 0;
  };

  template <typename MockFnType, typename ResultT, typename ArgTupleT>
  class FunctionHolderImpl final : public FunctionHolder {
   public:
    void Apply(void* args_tuple, void* result) override {



      *static_cast<ResultT*>(result) =
          absl::apply(MockFnCaller<MockFnType, ResultT, ArgTupleT>{&mock_fn_},
                      *static_cast<ArgTupleT*>(args_tuple));
    }

    MockFnType mock_fn_;
  };










  template <typename ResultT, typename ArgTupleT, typename SelfT>
  auto RegisterMock(SelfT&, base_internal::FastTypeIdType type)
      -> decltype(GetMockFnType(std::declval<ResultT>(),
                                std::declval<ArgTupleT>()))& {
    using MockFnType = decltype(GetMockFnType(std::declval<ResultT>(),
                                              std::declval<ArgTupleT>()));

    using WrappedFnType = absl::conditional_t<
        std::is_same<SelfT, ::testing::NiceMock<absl::MockingBitGen>>::value,
        ::testing::NiceMock<MockFnType>,
        absl::conditional_t<
            std::is_same<SelfT,
                         ::testing::NaggyMock<absl::MockingBitGen>>::value,
            ::testing::NaggyMock<MockFnType>,
            absl::conditional_t<
                std::is_same<SelfT,
                             ::testing::StrictMock<absl::MockingBitGen>>::value,
                ::testing::StrictMock<MockFnType>, MockFnType>>>;

    using ImplT = FunctionHolderImpl<WrappedFnType, ResultT, ArgTupleT>;
    auto& mock = mocks_[type];
    if (!mock) {
      mock = absl::make_unique<ImplT>();
    }
    return static_cast<ImplT*>(mock.get())->mock_fn_;
  }











  inline bool InvokeMock(base_internal::FastTypeIdType type, void* args_tuple,
                         void* result) {

    auto it = mocks_.find(type);
    if (it == mocks_.end()) return false;
    it->second->Apply(args_tuple, result);
    return true;
  }

  absl::flat_hash_map<base_internal::FastTypeIdType,
                      std::unique_ptr<FunctionHolder>>
      mocks_;
  absl::BitGen gen_;

  template <typename>
  friend struct ::absl::random_internal::DistributionCaller;  // for InvokeMock
  friend class ::absl::BitGenRef;                             // for InvokeMock
  friend class ::absl::random_internal::MockHelpers;  // for RegisterMock,

};

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_MOCKING_BIT_GEN_H_
