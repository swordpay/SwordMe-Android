// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Tests for Google Test itself. This file verifies that the parameter
// generators objects produce correct parameter sequences and that
// Google Test runtime instantiates correct tests from those sequences.

#include "gtest/gtest.h"

# include <algorithm>
# include <iostream>
# include <list>
# include <sstream>
# include <string>
# include <vector>

# include "src/gtest-internal-inl.h"  // for UnitTestOptions
# include "test/googletest-param-test-test.h"

using ::std::vector;
using ::std::sort;

using ::testing::AddGlobalTestEnvironment;
using ::testing::Bool;
using ::testing::Combine;
using ::testing::Message;
using ::testing::Range;
using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

using ::testing::internal::ParamGenerator;
using ::testing::internal::UnitTestOptions;

//
// FIXME: remove PrintValue() when we move matchers and
// EXPECT_THAT() from Google Mock to Google Test.  At that time, we
// can write EXPECT_THAT(x, Eq(y)) to compare two tuples x and y, as
// EXPECT_THAT() and the matchers know how to print tuples.
template <typename T>
::std::string PrintValue(const T& value) {
  return testing::PrintToString(value);
}

// via the iterator object matches the expected one using Google Test
// assertions.
template <typename T, size_t N>
void VerifyGenerator(const ParamGenerator<T>& generator,
                     const T (&expected_values)[N]) {
  typename ParamGenerator<T>::iterator it = generator.begin();
  for (size_t i = 0; i < N; ++i) {
    ASSERT_FALSE(it == generator.end())
        << "At element " << i << " when accessing via an iterator "
        << "created with the copy constructor.\n";


    EXPECT_TRUE(expected_values[i] == *it)
        << "where i is " << i
        << ", expected_values[i] is " << PrintValue(expected_values[i])
        << ", *it is " << PrintValue(*it)
        << ", and 'it' is an iterator created with the copy constructor.\n";
    ++it;
  }
  EXPECT_TRUE(it == generator.end())
        << "At the presumed end of sequence when accessing via an iterator "
        << "created with the copy constructor.\n";




  it = generator.begin();
  for (size_t i = 0; i < N; ++i) {
    ASSERT_FALSE(it == generator.end())
        << "At element " << i << " when accessing via an iterator "
        << "created with the assignment operator.\n";
    EXPECT_TRUE(expected_values[i] == *it)
        << "where i is " << i
        << ", expected_values[i] is " << PrintValue(expected_values[i])
        << ", *it is " << PrintValue(*it)
        << ", and 'it' is an iterator created with the copy constructor.\n";
    ++it;
  }
  EXPECT_TRUE(it == generator.end())
        << "At the presumed end of sequence when accessing via an iterator "
        << "created with the assignment operator.\n";
}

template <typename T>
void VerifyGeneratorIsEmpty(const ParamGenerator<T>& generator) {
  typename ParamGenerator<T>::iterator it = generator.begin();
  EXPECT_TRUE(it == generator.end());

  it = generator.begin();
  EXPECT_TRUE(it == generator.end());
}

// generates an expected sequence of values. The general test pattern
// instantiates a generator using one of the generator functions,
// checks the sequence produced by the generator using its iterator API,
// and then resets the iterator back to the beginning of the sequence
// and checks the sequence again.

// ForwardIterator concept.
TEST(IteratorTest, ParamIteratorConformsToForwardIteratorConcept) {
  const ParamGenerator<int> gen = Range(0, 10);
  ParamGenerator<int>::iterator it = gen.begin();

  ParamGenerator<int>::iterator it2 = it;
  EXPECT_TRUE(*it == *it2) << "Initialized iterators must point to the "
                           << "element same as its source points to";

  ++it;
  EXPECT_FALSE(*it == *it2);
  it2 = it;
  EXPECT_TRUE(*it == *it2) << "Assigned iterators must point to the "
                           << "element same as its source points to";

  EXPECT_EQ(&it, &(++it)) << "Result of the prefix operator++ must be "
                          << "refer to the original object";


  int original_value = *it;  // Have to compute it outside of macro call to be

  EXPECT_EQ(original_value, *(it++));


  it2 = it;
  ++it;
  ++it2;
  EXPECT_TRUE(*it == *it2);
}

TEST(RangeTest, IntRangeWithDefaultStep) {
  const ParamGenerator<int> gen = Range(0, 3);
  const int expected_values[] = {0, 1, 2};
  VerifyGenerator(gen, expected_values);
}

