// Copyright 2005, Google Inc.
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

// The Google C++ Testing and Mocking Framework (Google Test)
//
// This header file defines the public API for Google Test.  It should be
// included by any test program that uses Google Test.
//
// IMPORTANT NOTE: Due to limitation of the C++ language, we have to
// leave some internal implementation details in this header file.
// They are clearly marked by comments like this:
//
//   // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
//
// Such code is NOT meant to be used by a user directly, and is subject
// to CHANGE WITHOUT NOTICE.  Therefore DO NOT DEPEND ON IT in a user
// program!
//
// Acknowledgment: Google Test borrowed the idea of automatic test
// registration from Barthelemy Dagenais' (barthelemy@prologique.com)
// easyUnit framework.


#ifndef GTEST_INCLUDE_GTEST_GTEST_H_
#define GTEST_INCLUDE_GTEST_GTEST_H_

#include <cstddef>
#include <limits>
#include <memory>
#include <ostream>
#include <type_traits>
#include <vector>

#include "gtest/internal/gtest-internal.h"
#include "gtest/internal/gtest-string.h"
#include "gtest/gtest-death-test.h"
#include "gtest/gtest-matchers.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest-param-test.h"
#include "gtest/gtest-printers.h"
#include "gtest/gtest_prod.h"
#include "gtest/gtest-test-part.h"
#include "gtest/gtest-typed-test.h"

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
/* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {

// unsafe mix of type 'const int' and type 'const bool'
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4805)
# pragma warning(disable:4100)
#endif


GTEST_DECLARE_bool_(also_run_disabled_tests);

GTEST_DECLARE_bool_(break_on_failure);

// and logs them as failures.
GTEST_DECLARE_bool_(catch_exceptions);

// "yes" to enable colors, "no" (disable colors), or "auto" (the default)
// to let Google Test decide.
GTEST_DECLARE_string_(color);

// the tests to run. If the filter is not given all tests are executed.
GTEST_DECLARE_string_(filter);

// debugging information when fatal signals are raised.
GTEST_DECLARE_bool_(install_failure_signal_handler);

// are actually run if the flag is provided.
GTEST_DECLARE_bool_(list_tests);

// in addition to its normal textual output.
GTEST_DECLARE_string_(output);

// test.
GTEST_DECLARE_bool_(print_time);

GTEST_DECLARE_bool_(print_utf8);

GTEST_DECLARE_int32_(random_seed);

// is 1. If the value is -1 the tests are repeating forever.
GTEST_DECLARE_int32_(repeat);

// stack frames in failure stack traces.
GTEST_DECLARE_bool_(show_internal_stack_frames);

GTEST_DECLARE_bool_(shuffle);

// printed in a failure message.
GTEST_DECLARE_int32_(stack_trace_depth);

// exception if exceptions are enabled, or exit the program with a
// non-zero code otherwise. For use with an external test framework.
GTEST_DECLARE_bool_(throw_on_failure);

// platforms test results are streamed to the specified port on
// the specified host machine.
GTEST_DECLARE_string_(stream_result_to);

#if GTEST_USE_OWN_FLAGFILE_FLAG_
GTEST_DECLARE_string_(flagfile);
#endif  // GTEST_USE_OWN_FLAGFILE_FLAG_

const int kMaxStackTraceDepth = 100;

namespace internal {

class AssertHelper;
class DefaultGlobalTestPartResultReporter;
class ExecDeathTest;
class NoExecDeathTest;
class FinalSuccessChecker;
class GTestFlagSaver;
class StreamingListenerTest;
class TestResultAccessor;
class TestEventListenersAccessor;
class TestEventRepeater;
class UnitTestRecordPropertyTestHelper;
class WindowsDeathTest;
class FuchsiaDeathTest;
class UnitTestImpl* GetUnitTestImpl();
void ReportFailureInUnknownLocation(TestPartResult::Type result_type,
                                    const std::string& message);

}  // namespace internal

// If we don't forward declare them the compiler might confuse the classes
// in friendship clauses with same named classes on the scope.
class Test;
class TestSuite;

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
using TestCase = TestSuite;
#endif
class TestInfo;
class UnitTest;

// the assertion wasn't successful, the AssertionResult object
// remembers a non-empty message that describes how it failed.
//
// To create an instance of this class, use one of the factory functions
// (AssertionSuccess() and AssertionFailure()).
//
// This class is useful for two purposes:
//   1. Defining predicate functions to be used with Boolean test assertions
//      EXPECT_TRUE/EXPECT_FALSE and their ASSERT_ counterparts
//   2. Defining predicate-format functions to be
//      used with predicate assertions (ASSERT_PRED_FORMAT*, etc).
//
// For example, if you define IsEven predicate:
//
//   testing::AssertionResult IsEven(int n) {
//     if ((n % 2) == 0)
//       return testing::AssertionSuccess();
//     else
//       return testing::AssertionFailure() << n << " is odd";
//   }
//
// Then the failed expectation EXPECT_TRUE(IsEven(Fib(5)))
// will print the message
//
//   Value of: IsEven(Fib(5))
//     Actual: false (5 is odd)
//   Expected: true
//
// instead of a more opaque
//
//   Value of: IsEven(Fib(5))
//     Actual: false
//   Expected: true
//
// in case IsEven is a simple Boolean predicate.
//
// If you expect your predicate to be reused and want to support informative
// messages in EXPECT_FALSE and ASSERT_FALSE (negative assertions show up
// about half as often as positive ones in our tests), supply messages for
// both success and failure cases:
//
//   testing::AssertionResult IsEven(int n) {
//     if ((n % 2) == 0)
//       return testing::AssertionSuccess() << n << " is even";
//     else
//       return testing::AssertionFailure() << n << " is odd";
//   }
//
// Then a statement EXPECT_FALSE(IsEven(Fib(6))) will print
//
//   Value of: IsEven(Fib(6))
//     Actual: true (8 is even)
//   Expected: false
//
// NB: Predicates that support negative Boolean assertions have reduced
// performance in positive ones so be careful not to use them in tests
// that have lots (tens of thousands) of positive Boolean assertions.
//
// To use this class with EXPECT_PRED_FORMAT assertions such as:
//
//   // Verifies that Foo() returns an even number.
//   EXPECT_PRED_FORMAT1(IsEven, Foo());
//
// you need to define:
//
//   testing::AssertionResult IsEven(const char* expr, int n) {
//     if ((n % 2) == 0)
//       return testing::AssertionSuccess();
//     else
//       return testing::AssertionFailure()
//         << "Expected: " << expr << " is even\n  Actual: it's " << n;
//   }
//
// If Foo() returns 5, you will see the following message:
//
//   Expected: Foo() is even
//     Actual: it's 5
//
class GTEST_API_ AssertionResult {
 public:


  AssertionResult(const AssertionResult& other);

#if defined(_MSC_VER) && _MSC_VER < 1910
  GTEST_DISABLE_MSC_WARNINGS_PUSH_(4800 /* forcing value to bool */)
#endif







  template <typename T>
  explicit AssertionResult(
      const T& success,
      typename internal::EnableIf<
          !std::is_convertible<T, AssertionResult>::value>::type*
      /*enabler*/
      = nullptr)
      : success_(success) {}

#if defined(_MSC_VER) && _MSC_VER < 1910
  GTEST_DISABLE_MSC_WARNINGS_POP_()
