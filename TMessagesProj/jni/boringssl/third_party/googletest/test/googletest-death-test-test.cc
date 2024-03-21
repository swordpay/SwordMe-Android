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

// Tests for death tests.

#include "gtest/gtest-death-test.h"

#include "gtest/gtest.h"
#include "gtest/internal/gtest-filepath.h"

using testing::internal::AlwaysFalse;
using testing::internal::AlwaysTrue;

#if GTEST_HAS_DEATH_TEST

# if GTEST_OS_WINDOWS
#  include <direct.h>          // For chdir().
# else
#  include <unistd.h>
#  include <sys/wait.h>        // For waitpid.
# endif  // GTEST_OS_WINDOWS

# include <limits.h>
# include <signal.h>
# include <stdio.h>

# if GTEST_OS_LINUX
#  include <sys/time.h>
# endif  // GTEST_OS_LINUX

# include "gtest/gtest-spi.h"
# include "src/gtest-internal-inl.h"

namespace posix = ::testing::internal::posix;

using testing::ContainsRegex;
using testing::Matcher;
using testing::Message;
using testing::internal::DeathTest;
using testing::internal::DeathTestFactory;
using testing::internal::FilePath;
using testing::internal::GetLastErrnoDescription;
using testing::internal::GetUnitTestImpl;
using testing::internal::InDeathTestChild;
using testing::internal::ParseNaturalNumber;

namespace testing {
namespace internal {

// single UnitTest object during their lifetimes.
class ReplaceDeathTestFactory {
 public:
  explicit ReplaceDeathTestFactory(DeathTestFactory* new_factory)
      : unit_test_impl_(GetUnitTestImpl()) {
    old_factory_ = unit_test_impl_->death_test_factory_.release();
    unit_test_impl_->death_test_factory_.reset(new_factory);
  }

  ~ReplaceDeathTestFactory() {
    unit_test_impl_->death_test_factory_.release();
    unit_test_impl_->death_test_factory_.reset(old_factory_);
  }
 private:

  ReplaceDeathTestFactory(const ReplaceDeathTestFactory&);
  void operator=(const ReplaceDeathTestFactory&);

  UnitTestImpl* unit_test_impl_;
  DeathTestFactory* old_factory_;
};

}  // namespace internal
}  // namespace testing

namespace {

void DieWithMessage(const ::std::string& message) {
  fprintf(stderr, "%s", message.c_str());
  fflush(stderr);  // Make sure the text is printed before the process exits.








  if (AlwaysTrue())
    _exit(1);
}

void DieInside(const ::std::string& function) {
  DieWithMessage("death inside " + function + "().");
}


class TestForDeathTest : public testing::Test {
 protected:
  TestForDeathTest() : original_dir_(FilePath::GetCurrentDir()) {}

  ~TestForDeathTest() override { posix::ChDir(original_dir_.c_str()); }

  static void StaticMemberFunction() { DieInside("StaticMemberFunction"); }

  void MemberFunction() {
    if (should_die_)
      DieInside("MemberFunction");
  }

  bool should_die_;
  const FilePath original_dir_;
};

class MayDie {
 public:
  explicit MayDie(bool should_die) : should_die_(should_die) {}

  void MemberFunction() const {
    if (should_die_)
      DieInside("MayDie::MemberFunction");
  }

 private:

  bool should_die_;
};

void GlobalFunction() { DieInside("GlobalFunction"); }

int NonVoidFunction() {
  DieInside("NonVoidFunction");
  return 1;
}

void DieIf(bool should_die) {
  if (should_die)
    DieInside("DieIf");
}

bool DieIfLessThan(int x, int y) {
  if (x < y) {
    DieInside("DieIfLessThan");
  }
  return true;
}

void DeathTestSubroutine() {
  EXPECT_DEATH(GlobalFunction(), "death.*GlobalFunction");
  ASSERT_DEATH(GlobalFunction(), "death.*GlobalFunction");
}

int DieInDebugElse12(int* sideeffect) {
  if (sideeffect) *sideeffect = 12;

# ifndef NDEBUG

  DieInside("DieInDebugElse12");

# endif  // NDEBUG

  return 12;
}

# if GTEST_OS_WINDOWS || GTEST_OS_FUCHSIA

TEST(ExitStatusPredicateTest, ExitedWithCode) {


  EXPECT_TRUE(testing::ExitedWithCode(0)(0));
  EXPECT_TRUE(testing::ExitedWithCode(1)(1));
  EXPECT_TRUE(testing::ExitedWithCode(42)(42));
  EXPECT_FALSE(testing::ExitedWithCode(0)(1));
  EXPECT_FALSE(testing::ExitedWithCode(1)(0));
}

# else

// given exit code.  This is a helper function for the
// ExitStatusPredicateTest test suite.
static int NormalExitStatus(int exit_code) {
  pid_t child_pid = fork();
  if (child_pid == 0) {
    _exit(exit_code);
  }
  int status;
  waitpid(child_pid, &status, 0);
  return status;
}

// If the signal does not cause the process to die, then it returns
// instead the exit status of a process that exits normally with exit
// code 1.  This is a helper function for the ExitStatusPredicateTest
// test suite.
static int KilledExitStatus(int signum) {
  pid_t child_pid = fork();
  if (child_pid == 0) {
    raise(signum);
    _exit(1);
  }
  int status;
  waitpid(child_pid, &status, 0);
  return status;
}

TEST(ExitStatusPredicateTest, ExitedWithCode) {
  const int status0  = NormalExitStatus(0);
  const int status1  = NormalExitStatus(1);
  const int status42 = NormalExitStatus(42);
  const testing::ExitedWithCode pred0(0);
  const testing::ExitedWithCode pred1(1);
  const testing::ExitedWithCode pred42(42);
  EXPECT_PRED1(pred0,  status0);
  EXPECT_PRED1(pred1,  status1);
  EXPECT_PRED1(pred42, status42);
  EXPECT_FALSE(pred0(status1));
  EXPECT_FALSE(pred42(status0));
  EXPECT_FALSE(pred1(status42));
}

TEST(ExitStatusPredicateTest, KilledBySignal) {
  const int status_segv = KilledExitStatus(SIGSEGV);
  const int status_kill = KilledExitStatus(SIGKILL);
  const testing::KilledBySignal pred_segv(SIGSEGV);
  const testing::KilledBySignal pred_kill(SIGKILL);
  EXPECT_PRED1(pred_segv, status_segv);
  EXPECT_PRED1(pred_kill, status_kill);
  EXPECT_FALSE(pred_segv(status_kill));
  EXPECT_FALSE(pred_kill(status_segv));
}

# endif  // GTEST_OS_WINDOWS || GTEST_OS_FUCHSIA

// be followed by operator<<, and that in either case the complete text
// comprises only a single C++ statement.
TEST_F(TestForDeathTest, SingleStatement) {
  if (AlwaysFalse())

    ASSERT_DEATH(return, "");

  if (AlwaysTrue())
    EXPECT_DEATH(_exit(1), "");
  else


    ;

  if (AlwaysFalse())
    ASSERT_DEATH(return, "") << "did not die";

  if (AlwaysFalse())
    ;
  else
    EXPECT_DEATH(_exit(1), "") << 1 << 2 << 3;
}

# if GTEST_USES_PCRE

void DieWithEmbeddedNul() {
  fprintf(stderr, "Hello%cmy null world.\n", '\0');
  fflush(stderr);
  _exit(1);
}

// message has a NUL character in it.
TEST_F(TestForDeathTest, EmbeddedNulInMessage) {
  EXPECT_DEATH(DieWithEmbeddedNul(), "my null world");
  ASSERT_DEATH(DieWithEmbeddedNul(), "my null world");
}

# endif  // GTEST_USES_PCRE

// statements.
TEST_F(TestForDeathTest, SwitchStatement) {


  GTEST_DISABLE_MSC_WARNINGS_PUSH_(4065)

  switch (0)
    default:
      ASSERT_DEATH(_exit(1), "") << "exit in default switch handler";

  switch (0)
    case 0:
      EXPECT_DEATH(_exit(1), "") << "exit in switch case";

  GTEST_DISABLE_MSC_WARNINGS_POP_()
}

// death test.
TEST_F(TestForDeathTest, StaticMemberFunctionFastStyle) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  ASSERT_DEATH(StaticMemberFunction(), "death.*StaticMember");
}

// style death test.
TEST_F(TestForDeathTest, MemberFunctionFastStyle) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  should_die_ = true;
  EXPECT_DEATH(MemberFunction(), "inside.*MemberFunction");
}

void ChangeToRootDir() { posix::ChDir(GTEST_PATH_SEP_); }

// changed.
TEST_F(TestForDeathTest, FastDeathTestInChangedDir) {
  testing::GTEST_FLAG(death_test_style) = "fast";

  ChangeToRootDir();
  EXPECT_EXIT(_exit(1), testing::ExitedWithCode(1), "");

  ChangeToRootDir();
  ASSERT_DEATH(_exit(1), "");
}

# if GTEST_OS_LINUX
void SigprofAction(int, siginfo_t*, void*) { /* no op */ }

void SetSigprofActionAndTimer() {
  struct itimerval timer;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 1;
  timer.it_value = timer.it_interval;
  ASSERT_EQ(0, setitimer(ITIMER_PROF, &timer, nullptr));
  struct sigaction signal_action;
  memset(&signal_action, 0, sizeof(signal_action));
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_sigaction = SigprofAction;
  signal_action.sa_flags = SA_RESTART | SA_SIGINFO;
  ASSERT_EQ(0, sigaction(SIGPROF, &signal_action, nullptr));
}

void DisableSigprofActionAndTimer(struct sigaction* old_signal_action) {
  struct itimerval timer;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  timer.it_value = timer.it_interval;
  ASSERT_EQ(0, setitimer(ITIMER_PROF, &timer, nullptr));
  struct sigaction signal_action;
  memset(&signal_action, 0, sizeof(signal_action));
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_handler = SIG_IGN;
  ASSERT_EQ(0, sigaction(SIGPROF, &signal_action, old_signal_action));
}

TEST_F(TestForDeathTest, FastSigprofActionSet) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  SetSigprofActionAndTimer();
  EXPECT_DEATH(_exit(1), "");
  struct sigaction old_signal_action;
  DisableSigprofActionAndTimer(&old_signal_action);
  EXPECT_TRUE(old_signal_action.sa_sigaction == SigprofAction);
}

TEST_F(TestForDeathTest, ThreadSafeSigprofActionSet) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  SetSigprofActionAndTimer();
  EXPECT_DEATH(_exit(1), "");
  struct sigaction old_signal_action;
  DisableSigprofActionAndTimer(&old_signal_action);
  EXPECT_TRUE(old_signal_action.sa_sigaction == SigprofAction);
}
# endif  // GTEST_OS_LINUX


