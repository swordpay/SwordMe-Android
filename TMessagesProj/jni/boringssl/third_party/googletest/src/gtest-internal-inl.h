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

// This file contains purely Google Test's internal implementation.  Please
// DO NOT #INCLUDE IT IN A USER PROGRAM.

#ifndef GTEST_SRC_GTEST_INTERNAL_INL_H_
#define GTEST_SRC_GTEST_INTERNAL_INL_H_

#ifndef _WIN32_WCE
# include <errno.h>
#endif  // !_WIN32_WCE
#include <stddef.h>
#include <stdlib.h>  // For strtoll/_strtoul64/malloc/free.
#include <string.h>  // For memmove.

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "gtest/internal/gtest-port.h"

#if GTEST_CAN_STREAM_RESULTS_
# include <arpa/inet.h>  // NOLINT
# include <netdb.h>  // NOLINT
#endif

#if GTEST_OS_WINDOWS
# include <windows.h>  // NOLINT
#endif  // GTEST_OS_WINDOWS

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
/* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {

//
// We don't want the users to modify this flag in the code, but want
// Google Test's own unit tests to be able to access it. Therefore we
// declare it here as opposed to in gtest.h.
GTEST_DECLARE_bool_(death_test_use_fork);

namespace internal {

// library.  This is solely for testing GetTestTypeId().
GTEST_API_ extern const TypeId kTestTypeIdInGoogleTest;

const char kAlsoRunDisabledTestsFlag[] = "also_run_disabled_tests";
const char kBreakOnFailureFlag[] = "break_on_failure";
const char kCatchExceptionsFlag[] = "catch_exceptions";
const char kColorFlag[] = "color";
const char kFilterFlag[] = "filter";
const char kListTestsFlag[] = "list_tests";
const char kOutputFlag[] = "output";
const char kPrintTimeFlag[] = "print_time";
const char kPrintUTF8Flag[] = "print_utf8";
const char kRandomSeedFlag[] = "random_seed";
const char kRepeatFlag[] = "repeat";
const char kShuffleFlag[] = "shuffle";
const char kStackTraceDepthFlag[] = "stack_trace_depth";
const char kStreamResultToFlag[] = "stream_result_to";
const char kThrowOnFailureFlag[] = "throw_on_failure";
const char kFlagfileFlag[] = "flagfile";

const int kMaxRandomSeed = 99999;

// specified on the command line.
GTEST_API_ extern bool g_help_flag;

GTEST_API_ TimeInMillis GetTimeInMillis();

GTEST_API_ bool ShouldUseColor(bool stdout_is_tty);

GTEST_API_ std::string FormatTimeInMillisAsSeconds(TimeInMillis ms);

// format, without the timezone information.  N.B.: due to the use the
// non-reentrant localtime() function, this function is not thread safe.  Do
// not use it in any code that can be called from multiple threads.
GTEST_API_ std::string FormatEpochTimeInMillisAsIso8601(TimeInMillis ms);

//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
GTEST_API_ bool ParseInt32Flag(
    const char* str, const char* flag, Int32* value);

// given --gtest_random_seed flag value.
inline int GetRandomSeedFromFlag(Int32 random_seed_flag) {
  const unsigned int raw_seed = (random_seed_flag == 0) ?
      static_cast<unsigned int>(GetTimeInMillis()) :
      static_cast<unsigned int>(random_seed_flag);


  const int normalized_seed =
      static_cast<int>((raw_seed - 1U) %
                       static_cast<unsigned int>(kMaxRandomSeed)) + 1;
  return normalized_seed;
}

// undefined if 'seed' is invalid.  The seed after kMaxRandomSeed is
// considered to be 1.
inline int GetNextRandomSeed(int seed) {
  GTEST_CHECK_(1 <= seed && seed <= kMaxRandomSeed)
      << "Invalid random seed " << seed << " - must be in [1, "
      << kMaxRandomSeed << "].";
  const int next_seed = seed + 1;
  return (next_seed > kMaxRandomSeed) ? 1 : next_seed;
}

// restores them in its d'tor.
class GTestFlagSaver {
 public:

  GTestFlagSaver() {
    also_run_disabled_tests_ = GTEST_FLAG(also_run_disabled_tests);
    break_on_failure_ = GTEST_FLAG(break_on_failure);
    catch_exceptions_ = GTEST_FLAG(catch_exceptions);
    color_ = GTEST_FLAG(color);
    death_test_style_ = GTEST_FLAG(death_test_style);
    death_test_use_fork_ = GTEST_FLAG(death_test_use_fork);
    filter_ = GTEST_FLAG(filter);
    internal_run_death_test_ = GTEST_FLAG(internal_run_death_test);
    list_tests_ = GTEST_FLAG(list_tests);
    output_ = GTEST_FLAG(output);
    print_time_ = GTEST_FLAG(print_time);
    print_utf8_ = GTEST_FLAG(print_utf8);
    random_seed_ = GTEST_FLAG(random_seed);
    repeat_ = GTEST_FLAG(repeat);
    shuffle_ = GTEST_FLAG(shuffle);
    stack_trace_depth_ = GTEST_FLAG(stack_trace_depth);
    stream_result_to_ = GTEST_FLAG(stream_result_to);
    throw_on_failure_ = GTEST_FLAG(throw_on_failure);
  }

  ~GTestFlagSaver() {
    GTEST_FLAG(also_run_disabled_tests) = also_run_disabled_tests_;
    GTEST_FLAG(break_on_failure) = break_on_failure_;
    GTEST_FLAG(catch_exceptions) = catch_exceptions_;
    GTEST_FLAG(color) = color_;
    GTEST_FLAG(death_test_style) = death_test_style_;
    GTEST_FLAG(death_test_use_fork) = death_test_use_fork_;
    GTEST_FLAG(filter) = filter_;
    GTEST_FLAG(internal_run_death_test) = internal_run_death_test_;
    GTEST_FLAG(list_tests) = list_tests_;
    GTEST_FLAG(output) = output_;
    GTEST_FLAG(print_time) = print_time_;
    GTEST_FLAG(print_utf8) = print_utf8_;
    GTEST_FLAG(random_seed) = random_seed_;
    GTEST_FLAG(repeat) = repeat_;
    GTEST_FLAG(shuffle) = shuffle_;
    GTEST_FLAG(stack_trace_depth) = stack_trace_depth_;
    GTEST_FLAG(stream_result_to) = stream_result_to_;
    GTEST_FLAG(throw_on_failure) = throw_on_failure_;
  }

 private:

  bool also_run_disabled_tests_;
  bool break_on_failure_;
  bool catch_exceptions_;
  std::string color_;
  std::string death_test_style_;
  bool death_test_use_fork_;
  std::string filter_;
  std::string internal_run_death_test_;
  bool list_tests_;
  std::string output_;
  bool print_time_;
  bool print_utf8_;
  internal::Int32 random_seed_;
  internal::Int32 repeat_;
  bool shuffle_;
  internal::Int32 stack_trace_depth_;
  std::string stream_result_to_;
  bool throw_on_failure_;
} GTEST_ATTRIBUTE_UNUSED_;

// code_point parameter is of type UInt32 because wchar_t may not be
// wide enough to contain a code point.
// If the code_point is not a valid Unicode code point
// (i.e. outside of Unicode range U+0 to U+10FFFF) it will be converted
// to "(Invalid Unicode 0xXXXXXXXX)".
GTEST_API_ std::string CodePointToUtf8(UInt32 code_point);

// The wide string is assumed to have the following encoding:
//   UTF-16 if sizeof(wchar_t) == 2 (on Windows, Cygwin)
//   UTF-32 if sizeof(wchar_t) == 4 (on Linux)
// Parameter str points to a null-terminated wide string.
// Parameter num_chars may additionally limit the number
// of wchar_t characters processed. -1 is used when the entire string
// should be processed.
// If the string contains code points that are not valid Unicode code points
// (i.e. outside of Unicode range U+0 to U+10FFFF) they will be output
// as '(Invalid Unicode 0xXXXXXXXX)'. If the string is in UTF16 encoding
// and contains invalid UTF-16 surrogate pairs, values in those pairs
// will be encoded as individual Unicode characters from Basic Normal Plane.
GTEST_API_ std::string WideStringToUtf8(const wchar_t* str, int num_chars);

// if the variable is present. If a file already exists at this location, this
// function will write over it. If the variable is present, but the file cannot
// be created, prints an error and exits.
void WriteToShardStatusFileIfNeeded();

// environment variable values. If the variables are present,
// but inconsistent (e.g., shard_index >= total_shards), prints
// an error and exits. If in_subprocess_for_death_test, sharding is
// disabled because it must only be applied to the original test
// process. Otherwise, we could filter out death tests we intended to execute.
GTEST_API_ bool ShouldShard(const char* total_shards_str,
                            const char* shard_index_str,
                            bool in_subprocess_for_death_test);

// returns default_val. If it is not an Int32, prints an error and
// and aborts.
GTEST_API_ Int32 Int32FromEnvOrDie(const char* env_var, Int32 default_val);

// returns true iff the test should be run on this shard. The test id is
// some arbitrary but unique non-negative integer assigned to each test
// method. Assumes that 0 <= shard_index < total_shards.
GTEST_API_ bool ShouldRunTestOnShard(
    int total_shards, int shard_index, int test_id);


// the given predicate.
template <class Container, typename Predicate>
inline int CountIf(const Container& c, Predicate predicate) {


  int count = 0;
  for (typename Container::const_iterator it = c.begin(); it != c.end(); ++it) {
    if (predicate(*it))
      ++count;
  }
  return count;
}

template <class Container, typename Functor>
void ForEach(const Container& c, Functor functor) {
  std::for_each(c.begin(), c.end(), functor);
}

// in range [0, v.size()).
template <typename E>
inline E GetElementOr(const std::vector<E>& v, int i, E default_value) {
  return (i < 0 || i >= static_cast<int>(v.size())) ? default_value : v[i];
}

// 'begin' and 'end' are element indices as an STL-style range;
// i.e. [begin, end) are shuffled, where 'end' == size() means to
// shuffle to the end of the vector.
template <typename E>
void ShuffleRange(internal::Random* random, int begin, int end,
                  std::vector<E>* v) {
  const int size = static_cast<int>(v->size());
  GTEST_CHECK_(0 <= begin && begin <= size)
      << "Invalid shuffle range start " << begin << ": must be in range [0, "
      << size << "].";
  GTEST_CHECK_(begin <= end && end <= size)
      << "Invalid shuffle range finish " << end << ": must be in range ["
      << begin << ", " << size << "].";


  for (int range_width = end - begin; range_width >= 2; range_width--) {
    const int last_in_range = begin + range_width - 1;
    const int selected = begin + random->Generate(range_width);
    std::swap((*v)[selected], (*v)[last_in_range]);
  }
}

template <typename E>
inline void Shuffle(internal::Random* random, std::vector<E>* v) {
  ShuffleRange(random, 0, static_cast<int>(v->size()), v);
}

// functor.
template <typename T>
static void Delete(T* x) {
  delete x;
}

//
// TestPropertyKeyIs is copyable.
class TestPropertyKeyIs {
 public:



  explicit TestPropertyKeyIs(const std::string& key) : key_(key) {}

  bool operator()(const TestProperty& test_property) const {
    return test_property.key() == key_;
  }

 private:
  std::string key_;
};

//
// This class contains functions for processing options the user
// specifies when running the tests.  It has only static members.
//
// In most cases, the user can specify an option using either an
// environment variable or a command line flag.  E.g. you can set the
// test filter using either GTEST_FILTER or --gtest_filter.  If both
// the variable and the flag are present, the latter overrides the
// former.
class GTEST_API_ UnitTestOptions {
 public:


  static std::string GetOutputFormat();



  static std::string GetAbsolutePathToOutputFile();






  static bool PatternMatchesString(const char *pattern, const char *str);


  static bool FilterMatchesTest(const std::string& test_suite_name,
                                const std::string& test_name);

#if GTEST_OS_WINDOWS




  static int GTestShouldProcessSEH(DWORD exception_code);
#endif  // GTEST_OS_WINDOWS


  static bool MatchesFilter(const std::string& name, const char* filter);
};

// is present.  Used by UnitTestOptions::GetOutputFile.
GTEST_API_ FilePath GetCurrentExecutableName();

class OsStackTraceGetterInterface {
 public:
  OsStackTraceGetterInterface() {}
  virtual ~OsStackTraceGetterInterface() {}






  virtual std::string CurrentStackTrace(int max_depth, int skip_count) = 0;



  virtual void UponLeavingGTest() = 0;


  static const char* const kElidedFramesMarker;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(OsStackTraceGetterInterface);
};

class OsStackTraceGetter : public OsStackTraceGetterInterface {
 public:
  OsStackTraceGetter() {}

  std::string CurrentStackTrace(int max_depth, int skip_count) override;
  void UponLeavingGTest() override;

 private:
#if GTEST_HAS_ABSL
  Mutex mutex_;  // Protects all internal state.




  void* caller_frame_ = nullptr;
#endif  // GTEST_HAS_ABSL

  GTEST_DISALLOW_COPY_AND_ASSIGN_(OsStackTraceGetter);
};

struct TraceInfo {
  const char* file;
  int line;
  std::string message;
};

// This class should only be used by UnitTestImpl.
class DefaultGlobalTestPartResultReporter
  : public TestPartResultReporterInterface {
 public:
  explicit DefaultGlobalTestPartResultReporter(UnitTestImpl* unit_test);


  void ReportTestPartResult(const TestPartResult& result) override;

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultGlobalTestPartResultReporter);
};

// UnitTestImpl. This class should only be used by UnitTestImpl.
class DefaultPerThreadTestPartResultReporter
    : public TestPartResultReporterInterface {
 public:
  explicit DefaultPerThreadTestPartResultReporter(UnitTestImpl* unit_test);


  void ReportTestPartResult(const TestPartResult& result) override;

 private:
  UnitTestImpl* const unit_test_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(DefaultPerThreadTestPartResultReporter);
};

// the methods under a mutex, as this class is not accessible by a
// user and the UnitTest class that delegates work to this class does
// proper locking.
class GTEST_API_ UnitTestImpl {
 public:
  explicit UnitTestImpl(UnitTest* parent);
  virtual ~UnitTestImpl();







  TestPartResultReporterInterface* GetGlobalTestPartResultReporter();

  void SetGlobalTestPartResultReporter(
      TestPartResultReporterInterface* reporter);

  TestPartResultReporterInterface* GetTestPartResultReporterForCurrentThread();

  void SetTestPartResultReporterForCurrentThread(
      TestPartResultReporterInterface* reporter);

  int successful_test_suite_count() const;

  int failed_test_suite_count() const;

  int total_test_suite_count() const;


  int test_suite_to_run_count() const;

  int successful_test_count() const;

  int skipped_test_count() const;

  int failed_test_count() const;

  int reportable_disabled_test_count() const;

  int disabled_test_count() const;

  int reportable_test_count() const;

  int total_test_count() const;

  int test_to_run_count() const;


  TimeInMillis start_timestamp() const { return start_timestamp_; }

  TimeInMillis elapsed_time() const { return elapsed_time_; }

  bool Passed() const { return !Failed(); }


  bool Failed() const {
    return failed_test_suite_count() > 0 || ad_hoc_test_result()->Failed();
  }


  const TestSuite* GetTestSuite(int i) const {
    const int index = GetElementOr(test_suite_indices_, i, -1);
    return index < 0 ? nullptr : test_suites_[i];
  }

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  const TestCase* GetTestCase(int i) const { return GetTestSuite(i); }
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_


  TestSuite* GetMutableSuiteCase(int i) {
    const int index = GetElementOr(test_suite_indices_, i, -1);
    return index < 0 ? nullptr : test_suites_[index];
  }

  TestEventListeners* listeners() { return &listeners_; }


  TestResult* current_test_result();

  const TestResult* ad_hoc_test_result() const { return &ad_hoc_test_result_; }





  void set_os_stack_trace_getter(OsStackTraceGetterInterface* getter);



  OsStackTraceGetterInterface* os_stack_trace_getter();










  std::string CurrentOsStackTraceExceptTop(int skip_count) GTEST_NO_INLINE_;










  TestSuite* GetTestSuite(const char* test_suite_name, const char* type_param,
                          internal::SetUpTestSuiteFunc set_up_tc,
                          internal::TearDownTestSuiteFunc tear_down_tc);

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
  TestCase* GetTestCase(const char* test_case_name, const char* type_param,
                        internal::SetUpTestSuiteFunc set_up_tc,
                        internal::TearDownTestSuiteFunc tear_down_tc) {
    return GetTestSuite(test_case_name, type_param, set_up_tc, tear_down_tc);
  }
#endif  //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_







  void AddTestInfo(internal::SetUpTestSuiteFunc set_up_tc,
                   internal::TearDownTestSuiteFunc tear_down_tc,
                   TestInfo* test_info) {







    if (original_working_dir_.IsEmpty()) {
      original_working_dir_.Set(FilePath::GetCurrentDir());
      GTEST_CHECK_(!original_working_dir_.IsEmpty())
          << "Failed to get the current working directory.";
    }

    GetTestSuite(test_info->test_suite_name(), test_info->type_param(),
                 set_up_tc, tear_down_tc)
        ->AddTestInfo(test_info);
  }


  internal::ParameterizedTestSuiteRegistry& parameterized_test_registry() {
    return parameterized_test_registry_;
  }

  void set_current_test_suite(TestSuite* a_current_test_suite) {
    current_test_suite_ = a_current_test_suite;
  }



  void set_current_test_info(TestInfo* a_current_test_info) {
    current_test_info_ = a_current_test_info;
  }






  void RegisterParameterizedTests();




  bool RunAllTests();

  void ClearNonAdHocTestResult() {
    ForEach(test_suites_, TestSuite::ClearTestSuiteResult);
  }

  void ClearAdHocTestResult() {
    ad_hoc_test_result_.Clear();
  }




  void RecordProperty(const TestProperty& test_property);

  enum ReactionToSharding {
    HONOR_SHARDING_PROTOCOL,
    IGNORE_SHARDING_PROTOCOL
  };






  int FilterTests(ReactionToSharding shard_tests);

  void ListTestsMatchingFilter();

  const TestSuite* current_test_suite() const { return current_test_suite_; }
  TestInfo* current_test_info() { return current_test_info_; }
  const TestInfo* current_test_info() const { return current_test_info_; }


  std::vector<Environment*>& environments() { return environments_; }

  std::vector<TraceInfo>& gtest_trace_stack() {
    return *(gtest_trace_stack_.pointer());
  }
  const std::vector<TraceInfo>& gtest_trace_stack() const {
    return gtest_trace_stack_.get();
  }

#if GTEST_HAS_DEATH_TEST
  void InitDeathTestSubprocessControlInfo() {
    internal_run_death_test_flag_.reset(ParseInternalRunDeathTestFlag());
  }




  const InternalRunDeathTestFlag* internal_run_death_test_flag() const {
    return internal_run_death_test_flag_.get();
  }

  internal::DeathTestFactory* death_test_factory() {
    return death_test_factory_.get();
  }

  void SuppressTestEventsIfInSubprocess();

  friend class ReplaceDeathTestFactory;
#endif  // GTEST_HAS_DEATH_TEST


  void ConfigureXmlOutput();

#if GTEST_CAN_STREAM_RESULTS_


  void ConfigureStreamingOutput();
#endif





  void PostFlagParsingInit();

  int random_seed() const { return random_seed_; }

  internal::Random* random() { return &random_; }


  void ShuffleTests();

  void UnshuffleTests();


  bool catch_exceptions() const { return catch_exceptions_; }

 private:
  friend class ::testing::UnitTest;


  void set_catch_exceptions(bool value) { catch_exceptions_ = value; }

  UnitTest* const parent_;


  internal::FilePath original_working_dir_;

  DefaultGlobalTestPartResultReporter default_global_test_part_result_reporter_;
  DefaultPerThreadTestPartResultReporter
      default_per_thread_test_part_result_reporter_;

  TestPartResultReporterInterface* global_test_part_result_repoter_;

  internal::Mutex global_test_part_result_reporter_mutex_;

  internal::ThreadLocal<TestPartResultReporterInterface*>
      per_thread_test_part_result_reporter_;


  std::vector<Environment*> environments_;


  std::vector<TestSuite*> test_suites_;




  std::vector<int> test_suite_indices_;


  internal::ParameterizedTestSuiteRegistry parameterized_test_registry_;

  bool parameterized_tests_registered_;

  int last_death_test_suite_;




  TestSuite* current_test_suite_;




  TestInfo* current_test_info_;








  TestResult ad_hoc_test_result_;


  TestEventListeners listeners_;




  OsStackTraceGetterInterface* os_stack_trace_getter_;

  bool post_flag_parse_init_performed_;

  int random_seed_;

  internal::Random random_;


  TimeInMillis start_timestamp_;

  TimeInMillis elapsed_time_;

#if GTEST_HAS_DEATH_TEST


  std::unique_ptr<InternalRunDeathTestFlag> internal_run_death_test_flag_;
  std::unique_ptr<internal::DeathTestFactory> death_test_factory_;
#endif  // GTEST_HAS_DEATH_TEST

  internal::ThreadLocal<std::vector<TraceInfo> > gtest_trace_stack_;


  bool catch_exceptions_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(UnitTestImpl);
};  // class UnitTestImpl

// implementation object.
inline UnitTestImpl* GetUnitTestImpl() {
  return UnitTest::GetInstance()->impl();
}

#if GTEST_USES_SIMPLE_RE

// expression matcher.
GTEST_API_ bool IsInSet(char ch, const char* str);
GTEST_API_ bool IsAsciiDigit(char ch);
GTEST_API_ bool IsAsciiPunct(char ch);
GTEST_API_ bool IsRepeat(char ch);
GTEST_API_ bool IsAsciiWhiteSpace(char ch);
GTEST_API_ bool IsAsciiWordChar(char ch);
GTEST_API_ bool IsValidEscape(char ch);
GTEST_API_ bool AtomMatchesChar(bool escaped, char pattern, char ch);
GTEST_API_ bool ValidateRegex(const char* regex);
GTEST_API_ bool MatchRegexAtHead(const char* regex, const char* str);
GTEST_API_ bool MatchRepetitionAndRegexAtHead(
    bool escaped, char ch, char repeat, const char* regex, const char* str);
GTEST_API_ bool MatchRegexAnywhere(const char* regex, const char* str);

#endif  // GTEST_USES_SIMPLE_RE

// other parts of Google Test.
GTEST_API_ void ParseGoogleTestFlagsOnly(int* argc, char** argv);
GTEST_API_ void ParseGoogleTestFlagsOnly(int* argc, wchar_t** argv);

#if GTEST_HAS_DEATH_TEST

// platform.
GTEST_API_ std::string GetLastErrnoDescription();

// number parameter.  Returns true if that is possible.
// GTEST_HAS_DEATH_TEST implies that we have ::std::string, so we can use
// it here.
template <typename Integer>
bool ParseNaturalNumber(const ::std::string& str, Integer* number) {



  if (str.empty() || !IsDigit(str[0])) {
    return false;
  }
  errno = 0;

  char* end;



# if GTEST_OS_WINDOWS && !defined(__GNUC__)

  typedef unsigned __int64 BiggestConvertible;
  const BiggestConvertible parsed = _strtoui64(str.c_str(), &end, 10);

# else

  typedef unsigned long long BiggestConvertible;  // NOLINT
  const BiggestConvertible parsed = strtoull(str.c_str(), &end, 10);

# endif  // GTEST_OS_WINDOWS && !defined(__GNUC__)

  const bool parse_success = *end == '\0' && errno == 0;

  GTEST_CHECK_(sizeof(Integer) <= sizeof(parsed));

  const Integer result = static_cast<Integer>(parsed);
  if (parse_success && static_cast<BiggestConvertible>(result) == parsed) {
    *number = result;
    return true;
  }
  return false;
}
#endif  // GTEST_HAS_DEATH_TEST

// Google Test user but are required for testing. This class allow our tests
// to access them.
//
// This class is supplied only for the purpose of testing Google Test's own
// constructs. Do not use it in user tests, either directly or indirectly.
class TestResultAccessor {
 public:
  static void RecordProperty(TestResult* test_result,
                             const std::string& xml_element,
                             const TestProperty& property) {
    test_result->RecordProperty(xml_element, property);
  }

  static void ClearTestPartResults(TestResult* test_result) {
    test_result->ClearTestPartResults();
  }

  static const std::vector<testing::TestPartResult>& test_part_results(
      const TestResult& test_result) {
    return test_result.test_part_results();
  }
};

#if GTEST_CAN_STREAM_RESULTS_

class StreamingListener : public EmptyTestEventListener {
 public:

  class AbstractSocketWriter {
   public:
    virtual ~AbstractSocketWriter() {}

    virtual void Send(const std::string& message) = 0;

    virtual void CloseConnection() {}

    void SendLn(const std::string& message) { Send(message + "\n"); }
  };

  class SocketWriter : public AbstractSocketWriter {
   public:
    SocketWriter(const std::string& host, const std::string& port)
        : sockfd_(-1), host_name_(host), port_num_(port) {
      MakeConnection();
    }

    ~SocketWriter() override {
      if (sockfd_ != -1)
        CloseConnection();
    }

    void Send(const std::string& message) override {
      GTEST_CHECK_(sockfd_ != -1)
          << "Send() can be called only when there is a connection.";

      const int len = static_cast<int>(message.length());
      if (write(sockfd_, message.c_str(), len) != len) {
        GTEST_LOG_(WARNING)
            << "stream_result_to: failed to stream to "
            << host_name_ << ":" << port_num_;
      }
    }

   private:

    void MakeConnection();

    void CloseConnection() override {
      GTEST_CHECK_(sockfd_ != -1)
          << "CloseConnection() can be called only when there is a connection.";

      close(sockfd_);
      sockfd_ = -1;
    }

    int sockfd_;  // socket file descriptor
    const std::string host_name_;
    const std::string port_num_;

    GTEST_DISALLOW_COPY_AND_ASSIGN_(SocketWriter);
  };  // class SocketWriter

  static std::string UrlEncode(const char* str);

  StreamingListener(const std::string& host, const std::string& port)
      : socket_writer_(new SocketWriter(host, port)) {
    Start();
  }

  explicit StreamingListener(AbstractSocketWriter* socket_writer)
      : socket_writer_(socket_writer) { Start(); }

  void OnTestProgramStart(const UnitTest& /* unit_test */) override {
    SendLn("event=TestProgramStart");
  }

  void OnTestProgramEnd(const UnitTest& unit_test) override {


    SendLn("event=TestProgramEnd&passed=" + FormatBool(unit_test.Passed()));

    socket_writer_->CloseConnection();
  }

  void OnTestIterationStart(const UnitTest& /* unit_test */,
                            int iteration) override {
    SendLn("event=TestIterationStart&iteration=" +
           StreamableToString(iteration));
  }

  void OnTestIterationEnd(const UnitTest& unit_test,
                          int /* iteration */) override {
    SendLn("event=TestIterationEnd&passed=" +
           FormatBool(unit_test.Passed()) + "&elapsed_time=" +
           StreamableToString(unit_test.elapsed_time()) + "ms");
  }


  void OnTestCaseStart(const TestCase& test_case) override {
    SendLn(std::string("event=TestCaseStart&name=") + test_case.name());
  }


  void OnTestCaseEnd(const TestCase& test_case) override {
    SendLn("event=TestCaseEnd&passed=" + FormatBool(test_case.Passed()) +
           "&elapsed_time=" + StreamableToString(test_case.elapsed_time()) +
           "ms");
  }

  void OnTestStart(const TestInfo& test_info) override {
    SendLn(std::string("event=TestStart&name=") + test_info.name());
  }

  void OnTestEnd(const TestInfo& test_info) override {
    SendLn("event=TestEnd&passed=" +
           FormatBool((test_info.result())->Passed()) +
           "&elapsed_time=" +
           StreamableToString((test_info.result())->elapsed_time()) + "ms");
  }

  void OnTestPartResult(const TestPartResult& test_part_result) override {
    const char* file_name = test_part_result.file_name();
    if (file_name == nullptr) file_name = "";
    SendLn("event=TestPartResult&file=" + UrlEncode(file_name) +
           "&line=" + StreamableToString(test_part_result.line_number()) +
           "&message=" + UrlEncode(test_part_result.message()));
  }

 private:

  void SendLn(const std::string& message) { socket_writer_->SendLn(message); }


  void Start() { SendLn("gtest_streaming_protocol_version=1.0"); }

  std::string FormatBool(bool value) { return value ? "1" : "0"; }

  const std::unique_ptr<AbstractSocketWriter> socket_writer_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(StreamingListener);
};  // class StreamingListener

#endif  // GTEST_CAN_STREAM_RESULTS_

}  // namespace internal
}  // namespace testing

GTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251

#endif  // GTEST_SRC_GTEST_INTERNAL_INL_H_
