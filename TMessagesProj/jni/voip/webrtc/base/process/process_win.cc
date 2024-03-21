// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process.h"

#include "base/clang_profiling_buildflags.h"
#include "base/debug/activity_tracker.h"
#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/process/kill.h"
#include "base/threading/thread_restrictions.h"

#include <windows.h>

#if BUILDFLAG(CLANG_PROFILING)
#include "base/test/clang_profiling.h"
#endif

namespace {

DWORD kBasicProcessAccess =
  PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | SYNCHRONIZE;

} // namespace

namespace base {

Process::Process(ProcessHandle handle)
    : process_(handle), is_current_process_(false) {
  CHECK_NE(handle, ::GetCurrentProcess());
}

Process::Process(Process&& other)
    : process_(other.process_.Take()),
      is_current_process_(other.is_current_process_) {
  other.Close();
}

Process::~Process() {
}

Process& Process::operator=(Process&& other) {
  DCHECK_NE(this, &other);
  process_.Set(other.process_.Take());
  is_current_process_ = other.is_current_process_;
  other.Close();
  return *this;
}

Process Process::Current() {
  Process process;
  process.is_current_process_ = true;
  return process;
}

Process Process::Open(ProcessId pid) {
  return Process(::OpenProcess(kBasicProcessAccess, FALSE, pid));
}

Process Process::OpenWithExtraPrivileges(ProcessId pid) {
  DWORD access = kBasicProcessAccess | PROCESS_DUP_HANDLE | PROCESS_VM_READ;
  return Process(::OpenProcess(access, FALSE, pid));
}

Process Process::OpenWithAccess(ProcessId pid, DWORD desired_access) {
  return Process(::OpenProcess(desired_access, FALSE, pid));
}

Process Process::DeprecatedGetProcessFromHandle(ProcessHandle handle) {
  DCHECK_NE(handle, ::GetCurrentProcess());
  ProcessHandle out_handle;
  if (!::DuplicateHandle(GetCurrentProcess(), handle,
                         GetCurrentProcess(), &out_handle,
                         0, FALSE, DUPLICATE_SAME_ACCESS)) {
    return Process();
  }
  return Process(out_handle);
}

bool Process::CanBackgroundProcesses() {
  return true;
}

void Process::TerminateCurrentProcessImmediately(int exit_code) {
#if BUILDFLAG(CLANG_PROFILING)
  WriteClangProfilingProfile();
#endif
  ::TerminateProcess(GetCurrentProcess(), exit_code);


  IMMEDIATE_CRASH();
}

bool Process::IsValid() const {
  return process_.IsValid() || is_current();
}

ProcessHandle Process::Handle() const {
  return is_current_process_ ? GetCurrentProcess() : process_.Get();
}

Process Process::Duplicate() const {
  if (is_current())
    return Current();

  ProcessHandle out_handle;
  if (!IsValid() || !::DuplicateHandle(GetCurrentProcess(),
                                       Handle(),
                                       GetCurrentProcess(),
                                       &out_handle,
                                       0,
                                       FALSE,
                                       DUPLICATE_SAME_ACCESS)) {
    return Process();
  }
  return Process(out_handle);
}

ProcessId Process::Pid() const {
  DCHECK(IsValid());
  return GetProcId(Handle());
}

Time Process::CreationTime() const {
  FILETIME creation_time = {};
  FILETIME ignore1 = {};
  FILETIME ignore2 = {};
  FILETIME ignore3 = {};
  if (!::GetProcessTimes(Handle(), &creation_time, &ignore1, &ignore2,
                         &ignore3)) {
    return Time();
  }
  return Time::FromFileTime(creation_time);
}

bool Process::is_current() const {
  return is_current_process_;
}

void Process::Close() {
  is_current_process_ = false;
  if (!process_.IsValid())
    return;

  process_.Close();
}

bool Process::Terminate(int exit_code, bool wait) const {
  constexpr DWORD kWaitMs = 60 * 1000;

  DCHECK(IsValid());
  bool result = (::TerminateProcess(Handle(), exit_code) != FALSE);
  if (result) {

    if (wait && ::WaitForSingleObject(Handle(), kWaitMs) != WAIT_OBJECT_0)
      DPLOG(ERROR) << "Error waiting for process exit";
    Exited(exit_code);
  } else {





    if (GetLastError() != ERROR_ACCESS_DENIED)
      DPLOG(ERROR) << "Unable to terminate process";

    if (::WaitForSingleObject(Handle(), kWaitMs) == WAIT_OBJECT_0) {
      DWORD actual_exit;
      Exited(::GetExitCodeProcess(Handle(), &actual_exit) ? actual_exit
                                                          : exit_code);
      result = true;
    }
  }
  return result;
}

Process::WaitExitStatus Process::WaitForExitOrEvent(
    const base::win::ScopedHandle& stop_event_handle,
    int* exit_code) const {

  base::debug::ScopedProcessWaitActivity process_activity(this);

  HANDLE events[] = {Handle(), stop_event_handle.Get()};
  DWORD wait_result =
      ::WaitForMultipleObjects(base::size(events), events, FALSE, INFINITE);

  if (wait_result == WAIT_OBJECT_0) {
    DWORD temp_code;  // Don't clobber out-parameters in case of failure.
    if (!::GetExitCodeProcess(Handle(), &temp_code))
      return Process::WaitExitStatus::FAILED;

    if (exit_code)
      *exit_code = temp_code;

    Exited(temp_code);
    return Process::WaitExitStatus::PROCESS_EXITED;
  }

  if (wait_result == WAIT_OBJECT_0 + 1) {
    return Process::WaitExitStatus::STOP_EVENT_SIGNALED;
  }

  return Process::WaitExitStatus::FAILED;
}

bool Process::WaitForExit(int* exit_code) const {
  return WaitForExitWithTimeout(TimeDelta::FromMilliseconds(INFINITE),
                                exit_code);
}

bool Process::WaitForExitWithTimeout(TimeDelta timeout, int* exit_code) const {



  if (!timeout.is_zero())
    internal::AssertBaseSyncPrimitivesAllowed();

  base::debug::ScopedProcessWaitActivity process_activity(this);

  DWORD timeout_ms = saturated_cast<DWORD>(timeout.InMilliseconds());
  if (::WaitForSingleObject(Handle(), timeout_ms) != WAIT_OBJECT_0)
    return false;

  DWORD temp_code;  // Don't clobber out-parameters in case of failure.
  if (!::GetExitCodeProcess(Handle(), &temp_code))
    return false;

  if (exit_code)
    *exit_code = temp_code;

  Exited(temp_code);
  return true;
}

void Process::Exited(int exit_code) const {
  base::debug::GlobalActivityTracker::RecordProcessExitIfEnabled(Pid(),
                                                                 exit_code);
}

bool Process::IsProcessBackgrounded() const {
  DCHECK(IsValid());
  DWORD priority = GetPriority();
  if (priority == 0)
    return false;  // Failure case.
  return ((priority == BELOW_NORMAL_PRIORITY_CLASS) ||
          (priority == IDLE_PRIORITY_CLASS));
}

bool Process::SetProcessBackgrounded(bool value) {
  DCHECK(IsValid());



  DWORD priority;
  if (is_current()) {
    priority = value ? PROCESS_MODE_BACKGROUND_BEGIN :
                       PROCESS_MODE_BACKGROUND_END;
  } else {
    priority = value ? IDLE_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
  }

  return (::SetPriorityClass(Handle(), priority) != 0);
}

int Process::GetPriority() const {
  DCHECK(IsValid());
  return ::GetPriorityClass(Handle());
}

}  // namespace base