TEST_F(TestForDeathTest, StaticMemberFunctionThreadsafeStyle) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  ASSERT_DEATH(StaticMemberFunction(), "death.*StaticMember");
}

TEST_F(TestForDeathTest, MemberFunctionThreadsafeStyle) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  should_die_ = true;
  EXPECT_DEATH(MemberFunction(), "inside.*MemberFunction");
}

TEST_F(TestForDeathTest, ThreadsafeDeathTestInLoop) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";

  for (int i = 0; i < 3; ++i)
    EXPECT_EXIT(_exit(i), testing::ExitedWithCode(i), "") << ": i = " << i;
}

TEST_F(TestForDeathTest, ThreadsafeDeathTestInChangedDir) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";

  ChangeToRootDir();
  EXPECT_EXIT(_exit(1), testing::ExitedWithCode(1), "");

  ChangeToRootDir();
  ASSERT_DEATH(_exit(1), "");
}

TEST_F(TestForDeathTest, MixedStyles) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  EXPECT_DEATH(_exit(1), "");
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_DEATH(_exit(1), "");
}

# if GTEST_HAS_CLONE && GTEST_HAS_PTHREAD

bool pthread_flag;

void SetPthreadFlag() {
  pthread_flag = true;
}

TEST_F(TestForDeathTest, DoesNotExecuteAtforkHooks) {
  if (!testing::GTEST_FLAG(death_test_use_fork)) {
    testing::GTEST_FLAG(death_test_style) = "threadsafe";
    pthread_flag = false;
    ASSERT_EQ(0, pthread_atfork(&SetPthreadFlag, nullptr, nullptr));
    ASSERT_DEATH(_exit(1), "");
    ASSERT_FALSE(pthread_flag);
  }
}

# endif  // GTEST_HAS_CLONE && GTEST_HAS_PTHREAD

TEST_F(TestForDeathTest, MethodOfAnotherClass) {
  const MayDie x(true);
  ASSERT_DEATH(x.MemberFunction(), "MayDie\\:\\:MemberFunction");
}

TEST_F(TestForDeathTest, GlobalFunction) {
  EXPECT_DEATH(GlobalFunction(), "GlobalFunction");
}

// argument to EXPECT_DEATH.
TEST_F(TestForDeathTest, AcceptsAnythingConvertibleToRE) {
  static const char regex_c_str[] = "GlobalFunction";
  EXPECT_DEATH(GlobalFunction(), regex_c_str);

  const testing::internal::RE regex(regex_c_str);
  EXPECT_DEATH(GlobalFunction(), regex);

# if !GTEST_USES_PCRE

  const ::std::string regex_std_str(regex_c_str);
  EXPECT_DEATH(GlobalFunction(), regex_std_str);


  EXPECT_DEATH(GlobalFunction(), ::std::string(regex_c_str).c_str());

# endif  // !GTEST_USES_PCRE
}

TEST_F(TestForDeathTest, NonVoidFunction) {
  ASSERT_DEATH(NonVoidFunction(), "NonVoidFunction");
}

TEST_F(TestForDeathTest, FunctionWithParameter) {
  EXPECT_DEATH(DieIf(true), "DieIf\\(\\)");
  EXPECT_DEATH(DieIfLessThan(2, 3), "DieIfLessThan");
}

TEST_F(TestForDeathTest, OutsideFixture) {
  DeathTestSubroutine();
}

TEST_F(TestForDeathTest, InsideLoop) {
  for (int i = 0; i < 5; i++) {
    EXPECT_DEATH(DieIfLessThan(-1, i), "DieIfLessThan") << "where i == " << i;
  }
}

TEST_F(TestForDeathTest, CompoundStatement) {
  EXPECT_DEATH({  // NOLINT
    const int x = 2;
    const int y = x + 1;
    DieIfLessThan(x, y);
  },
  "DieIfLessThan");
}

TEST_F(TestForDeathTest, DoesNotDie) {
  EXPECT_NONFATAL_FAILURE(EXPECT_DEATH(DieIf(false), "DieIf"),
                          "failed to die");
}

TEST_F(TestForDeathTest, ErrorMessageMismatch) {
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_DEATH(DieIf(true), "DieIfLessThan") << "End of death test message.";
  }, "died but not with expected error");
}

// aborted the function.
void ExpectDeathTestHelper(bool* aborted) {
  *aborted = true;
  EXPECT_DEATH(DieIf(false), "DieIf");  // This assertion should fail.
  *aborted = false;
}

TEST_F(TestForDeathTest, EXPECT_DEATH) {
  bool aborted = true;
  EXPECT_NONFATAL_FAILURE(ExpectDeathTestHelper(&aborted),
                          "failed to die");
  EXPECT_FALSE(aborted);
}

TEST_F(TestForDeathTest, ASSERT_DEATH) {
  static bool aborted;
  EXPECT_FATAL_FAILURE({  // NOLINT
    aborted = true;
    ASSERT_DEATH(DieIf(false), "DieIf");  // This assertion should fail.
    aborted = false;
  }, "failed to die");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, SingleEvaluation) {
  int x = 3;
  EXPECT_DEATH(DieIf((++x) == 4), "DieIf");

  const char* regex = "DieIf";
  const char* regex_save = regex;
  EXPECT_DEATH(DieIfLessThan(3, 4), regex++);
  EXPECT_EQ(regex_save + 1, regex);
}