// as expected when provided with range limits that are equal.
TEST(RangeTest, IntRangeSingleValue) {
  const ParamGenerator<int> gen = Range(0, 1);
  const int expected_values[] = {0};
  VerifyGenerator(gen, expected_values);
}

// supplied with an empty range.
TEST(RangeTest, IntRangeEmpty) {
  const ParamGenerator<int> gen = Range(0, 0);
  VerifyGeneratorIsEmpty(gen);
}

// the expected sequence.
TEST(RangeTest, IntRangeWithCustomStep) {
  const ParamGenerator<int> gen = Range(0, 9, 3);
  const int expected_values[] = {0, 3, 6};
  VerifyGenerator(gen, expected_values);
}

// the expected sequence when the last element does not fall on the
// upper range limit. Sequences generated by Range() must not have
// elements beyond the range limits.
TEST(RangeTest, IntRangeWithCustomStepOverUpperBound) {
  const ParamGenerator<int> gen = Range(0, 4, 3);
  const int expected_values[] = {0, 3};
  VerifyGenerator(gen, expected_values);
}

// copy constructor, operator=(), operator+(), and operator<().
class DogAdder {
 public:
  explicit DogAdder(const char* a_value) : value_(a_value) {}
  DogAdder(const DogAdder& other) : value_(other.value_.c_str()) {}

  DogAdder operator=(const DogAdder& other) {
    if (this != &other)
      value_ = other.value_;
    return *this;
  }
  DogAdder operator+(const DogAdder& other) const {
    Message msg;
    msg << value_.c_str() << other.value_.c_str();
    return DogAdder(msg.GetString().c_str());
  }
  bool operator<(const DogAdder& other) const {
    return value_ < other.value_;
  }
  const std::string& value() const { return value_; }

 private:
  std::string value_;
};

TEST(RangeTest, WorksWithACustomType) {
  const ParamGenerator<DogAdder> gen =
      Range(DogAdder("cat"), DogAdder("catdogdog"), DogAdder("dog"));
  ParamGenerator<DogAdder>::iterator it = gen.begin();

  ASSERT_FALSE(it == gen.end());
  EXPECT_STREQ("cat", it->value().c_str());

  ASSERT_FALSE(++it == gen.end());
  EXPECT_STREQ("catdog", it->value().c_str());

  EXPECT_TRUE(++it == gen.end());
}

class IntWrapper {
 public:
  explicit IntWrapper(int a_value) : value_(a_value) {}
  IntWrapper(const IntWrapper& other) : value_(other.value_) {}

  IntWrapper operator=(const IntWrapper& other) {
    value_ = other.value_;
    return *this;
  }

  IntWrapper operator+(int other) const { return IntWrapper(value_ + other); }
  bool operator<(const IntWrapper& other) const {
    return value_ < other.value_;
  }
  int value() const { return value_; }

 private:
  int value_;
};

TEST(RangeTest, WorksWithACustomTypeWithDifferentIncrementType) {
  const ParamGenerator<IntWrapper> gen = Range(IntWrapper(0), IntWrapper(2));
  ParamGenerator<IntWrapper>::iterator it = gen.begin();

  ASSERT_FALSE(it == gen.end());
  EXPECT_EQ(0, it->value());

  ASSERT_FALSE(++it == gen.end());
  EXPECT_EQ(1, it->value());

  EXPECT_TRUE(++it == gen.end());
}

// the expected sequence.
TEST(ValuesInTest, ValuesInArray) {
  int array[] = {3, 5, 8};
  const ParamGenerator<int> gen = ValuesIn(array);
  VerifyGenerator(gen, array);
}

// the expected sequence.
TEST(ValuesInTest, ValuesInConstArray) {
  const int array[] = {3, 5, 8};
  const ParamGenerator<int> gen = ValuesIn(array);
  VerifyGenerator(gen, array);
}

// single element generates the single element sequence.
TEST(ValuesInTest, ValuesInSingleElementArray) {
  int array[] = {42};
  const ParamGenerator<int> gen = ValuesIn(array);
  VerifyGenerator(gen, array);
}

// container (vector).
TEST(ValuesInTest, ValuesInVector) {
  typedef ::std::vector<int> ContainerType;
  ContainerType values;
  values.push_back(3);
  values.push_back(5);
  values.push_back(8);
  const ParamGenerator<int> gen = ValuesIn(values);

  const int expected_values[] = {3, 5, 8};
  VerifyGenerator(gen, expected_values);
}