#endif

  AssertionResult& operator=(AssertionResult other) {
    swap(other);
    return *this;
  }

  operator bool() const { return success_; }  // NOLINT

  AssertionResult operator!() const;




  const char* message() const {
    return message_.get() != nullptr ? message_->c_str() : "";
  }

  const char* failure_message() const { return message(); }

  template <typename T> AssertionResult& operator<<(const T& value) {
    AppendMessage(Message() << value);
    return *this;
  }


  AssertionResult& operator<<(
      ::std::ostream& (*basic_manipulator)(::std::ostream& stream)) {
    AppendMessage(Message() << basic_manipulator);
    return *this;
  }

 private:

  void AppendMessage(const Message& a_message) {
    if (message_.get() == nullptr) message_.reset(new ::std::string);
    message_->append(a_message.GetString().c_str());
  }

  void swap(AssertionResult& other);

  bool success_;




  std::unique_ptr< ::std::string> message_;
};

GTEST_API_ AssertionResult AssertionSuccess();

GTEST_API_ AssertionResult AssertionFailure();

// Deprecated; use AssertionFailure() << msg.
GTEST_API_ AssertionResult AssertionFailure(const Message& msg);

}  // namespace testing

// predicate assertion macros. This include comes late because it relies on
// APIs declared above.
#include "gtest/gtest_pred_impl.h"

namespace testing {

//
// In Google Test, a unit test program contains one or many TestSuites, and
// each TestSuite contains one or many Tests.
//
// When you define a test using the TEST macro, you don't need to
// explicitly derive from Test - the TEST macro automatically does
// this for you.
//
// The only time you derive from Test is when defining a test fixture
// to be used in a TEST_F.  For example:
//
//   class FooTest : public testing::Test {
//    protected:
//     void SetUp() override { ... }
//     void TearDown() override { ... }
//     ...
//   };
//
//   TEST_F(FooTest, Bar) { ... }
//   TEST_F(FooTest, Baz) { ... }
//
// Test is not copyable.
class GTEST_API_ Test {
 public:
  friend class TestInfo;

  virtual ~Test();






  static void SetUpTestSuite() {}






  static void TearDownTestSuite() {}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  static void TearDownTestCase() {}
  static void SetUpTestCase() {}
#endif  // GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  static bool HasFatalFailure();

  static bool HasNonfatalFailure();

  static bool IsSkipped();


  static bool HasFailure() { return HasFatalFailure() || HasNonfatalFailure(); }













  static void RecordProperty(const std::string& key, const std::string& value);
  static void RecordProperty(const std::string& key, int value);

 protected:

  Test();

  virtual void SetUp();

  virtual void TearDown();

 private:


  static bool HasSameFixtureClass();






  virtual void TestBody() = 0;

  void Run();


  void DeleteSelf_() { delete this; }

  const std::unique_ptr<GTEST_FLAG_SAVER_> gtest_flag_saver_;
















  struct Setup_should_be_spelled_SetUp {};
  virtual Setup_should_be_spelled_SetUp* Setup() { return nullptr; }

  GTEST_DISALLOW_COPY_AND_ASSIGN_(Test);
};

typedef internal::TimeInMillis TimeInMillis;

// output as a key/value string pair.
//
// Don't inherit from TestProperty as its destructor is not virtual.
class TestProperty {
 public:



  TestProperty(const std::string& a_key, const std::string& a_value) :
    key_(a_key), value_(a_value) {
  }

  const char* key() const {
    return key_.c_str();
  }

  const char* value() const {
    return value_.c_str();
  }

  void SetValue(const std::string& new_value) {
    value_ = new_value;
  }

 private:

  std::string key_;

  std::string value_;
};

// TestPartResults, a list of TestProperties, a count of how many
// death tests there are in the Test, and how much time it took to run
// the Test.
//
// TestResult is not copyable.
class GTEST_API_ TestResult {
 public:

  TestResult();

  ~TestResult();


  int total_part_count() const;

  int test_property_count() const;

  bool Passed() const { return !Skipped() && !Failed(); }

  bool Skipped() const;

  bool Failed() const;

  bool HasFatalFailure() const;

  bool HasNonfatalFailure() const;

  TimeInMillis elapsed_time() const { return elapsed_time_; }


  const TestPartResult& GetTestPartResult(int i) const;



  const TestProperty& GetTestProperty(int i) const;

 private:
  friend class TestInfo;
  friend class TestSuite;
  friend class UnitTest;
  friend class internal::DefaultGlobalTestPartResultReporter;
  friend class internal::ExecDeathTest;
  friend class internal::TestResultAccessor;
  friend class internal::UnitTestImpl;
  friend class internal::WindowsDeathTest;
  friend class internal::FuchsiaDeathTest;

  const std::vector<TestPartResult>& test_part_results() const {
    return test_part_results_;
  }

  const std::vector<TestProperty>& test_properties() const {
    return test_properties_;
  }

  void set_elapsed_time(TimeInMillis elapsed) { elapsed_time_ = elapsed; }






  void RecordProperty(const std::string& xml_element,
                      const TestProperty& test_property);



  static bool ValidateTestProperty(const std::string& xml_element,
                                   const TestProperty& test_property);

  void AddTestPartResult(const TestPartResult& test_part_result);

  int death_test_count() const { return death_test_count_; }

  int increment_death_test_count() { return ++death_test_count_; }

  void ClearTestPartResults();

  void Clear();


  internal::Mutex test_properites_mutex_;

  std::vector<TestPartResult> test_part_results_;

  std::vector<TestProperty> test_properties_;

  int death_test_count_;

  TimeInMillis elapsed_time_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestResult);
};  // class TestResult

//
//   Test suite name
//   Test name
//   Whether the test should be run
//   A function pointer that creates the test object when invoked
//   Test result
//
// The constructor of TestInfo registers itself with the UnitTest
// singleton such that the RUN_ALL_TESTS() macro knows which tests to
// run.
class GTEST_API_ TestInfo {
 public:


  ~TestInfo();

  const char* test_suite_name() const { return test_suite_name_.c_str(); }

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  const char* test_case_name() const { return test_suite_name(); }
#endif  // GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  const char* name() const { return name_.c_str(); }


  const char* type_param() const {
    if (type_param_.get() != nullptr) return type_param_->c_str();
    return nullptr;
  }


  const char* value_param() const {
    if (value_param_.get() != nullptr) return value_param_->c_str();
    return nullptr;
  }

  const char* file() const { return location_.file.c_str(); }

  int line() const { return location_.line; }

  bool is_in_another_shard() const { return is_in_another_shard_; }
















  bool should_run() const { return should_run_; }

  bool is_reportable() const {


    return matches_filter_ && !is_in_another_shard_;
  }

  const TestResult* result() const { return &result_; }

 private:
#if GTEST_HAS_DEATH_TEST
  friend class internal::DefaultDeathTestFactory;
#endif  // GTEST_HAS_DEATH_TEST
  friend class Test;
  friend class TestSuite;
  friend class internal::UnitTestImpl;
  friend class internal::StreamingListenerTest;
  friend TestInfo* internal::MakeAndRegisterTestInfo(
      const char* test_suite_name, const char* name, const char* type_param,
      const char* value_param, internal::CodeLocation code_location,
      internal::TypeId fixture_class_id, internal::SetUpTestSuiteFunc set_up_tc,
      internal::TearDownTestSuiteFunc tear_down_tc,
      internal::TestFactoryBase* factory);


  TestInfo(const std::string& test_suite_name, const std::string& name,
           const char* a_type_param,   // NULL if not a type-parameterized test
           const char* a_value_param,  // NULL if not a value-parameterized test
           internal::CodeLocation a_code_location,
           internal::TypeId fixture_class_id,
           internal::TestFactoryBase* factory);


  int increment_death_test_count() {
    return result_.increment_death_test_count();
  }


  void Run();