TEST_F(TestForDeathTest, RunawayIsFailure) {
  EXPECT_NONFATAL_FAILURE(EXPECT_DEATH(static_cast<void>(0), "Foo"),
                          "failed to die.");
}

// failure.
TEST_F(TestForDeathTest, ReturnIsFailure) {
  EXPECT_FATAL_FAILURE(ASSERT_DEATH(return, "Bar"),
                       "illegal return in test statement.");
}

// message to it, and in debug mode it:
// 1. Asserts on death.
// 2. Has no side effect.
//
// And in opt mode, it:
// 1.  Has side effects but does not assert.
TEST_F(TestForDeathTest, TestExpectDebugDeath) {
  int sideeffect = 0;


  const char* regex = "death.*DieInDebugElse12";

  EXPECT_DEBUG_DEATH(DieInDebugElse12(&sideeffect), regex)
      << "Must accept a streamed message";

# ifdef NDEBUG

  EXPECT_EQ(12, sideeffect);

# else

  EXPECT_EQ(0, sideeffect);

# endif
}

// message to it, and in debug mode it:
// 1. Asserts on death.
// 2. Has no side effect.
//
// And in opt mode, it:
// 1.  Has side effects but does not assert.
TEST_F(TestForDeathTest, TestAssertDebugDeath) {
  int sideeffect = 0;

  ASSERT_DEBUG_DEATH(DieInDebugElse12(&sideeffect), "death.*DieInDebugElse12")
      << "Must accept a streamed message";

# ifdef NDEBUG

  EXPECT_EQ(12, sideeffect);

# else

  EXPECT_EQ(0, sideeffect);

# endif
}

# ifndef NDEBUG

void ExpectDebugDeathHelper(bool* aborted) {
  *aborted = true;
  EXPECT_DEBUG_DEATH(return, "") << "This is expected to fail.";
  *aborted = false;
}

#  if GTEST_OS_WINDOWS
TEST(PopUpDeathTest, DoesNotShowPopUpOnAbort) {
  printf("This test should be considered failing if it shows "
         "any pop-up dialogs.\n");
  fflush(stdout);

  EXPECT_DEATH({
    testing::GTEST_FLAG(catch_exceptions) = false;
    abort();
  }, "");
}
#  endif  // GTEST_OS_WINDOWS

// the function.
TEST_F(TestForDeathTest, ExpectDebugDeathDoesNotAbort) {
  bool aborted = true;
  EXPECT_NONFATAL_FAILURE(ExpectDebugDeathHelper(&aborted), "");
  EXPECT_FALSE(aborted);
}

void AssertDebugDeathHelper(bool* aborted) {
  *aborted = true;
  GTEST_LOG_(INFO) << "Before ASSERT_DEBUG_DEATH";
  ASSERT_DEBUG_DEATH(GTEST_LOG_(INFO) << "In ASSERT_DEBUG_DEATH"; return, "")
      << "This is expected to fail.";
  GTEST_LOG_(INFO) << "After ASSERT_DEBUG_DEATH";
  *aborted = false;
}

// failure.
TEST_F(TestForDeathTest, AssertDebugDeathAborts) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts2) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts3) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts4) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts5) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts6) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts7) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts8) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts9) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

TEST_F(TestForDeathTest, AssertDebugDeathAborts10) {
  static bool aborted;
  aborted = false;
  EXPECT_FATAL_FAILURE(AssertDebugDeathHelper(&aborted), "");
  EXPECT_TRUE(aborted);
}

# endif  // _NDEBUG

static void TestExitMacros() {
  EXPECT_EXIT(_exit(1),  testing::ExitedWithCode(1),  "");
  ASSERT_EXIT(_exit(42), testing::ExitedWithCode(42), "");

# if GTEST_OS_WINDOWS



  EXPECT_EXIT(raise(SIGABRT), testing::ExitedWithCode(3), "") << "b_ar";

# elif !GTEST_OS_FUCHSIA

  EXPECT_EXIT(raise(SIGKILL), testing::KilledBySignal(SIGKILL), "") << "foo";
  ASSERT_EXIT(raise(SIGUSR2), testing::KilledBySignal(SIGUSR2), "") << "bar";

  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_EXIT(_exit(0), testing::KilledBySignal(SIGSEGV), "")
      << "This failure is expected, too.";
  }, "This failure is expected, too.");

# endif  // GTEST_OS_WINDOWS

  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_EXIT(raise(SIGSEGV), testing::ExitedWithCode(0), "")
      << "This failure is expected.";
  }, "This failure is expected.");
}

TEST_F(TestForDeathTest, ExitMacros) {
  TestExitMacros();
}

TEST_F(TestForDeathTest, ExitMacrosUsingFork) {
  testing::GTEST_FLAG(death_test_use_fork) = true;
  TestExitMacros();
}

TEST_F(TestForDeathTest, InvalidStyle) {
  testing::GTEST_FLAG(death_test_style) = "rococo";
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_DEATH(_exit(0), "") << "This failure is expected.";
  }, "This failure is expected.");
}

TEST_F(TestForDeathTest, DeathTestFailedOutput) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_NONFATAL_FAILURE(
      EXPECT_DEATH(DieWithMessage("death\n"),
                   "expected message"),
      "Actual msg:\n"
      "[  DEATH   ] death\n");
}