TEST(ValuesInTest, ValuesInIteratorRange) {
  typedef ::std::vector<int> ContainerType;
  ContainerType values;
  values.push_back(3);
  values.push_back(5);
  values.push_back(8);
  const ParamGenerator<int> gen = ValuesIn(values.begin(), values.end());

  const int expected_values[] = {3, 5, 8};
  VerifyGenerator(gen, expected_values);
}

// single value generates a single-element sequence.
TEST(ValuesInTest, ValuesInSingleElementIteratorRange) {
  typedef ::std::vector<int> ContainerType;
  ContainerType values;
  values.push_back(42);
  const ParamGenerator<int> gen = ValuesIn(values.begin(), values.end());

  const int expected_values[] = {42};
  VerifyGenerator(gen, expected_values);
}

// generates an empty sequence.
TEST(ValuesInTest, ValuesInEmptyIteratorRange) {
  typedef ::std::vector<int> ContainerType;
  ContainerType values;
  const ParamGenerator<int> gen = ValuesIn(values.begin(), values.end());

  VerifyGeneratorIsEmpty(gen);
}

TEST(ValuesTest, ValuesWorks) {
  const ParamGenerator<int> gen = Values(3, 5, 8);

  const int expected_values[] = {3, 5, 8};
  VerifyGenerator(gen, expected_values);
}

// different types convertible to ParamGenerator's parameter type.
TEST(ValuesTest, ValuesWorksForValuesOfCompatibleTypes) {
  const ParamGenerator<double> gen = Values(3, 5.0f, 8.0);

  const double expected_values[] = {3.0, 5.0, 8.0};
  VerifyGenerator(gen, expected_values);
}

TEST(ValuesTest, ValuesWorksForMaxLengthList) {
  const ParamGenerator<int> gen = Values(
      10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
      110, 120, 130, 140, 150, 160, 170, 180, 190, 200,
      210, 220, 230, 240, 250, 260, 270, 280, 290, 300,
      310, 320, 330, 340, 350, 360, 370, 380, 390, 400,
      410, 420, 430, 440, 450, 460, 470, 480, 490, 500);

  const int expected_values[] = {
      10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
      110, 120, 130, 140, 150, 160, 170, 180, 190, 200,
      210, 220, 230, 240, 250, 260, 270, 280, 290, 300,
      310, 320, 330, 340, 350, 360, 370, 380, 390, 400,
      410, 420, 430, 440, 450, 460, 470, 480, 490, 500};
  VerifyGenerator(gen, expected_values);
}

// with the single value.
TEST(ValuesTest, ValuesWithSingleParameter) {
  const ParamGenerator<int> gen = Values(42);

  const int expected_values[] = {42};
  VerifyGenerator(gen, expected_values);
}

TEST(BoolTest, BoolWorks) {
  const ParamGenerator<bool> gen = Bool();

  const bool expected_values[] = {false, true};
  VerifyGenerator(gen, expected_values);
}

TEST(CombineTest, CombineWithTwoParameters) {
  const char* foo = "foo";
  const char* bar = "bar";
  const ParamGenerator<std::tuple<const char*, int> > gen =
      Combine(Values(foo, bar), Values(3, 4));

  std::tuple<const char*, int> expected_values[] = {
      std::make_tuple(foo, 3), std::make_tuple(foo, 4), std::make_tuple(bar, 3),
      std::make_tuple(bar, 4)};
  VerifyGenerator(gen, expected_values);
}

TEST(CombineTest, CombineWithThreeParameters) {
  const ParamGenerator<std::tuple<int, int, int> > gen =
      Combine(Values(0, 1), Values(3, 4), Values(5, 6));
  std::tuple<int, int, int> expected_values[] = {
      std::make_tuple(0, 3, 5), std::make_tuple(0, 3, 6),
      std::make_tuple(0, 4, 5), std::make_tuple(0, 4, 6),
      std::make_tuple(1, 3, 5), std::make_tuple(1, 3, 6),
      std::make_tuple(1, 4, 5), std::make_tuple(1, 4, 6)};
  VerifyGenerator(gen, expected_values);
}

// sequence generates a sequence with the number of elements equal to the
// number of elements in the sequence generated by the second parameter.
TEST(CombineTest, CombineWithFirstParameterSingleValue) {
  const ParamGenerator<std::tuple<int, int> > gen =
      Combine(Values(42), Values(0, 1));

  std::tuple<int, int> expected_values[] = {std::make_tuple(42, 0),
                                            std::make_tuple(42, 1)};
  VerifyGenerator(gen, expected_values);
}

