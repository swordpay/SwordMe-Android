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

#include "absl/types/variant.h"

#include "absl/base/config.h"

// exceptions are not enabled.
#if !defined(ABSL_USES_STD_VARIANT) && defined(ABSL_HAVE_EXCEPTIONS)

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/base/internal/exception_safety_testing.h"
#include "absl/memory/memory.h"

#if !defined(ABSL_INTERNAL_MSVC_2017_DBG_MODE)

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace {

using ::testing::MakeExceptionSafetyTester;
using ::testing::strong_guarantee;
using ::testing::TestNothrowOp;
using ::testing::TestThrowingCtor;

using Thrower = testing::ThrowingValue<>;
using CopyNothrow = testing::ThrowingValue<testing::TypeSpec::kNoThrowCopy>;
using MoveNothrow = testing::ThrowingValue<testing::TypeSpec::kNoThrowMove>;
using ThrowingAlloc = testing::ThrowingAllocator<Thrower>;
using ThrowerVec = std::vector<Thrower, ThrowingAlloc>;
using ThrowingVariant =
    absl::variant<Thrower, CopyNothrow, MoveNothrow, ThrowerVec>;

struct ConversionException {};

template <class T>
struct ExceptionOnConversion {
  operator T() const {  // NOLINT
    throw ConversionException();
  }
};

void ToValuelessByException(ThrowingVariant& v) {  // NOLINT
  try {
    v.emplace<Thrower>();
    v.emplace<Thrower>(ExceptionOnConversion<Thrower>());
  } catch (const ConversionException&) {

  }
}

testing::AssertionResult VariantInvariants(ThrowingVariant* v) {
  using testing::AssertionFailure;
  using testing::AssertionSuccess;

  if (absl::holds_alternative<Thrower>(*v)) {
    auto& t = absl::get<Thrower>(*v);
    t = Thrower{-100};
    if (t.Get() != -100) {
      return AssertionFailure() << "Thrower should be assigned -100";
    }
  } else if (absl::holds_alternative<ThrowerVec>(*v)) {
    auto& tv = absl::get<ThrowerVec>(*v);
    tv.clear();
    tv.emplace_back(-100);
    if (tv.size() != 1 || tv[0].Get() != -100) {
      return AssertionFailure() << "ThrowerVec should be {Thrower{-100}}";
    }
  } else if (absl::holds_alternative<CopyNothrow>(*v)) {
    auto& t = absl::get<CopyNothrow>(*v);
    t = CopyNothrow{-100};
    if (t.Get() != -100) {
      return AssertionFailure() << "CopyNothrow should be assigned -100";
    }
  } else if (absl::holds_alternative<MoveNothrow>(*v)) {
    auto& t = absl::get<MoveNothrow>(*v);
    t = MoveNothrow{-100};
    if (t.Get() != -100) {
      return AssertionFailure() << "MoveNothrow should be assigned -100";
    }
  }

  if (!v->valueless_by_exception()) ToValuelessByException(*v);
  if (!v->valueless_by_exception()) {
    return AssertionFailure() << "Variant should be valueless_by_exception";
  }
  try {
    auto unused = absl::get<Thrower>(*v);
    static_cast<void>(unused);
    return AssertionFailure() << "Variant should not contain Thrower";
  } catch (const absl::bad_variant_access&) {
  } catch (...) {
    return AssertionFailure() << "Unexpected exception throw from absl::get";
  }

  v->emplace<Thrower>(100);
  if (!absl::holds_alternative<Thrower>(*v) ||
      absl::get<Thrower>(*v) != Thrower(100)) {
    return AssertionFailure() << "Variant should contain Thrower(100)";
  }
  v->emplace<ThrowerVec>({Thrower(100)});
  if (!absl::holds_alternative<ThrowerVec>(*v) ||
      absl::get<ThrowerVec>(*v)[0] != Thrower(100)) {
    return AssertionFailure()
           << "Variant should contain ThrowerVec{Thrower(100)}";
  }
  return AssertionSuccess();
}

template <typename... Args>
Thrower ExpectedThrower(Args&&... args) {
  return Thrower(42, args...);
}

ThrowerVec ExpectedThrowerVec() { return {Thrower(100), Thrower(200)}; }
ThrowingVariant ValuelessByException() {
  ThrowingVariant v;
  ToValuelessByException(v);
  return v;
}
ThrowingVariant WithThrower() { return Thrower(39); }
ThrowingVariant WithThrowerVec() {
  return ThrowerVec{Thrower(1), Thrower(2), Thrower(3)};
}
ThrowingVariant WithCopyNoThrow() { return CopyNothrow(39); }
ThrowingVariant WithMoveNoThrow() { return MoveNothrow(39); }

TEST(VariantExceptionSafetyTest, DefaultConstructor) {
  TestThrowingCtor<ThrowingVariant>();
}

TEST(VariantExceptionSafetyTest, CopyConstructor) {
  {
    ThrowingVariant v(ExpectedThrower());
    TestThrowingCtor<ThrowingVariant>(v);
  }
  {
    ThrowingVariant v(ExpectedThrowerVec());
    TestThrowingCtor<ThrowingVariant>(v);
  }
  {
    ThrowingVariant v(ValuelessByException());
    TestThrowingCtor<ThrowingVariant>(v);
  }
}

TEST(VariantExceptionSafetyTest, MoveConstructor) {
  {
    ThrowingVariant v(ExpectedThrower());
    TestThrowingCtor<ThrowingVariant>(std::move(v));
  }
  {
    ThrowingVariant v(ExpectedThrowerVec());
    TestThrowingCtor<ThrowingVariant>(std::move(v));
  }
  {
    ThrowingVariant v(ValuelessByException());
    TestThrowingCtor<ThrowingVariant>(std::move(v));
  }
}

TEST(VariantExceptionSafetyTest, ValueConstructor) {
  TestThrowingCtor<ThrowingVariant>(ExpectedThrower());
  TestThrowingCtor<ThrowingVariant>(ExpectedThrowerVec());
}

TEST(VariantExceptionSafetyTest, InPlaceTypeConstructor) {
  TestThrowingCtor<ThrowingVariant>(absl::in_place_type_t<Thrower>{},
                                    ExpectedThrower());
  TestThrowingCtor<ThrowingVariant>(absl::in_place_type_t<ThrowerVec>{},
                                    ExpectedThrowerVec());
}

TEST(VariantExceptionSafetyTest, InPlaceIndexConstructor) {
  TestThrowingCtor<ThrowingVariant>(absl::in_place_index_t<0>{},
                                    ExpectedThrower());
  TestThrowingCtor<ThrowingVariant>(absl::in_place_index_t<3>{},
                                    ExpectedThrowerVec());
}

TEST(VariantExceptionSafetyTest, CopyAssign) {


  {

    const ThrowingVariant rhs = ValuelessByException();
    ThrowingVariant lhs = ValuelessByException();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = rhs; }));
  }
  {

    const ThrowingVariant rhs = ValuelessByException();
    ThrowingVariant lhs = WithThrower();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = rhs; }));
  }

  {
    const ThrowingVariant rhs(ExpectedThrower());
    auto tester =
        MakeExceptionSafetyTester()
            .WithInitialValue(WithThrower())
            .WithOperation([&rhs](ThrowingVariant* lhs) { *lhs = rhs; });
    EXPECT_TRUE(tester.WithContracts(VariantInvariants).Test());
    EXPECT_FALSE(tester.WithContracts(strong_guarantee).Test());
  }
  {
    const ThrowingVariant rhs(ExpectedThrowerVec());
    auto tester =
        MakeExceptionSafetyTester()
            .WithInitialValue(WithThrowerVec())
            .WithOperation([&rhs](ThrowingVariant* lhs) { *lhs = rhs; });
    EXPECT_TRUE(tester.WithContracts(VariantInvariants).Test());
    EXPECT_FALSE(tester.WithContracts(strong_guarantee).Test());
  }