  static void ClearTestResult(TestInfo* test_info) {
    test_info->result_.Clear();
  }

  const std::string test_suite_name_;    // test suite name
  const std::string name_;               // Test name


  const std::unique_ptr<const ::std::string> type_param_;


  const std::unique_ptr<const ::std::string> value_param_;
  internal::CodeLocation location_;
  const internal::TypeId fixture_class_id_;   // ID of the test fixture class
  bool should_run_;                 // True iff this test should run
  bool is_disabled_;                // True iff this test is disabled
  bool matches_filter_;             // True if this test matches the

  bool is_in_another_shard_;        // Will be run in another shard.
  internal::TestFactoryBase* const factory_;  // The factory that creates



  TestResult result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestInfo);
};

//
// TestSuite is not copyable.
class GTEST_API_ TestSuite {
 public:












  TestSuite(const char* name, const char* a_type_param,
            internal::SetUpTestSuiteFunc set_up_tc,
            internal::TearDownTestSuiteFunc tear_down_tc);

  virtual ~TestSuite();

  const char* name() const { return name_.c_str(); }


  const char* type_param() const {
    if (type_param_.get() != nullptr) return type_param_->c_str();
    return nullptr;
  }

  bool should_run() const { return should_run_; }

  int successful_test_count() const;

  int skipped_test_count() const;

  int failed_test_count() const;

  int reportable_disabled_test_count() const;

  int disabled_test_count() const;

  int reportable_test_count() const;

  int test_to_run_count() const;

  int total_test_count() const;

  bool Passed() const { return !Failed(); }

  bool Failed() const { return failed_test_count() > 0; }

  TimeInMillis elapsed_time() const { return elapsed_time_; }


  const TestInfo* GetTestInfo(int i) const;


  const TestResult& ad_hoc_test_result() const { return ad_hoc_test_result_; }

 private:
  friend class Test;
  friend class internal::UnitTestImpl;

  std::vector<TestInfo*>& test_info_list() { return test_info_list_; }

  const std::vector<TestInfo*>& test_info_list() const {
    return test_info_list_;
  }


  TestInfo* GetMutableTestInfo(int i);

  void set_should_run(bool should) { should_run_ = should; }


  void AddTestInfo(TestInfo * test_info);

  void ClearResult();

  static void ClearTestSuiteResult(TestSuite* test_suite) {
    test_suite->ClearResult();
  }

  void Run();


  void RunSetUpTestSuite() {
    if (set_up_tc_ != nullptr) {
      (*set_up_tc_)();
    }
  }


  void RunTearDownTestSuite() {
    if (tear_down_tc_ != nullptr) {
      (*tear_down_tc_)();
    }
  }

  static bool TestPassed(const TestInfo* test_info) {
    return test_info->should_run() && test_info->result()->Passed();
  }

  static bool TestSkipped(const TestInfo* test_info) {
    return test_info->should_run() && test_info->result()->Skipped();
  }

  static bool TestFailed(const TestInfo* test_info) {
    return test_info->should_run() && test_info->result()->Failed();
  }


  static bool TestReportableDisabled(const TestInfo* test_info) {
    return test_info->is_reportable() && test_info->is_disabled_;
  }

  static bool TestDisabled(const TestInfo* test_info) {
    return test_info->is_disabled_;
  }

  static bool TestReportable(const TestInfo* test_info) {
    return test_info->is_reportable();
  }

  static bool ShouldRunTest(const TestInfo* test_info) {
    return test_info->should_run();
  }

  void ShuffleTests(internal::Random* random);

  void UnshuffleTests();

  std::string name_;


  const std::unique_ptr<const ::std::string> type_param_;


  std::vector<TestInfo*> test_info_list_;



  std::vector<int> test_indices_;

  internal::SetUpTestSuiteFunc set_up_tc_;

  internal::TearDownTestSuiteFunc tear_down_tc_;

  bool should_run_;

  TimeInMillis elapsed_time_;


  TestResult ad_hoc_test_result_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestSuite);
};

// environment.  You should subclass this to define your own
// environment(s).
//
// An Environment object does the set-up and tear-down in virtual
// methods SetUp() and TearDown() instead of the constructor and the
// destructor, as:
//
//   1. You cannot safely throw from a destructor.  This is a problem
//      as in some cases Google Test is used where exceptions are enabled, and
//      we may want to implement ASSERT_* using exceptions where they are
//      available.
//   2. You cannot use ASSERT_* directly in a constructor or
//      destructor.
class Environment {
 public:

  virtual ~Environment() {}

  virtual void SetUp() {}

  virtual void TearDown() {}
 private:


  struct Setup_should_be_spelled_SetUp {};
  virtual Setup_should_be_spelled_SetUp* Setup() { return nullptr; }
};

#if GTEST_HAS_EXCEPTIONS

class GTEST_API_ AssertionException
    : public internal::GoogleTestFailureException {
 public:
  explicit AssertionException(const TestPartResult& result)
      : GoogleTestFailureException(result) {}
};

#endif  // GTEST_HAS_EXCEPTIONS

// the order the corresponding events are fired.
class TestEventListener {
 public:
  virtual ~TestEventListener() {}

  virtual void OnTestProgramStart(const UnitTest& unit_test) = 0;



  virtual void OnTestIterationStart(const UnitTest& unit_test,
                                    int iteration) = 0;

  virtual void OnEnvironmentsSetUpStart(const UnitTest& unit_test) = 0;

  virtual void OnEnvironmentsSetUpEnd(const UnitTest& unit_test) = 0;

  virtual void OnTestSuiteStart(const TestSuite& /*test_suite*/) {}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  virtual void OnTestCaseStart(const TestCase& /*test_case*/) {}
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  virtual void OnTestStart(const TestInfo& test_info) = 0;



  virtual void OnTestPartResult(const TestPartResult& test_part_result) = 0;

  virtual void OnTestEnd(const TestInfo& test_info) = 0;

  virtual void OnTestSuiteEnd(const TestSuite& /*test_suite*/) {}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  virtual void OnTestCaseEnd(const TestCase& /*test_case*/) {}
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  virtual void OnEnvironmentsTearDownStart(const UnitTest& unit_test) = 0;

  virtual void OnEnvironmentsTearDownEnd(const UnitTest& unit_test) = 0;

  virtual void OnTestIterationEnd(const UnitTest& unit_test,
                                  int iteration) = 0;

  virtual void OnTestProgramEnd(const UnitTest& unit_test) = 0;
};

// methods and are not concerned that a possible change to a signature of
// the methods they override will not be caught during the build.  For
// comments about each method please see the definition of TestEventListener
// above.
class EmptyTestEventListener : public TestEventListener {
 public:
  void OnTestProgramStart(const UnitTest& /*unit_test*/) override {}
  void OnTestIterationStart(const UnitTest& /*unit_test*/,
                            int /*iteration*/) override {}
  void OnEnvironmentsSetUpStart(const UnitTest& /*unit_test*/) override {}
  void OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/) override {}
  void OnTestSuiteStart(const TestSuite& /*test_suite*/) override {}
//  Legacy API is deprecated but still available
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseStart(const TestCase& /*test_case*/) override {}
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  void OnTestStart(const TestInfo& /*test_info*/) override {}
  void OnTestPartResult(const TestPartResult& /*test_part_result*/) override {}
  void OnTestEnd(const TestInfo& /*test_info*/) override {}
  void OnTestSuiteEnd(const TestSuite& /*test_suite*/) override {}
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  void OnTestCaseEnd(const TestCase& /*test_case*/) override {}
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

  void OnEnvironmentsTearDownStart(const UnitTest& /*unit_test*/) override {}
  void OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/) override {}
  void OnTestIterationEnd(const UnitTest& /*unit_test*/,
                          int /*iteration*/) override {}
  void OnTestProgramEnd(const UnitTest& /*unit_test*/) override {}
};