TEST_F(TestForDeathTest, DeathTestUnexpectedReturnOutput) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_NONFATAL_FAILURE(
      EXPECT_DEATH({
          fprintf(stderr, "returning\n");
          fflush(stderr);
          return;
        }, ""),
      "    Result: illegal return in test statement.\n"
      " Error msg:\n"
      "[  DEATH   ] returning\n");
}

TEST_F(TestForDeathTest, DeathTestBadExitCodeOutput) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_NONFATAL_FAILURE(
      EXPECT_EXIT(DieWithMessage("exiting with rc 1\n"),
                  testing::ExitedWithCode(3),
                  "expected message"),
      "    Result: died but not with expected exit code:\n"
      "            Exited with exit status 1\n"
      "Actual msg:\n"
      "[  DEATH   ] exiting with rc 1\n");
}

TEST_F(TestForDeathTest, DeathTestMultiLineMatchFail) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_NONFATAL_FAILURE(
      EXPECT_DEATH(DieWithMessage("line 1\nline 2\nline 3\n"),
                   "line 1\nxyz\nline 3\n"),
      "Actual msg:\n"
      "[  DEATH   ] line 1\n"
      "[  DEATH   ] line 2\n"
      "[  DEATH   ] line 3\n");
}

TEST_F(TestForDeathTest, DeathTestMultiLineMatchPass) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_DEATH(DieWithMessage("line 1\nline 2\nline 3\n"),
               "line 1\nline 2\nline 3\n");
}

class MockDeathTestFactory : public DeathTestFactory {
 public:
  MockDeathTestFactory();
  bool Create(const char* statement,
              testing::Matcher<const std::string&> matcher, const char* file,
              int line, DeathTest** test) override;

  void SetParameters(bool create, DeathTest::TestRole role,
                     int status, bool passed);

  int AssumeRoleCalls() const { return assume_role_calls_; }
  int WaitCalls() const { return wait_calls_; }
  size_t PassedCalls() const { return passed_args_.size(); }
  bool PassedArgument(int n) const { return passed_args_[n]; }
  size_t AbortCalls() const { return abort_args_.size(); }
  DeathTest::AbortReason AbortArgument(int n) const {
    return abort_args_[n];
  }
  bool TestDeleted() const { return test_deleted_; }

 private:
  friend class MockDeathTest;


  bool create_;

  DeathTest::TestRole role_;

  int status_;

  bool passed_;

  int assume_role_calls_;

  int wait_calls_;


  std::vector<bool> passed_args_;


  std::vector<DeathTest::AbortReason> abort_args_;


  bool test_deleted_;
};

// at its creation from its various inherited DeathTest methods, and
// reports calls to those methods to its parent MockDeathTestFactory
// object.
class MockDeathTest : public DeathTest {
 public:
  MockDeathTest(MockDeathTestFactory *parent,
                TestRole role, int status, bool passed) :
      parent_(parent), role_(role), status_(status), passed_(passed) {
  }
  ~MockDeathTest() override { parent_->test_deleted_ = true; }
  TestRole AssumeRole() override {
    ++parent_->assume_role_calls_;
    return role_;
  }
  int Wait() override {
    ++parent_->wait_calls_;
    return status_;
  }
  bool Passed(bool exit_status_ok) override {
    parent_->passed_args_.push_back(exit_status_ok);
    return passed_;
  }
  void Abort(AbortReason reason) override {
    parent_->abort_args_.push_back(reason);
  }

 private:
  MockDeathTestFactory* const parent_;
  const TestRole role_;
  const int status_;
  const bool passed_;
};

MockDeathTestFactory::MockDeathTestFactory()
    : create_(true),
      role_(DeathTest::OVERSEE_TEST),
      status_(0),
      passed_(true),
      assume_role_calls_(0),
      wait_calls_(0),
      passed_args_(),
      abort_args_() {
}

void MockDeathTestFactory::SetParameters(bool create,
                                         DeathTest::TestRole role,
                                         int status, bool passed) {
  create_ = create;
  role_ = role;
  status_ = status;
  passed_ = passed;

  assume_role_calls_ = 0;
  wait_calls_ = 0;
  passed_args_.clear();
  abort_args_.clear();
}

// MockDeathTest object with parameters taken from the last call
// to SetParameters (if create_ is true).  Always returns true.
bool MockDeathTestFactory::Create(
    const char* /*statement*/, testing::Matcher<const std::string&> /*matcher*/,
    const char* /*file*/, int /*line*/, DeathTest** test) {
  test_deleted_ = false;
  if (create_) {
    *test = new MockDeathTest(this, role_, status_, passed_);
  } else {
    *test = nullptr;
  }
  return true;
}

// It installs a MockDeathTestFactory that is used for the duration
// of the test case.
class MacroLogicDeathTest : public testing::Test {
 protected:
  static testing::internal::ReplaceDeathTestFactory* replacer_;
  static MockDeathTestFactory* factory_;

  static void SetUpTestSuite() {
    factory_ = new MockDeathTestFactory;
    replacer_ = new testing::internal::ReplaceDeathTestFactory(factory_);
  }

  static void TearDownTestSuite() {
    delete replacer_;
    replacer_ = nullptr;
    delete factory_;
    factory_ = nullptr;
  }