#if !(defined(ABSL_USES_STD_VARIANT) && defined(__GLIBCXX__))




  {



    const ThrowingVariant rhs(CopyNothrow{});
    ThrowingVariant lhs = WithThrower();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = rhs; }));
  }
  {




    const ThrowingVariant rhs(ExpectedThrower());
    auto tester =
        MakeExceptionSafetyTester()
            .WithInitialValue(WithCopyNoThrow())
            .WithOperation([&rhs](ThrowingVariant* lhs) { *lhs = rhs; });
    EXPECT_TRUE(tester
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* lhs) {
                                     return lhs->valueless_by_exception();
                                   })
                    .Test());
    EXPECT_FALSE(tester.WithContracts(strong_guarantee).Test());
  }
#endif  // !(defined(ABSL_USES_STD_VARIANT) && defined(__GLIBCXX__))
  {





    const ThrowingVariant rhs(MoveNothrow{});
    EXPECT_TRUE(MakeExceptionSafetyTester()
                    .WithInitialValue(WithThrower())
                    .WithContracts(VariantInvariants, strong_guarantee)
                    .Test([&rhs](ThrowingVariant* lhs) { *lhs = rhs; }));
  }
}

TEST(VariantExceptionSafetyTest, MoveAssign) {


  {

    ThrowingVariant rhs = ValuelessByException();
    ThrowingVariant lhs = ValuelessByException();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = std::move(rhs); }));
  }
  {

    ThrowingVariant rhs = ValuelessByException();
    ThrowingVariant lhs = WithThrower();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = std::move(rhs); }));
  }
  {





    ThrowingVariant rhs(ExpectedThrower());
    size_t j = rhs.index();

    auto tester = MakeExceptionSafetyTester()
                      .WithInitialValue(WithThrower())
                      .WithOperation([&](ThrowingVariant* lhs) {
                        auto copy = rhs;
                        *lhs = std::move(copy);
                      });
    EXPECT_TRUE(tester
                    .WithContracts(
                        VariantInvariants,
                        [&](ThrowingVariant* lhs) { return lhs->index() == j; })
                    .Test());
    EXPECT_FALSE(tester.WithContracts(strong_guarantee).Test());
  }
  {




#if !(defined(ABSL_USES_STD_VARIANT) && \
      defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE == 8)




    ThrowingVariant rhs(CopyNothrow{});
    EXPECT_TRUE(MakeExceptionSafetyTester()
                    .WithInitialValue(WithThrower())
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* lhs) {
                                     return lhs->valueless_by_exception();
                                   })
                    .Test([&](ThrowingVariant* lhs) {
                      auto copy = rhs;
                      *lhs = std::move(copy);
                    }));