class GTEST_API_ TestEventListeners {
 public:
  TestEventListeners();
  ~TestEventListeners();



  void Append(TestEventListener* listener);



  TestEventListener* Release(TestEventListener* listener);





  TestEventListener* default_result_printer() const {
    return default_result_printer_;
  }







  TestEventListener* default_xml_generator() const {
    return default_xml_generator_;
  }

 private:
  friend class TestSuite;
  friend class TestInfo;
  friend class internal::DefaultGlobalTestPartResultReporter;
  friend class internal::NoExecDeathTest;
  friend class internal::TestEventListenersAccessor;
  friend class internal::UnitTestImpl;


  TestEventListener* repeater();





  void SetDefaultResultPrinter(TestEventListener* listener);





  void SetDefaultXmlGenerator(TestEventListener* listener);


  bool EventForwardingEnabled() const;
  void SuppressEventForwarding();

  internal::TestEventRepeater* repeater_;

  TestEventListener* default_result_printer_;

  TestEventListener* default_xml_generator_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestEventListeners);
};

//
// This is a singleton class.  The only instance of UnitTest is
// created when UnitTest::GetInstance() is first called.  This
// instance is never deleted.
//
// UnitTest is not copyable.
//
// This class is thread-safe as long as the methods are called
// according to their specification.
class GTEST_API_ UnitTest {
 public:



  static UnitTest* GetInstance();






  int Run() GTEST_MUST_USE_RESULT_;


  const char* original_working_dir() const;


  const TestSuite* current_test_suite() const GTEST_LOCK_EXCLUDED_(mutex_);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  const TestCase* current_test_case() const GTEST_LOCK_EXCLUDED_(mutex_);
#endif


  const TestInfo* current_test_info() const
      GTEST_LOCK_EXCLUDED_(mutex_);

  int random_seed() const;




  internal::ParameterizedTestSuiteRegistry& parameterized_test_registry()
      GTEST_LOCK_EXCLUDED_(mutex_);

  int successful_test_suite_count() const;

  int failed_test_suite_count() const;

  int total_test_suite_count() const;


  int test_suite_to_run_count() const;

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  int successful_test_case_count() const;
  int failed_test_case_count() const;
  int total_test_case_count() const;
  int test_case_to_run_count() const;
#endif  //  EMOVE_LEGACY_TEST_CASEAPI

  int successful_test_count() const;

  int skipped_test_count() const;

  int failed_test_count() const;

  int reportable_disabled_test_count() const;

  int disabled_test_count() const;

  int reportable_test_count() const;

  int total_test_count() const;

  int test_to_run_count() const;


  TimeInMillis start_timestamp() const;

  TimeInMillis elapsed_time() const;

  bool Passed() const;


  bool Failed() const;


  const TestSuite* GetTestSuite(int i) const;

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  const TestCase* GetTestCase(int i) const;
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_


  const TestResult& ad_hoc_test_result() const;


  TestEventListeners& listeners();

 private:









  Environment* AddEnvironment(Environment* env);




  void AddTestPartResult(TestPartResult::Type result_type,
                         const char* file_name,
                         int line_number,
                         const std::string& message,
                         const std::string& os_stack_trace)
      GTEST_LOCK_EXCLUDED_(mutex_);





  void RecordProperty(const std::string& key, const std::string& value);


  TestSuite* GetMutableTestSuite(int i);

  internal::UnitTestImpl* impl() { return impl_; }
  const internal::UnitTestImpl* impl() const { return impl_; }


  friend class ScopedTrace;
  friend class Test;
  friend class internal::AssertHelper;
  friend class internal::StreamingListenerTest;
  friend class internal::UnitTestRecordPropertyTestHelper;
  friend Environment* AddGlobalTestEnvironment(Environment* env);
  friend internal::UnitTestImpl* internal::GetUnitTestImpl();
  friend void internal::ReportFailureInUnknownLocation(
      TestPartResult::Type result_type,
      const std::string& message);

  UnitTest();

  virtual ~UnitTest();


  void PushGTestTrace(const internal::TraceInfo& trace)
      GTEST_LOCK_EXCLUDED_(mutex_);

  void PopGTestTrace()
      GTEST_LOCK_EXCLUDED_(mutex_);


  mutable internal::Mutex mutex_;




  internal::UnitTestImpl* impl_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTest);
};

// program.
//
// You should call this before RUN_ALL_TESTS() is called, probably in
// main().  If you use gtest_main, you need to call this before main()
// starts for it to take effect.  For example, you can define a global
// variable like this:
//
//   testing::Environment* const foo_env =
//       testing::AddGlobalTestEnvironment(new FooEnvironment);
//
// However, we strongly recommend you to write your own main() and
// call AddGlobalTestEnvironment() there, as relying on initialization
// of global variables makes the code harder to read and may cause
// problems when you register multiple environments from different
// translation units and the environments have dependencies among them
// (remember that the compiler doesn't guarantee the order in which
// global variables from different translation units are initialized).
inline Environment* AddGlobalTestEnvironment(Environment* env) {
  return UnitTest::GetInstance()->AddEnvironment(env);
}

// RUN_ALL_TESTS().  In particular, it parses a command line for the
// flags that Google Test recognizes.  Whenever a Google Test flag is
// seen, it is removed from argv, and *argc is decremented.
//
// No value is returned.  Instead, the Google Test flag variables are
// updated.
//
// Calling the function for the second time has no user-visible effect.
GTEST_API_ void InitGoogleTest(int* argc, char** argv);

// UNICODE mode.
GTEST_API_ void InitGoogleTest(int* argc, wchar_t** argv);

// there is no argc/argv.
GTEST_API_ void InitGoogleTest();

namespace internal {

// frame size of CmpHelperEQ. This helps reduce the overhead of some sanitizers
// when calling EXPECT_* in a tight loop.
template <typename T1, typename T2>
AssertionResult CmpHelperEQFailure(const char* lhs_expression,
                                   const char* rhs_expression,
                                   const T1& lhs, const T2& rhs) {
  return EqFailure(lhs_expression,
                   rhs_expression,
                   FormatForComparisonFailureMessage(lhs, rhs),
                   FormatForComparisonFailureMessage(rhs, lhs),
                   false);
}

// to block lexical scope lookup.
// It prevents using invalid operator==/!= defined at namespace scope.
struct faketype {};
inline bool operator==(faketype, faketype) { return true; }
inline bool operator!=(faketype, faketype) { return false; }

template <typename T1, typename T2>
AssertionResult CmpHelperEQ(const char* lhs_expression,
                            const char* rhs_expression,
                            const T1& lhs,
                            const T2& rhs) {
  if (lhs == rhs) {
    return AssertionSuccess();
  }

  return CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
}

// in {ASSERT|EXPECT}_EQ when compiled with gcc 4, as anonymous enums
// can be implicitly cast to BiggestInt.
GTEST_API_ AssertionResult CmpHelperEQ(const char* lhs_expression,
                                       const char* rhs_expression,
                                       BiggestInt lhs,
                                       BiggestInt rhs);

class EqHelper {
 public:

  template <
      typename T1, typename T2,


      typename std::enable_if<!std::is_integral<T1>::value ||
                              !std::is_pointer<T2>::value>::type* = nullptr>
  static AssertionResult Compare(const char* lhs_expression,
                                 const char* rhs_expression, const T1& lhs,
                                 const T2& rhs) {
    return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
  }