  static void RunReturningDeathTest(bool* flag) {
    ASSERT_DEATH({  // NOLINT
      *flag = true;
      return;
    }, "");
  }
};

testing::internal::ReplaceDeathTestFactory* MacroLogicDeathTest::replacer_ =
    nullptr;
MockDeathTestFactory* MacroLogicDeathTest::factory_ = nullptr;

TEST_F(MacroLogicDeathTest, NothingHappens) {
  bool flag = false;
  factory_->SetParameters(false, DeathTest::OVERSEE_TEST, 0, true);
  EXPECT_DEATH(flag = true, "");
  EXPECT_FALSE(flag);
  EXPECT_EQ(0, factory_->AssumeRoleCalls());
  EXPECT_EQ(0, factory_->WaitCalls());
  EXPECT_EQ(0U, factory_->PassedCalls());
  EXPECT_EQ(0U, factory_->AbortCalls());
  EXPECT_FALSE(factory_->TestDeleted());
}

// and that the Passed method returns false when the (simulated)
// child process exits with status 0:
TEST_F(MacroLogicDeathTest, ChildExitsSuccessfully) {
  bool flag = false;
  factory_->SetParameters(true, DeathTest::OVERSEE_TEST, 0, true);
  EXPECT_DEATH(flag = true, "");
  EXPECT_FALSE(flag);
  EXPECT_EQ(1, factory_->AssumeRoleCalls());
  EXPECT_EQ(1, factory_->WaitCalls());
  ASSERT_EQ(1U, factory_->PassedCalls());
  EXPECT_FALSE(factory_->PassedArgument(0));
  EXPECT_EQ(0U, factory_->AbortCalls());
  EXPECT_TRUE(factory_->TestDeleted());
}

// the (simulated) child process exits with status 1:
TEST_F(MacroLogicDeathTest, ChildExitsUnsuccessfully) {
  bool flag = false;
  factory_->SetParameters(true, DeathTest::OVERSEE_TEST, 1, true);
  EXPECT_DEATH(flag = true, "");
  EXPECT_FALSE(flag);
  EXPECT_EQ(1, factory_->AssumeRoleCalls());
  EXPECT_EQ(1, factory_->WaitCalls());
  ASSERT_EQ(1U, factory_->PassedCalls());
  EXPECT_TRUE(factory_->PassedArgument(0));
  EXPECT_EQ(0U, factory_->AbortCalls());
  EXPECT_TRUE(factory_->TestDeleted());
}

// code, and is aborted with the correct AbortReason if it
// executes a return statement.
TEST_F(MacroLogicDeathTest, ChildPerformsReturn) {
  bool flag = false;
  factory_->SetParameters(true, DeathTest::EXECUTE_TEST, 0, true);
  RunReturningDeathTest(&flag);
  EXPECT_TRUE(flag);
  EXPECT_EQ(1, factory_->AssumeRoleCalls());
  EXPECT_EQ(0, factory_->WaitCalls());
  EXPECT_EQ(0U, factory_->PassedCalls());
  EXPECT_EQ(1U, factory_->AbortCalls());
  EXPECT_EQ(DeathTest::TEST_ENCOUNTERED_RETURN_STATEMENT,
            factory_->AbortArgument(0));
  EXPECT_TRUE(factory_->TestDeleted());
}

// correct AbortReason if it does not die.
TEST_F(MacroLogicDeathTest, ChildDoesNotDie) {
  bool flag = false;
  factory_->SetParameters(true, DeathTest::EXECUTE_TEST, 0, true);
  EXPECT_DEATH(flag = true, "");
  EXPECT_TRUE(flag);
  EXPECT_EQ(1, factory_->AssumeRoleCalls());
  EXPECT_EQ(0, factory_->WaitCalls());
  EXPECT_EQ(0U, factory_->PassedCalls());





  ASSERT_EQ(2U, factory_->AbortCalls());
  EXPECT_EQ(DeathTest::TEST_DID_NOT_DIE,
            factory_->AbortArgument(0));
  EXPECT_EQ(DeathTest::TEST_ENCOUNTERED_RETURN_STATEMENT,
            factory_->AbortArgument(1));
  EXPECT_TRUE(factory_->TestDeleted());
}

// test part.
TEST(SuccessRegistrationDeathTest, NoSuccessPart) {
  EXPECT_DEATH(_exit(1), "");
  EXPECT_EQ(0, GetUnitTestImpl()->current_test_result()->total_part_count());
}

TEST(StreamingAssertionsDeathTest, DeathTest) {
  EXPECT_DEATH(_exit(1), "") << "unexpected failure";
  ASSERT_DEATH(_exit(1), "") << "unexpected failure";
  EXPECT_NONFATAL_FAILURE({  // NOLINT
    EXPECT_DEATH(_exit(0), "") << "expected failure";
  }, "expected failure");
  EXPECT_FATAL_FAILURE({  // NOLINT
    ASSERT_DEATH(_exit(0), "") << "expected failure";
  }, "expected failure");
}

// last error is 0 and non-empty string when it is non-zero.
TEST(GetLastErrnoDescription, GetLastErrnoDescriptionWorks) {
  errno = ENOENT;
  EXPECT_STRNE("", GetLastErrnoDescription().c_str());
  errno = 0;
  EXPECT_STREQ("", GetLastErrnoDescription().c_str());
}