#endif  // !(defined(ABSL_USES_STD_VARIANT) &&

  }
}

TEST(VariantExceptionSafetyTest, ValueAssign) {


  {







    Thrower rhs = ExpectedThrower();

    auto copy_tester =
        MakeExceptionSafetyTester()
            .WithInitialValue(WithThrower())
            .WithOperation([rhs](ThrowingVariant* lhs) { *lhs = rhs; });
    EXPECT_TRUE(copy_tester
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* lhs) {
                                     return !lhs->valueless_by_exception();
                                   })
                    .Test());
    EXPECT_FALSE(copy_tester.WithContracts(strong_guarantee).Test());

    auto move_tester = MakeExceptionSafetyTester()
                           .WithInitialValue(WithThrower())
                           .WithOperation([&](ThrowingVariant* lhs) {
                             auto copy = rhs;
                             *lhs = std::move(copy);
                           });
    EXPECT_TRUE(move_tester
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* lhs) {
                                     return !lhs->valueless_by_exception();
                                   })
                    .Test());

    EXPECT_FALSE(move_tester.WithContracts(strong_guarantee).Test());
  }








  {
    const CopyNothrow rhs;
    ThrowingVariant lhs = WithThrower();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = rhs; }));
  }
  {
    MoveNothrow rhs;
    ThrowingVariant lhs = WithThrower();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs = std::move(rhs); }));
  }




  {
    Thrower rhs = ExpectedThrower();

    auto copy_tester =
        MakeExceptionSafetyTester()
            .WithInitialValue(WithCopyNoThrow())
            .WithOperation([&rhs](ThrowingVariant* lhs) { *lhs = rhs; });
    EXPECT_TRUE(copy_tester
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* lhs) {
                                     return lhs->valueless_by_exception();
                                   })
                    .Test());
    EXPECT_FALSE(copy_tester.WithContracts(strong_guarantee).Test());

    auto move_tester = MakeExceptionSafetyTester()
                           .WithInitialValue(WithCopyNoThrow())
                           .WithOperation([](ThrowingVariant* lhs) {
                             *lhs = ExpectedThrower(testing::nothrow_ctor);
                           });
    EXPECT_TRUE(move_tester
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* lhs) {
                                     return lhs->valueless_by_exception();
                                   })
                    .Test());
    EXPECT_FALSE(move_tester.WithContracts(strong_guarantee).Test());
  }