  static AssertionResult Compare(const char* lhs_expression,
                                 const char* rhs_expression,
                                 BiggestInt lhs,
                                 BiggestInt rhs) {
    return CmpHelperEQ(lhs_expression, rhs_expression, lhs, rhs);
  }

  template <typename T>
  static AssertionResult Compare(
      const char* lhs_expression, const char* rhs_expression,

      std::nullptr_t /* lhs */, T* rhs) {

    return CmpHelperEQ(lhs_expression, rhs_expression, static_cast<T*>(nullptr),
                       rhs);
  }
};

// frame size of CmpHelperOP. This helps reduce the overhead of some sanitizers
// when calling EXPECT_OP in a tight loop.
template <typename T1, typename T2>
AssertionResult CmpHelperOpFailure(const char* expr1, const char* expr2,
                                   const T1& val1, const T2& val2,
                                   const char* op) {
  return AssertionFailure()
         << "Expected: (" << expr1 << ") " << op << " (" << expr2
         << "), actual: " << FormatForComparisonFailureMessage(val1, val2)
         << " vs " << FormatForComparisonFailureMessage(val2, val1);
}

// ASSERT_?? and EXPECT_??.  It is here just to avoid copy-and-paste
// of similar code.
//
// For each templatized helper function, we also define an overloaded
// version for BiggestInt in order to reduce code bloat and allow
// anonymous enums to be used with {ASSERT|EXPECT}_?? when compiled
// with gcc 4.
//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.

#define GTEST_IMPL_CMP_HELPER_(op_name, op)\
template <typename T1, typename T2>\
AssertionResult CmpHelper##op_name(const char* expr1, const char* expr2, \
                                   const T1& val1, const T2& val2) {\
  if (val1 op val2) {\
    return AssertionSuccess();\
  } else {\
    return CmpHelperOpFailure(expr1, expr2, val1, val2, #op);\
  }\
}\
GTEST_API_ AssertionResult CmpHelper##op_name(\
    const char* expr1, const char* expr2, BiggestInt val1, BiggestInt val2)


GTEST_IMPL_CMP_HELPER_(NE, !=);
// Implements the helper function for {ASSERT|EXPECT}_LE
GTEST_IMPL_CMP_HELPER_(LE, <=);
// Implements the helper function for {ASSERT|EXPECT}_LT
GTEST_IMPL_CMP_HELPER_(LT, <);
// Implements the helper function for {ASSERT|EXPECT}_GE
GTEST_IMPL_CMP_HELPER_(GE, >=);
// Implements the helper function for {ASSERT|EXPECT}_GT
GTEST_IMPL_CMP_HELPER_(GT, >);

#undef GTEST_IMPL_CMP_HELPER_

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult CmpHelperSTREQ(const char* s1_expression,
                                          const char* s2_expression,
                                          const char* s1,
                                          const char* s2);

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult CmpHelperSTRCASEEQ(const char* s1_expression,
                                              const char* s2_expression,
                                              const char* s1,
                                              const char* s2);

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult CmpHelperSTRNE(const char* s1_expression,
                                          const char* s2_expression,
                                          const char* s1,
                                          const char* s2);

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult CmpHelperSTRCASENE(const char* s1_expression,
                                              const char* s2_expression,
                                              const char* s1,
                                              const char* s2);

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult CmpHelperSTREQ(const char* s1_expression,
                                          const char* s2_expression,
                                          const wchar_t* s1,
                                          const wchar_t* s2);

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult CmpHelperSTRNE(const char* s1_expression,
                                          const char* s2_expression,
                                          const wchar_t* s1,
                                          const wchar_t* s2);

}  // namespace internal

// first argument to {EXPECT,ASSERT}_PRED_FORMAT2(), not by
// themselves.  They check whether needle is a substring of haystack
// (NULL is considered a substring of itself only), and return an
// appropriate error message when they fail.
//
// The {needle,haystack}_expr arguments are the stringified
// expressions that generated the two real arguments.
GTEST_API_ AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const char* needle, const char* haystack);
GTEST_API_ AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const wchar_t* needle, const wchar_t* haystack);
GTEST_API_ AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const char* needle, const char* haystack);
GTEST_API_ AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const wchar_t* needle, const wchar_t* haystack);
GTEST_API_ AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::string& needle, const ::std::string& haystack);
GTEST_API_ AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::string& needle, const ::std::string& haystack);

#if GTEST_HAS_STD_WSTRING
GTEST_API_ AssertionResult IsSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::wstring& needle, const ::std::wstring& haystack);
GTEST_API_ AssertionResult IsNotSubstring(
    const char* needle_expr, const char* haystack_expr,
    const ::std::wstring& needle, const ::std::wstring& haystack);
#endif  // GTEST_HAS_STD_WSTRING

namespace internal {

//
// Template parameter:
//
//   RawType: the raw floating-point type (either float or double)
//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
template <typename RawType>
AssertionResult CmpHelperFloatingPointEQ(const char* lhs_expression,
                                         const char* rhs_expression,
                                         RawType lhs_value,
                                         RawType rhs_value) {
  const FloatingPoint<RawType> lhs(lhs_value), rhs(rhs_value);

  if (lhs.AlmostEquals(rhs)) {
    return AssertionSuccess();
  }

  ::std::stringstream lhs_ss;
  lhs_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
         << lhs_value;

  ::std::stringstream rhs_ss;
  rhs_ss << std::setprecision(std::numeric_limits<RawType>::digits10 + 2)
         << rhs_value;

  return EqFailure(lhs_expression,
                   rhs_expression,
                   StringStreamToString(&lhs_ss),
                   StringStreamToString(&rhs_ss),
                   false);
}

//
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
GTEST_API_ AssertionResult DoubleNearPredFormat(const char* expr1,
                                                const char* expr2,
                                                const char* abs_error_expr,
                                                double val1,
                                                double val2,
                                                double abs_error);

// A class that enables one to stream messages to assertion macros
class GTEST_API_ AssertHelper {
 public:

  AssertHelper(TestPartResult::Type type,
               const char* file,
               int line,
               const char* message);
  ~AssertHelper();


  void operator=(const Message& message) const;

 private:




  struct AssertHelperData {
    AssertHelperData(TestPartResult::Type t,
                     const char* srcfile,
                     int line_num,
                     const char* msg)
        : type(t), file(srcfile), line(line_num), message(msg) { }

    TestPartResult::Type const type;
    const char* const file;
    int const line;
    std::string const message;

   private:
    GTEST_DISALLOW_COPY_AND_ASSIGN_(AssertHelperData);
  };

  AssertHelperData* const data_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(AssertHelper);
};

enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

GTEST_API_ GTEST_ATTRIBUTE_PRINTF_(2, 3) void ColoredPrintf(GTestColor color,
                                                            const char* fmt,
                                                            ...);

}  // namespace internal

// A value-parameterized class must inherit from both ::testing::Test and
// ::testing::WithParamInterface. In most cases that just means inheriting
// from ::testing::TestWithParam, but more complicated test hierarchies
// may need to inherit from Test and WithParamInterface at different levels.
//
// This interface has support for accessing the test parameter value via
// the GetParam() method.
//
// Use it with one of the parameter generator defining functions, like Range(),
// Values(), ValuesIn(), Bool(), and Combine().
//
// class FooTest : public ::testing::TestWithParam<int> {
//  protected:
//   FooTest() {
//     // Can use GetParam() here.
//   }
//   ~FooTest() override {
//     // Can use GetParam() here.
//   }
//   void SetUp() override {
//     // Can use GetParam() here.
//   }
//   void TearDown override {
//     // Can use GetParam() here.
//   }
// };
// TEST_P(FooTest, DoesBar) {
//   // Can use GetParam() method here.
//   Foo foo;
//   ASSERT_TRUE(foo.DoesBar(GetParam()));
// }
// INSTANTIATE_TEST_SUITE_P(OneToTenRange, FooTest, ::testing::Range(1, 10));