// sequence generates a sequence with the number of elements equal to the
// number of elements in the sequence generated by the first parameter.
TEST(CombineTest, CombineWithSecondParameterSingleValue) {
  const ParamGenerator<std::tuple<int, int> > gen =
      Combine(Values(0, 1), Values(42));

  std::tuple<int, int> expected_values[] = {std::make_tuple(0, 42),
                                            std::make_tuple(1, 42)};
  VerifyGenerator(gen, expected_values);
}

// Combine() produces an empty sequence, too.
TEST(CombineTest, CombineWithFirstParameterEmptyRange) {
  const ParamGenerator<std::tuple<int, int> > gen =
      Combine(Range(0, 0), Values(0, 1));
  VerifyGeneratorIsEmpty(gen);
}

// Combine() produces an empty sequence, too.
TEST(CombineTest, CombineWithSecondParameterEmptyRange) {
  const ParamGenerator<std::tuple<int, int> > gen =
      Combine(Values(0, 1), Range(1, 1));
  VerifyGeneratorIsEmpty(gen);
}

// of parameters supported by Google Test (currently 10).
TEST(CombineTest, CombineWithMaxNumberOfParameters) {
  const char* foo = "foo";
  const char* bar = "bar";
  const ParamGenerator<
      std::tuple<const char*, int, int, int, int, int, int, int, int, int> >
      gen =
          Combine(Values(foo, bar), Values(1), Values(2), Values(3), Values(4),
                  Values(5), Values(6), Values(7), Values(8), Values(9));

  std::tuple<const char*, int, int, int, int, int, int, int, int, int>
      expected_values[] = {std::make_tuple(foo, 1, 2, 3, 4, 5, 6, 7, 8, 9),
                           std::make_tuple(bar, 1, 2, 3, 4, 5, 6, 7, 8, 9)};
  VerifyGenerator(gen, expected_values);
}

class NonDefaultConstructAssignString {
 public:
  NonDefaultConstructAssignString(const std::string& s) : str_(s) {}

  const std::string& str() const { return str_; }

 private:
  std::string str_;

  NonDefaultConstructAssignString();

  void operator=(const NonDefaultConstructAssignString&);
};

TEST(CombineTest, NonDefaultConstructAssign) {
  const ParamGenerator<std::tuple<int, NonDefaultConstructAssignString> > gen =
      Combine(Values(0, 1), Values(NonDefaultConstructAssignString("A"),
                                   NonDefaultConstructAssignString("B")));

  ParamGenerator<std::tuple<int, NonDefaultConstructAssignString> >::iterator
      it = gen.begin();

  EXPECT_EQ(0, std::get<0>(*it));
  EXPECT_EQ("A", std::get<1>(*it).str());
  ++it;

  EXPECT_EQ(0, std::get<0>(*it));
  EXPECT_EQ("B", std::get<1>(*it).str());
  ++it;

  EXPECT_EQ(1, std::get<0>(*it));
  EXPECT_EQ("A", std::get<1>(*it).str());
  ++it;

  EXPECT_EQ(1, std::get<0>(*it));
  EXPECT_EQ("B", std::get<1>(*it).str());
  ++it;

  EXPECT_TRUE(it == gen.end());
}

// assigned from another generator.
TEST(ParamGeneratorTest, AssignmentWorks) {
  ParamGenerator<int> gen = Values(1, 2);
  const ParamGenerator<int> gen2 = Values(3, 4);
  gen = gen2;

  const int expected_values[] = {3, 4};
  VerifyGenerator(gen, expected_values);
}

// one test per element from the sequence produced by the generator
// specified in INSTANTIATE_TEST_SUITE_P. It also verifies that the test's
// fixture constructor, SetUp(), and TearDown() have run and have been
// supplied with the correct parameters.

// case functionality is run at all. In this case TearDownTestSuite will not
// be able to detect missing tests, naturally.
template <int kExpectedCalls>
class TestGenerationEnvironment : public ::testing::Environment {
 public:
  static TestGenerationEnvironment* Instance() {
    static TestGenerationEnvironment* instance = new TestGenerationEnvironment;
    return instance;
  }

  void FixtureConstructorExecuted() { fixture_constructor_count_++; }
  void SetUpExecuted() { set_up_count_++; }
  void TearDownExecuted() { tear_down_count_++; }
  void TestBodyExecuted() { test_body_count_++; }