# if GTEST_OS_WINDOWS
TEST(AutoHandleTest, AutoHandleWorks) {
  HANDLE handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
  ASSERT_NE(INVALID_HANDLE_VALUE, handle);

  testing::internal::AutoHandle auto_handle(handle);
  EXPECT_EQ(handle, auto_handle.Get());


  auto_handle.Reset();
  EXPECT_EQ(INVALID_HANDLE_VALUE, auto_handle.Get());


  handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
  ASSERT_NE(INVALID_HANDLE_VALUE, handle);
  auto_handle.Reset(handle);
  EXPECT_EQ(handle, auto_handle.Get());

  testing::internal::AutoHandle auto_handle2;
  EXPECT_EQ(INVALID_HANDLE_VALUE, auto_handle2.Get());
}
# endif  // GTEST_OS_WINDOWS

# if GTEST_OS_WINDOWS
typedef unsigned __int64 BiggestParsable;
typedef signed __int64 BiggestSignedParsable;
# else
typedef unsigned long long BiggestParsable;
typedef signed long long BiggestSignedParsable;
# endif  // GTEST_OS_WINDOWS

// max() macro defined by <windows.h>.
const BiggestParsable kBiggestParsableMax = ULLONG_MAX;
const BiggestSignedParsable kBiggestSignedParsableMax = LLONG_MAX;

TEST(ParseNaturalNumberTest, RejectsInvalidFormat) {
  BiggestParsable result = 0;

  EXPECT_FALSE(ParseNaturalNumber("non-number string", &result));

  EXPECT_FALSE(ParseNaturalNumber(" 123", &result));

  EXPECT_FALSE(ParseNaturalNumber("-123", &result));

  EXPECT_FALSE(ParseNaturalNumber("+123", &result));
  errno = 0;
}

TEST(ParseNaturalNumberTest, RejectsOverflownNumbers) {
  BiggestParsable result = 0;

  EXPECT_FALSE(ParseNaturalNumber("99999999999999999999999", &result));

  signed char char_result = 0;
  EXPECT_FALSE(ParseNaturalNumber("200", &char_result));
  errno = 0;
}

TEST(ParseNaturalNumberTest, AcceptsValidNumbers) {
  BiggestParsable result = 0;

  result = 0;
  ASSERT_TRUE(ParseNaturalNumber("123", &result));
  EXPECT_EQ(123U, result);

  result = 1;
  ASSERT_TRUE(ParseNaturalNumber("0", &result));
  EXPECT_EQ(0U, result);

  result = 1;
  ASSERT_TRUE(ParseNaturalNumber("00000", &result));
  EXPECT_EQ(0U, result);
}

TEST(ParseNaturalNumberTest, AcceptsTypeLimits) {
  Message msg;
  msg << kBiggestParsableMax;

  BiggestParsable result = 0;
  EXPECT_TRUE(ParseNaturalNumber(msg.GetString(), &result));
  EXPECT_EQ(kBiggestParsableMax, result);

  Message msg2;
  msg2 << kBiggestSignedParsableMax;

  BiggestSignedParsable signed_result = 0;
  EXPECT_TRUE(ParseNaturalNumber(msg2.GetString(), &signed_result));
  EXPECT_EQ(kBiggestSignedParsableMax, signed_result);

  Message msg3;
  msg3 << INT_MAX;

  int int_result = 0;
  EXPECT_TRUE(ParseNaturalNumber(msg3.GetString(), &int_result));
  EXPECT_EQ(INT_MAX, int_result);

  Message msg4;
  msg4 << UINT_MAX;

  unsigned int uint_result = 0;
  EXPECT_TRUE(ParseNaturalNumber(msg4.GetString(), &uint_result));
  EXPECT_EQ(UINT_MAX, uint_result);
}

TEST(ParseNaturalNumberTest, WorksForShorterIntegers) {
  short short_result = 0;
  ASSERT_TRUE(ParseNaturalNumber("123", &short_result));
  EXPECT_EQ(123, short_result);

  signed char char_result = 0;
  ASSERT_TRUE(ParseNaturalNumber("123", &char_result));
  EXPECT_EQ(123, char_result);
}

# if GTEST_OS_WINDOWS
TEST(EnvironmentTest, HandleFitsIntoSizeT) {
  ASSERT_TRUE(sizeof(HANDLE) <= sizeof(size_t));
}
# endif  // GTEST_OS_WINDOWS

// failures when death tests are available on the system.
TEST(ConditionalDeathMacrosDeathTest, ExpectsDeathWhenDeathTestsAvailable) {
  EXPECT_DEATH_IF_SUPPORTED(DieInside("CondDeathTestExpectMacro"),
                            "death inside CondDeathTestExpectMacro");
  ASSERT_DEATH_IF_SUPPORTED(DieInside("CondDeathTestAssertMacro"),
                            "death inside CondDeathTestAssertMacro");

  EXPECT_NONFATAL_FAILURE(EXPECT_DEATH_IF_SUPPORTED(;, ""), "");
  EXPECT_FATAL_FAILURE(ASSERT_DEATH_IF_SUPPORTED(;, ""), "");
}

TEST(InDeathTestChildDeathTest, ReportsDeathTestCorrectlyInFastStyle) {
  testing::GTEST_FLAG(death_test_style) = "fast";
  EXPECT_FALSE(InDeathTestChild());
  EXPECT_DEATH({
    fprintf(stderr, InDeathTestChild() ? "Inside" : "Outside");
    fflush(stderr);
    _exit(1);
  }, "Inside");
}