template <typename T>
class WithParamInterface {
 public:
  typedef T ParamType;
  virtual ~WithParamInterface() {}


  static const ParamType& GetParam() {
    GTEST_CHECK_(parameter_ != nullptr)
        << "GetParam() can only be called inside a value-parameterized test "
        << "-- did you intend to write TEST_P instead of TEST_F?";
    return *parameter_;
  }

 private:


  static void SetParam(const ParamType* parameter) {
    parameter_ = parameter;
  }

  static const ParamType* parameter_;

  template <class TestClass> friend class internal::ParameterizedTestFactory;
};

template <typename T>
const T* WithParamInterface<T>::parameter_ = nullptr;

// WithParamInterface, and can just inherit from ::testing::TestWithParam.

template <typename T>
class TestWithParam : public Test, public WithParamInterface<T> {
};


// Skipping test aborts current function.
// Skipped tests are neither successful nor failed.
#define GTEST_SKIP() GTEST_SKIP_("Skipped")

// SUCCEED generates a success - it doesn't automatically make the
// current test successful, as a test is only successful when it has
// no failure.
//
// EXPECT_* verifies that a certain condition is satisfied.  If not,
// it behaves like ADD_FAILURE.  In particular:
//
//   EXPECT_TRUE  verifies that a Boolean condition is true.
//   EXPECT_FALSE verifies that a Boolean condition is false.
//
// FAIL and ASSERT_* are similar to ADD_FAILURE and EXPECT_*, except
// that they will also abort the current function on failure.  People
// usually want the fail-fast behavior of FAIL and ASSERT_*, but those
// writing data-driven tests often find themselves using ADD_FAILURE
// and EXPECT_* more.

#define ADD_FAILURE() GTEST_NONFATAL_FAILURE_("Failed")

// a generic message.
#define ADD_FAILURE_AT(file, line) \
  GTEST_MESSAGE_AT_(file, line, "Failed", \
                    ::testing::TestPartResult::kNonFatalFailure)

#define GTEST_FAIL() GTEST_FATAL_FAILURE_("Failed")

// generic name and clashes with some other libraries.
#if !GTEST_DONT_DEFINE_FAIL
# define FAIL() GTEST_FAIL()
#endif

#define GTEST_SUCCEED() GTEST_SUCCESS_("Succeeded")

// is a generic name and clashes with some other libraries.
#if !GTEST_DONT_DEFINE_SUCCEED
# define SUCCEED() GTEST_SUCCEED()
#endif

//
//    * {ASSERT|EXPECT}_THROW(statement, expected_exception):
//         Tests that the statement throws the expected exception.
//    * {ASSERT|EXPECT}_NO_THROW(statement):
//         Tests that the statement doesn't throw any exception.
//    * {ASSERT|EXPECT}_ANY_THROW(statement):
//         Tests that the statement throws an exception.

#define EXPECT_THROW(statement, expected_exception) \
  GTEST_TEST_THROW_(statement, expected_exception, GTEST_NONFATAL_FAILURE_)
#define EXPECT_NO_THROW(statement) \
  GTEST_TEST_NO_THROW_(statement, GTEST_NONFATAL_FAILURE_)
#define EXPECT_ANY_THROW(statement) \
  GTEST_TEST_ANY_THROW_(statement, GTEST_NONFATAL_FAILURE_)
#define ASSERT_THROW(statement, expected_exception) \
  GTEST_TEST_THROW_(statement, expected_exception, GTEST_FATAL_FAILURE_)
#define ASSERT_NO_THROW(statement) \
  GTEST_TEST_NO_THROW_(statement, GTEST_FATAL_FAILURE_)
#define ASSERT_ANY_THROW(statement) \
  GTEST_TEST_ANY_THROW_(statement, GTEST_FATAL_FAILURE_)

// AssertionResult. For more information on how to use AssertionResult with
// these macros see comments on that class.
#define EXPECT_TRUE(condition) \
  GTEST_TEST_BOOLEAN_(condition, #condition, false, true, \
                      GTEST_NONFATAL_FAILURE_)
#define EXPECT_FALSE(condition) \
  GTEST_TEST_BOOLEAN_(!(condition), #condition, true, false, \
                      GTEST_NONFATAL_FAILURE_)
#define ASSERT_TRUE(condition) \
  GTEST_TEST_BOOLEAN_(condition, #condition, false, true, \
                      GTEST_FATAL_FAILURE_)
#define ASSERT_FALSE(condition) \
  GTEST_TEST_BOOLEAN_(!(condition), #condition, true, false, \
                      GTEST_FATAL_FAILURE_)

//
//    * {ASSERT|EXPECT}_EQ(v1, v2): Tests that v1 == v2
//    * {ASSERT|EXPECT}_NE(v1, v2): Tests that v1 != v2
//    * {ASSERT|EXPECT}_LT(v1, v2): Tests that v1 < v2
//    * {ASSERT|EXPECT}_LE(v1, v2): Tests that v1 <= v2
//    * {ASSERT|EXPECT}_GT(v1, v2): Tests that v1 > v2
//    * {ASSERT|EXPECT}_GE(v1, v2): Tests that v1 >= v2
//
// When they are not, Google Test prints both the tested expressions and
// their actual values.  The values must be compatible built-in types,
// or you will get a compiler error.  By "compatible" we mean that the
// values can be compared by the respective operator.
//
// Note:
//
//   1. It is possible to make a user-defined type work with
//   {ASSERT|EXPECT}_??(), but that requires overloading the
//   comparison operators and is thus discouraged by the Google C++
//   Usage Guide.  Therefore, you are advised to use the
//   {ASSERT|EXPECT}_TRUE() macro to assert that two objects are
//   equal.
//
//   2. The {ASSERT|EXPECT}_??() macros do pointer comparisons on
//   pointers (in particular, C strings).  Therefore, if you use it
//   with two C strings, you are testing how their locations in memory
//   are related, not how their content is related.  To compare two C
//   strings by content, use {ASSERT|EXPECT}_STR*().
//
//   3. {ASSERT|EXPECT}_EQ(v1, v2) is preferred to
//   {ASSERT|EXPECT}_TRUE(v1 == v2), as the former tells you
//   what the actual value is when it fails, and similarly for the
//   other comparisons.
//
//   4. Do not depend on the order in which {ASSERT|EXPECT}_??()
//   evaluate their arguments, which is undefined.
//
//   5. These macros evaluate their arguments exactly once.
//
// Examples:
//
//   EXPECT_NE(Foo(), 5);
//   EXPECT_EQ(a_pointer, NULL);
//   ASSERT_LT(i, array_size);
//   ASSERT_GT(records.size(), 0) << "There is no record left.";

#define EXPECT_EQ(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::EqHelper::Compare, val1, val2)
#define EXPECT_NE(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperNE, val1, val2)
#define EXPECT_LE(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperLE, val1, val2)
#define EXPECT_LT(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperLT, val1, val2)
#define EXPECT_GE(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperGE, val1, val2)
#define EXPECT_GT(val1, val2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperGT, val1, val2)