  void TearDown() override {


    bool perform_check = false;

    for (int i = 0; i < kExpectedCalls; ++i) {
      Message msg;
      msg << "TestsExpandedAndRun/" << i;
      if (UnitTestOptions::FilterMatchesTest(
             "TestExpansionModule/MultipleTestGenerationTest",
              msg.GetString().c_str())) {
        perform_check = true;
      }
    }
    if (perform_check) {
      EXPECT_EQ(kExpectedCalls, fixture_constructor_count_)
          << "Fixture constructor of ParamTestGenerationTest test case "
          << "has not been run as expected.";
      EXPECT_EQ(kExpectedCalls, set_up_count_)
          << "Fixture SetUp method of ParamTestGenerationTest test case "
          << "has not been run as expected.";
      EXPECT_EQ(kExpectedCalls, tear_down_count_)
          << "Fixture TearDown method of ParamTestGenerationTest test case "
          << "has not been run as expected.";
      EXPECT_EQ(kExpectedCalls, test_body_count_)
          << "Test in ParamTestGenerationTest test case "
          << "has not been run as expected.";
    }
  }

 private:
  TestGenerationEnvironment() : fixture_constructor_count_(0), set_up_count_(0),
                                tear_down_count_(0), test_body_count_(0) {}

  int fixture_constructor_count_;
  int set_up_count_;
  int tear_down_count_;
  int test_body_count_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestGenerationEnvironment);
};

const int test_generation_params[] = {36, 42, 72};

class TestGenerationTest : public TestWithParam<int> {
 public:
  enum {
    PARAMETER_COUNT =
        sizeof(test_generation_params)/sizeof(test_generation_params[0])
  };

  typedef TestGenerationEnvironment<PARAMETER_COUNT> Environment;

  TestGenerationTest() {
    Environment::Instance()->FixtureConstructorExecuted();
    current_parameter_ = GetParam();
  }
  void SetUp() override {
    Environment::Instance()->SetUpExecuted();
    EXPECT_EQ(current_parameter_, GetParam());
  }
  void TearDown() override {
    Environment::Instance()->TearDownExecuted();
    EXPECT_EQ(current_parameter_, GetParam());
  }

  static void SetUpTestSuite() {
    bool all_tests_in_test_case_selected = true;

    for (int i = 0; i < PARAMETER_COUNT; ++i) {
      Message test_name;
      test_name << "TestsExpandedAndRun/" << i;
      if ( !UnitTestOptions::FilterMatchesTest(
                "TestExpansionModule/MultipleTestGenerationTest",
                test_name.GetString())) {
        all_tests_in_test_case_selected = false;
      }
    }
    EXPECT_TRUE(all_tests_in_test_case_selected)
        << "When running the TestGenerationTest test case all of its tests\n"
        << "must be selected by the filter flag for the test case to pass.\n"
        << "If not all of them are enabled, we can't reliably conclude\n"
        << "that the correct number of tests have been generated.";

    collected_parameters_.clear();
  }

  static void TearDownTestSuite() {
    vector<int> expected_values(test_generation_params,
                                test_generation_params + PARAMETER_COUNT);



    sort(expected_values.begin(), expected_values.end());
    sort(collected_parameters_.begin(), collected_parameters_.end());

    EXPECT_TRUE(collected_parameters_ == expected_values);
  }

 protected:
  int current_parameter_;
  static vector<int> collected_parameters_;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestGenerationTest);
};
vector<int> TestGenerationTest::collected_parameters_;

TEST_P(TestGenerationTest, TestsExpandedAndRun) {
  Environment::Instance()->TestBodyExecuted();
  EXPECT_EQ(current_parameter_, GetParam());
  collected_parameters_.push_back(GetParam());
}
INSTANTIATE_TEST_SUITE_P(TestExpansionModule, TestGenerationTest,
                         ValuesIn(test_generation_params));

// INSTANTIATE_TEST_SUITE_P) is evaluated in InitGoogleTest() and neither at
// the call site of INSTANTIATE_TEST_SUITE_P nor in RUN_ALL_TESTS().  For
// that, we declare param_value_ to be a static member of
// GeneratorEvaluationTest and initialize it to 0.  We set it to 1 in
// main(), just before invocation of InitGoogleTest().  After calling
// InitGoogleTest(), we set the value to 2.  If the sequence is evaluated
// before or after InitGoogleTest, INSTANTIATE_TEST_SUITE_P will create a
// test with parameter other than 1, and the test body will fail the
// assertion.
class GeneratorEvaluationTest : public TestWithParam<int> {
 public:
  static int param_value() { return param_value_; }
  static void set_param_value(int param_value) { param_value_ = param_value; }