TEST(InDeathTestChildDeathTest, ReportsDeathTestCorrectlyInThreadSafeStyle) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  EXPECT_FALSE(InDeathTestChild());
  EXPECT_DEATH({
    fprintf(stderr, InDeathTestChild() ? "Inside" : "Outside");
    fflush(stderr);
    _exit(1);
  }, "Inside");
}

void DieWithMessage(const char* message) {
  fputs(message, stderr);
  fflush(stderr);  // Make sure the text is printed before the process exits.
  _exit(1);
}

TEST(MatcherDeathTest, DoesNotBreakBareRegexMatching) {


  EXPECT_DEATH(DieWithMessage("O, I die, Horatio."), "I d[aeiou]e");
}

TEST(MatcherDeathTest, MonomorphicMatcherMatches) {
  EXPECT_DEATH(DieWithMessage("Behind O, I am slain!"),
               Matcher<const std::string&>(ContainsRegex("I am slain")));
}

TEST(MatcherDeathTest, MonomorphicMatcherDoesNotMatch) {
  EXPECT_NONFATAL_FAILURE(
      EXPECT_DEATH(
          DieWithMessage("Behind O, I am slain!"),
          Matcher<const std::string&>(ContainsRegex("Ow, I am slain"))),
      "Expected: contains regular expression \"Ow, I am slain\"");
}

TEST(MatcherDeathTest, PolymorphicMatcherMatches) {
  EXPECT_DEATH(DieWithMessage("The rest is silence."),
               ContainsRegex("rest is silence"));
}

TEST(MatcherDeathTest, PolymorphicMatcherDoesNotMatch) {
  EXPECT_NONFATAL_FAILURE(
      EXPECT_DEATH(DieWithMessage("The rest is silence."),
                   ContainsRegex("rest is science")),
      "Expected: contains regular expression \"rest is science\"");
}

}  // namespace

#else  // !GTEST_HAS_DEATH_TEST follows

namespace {

using testing::internal::CaptureStderr;
using testing::internal::GetCapturedStderr;

// defined but do not trigger failures when death tests are not available on
// the system.
TEST(ConditionalDeathMacrosTest, WarnsWhenDeathTestsNotAvailable) {


  CaptureStderr();
  EXPECT_DEATH_IF_SUPPORTED(;, "");
  std::string output = GetCapturedStderr();
  ASSERT_TRUE(NULL != strstr(output.c_str(),
                             "Death tests are not supported on this platform"));
  ASSERT_TRUE(NULL != strstr(output.c_str(), ";"));

  CaptureStderr();
  EXPECT_DEATH_IF_SUPPORTED(;, "") << "streamed message";
  output = GetCapturedStderr();
  ASSERT_TRUE(NULL == strstr(output.c_str(), "streamed message"));

  CaptureStderr();
  ASSERT_DEATH_IF_SUPPORTED(;, "");  // NOLINT
  output = GetCapturedStderr();
  ASSERT_TRUE(NULL != strstr(output.c_str(),
                             "Death tests are not supported on this platform"));
  ASSERT_TRUE(NULL != strstr(output.c_str(), ";"));

  CaptureStderr();
  ASSERT_DEATH_IF_SUPPORTED(;, "") << "streamed message";  // NOLINT
  output = GetCapturedStderr();
  ASSERT_TRUE(NULL == strstr(output.c_str(), "streamed message"));
}

void FuncWithAssert(int* n) {
  ASSERT_DEATH_IF_SUPPORTED(return;, "");
  (*n)++;
}

// function (as ASSERT_DEATH does) if death tests are not supported.
TEST(ConditionalDeathMacrosTest, AssertDeatDoesNotReturnhIfUnsupported) {
  int n = 0;
  FuncWithAssert(&n);
  EXPECT_EQ(1, n);
}

}  // namespace

#endif  // !GTEST_HAS_DEATH_TEST

namespace {

// be followed by operator<<, and that in either case the complete text
// comprises only a single C++ statement.
//
// The syntax should work whether death tests are available or not.
TEST(ConditionalDeathMacrosSyntaxDeathTest, SingleStatement) {
  if (AlwaysFalse())

    ASSERT_DEATH_IF_SUPPORTED(return, "");

  if (AlwaysTrue())
    EXPECT_DEATH_IF_SUPPORTED(_exit(1), "");
  else


    ;  // NOLINT

  if (AlwaysFalse())
    ASSERT_DEATH_IF_SUPPORTED(return, "") << "did not die";

  if (AlwaysFalse())
    ;  // NOLINT
  else
    EXPECT_DEATH_IF_SUPPORTED(_exit(1), "") << 1 << 2 << 3;
}

// well with switch statements.
TEST(ConditionalDeathMacrosSyntaxDeathTest, SwitchStatement) {


  GTEST_DISABLE_MSC_WARNINGS_PUSH_(4065)

  switch (0)
    default:
      ASSERT_DEATH_IF_SUPPORTED(_exit(1), "")
          << "exit in default switch handler";

  switch (0)
    case 0:
      EXPECT_DEATH_IF_SUPPORTED(_exit(1), "") << "exit in switch case";

  GTEST_DISABLE_MSC_WARNINGS_POP_()
}

// on Windows.
TEST(NotADeathTest, Test) {
  SUCCEED();
}

}  // namespace