#define GTEST_ASSERT_EQ(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal::EqHelper::Compare, val1, val2)
#define GTEST_ASSERT_NE(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperNE, val1, val2)
#define GTEST_ASSERT_LE(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperLE, val1, val2)
#define GTEST_ASSERT_LT(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperLT, val1, val2)
#define GTEST_ASSERT_GE(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperGE, val1, val2)
#define GTEST_ASSERT_GT(val1, val2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperGT, val1, val2)

// ASSERT_XY(), which clashes with some users' own code.

#if !GTEST_DONT_DEFINE_ASSERT_EQ
# define ASSERT_EQ(val1, val2) GTEST_ASSERT_EQ(val1, val2)
#endif

#if !GTEST_DONT_DEFINE_ASSERT_NE
# define ASSERT_NE(val1, val2) GTEST_ASSERT_NE(val1, val2)
#endif

#if !GTEST_DONT_DEFINE_ASSERT_LE
# define ASSERT_LE(val1, val2) GTEST_ASSERT_LE(val1, val2)
#endif

#if !GTEST_DONT_DEFINE_ASSERT_LT
# define ASSERT_LT(val1, val2) GTEST_ASSERT_LT(val1, val2)
#endif

#if !GTEST_DONT_DEFINE_ASSERT_GE
# define ASSERT_GE(val1, val2) GTEST_ASSERT_GE(val1, val2)
#endif

#if !GTEST_DONT_DEFINE_ASSERT_GT
# define ASSERT_GT(val1, val2) GTEST_ASSERT_GT(val1, val2)
#endif

// as different.  Two NULLs are equal.
//
//    * {ASSERT|EXPECT}_STREQ(s1, s2):     Tests that s1 == s2
//    * {ASSERT|EXPECT}_STRNE(s1, s2):     Tests that s1 != s2
//    * {ASSERT|EXPECT}_STRCASEEQ(s1, s2): Tests that s1 == s2, ignoring case
//    * {ASSERT|EXPECT}_STRCASENE(s1, s2): Tests that s1 != s2, ignoring case
//
// For wide or narrow string objects, you can use the
// {ASSERT|EXPECT}_??() macros.
//
// Don't depend on the order in which the arguments are evaluated,
// which is undefined.
//
// These macros evaluate their arguments exactly once.

#define EXPECT_STREQ(s1, s2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperSTREQ, s1, s2)
#define EXPECT_STRNE(s1, s2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperSTRNE, s1, s2)
#define EXPECT_STRCASEEQ(s1, s2) \
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperSTRCASEEQ, s1, s2)
#define EXPECT_STRCASENE(s1, s2)\
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperSTRCASENE, s1, s2)

#define ASSERT_STREQ(s1, s2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperSTREQ, s1, s2)
#define ASSERT_STRNE(s1, s2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperSTRNE, s1, s2)
#define ASSERT_STRCASEEQ(s1, s2) \
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperSTRCASEEQ, s1, s2)
#define ASSERT_STRCASENE(s1, s2)\
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperSTRCASENE, s1, s2)

//
//    * {ASSERT|EXPECT}_FLOAT_EQ(val1, val2):
//         Tests that two float values are almost equal.
//    * {ASSERT|EXPECT}_DOUBLE_EQ(val1, val2):
//         Tests that two double values are almost equal.
//    * {ASSERT|EXPECT}_NEAR(v1, v2, abs_error):
//         Tests that v1 and v2 are within the given distance to each other.
//
// Google Test uses ULP-based comparison to automatically pick a default
// error bound that is appropriate for the operands.  See the
// FloatingPoint template class in gtest-internal.h if you are
// interested in the implementation details.

#define EXPECT_FLOAT_EQ(val1, val2)\
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperFloatingPointEQ<float>, \
                      val1, val2)

#define EXPECT_DOUBLE_EQ(val1, val2)\
  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperFloatingPointEQ<double>, \
                      val1, val2)

#define ASSERT_FLOAT_EQ(val1, val2)\
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperFloatingPointEQ<float>, \
                      val1, val2)

#define ASSERT_DOUBLE_EQ(val1, val2)\
  ASSERT_PRED_FORMAT2(::testing::internal::CmpHelperFloatingPointEQ<double>, \
                      val1, val2)

#define EXPECT_NEAR(val1, val2, abs_error)\
  EXPECT_PRED_FORMAT3(::testing::internal::DoubleNearPredFormat, \
                      val1, val2, abs_error)

#define ASSERT_NEAR(val1, val2, abs_error)\
  ASSERT_PRED_FORMAT3(::testing::internal::DoubleNearPredFormat, \
                      val1, val2, abs_error)

// can be used in {ASSERT|EXPECT}_PRED_FORMAT2*(), e.g.
//
//   EXPECT_PRED_FORMAT2(testing::DoubleLE, Foo(), 5.0);

// otherwise.  In particular, it fails if either val1 or val2 is NaN.
GTEST_API_ AssertionResult FloatLE(const char* expr1, const char* expr2,
                                   float val1, float val2);
GTEST_API_ AssertionResult DoubleLE(const char* expr1, const char* expr2,
                                    double val1, double val2);


#if GTEST_OS_WINDOWS

// on Windows, and rely on Windows SDK macros and APIs to compile.
//
//    * {ASSERT|EXPECT}_HRESULT_{SUCCEEDED|FAILED}(expr)
//
// When expr unexpectedly fails or succeeds, Google Test prints the
// expected result and the actual result with both a human-readable
// string representation of the error, if available, as well as the
// hex result code.
# define EXPECT_HRESULT_SUCCEEDED(expr) \
    EXPECT_PRED_FORMAT1(::testing::internal::IsHRESULTSuccess, (expr))

# define ASSERT_HRESULT_SUCCEEDED(expr) \
    ASSERT_PRED_FORMAT1(::testing::internal::IsHRESULTSuccess, (expr))

# define EXPECT_HRESULT_FAILED(expr) \
    EXPECT_PRED_FORMAT1(::testing::internal::IsHRESULTFailure, (expr))

# define ASSERT_HRESULT_FAILED(expr) \
    ASSERT_PRED_FORMAT1(::testing::internal::IsHRESULTFailure, (expr))

#endif  // GTEST_OS_WINDOWS

// failures in the current thread.
//
//   * {ASSERT|EXPECT}_NO_FATAL_FAILURE(statement);
//
// Examples:
//
//   EXPECT_NO_FATAL_FAILURE(Process());
//   ASSERT_NO_FATAL_FAILURE(Process()) << "Process() failed";
//
#define ASSERT_NO_FATAL_FAILURE(statement) \
    GTEST_TEST_NO_FATAL_FAILURE_(statement, GTEST_FATAL_FAILURE_)
#define EXPECT_NO_FATAL_FAILURE(statement) \
    GTEST_TEST_NO_FATAL_FAILURE_(statement, GTEST_NONFATAL_FAILURE_)

// and the given message) to be included in every test failure message generated
// by code in the scope of the lifetime of an instance of this class. The effect
// is undone with the destruction of the instance.
//
// The message argument can be anything streamable to std::ostream.
//
// Example:
//   testing::ScopedTrace trace("file.cc", 123, "message");
//
class GTEST_API_ ScopedTrace {
 public:




  template <typename T>
  ScopedTrace(const char* file, int line, const T& message) {
    PushTrace(file, line, (Message() << message).GetString());
  }

  ScopedTrace(const char* file, int line, const char* message) {
    PushTrace(file, line, message ? message : "(null)");
  }

  ScopedTrace(const char* file, int line, const std::string& message) {
    PushTrace(file, line, message);
  }




  ~ScopedTrace();

 private:
  void PushTrace(const char* file, int line, std::string message);

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ScopedTrace);
} GTEST_ATTRIBUTE_UNUSED_;  // A ScopedTrace object does its job in its