 private:
  static int param_value_;
};
int GeneratorEvaluationTest::param_value_ = 0;

TEST_P(GeneratorEvaluationTest, GeneratorsEvaluatedInMain) {
  EXPECT_EQ(1, GetParam());
}
INSTANTIATE_TEST_SUITE_P(GenEvalModule, GeneratorEvaluationTest,
                         Values(GeneratorEvaluationTest::param_value()));

// functional. Generator extern_gen is defined in gtest-param-test_test2.cc.
extern ParamGenerator<int> extern_gen;
class ExternalGeneratorTest : public TestWithParam<int> {};
TEST_P(ExternalGeneratorTest, ExternalGenerator) {


  EXPECT_EQ(GetParam(), 33);
}
INSTANTIATE_TEST_SUITE_P(ExternalGeneratorModule, ExternalGeneratorTest,
                         extern_gen);

// unit and instantiated in another. This test will be instantiated in
// gtest-param-test_test2.cc. ExternalInstantiationTest fixture class is
// defined in gtest-param-test_test.h.
TEST_P(ExternalInstantiationTest, IsMultipleOf33) {
  EXPECT_EQ(0, GetParam() % 33);
}

// generators.
class MultipleInstantiationTest : public TestWithParam<int> {};
TEST_P(MultipleInstantiationTest, AllowsMultipleInstances) {
}
INSTANTIATE_TEST_SUITE_P(Sequence1, MultipleInstantiationTest, Values(1, 2));
INSTANTIATE_TEST_SUITE_P(Sequence2, MultipleInstantiationTest, Range(3, 5));

// in multiple translation units. This test will be instantiated
// here and in gtest-param-test_test2.cc.
// InstantiationInMultipleTranslationUnitsTest fixture class
// is defined in gtest-param-test_test.h.
TEST_P(InstantiationInMultipleTranslationUnitsTest, IsMultipleOf42) {
  EXPECT_EQ(0, GetParam() % 42);
}
INSTANTIATE_TEST_SUITE_P(Sequence1, InstantiationInMultipleTranslationUnitsTest,
                         Values(42, 42 * 2));

// object.
class SeparateInstanceTest : public TestWithParam<int> {
 public:
  SeparateInstanceTest() : count_(0) {}

  static void TearDownTestSuite() {
    EXPECT_GE(global_count_, 2)
        << "If some (but not all) SeparateInstanceTest tests have been "
        << "filtered out this test will fail. Make sure that all "
        << "GeneratorEvaluationTest are selected or de-selected together "
        << "by the test filter.";
  }

 protected:
  int count_;
  static int global_count_;
};
int SeparateInstanceTest::global_count_ = 0;

TEST_P(SeparateInstanceTest, TestsRunInSeparateInstances) {
  EXPECT_EQ(0, count_++);
  global_count_++;
}
INSTANTIATE_TEST_SUITE_P(FourElemSequence, SeparateInstanceTest, Range(1, 4));

// defined with TEST_P(TestSuiteName, TestName) and instantiated with
// INSTANTIATE_TEST_SUITE_P(SequenceName, TestSuiteName, generator) must be
// named SequenceName/TestSuiteName.TestName/i, where i is the 0-based index of
// the sequence element used to instantiate the test.
class NamingTest : public TestWithParam<int> {};

TEST_P(NamingTest, TestsReportCorrectNamesAndParameters) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();

  EXPECT_STREQ("ZeroToFiveSequence/NamingTest", test_info->test_suite_name());

  Message index_stream;
  index_stream << "TestsReportCorrectNamesAndParameters/" << GetParam();
  EXPECT_STREQ(index_stream.GetString().c_str(), test_info->name());

  EXPECT_EQ(::testing::PrintToString(GetParam()), test_info->value_param());
}

INSTANTIATE_TEST_SUITE_P(ZeroToFiveSequence, NamingTest, Range(0, 5));

class MacroNamingTest : public TestWithParam<int> {};

#define PREFIX_WITH_FOO(test_name) Foo##test_name
#define PREFIX_WITH_MACRO(test_name) Macro##test_name

