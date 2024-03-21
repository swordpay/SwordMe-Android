// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process.h"

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "base/clang_profiling_buildflags.h"
#include "base/debug/activity_tracker.h"
#include "base/files/scoped_file.h"
#include "base/logging.h"
#include "base/posix/eintr_wrapper.h"
#include "base/process/kill.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"

#if defined(OS_MACOSX)
#include <sys/event.h>
#endif

#if BUILDFLAG(CLANG_PROFILING)
#include "base/test/clang_profiling.h"
#endif

namespace {

#if !defined(OS_NACL_NONSFI)

bool WaitpidWithTimeout(base::ProcessHandle handle,
                        int* status,
                        base::TimeDelta wait) {
























  if (wait == base::TimeDelta::Max()) {
    return HANDLE_EINTR(waitpid(handle, status, 0)) > 0;
  }

  pid_t ret_pid = HANDLE_EINTR(waitpid(handle, status, WNOHANG));
  static const int64_t kMaxSleepInMicroseconds = 1 << 18;  // ~256 milliseconds.
  int64_t max_sleep_time_usecs = 1 << 10;                  // ~1 milliseconds.
  int64_t double_sleep_time = 0;

  base::TimeTicks wakeup_time = base::TimeTicks::Now() + wait;
  while (ret_pid == 0) {
    base::TimeTicks now = base::TimeTicks::Now();
    if (now > wakeup_time)
      break;

    int64_t sleep_time_usecs = (wakeup_time - now).InMicroseconds();

    if (sleep_time_usecs > max_sleep_time_usecs)
      sleep_time_usecs = max_sleep_time_usecs;


    usleep(sleep_time_usecs);
    ret_pid = HANDLE_EINTR(waitpid(handle, status, WNOHANG));

    if ((max_sleep_time_usecs < kMaxSleepInMicroseconds) &&
        (double_sleep_time++ % 4 == 0)) {
      max_sleep_time_usecs *= 2;
    }
  }

  return ret_pid > 0;
}

#if defined(OS_MACOSX)
// Using kqueue on Mac so that we can wait on non-child processes.
// We can't use kqueues on child processes because we need to reap
// our own children using wait.
bool WaitForSingleNonChildProcess(base::ProcessHandle handle,
                                  base::TimeDelta wait) {
  DCHECK_GT(handle, 0);

  base::ScopedFD kq(kqueue());
  if (!kq.is_valid()) {
    DPLOG(ERROR) << "kqueue";
    return false;
  }

  struct kevent change = {0};
  EV_SET(&change, handle, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
  int result = HANDLE_EINTR(kevent(kq.get(), &change, 1, NULL, 0, NULL));
  if (result == -1) {
    if (errno == ESRCH) {

      return true;
    }

    DPLOG(ERROR) << "kevent (setup " << handle << ")";
    return false;
  }


  bool wait_forever = (wait == base::TimeDelta::Max());
  base::TimeDelta remaining_delta;
  base::TimeTicks deadline;
  if (!wait_forever) {
    remaining_delta = wait;
    deadline = base::TimeTicks::Now() + remaining_delta;
  }

  result = -1;
  struct kevent event = {0};

  do {
    struct timespec remaining_timespec;
    struct timespec* remaining_timespec_ptr;
    if (wait_forever) {
      remaining_timespec_ptr = NULL;
    } else {
      remaining_timespec = remaining_delta.ToTimeSpec();
      remaining_timespec_ptr = &remaining_timespec;
    }

    result = kevent(kq.get(), NULL, 0, &event, 1, remaining_timespec_ptr);

    if (result == -1 && errno == EINTR) {
      if (!wait_forever) {
        remaining_delta = deadline - base::TimeTicks::Now();
      }
      result = 0;
    } else {
      break;
    }
  } while (wait_forever || remaining_delta > base::TimeDelta());

  if (result < 0) {
    DPLOG(ERROR) << "kevent (wait " << handle << ")";
    return false;
  } else if (result > 1) {
    DLOG(ERROR) << "kevent (wait " << handle << "): unexpected result "
                << result;
    return false;
  } else if (result == 0) {

    return false;
  }

  DCHECK_EQ(result, 1);

  if (event.filter != EVFILT_PROC ||
      (event.fflags & NOTE_EXIT) == 0 ||
      event.ident != static_cast<uintptr_t>(handle)) {
    DLOG(ERROR) << "kevent (wait " << handle
                << "): unexpected event: filter=" << event.filter
                << ", fflags=" << event.fflags
                << ", ident=" << event.ident;
    return false;
  }

  return true;
}
#endif  // OS_MACOSX

bool WaitForExitWithTimeoutImpl(base::ProcessHandle handle,
                                int* exit_code,
                                base::TimeDelta timeout) {
  const base::ProcessHandle our_pid = base::GetCurrentProcessHandle();
  if (handle == our_pid) {

    return false;
  }

  const base::ProcessHandle parent_pid = base::GetParentProcessId(handle);
  const bool exited = (parent_pid < 0);

  if (!exited && parent_pid != our_pid) {
#if defined(OS_MACOSX)

    return WaitForSingleNonChildProcess(handle, timeout);
#else

    NOTIMPLEMENTED();
#endif  // OS_MACOSX
  }

  int status;
  if (!WaitpidWithTimeout(handle, &status, timeout))
    return exited;
  if (WIFSIGNALED(status)) {
    if (exit_code)
      *exit_code = -1;
    return true;
  }
  if (WIFEXITED(status)) {
    if (exit_code)
      *exit_code = WEXITSTATUS(status);
    return true;
  }
  return exited;
}
#endif  // !defined(OS_NACL_NONSFI)

}  // namespace

namespace base {

Process::Process(ProcessHandle handle) : process_(handle) {
}

Process::~Process() = default;

Process::Process(Process&& other) : process_(other.process_) {
  other.Close();
}

Process& Process::operator=(Process&& other) {
  process_ = other.process_;
  other.Close();
  return *this;
}

Process Process::Current() {
  return Process(GetCurrentProcessHandle());
}

Process Process::Open(ProcessId pid) {
  if (pid == GetCurrentProcId())
    return Current();

  return Process(pid);
}

Process Process::OpenWithExtraPrivileges(ProcessId pid) {

  return Open(pid);
}

Process Process::DeprecatedGetProcessFromHandle(ProcessHandle handle) {
  DCHECK_NE(handle, GetCurrentProcessHandle());
  return Process(handle);
}

#if !defined(OS_LINUX) && !defined(OS_MACOSX) && !defined(OS_AIX)
// static
bool Process::CanBackgroundProcesses() {
  return false;
}
#endif  // !defined(OS_LINUX) && !defined(OS_MACOSX) && !defined(OS_AIX)

void Process::TerminateCurrentProcessImmediately(int exit_code) {
#if BUILDFLAG(CLANG_PROFILING)
  WriteClangProfilingProfile();
#endif
  _exit(exit_code);
}

bool Process::IsValid() const {
  return process_ != kNullProcessHandle;
}

ProcessHandle Process::Handle() const {
  return process_;
}

Process Process::Duplicate() const {
  if (is_current())
    return Current();

  return Process(process_);
}

ProcessId Process::Pid() const {
  DCHECK(IsValid());
  return GetProcId(process_);
}

bool Process::is_current() const {
  return process_ == GetCurrentProcessHandle();
}

void Process::Close() {
  process_ = kNullProcessHandle;



}

#if !defined(OS_NACL_NONSFI)
bool Process::Terminate(int exit_code, bool wait) const {

  DCHECK(IsValid());
  CHECK_GT(process_, 0);

  bool did_terminate = kill(process_, SIGTERM) == 0;

  if (wait && did_terminate) {
    if (WaitForExitWithTimeout(TimeDelta::FromSeconds(60), nullptr))
      return true;
    did_terminate = kill(process_, SIGKILL) == 0;
    if (did_terminate)
      return WaitForExit(nullptr);
  }

  if (!did_terminate)
    DPLOG(ERROR) << "Unable to terminate process " << process_;

  return did_terminate;
}
#endif  // !defined(OS_NACL_NONSFI)

bool Process::WaitForExit(int* exit_code) const {
  return WaitForExitWithTimeout(TimeDelta::Max(), exit_code);
}

bool Process::WaitForExitWithTimeout(TimeDelta timeout, int* exit_code) const {



  if (!timeout.is_zero())
    internal::AssertBaseSyncPrimitivesAllowed();

  base::debug::ScopedProcessWaitActivity process_activity(this);

  int local_exit_code = 0;
  bool exited = WaitForExitWithTimeoutImpl(Handle(), &local_exit_code, timeout);
  if (exited) {
    Exited(local_exit_code);
    if (exit_code)
      *exit_code = local_exit_code;
  }
  return exited;
}

void Process::Exited(int exit_code) const {}

#if !defined(OS_LINUX) && !defined(OS_MACOSX) && !defined(OS_AIX)
bool Process::IsProcessBackgrounded() const {

  DCHECK(IsValid());
  return false;
}

bool Process::SetProcessBackgrounded(bool value) {



  NOTIMPLEMENTED();
  return false;
}
#endif  // !defined(OS_LINUX) && !defined(OS_MACOSX) && !defined(OS_AIX)

int Process::GetPriority() const {
  DCHECK(IsValid());
  return getpriority(PRIO_PROCESS, process_);
}

}  // namespace base