// number, and the given message) to be included in every test failure
// message generated by code in the current scope.  The effect is
// undone when the control leaves the current scope.
//
// The message argument can be anything streamable to std::ostream.
//
// In the implementation, we include the current line number as part
// of the dummy variable name, thus allowing multiple SCOPED_TRACE()s
// to appear in the same block - as long as they are on different
// lines.
//
// Assuming that each thread maintains its own stack of traces.
// Therefore, a SCOPED_TRACE() would (correctly) only affect the
// assertions in its own thread.
#define SCOPED_TRACE(message) \
  ::testing::ScopedTrace GTEST_CONCAT_TOKEN_(gtest_trace_, __LINE__)(\
    __FILE__, __LINE__, (message))

// StaticAssertTypeEq<type1, type2>() compiles iff type1 and type2 are
// the same type.  The value it returns is not interesting.
//
// Instead of making StaticAssertTypeEq a class template, we make it a
// function template that invokes a helper class template.  This
// prevents a user from misusing StaticAssertTypeEq<T1, T2> by
// defining objects of that type.
//
// CAVEAT:
//
// When used inside a method of a class template,
// StaticAssertTypeEq<T1, T2>() is effective ONLY IF the method is
// instantiated.  For example, given:
//
//   template <typename T> class Foo {
//    public:
//     void Bar() { testing::StaticAssertTypeEq<int, T>(); }
//   };
//
// the code:
//
//   void Test1() { Foo<bool> foo; }
//
// will NOT generate a compiler error, as Foo<bool>::Bar() is never
// actually instantiated.  Instead, you need:
//
//   void Test2() { Foo<bool> foo; foo.Bar(); }
//
// to cause a compiler error.
template <typename T1, typename T2>
bool StaticAssertTypeEq() {
  (void)internal::StaticAssertTypeEqHelper<T1, T2>();
  return true;
}

//
// The first parameter is the name of the test suite, and the second
// parameter is the name of the test within the test suite.
//
// The convention is to end the test suite name with "Test".  For
// example, a test suite for the Foo class can be named FooTest.
//
// Test code should appear between braces after an invocation of
// this macro.  Example:
//
//   TEST(FooTest, InitializesCorrectly) {
//     Foo foo;
//     EXPECT_TRUE(foo.StatusIsOK());
//   }

// ::testing::Test>() here to get the type ID of testing::Test.  This
// is to work around a suspected linker bug when using Google Test as
// a framework on Mac OS X.  The bug causes GetTypeId<
// ::testing::Test>() to return different values depending on whether
// the call is from the Google Test framework itself or from user test
// code.  GetTestTypeId() is guaranteed to always return the same
// value, as it always calls GetTypeId<>() from the Google Test
// framework.
#define GTEST_TEST(test_suite_name, test_name)             \
  GTEST_TEST_(test_suite_name, test_name, ::testing::Test, \
              ::testing::internal::GetTestTypeId())

// is a generic name and clashes with some other libraries.
#if !GTEST_DONT_DEFINE_TEST
#define TEST(test_suite_name, test_name) GTEST_TEST(test_suite_name, test_name)
#endif

//
// The first parameter is the name of the test fixture class, which
// also doubles as the test suite name.  The second parameter is the
// name of the test within the test suite.
//
// A test fixture class must be declared earlier.  The user should put
// the test code between braces after using this macro.  Example:
//
//   class FooTest : public testing::Test {
//    protected:
//     void SetUp() override { b_.AddElement(3); }
//
//     Foo a_;
//     Foo b_;
//   };
//
//   TEST_F(FooTest, InitializesCorrectly) {
//     EXPECT_TRUE(a_.StatusIsOK());
//   }
//
//   TEST_F(FooTest, ReturnsElementCountCorrectly) {
//     EXPECT_EQ(a_.size(), 0);
//     EXPECT_EQ(b_.size(), 1);
//   }
//
// GOOGLETEST_CM0011 DO NOT DELETE
#define TEST_F(test_fixture, test_name)\
  GTEST_TEST_(test_fixture, test_name, test_fixture, \
              ::testing::internal::GetTypeId<test_fixture>())

// Tries to determine an appropriate directory for the platform.
GTEST_API_ std::string TempDir();

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

//
// This is an advanced API only to be used when the `TEST` macros are
// insufficient. The macros should be preferred when possible, as they avoid
// most of the complexity of calling this function.
//
// The `factory` argument is a factory callable (move-constructible) object or
// function pointer that creates a new instance of the Test object. It
// handles ownership to the caller. The signature of the callable is
// `Fixture*()`, where `Fixture` is the test fixture class for the test. All
// tests registered with the same `test_suite_name` must return the same
// fixture type. This is checked at runtime.
//
// The framework will infer the fixture class from the factory and will call
// the `SetUpTestSuite` and `TearDownTestSuite` for it.
//
// Must be called before `RUN_ALL_TESTS()` is invoked, otherwise behavior is
// undefined.
//
// Use case example:
//
// class MyFixture : public ::testing::Test {
//  public:
//   // All of these optional, just like in regular macro usage.
//   static void SetUpTestSuite() { ... }
//   static void TearDownTestSuite() { ... }
//   void SetUp() override { ... }
//   void TearDown() override { ... }
// };
//
// class MyTest : public MyFixture {
//  public:
//   explicit MyTest(int data) : data_(data) {}
//   void TestBody() override { ... }
//
//  private:
//   int data_;
// };
//
// void RegisterMyTests(const std::vector<int>& values) {
//   for (int v : values) {
//     ::testing::RegisterTest(
//         "MyFixture", ("Test" + std::to_string(v)).c_str(), nullptr,
//         std::to_string(v).c_str(),
//         __FILE__, __LINE__,
//         // Important to use the fixture type as the return type here.
//         [=]() -> MyFixture* { return new MyTest(v); });
//   }
// }
// ...
// int main(int argc, char** argv) {
//   std::vector<int> values_to_test = LoadValuesFromConfig();
//   RegisterMyTests(values_to_test);
//   ...
//   return RUN_ALL_TESTS();
// }
//
template <int&... ExplicitParameterBarrier, typename Factory>
TestInfo* RegisterTest(const char* test_suite_name, const char* test_name,
                       const char* type_param, const char* value_param,
                       const char* file, int line, Factory factory) {
  using TestT = typename std::remove_pointer<decltype(factory())>::type;

  class FactoryImpl : public internal::TestFactoryBase {
   public:
    explicit FactoryImpl(Factory f) : factory_(std::move(f)) {}
    Test* CreateTest() override { return factory_(); }

   private:
    Factory factory_;
  };

  return internal::MakeAndRegisterTestInfo(
      test_suite_name, test_name, type_param, value_param,
      internal::CodeLocation(file, line), internal::GetTypeId<TestT>(),
      internal::SuiteApiResolver<TestT>::GetSetUpCaseOrSuite(),
      internal::SuiteApiResolver<TestT>::GetTearDownCaseOrSuite(),
      new FactoryImpl{std::move(factory)});
}

}  // namespace testing

// tests are successful, or 1 otherwise.
//
// RUN_ALL_TESTS() should be invoked after the command line has been
// parsed by InitGoogleTest().
//
// This function was formerly a macro; thus, it is in the global
// namespace and has an all-caps name.
int RUN_ALL_TESTS() GTEST_MUST_USE_RESULT_;

inline int RUN_ALL_TESTS() {
  return ::testing::UnitTest::GetInstance()->Run();
}

GTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251

#endif  // GTEST_INCLUDE_GTEST_GTEST_H_