TEST_P(PREFIX_WITH_MACRO(NamingTest), PREFIX_WITH_FOO(SomeTestName)) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();

  EXPECT_STREQ("FortyTwo/MacroNamingTest", test_info->test_suite_name());
  EXPECT_STREQ("FooSomeTestName", test_info->name());
}

INSTANTIATE_TEST_SUITE_P(FortyTwo, MacroNamingTest, Values(42));

class MacroNamingTestNonParametrized : public ::testing::Test {};

TEST_F(PREFIX_WITH_MACRO(NamingTestNonParametrized),
       PREFIX_WITH_FOO(SomeTestName)) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();

  EXPECT_STREQ("MacroNamingTestNonParametrized", test_info->test_suite_name());
  EXPECT_STREQ("FooSomeTestName", test_info->name());
}

// Runs the test with a builtin helper method which uses PrintToString,
// as well as a custom function and custom functor to ensure all possible
// uses work correctly.
class CustomFunctorNamingTest : public TestWithParam<std::string> {};
TEST_P(CustomFunctorNamingTest, CustomTestNames) {}

struct CustomParamNameFunctor {
  std::string operator()(const ::testing::TestParamInfo<std::string>& inf) {
    return inf.param;
  }
};

INSTANTIATE_TEST_SUITE_P(CustomParamNameFunctor, CustomFunctorNamingTest,
                         Values(std::string("FunctorName")),
                         CustomParamNameFunctor());

INSTANTIATE_TEST_SUITE_P(AllAllowedCharacters, CustomFunctorNamingTest,
                         Values("abcdefghijklmnopqrstuvwxyz",
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "01234567890_"),
                         CustomParamNameFunctor());

inline std::string CustomParamNameFunction(
    const ::testing::TestParamInfo<std::string>& inf) {
  return inf.param;
}

class CustomFunctionNamingTest : public TestWithParam<std::string> {};
TEST_P(CustomFunctionNamingTest, CustomTestNames) {}

INSTANTIATE_TEST_SUITE_P(CustomParamNameFunction, CustomFunctionNamingTest,
                         Values(std::string("FunctionName")),
                         CustomParamNameFunction);

INSTANTIATE_TEST_SUITE_P(CustomParamNameFunctionP, CustomFunctionNamingTest,
                         Values(std::string("FunctionNameP")),
                         &CustomParamNameFunction);


class CustomLambdaNamingTest : public TestWithParam<std::string> {};
TEST_P(CustomLambdaNamingTest, CustomTestNames) {}

INSTANTIATE_TEST_SUITE_P(CustomParamNameLambda, CustomLambdaNamingTest,
                         Values(std::string("LambdaName")),
                         [](const ::testing::TestParamInfo<std::string>& inf) {
                           return inf.param;
                         });

TEST(CustomNamingTest, CheckNameRegistry) {
  ::testing::UnitTest* unit_test = ::testing::UnitTest::GetInstance();
  std::set<std::string> test_names;
  for (int suite_num = 0; suite_num < unit_test->total_test_suite_count();
       ++suite_num) {
    const ::testing::TestSuite* test_suite = unit_test->GetTestSuite(suite_num);
    for (int test_num = 0; test_num < test_suite->total_test_count();
         ++test_num) {
      const ::testing::TestInfo* test_info = test_suite->GetTestInfo(test_num);
      test_names.insert(std::string(test_info->name()));
    }
  }
  EXPECT_EQ(1u, test_names.count("CustomTestNames/FunctorName"));
  EXPECT_EQ(1u, test_names.count("CustomTestNames/FunctionName"));
  EXPECT_EQ(1u, test_names.count("CustomTestNames/FunctionNameP"));
  EXPECT_EQ(1u, test_names.count("CustomTestNames/LambdaName"));
}


class CustomIntegerNamingTest : public TestWithParam<int> {};

TEST_P(CustomIntegerNamingTest, TestsReportCorrectNames) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();
  Message test_name_stream;
  test_name_stream << "TestsReportCorrectNames/" << GetParam();
  EXPECT_STREQ(test_name_stream.GetString().c_str(), test_info->name());
}

INSTANTIATE_TEST_SUITE_P(PrintToString, CustomIntegerNamingTest, Range(0, 5),
                         ::testing::PrintToStringParamName());


struct CustomStruct {
  explicit CustomStruct(int value) : x(value) {}
  int x;
};

std::ostream& operator<<(std::ostream& stream, const CustomStruct& val) {
  stream << val.x;
  return stream;
}