#if !(defined(ABSL_USES_STD_VARIANT) && defined(__GLIBCXX__))
  {
    MoveNothrow rhs;
    EXPECT_TRUE(MakeExceptionSafetyTester()
                    .WithInitialValue(WithThrower())
                    .WithContracts(VariantInvariants, strong_guarantee)
                    .Test([&rhs](ThrowingVariant* lhs) { *lhs = rhs; }));
  }
#endif  // !(defined(ABSL_USES_STD_VARIANT) && defined(__GLIBCXX__))
}

TEST(VariantExceptionSafetyTest, Emplace) {



  {
    Thrower args = ExpectedThrower();
    auto tester = MakeExceptionSafetyTester()
                      .WithInitialValue(WithThrower())
                      .WithOperation([&args](ThrowingVariant* v) {
                        v->emplace<Thrower>(args);
                      });
    EXPECT_TRUE(tester
                    .WithContracts(VariantInvariants,
                                   [](ThrowingVariant* v) {
                                     return v->valueless_by_exception();
                                   })
                    .Test());
    EXPECT_FALSE(tester.WithContracts(strong_guarantee).Test());
  }
}

TEST(VariantExceptionSafetyTest, Swap) {

  {
    ThrowingVariant rhs = ValuelessByException();
    ThrowingVariant lhs = ValuelessByException();
    EXPECT_TRUE(TestNothrowOp([&]() { lhs.swap(rhs); }));
  }


  {
    ThrowingVariant rhs = ExpectedThrower();
    EXPECT_TRUE(MakeExceptionSafetyTester()
                    .WithInitialValue(WithThrower())
                    .WithContracts(VariantInvariants)
                    .Test([&](ThrowingVariant* lhs) {
                      auto copy = rhs;
                      lhs->swap(copy);
                    }));
  }




  {
    ThrowingVariant rhs = ExpectedThrower();
    EXPECT_TRUE(MakeExceptionSafetyTester()
                    .WithInitialValue(WithCopyNoThrow())
                    .WithContracts(VariantInvariants)
                    .Test([&](ThrowingVariant* lhs) {
                      auto copy = rhs;
                      lhs->swap(copy);
                    }));
  }
  {
    ThrowingVariant rhs = ExpectedThrower();
    EXPECT_TRUE(MakeExceptionSafetyTester()
                    .WithInitialValue(WithCopyNoThrow())
                    .WithContracts(VariantInvariants)
                    .Test([&](ThrowingVariant* lhs) {
                      auto copy = rhs;
                      copy.swap(*lhs);
                    }));
  }
}

}  // namespace
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // !defined(ABSL_INTERNAL_MSVC_2017_DBG_MODE)

#endif  // #if !defined(ABSL_USES_STD_VARIANT) && defined(ABSL_HAVE_EXCEPTIONS)