class CustomStructNamingTest : public TestWithParam<CustomStruct> {};

TEST_P(CustomStructNamingTest, TestsReportCorrectNames) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();
  Message test_name_stream;
  test_name_stream << "TestsReportCorrectNames/" << GetParam();
  EXPECT_STREQ(test_name_stream.GetString().c_str(), test_info->name());
}

INSTANTIATE_TEST_SUITE_P(PrintToString, CustomStructNamingTest,
                         Values(CustomStruct(0), CustomStruct(1)),
                         ::testing::PrintToStringParamName());


struct StatefulNamingFunctor {
  StatefulNamingFunctor() : sum(0) {}
  std::string operator()(const ::testing::TestParamInfo<int>& info) {
    int value = info.param + sum;
    sum += info.param;
    return ::testing::PrintToString(value);
  }
  int sum;
};

class StatefulNamingTest : public ::testing::TestWithParam<int> {
 protected:
  StatefulNamingTest() : sum_(0) {}
  int sum_;
};

TEST_P(StatefulNamingTest, TestsReportCorrectNames) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();
  sum_ += GetParam();
  Message test_name_stream;
  test_name_stream << "TestsReportCorrectNames/" << sum_;
  EXPECT_STREQ(test_name_stream.GetString().c_str(), test_info->name());
}

INSTANTIATE_TEST_SUITE_P(StatefulNamingFunctor, StatefulNamingTest, Range(0, 5),
                         StatefulNamingFunctor());

// (and, in case of MSVC, also assignable) in order to be a test parameter
// type.  Its default copy constructor and assignment operator do exactly
// what we need.
class Unstreamable {
 public:
  explicit Unstreamable(int value) : value_(value) {}

  const int& dummy_value() const { return value_; }

 private:
  int value_;
};

class CommentTest : public TestWithParam<Unstreamable> {};

TEST_P(CommentTest, TestsCorrectlyReportUnstreamableParams) {
  const ::testing::TestInfo* const test_info =
     ::testing::UnitTest::GetInstance()->current_test_info();

  EXPECT_EQ(::testing::PrintToString(GetParam()), test_info->value_param());
}

INSTANTIATE_TEST_SUITE_P(InstantiationWithComments, CommentTest,
                         Values(Unstreamable(1)));

// class fixture is not parameterized and the derived class is. In this case
// ParameterizedDerivedTest inherits from NonParameterizedBaseTest.  We
// perform simple tests on both.
class NonParameterizedBaseTest : public ::testing::Test {
 public:
  NonParameterizedBaseTest() : n_(17) { }
 protected:
  int n_;
};

class ParameterizedDerivedTest : public NonParameterizedBaseTest,
                                 public ::testing::WithParamInterface<int> {
 protected:
  ParameterizedDerivedTest() : count_(0) { }
  int count_;
  static int global_count_;
};

int ParameterizedDerivedTest::global_count_ = 0;

TEST_F(NonParameterizedBaseTest, FixtureIsInitialized) {
  EXPECT_EQ(17, n_);
}

TEST_P(ParameterizedDerivedTest, SeesSequence) {
  EXPECT_EQ(17, n_);
  EXPECT_EQ(0, count_++);
  EXPECT_EQ(GetParam(), global_count_++);
}

class ParameterizedDeathTest : public ::testing::TestWithParam<int> { };

TEST_F(ParameterizedDeathTest, GetParamDiesFromTestF) {
  EXPECT_DEATH_IF_SUPPORTED(GetParam(),
                            ".* value-parameterized test .*");
}

INSTANTIATE_TEST_SUITE_P(RangeZeroToFive, ParameterizedDerivedTest,
                         Range(0, 5));

enum MyEnums {
  ENUM1 = 1,
  ENUM2 = 3,
  ENUM3 = 8,
};

class MyEnumTest : public testing::TestWithParam<MyEnums> {};

TEST_P(MyEnumTest, ChecksParamMoreThanZero) { EXPECT_GE(10, GetParam()); }
INSTANTIATE_TEST_SUITE_P(MyEnumTests, MyEnumTest,
                         ::testing::Values(ENUM1, ENUM2, 0));

int main(int argc, char **argv) {

  AddGlobalTestEnvironment(TestGenerationTest::Environment::Instance());


  GeneratorEvaluationTest::set_param_value(1);

  ::testing::InitGoogleTest(&argc, argv);



  GeneratorEvaluationTest::set_param_value(2);

  return RUN_ALL_TESTS();
}
