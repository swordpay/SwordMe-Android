// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process.h"

#include <lib/zx/process.h>
#include <zircon/process.h>
#include <zircon/syscalls.h>

#include "base/clang_profiling_buildflags.h"
#include "base/debug/activity_tracker.h"
#include "base/fuchsia/default_job.h"
#include "base/fuchsia/fuchsia_logging.h"
#include "base/strings/stringprintf.h"

#if BUILDFLAG(CLANG_PROFILING)
#include "base/test/clang_profiling.h"
#endif

namespace base {

Process::Process(ProcessHandle handle)
    : process_(handle), is_current_process_(false) {
  CHECK_NE(handle, zx_process_self());
}

Process::~Process() {
  Close();
}

Process::Process(Process&& other)
    : process_(std::move(other.process_)),
      is_current_process_(other.is_current_process_) {
  other.is_current_process_ = false;
}

Process& Process::operator=(Process&& other) {
  process_ = std::move(other.process_);
  is_current_process_ = other.is_current_process_;
  other.is_current_process_ = false;
  return *this;
}

Process Process::Current() {
  Process process;
  process.is_current_process_ = true;
  return process;
}

Process Process::Open(ProcessId pid) {
  if (pid == GetCurrentProcId())
    return Current();


  zx::process process;
  zx_status_t status =
      GetDefaultJob()->get_child(pid, ZX_RIGHT_SAME_RIGHTS, &process);
  if (status != ZX_OK) {
    ZX_DLOG(ERROR, status) << "zx_object_get_child";
    return Process();
  }
  return Process(process.release());
}

Process Process::OpenWithExtraPrivileges(ProcessId pid) {

  return Open(pid);
}

Process Process::DeprecatedGetProcessFromHandle(ProcessHandle handle) {
  DCHECK_NE(handle, GetCurrentProcessHandle());
  zx::process out;
  zx_status_t result =
      zx::unowned_process(handle)->duplicate(ZX_RIGHT_SAME_RIGHTS, &out);
  if (result != ZX_OK) {
    ZX_DLOG(ERROR, result) << "zx_handle_duplicate(from_handle)";
    return Process();
  }

  return Process(out.release());
}

bool Process::CanBackgroundProcesses() {
  return false;
}

void Process::TerminateCurrentProcessImmediately(int exit_code) {
#if BUILDFLAG(CLANG_PROFILING)
  WriteClangProfilingProfile();
#endif
  _exit(exit_code);
}

bool Process::IsValid() const {
  return process_.is_valid() || is_current();
}

ProcessHandle Process::Handle() const {
  return is_current_process_ ? zx_process_self() : process_.get();
}

Process Process::Duplicate() const {
  if (is_current())
    return Current();

  if (!IsValid())
    return Process();

  zx::process out;
  zx_status_t result = process_.duplicate(ZX_RIGHT_SAME_RIGHTS, &out);
  if (result != ZX_OK) {
    ZX_DLOG(ERROR, result) << "zx_handle_duplicate";
    return Process();
  }

  return Process(out.release());
}

ProcessId Process::Pid() const {
  DCHECK(IsValid());
  return GetProcId(Handle());
}

Time Process::CreationTime() const {

  NOTIMPLEMENTED();
  return Time();
}

bool Process::is_current() const {
  return is_current_process_;
}

void Process::Close() {
  is_current_process_ = false;
  process_.reset();
}

bool Process::Terminate(int exit_code, bool wait) const {

  zx_status_t status = zx_task_kill(Handle());
  if (status == ZX_OK && wait) {
    zx_signals_t signals;
    status = zx_object_wait_one(Handle(), ZX_TASK_TERMINATED,
                                zx_deadline_after(ZX_SEC(60)), &signals);
    if (status != ZX_OK) {
      ZX_DLOG(ERROR, status) << "zx_object_wait_one(terminate)";
    } else {
      CHECK(signals & ZX_TASK_TERMINATED);
    }
  } else if (status != ZX_OK) {
    ZX_DLOG(ERROR, status) << "zx_task_kill";
  }

  return status >= 0;
}

bool Process::WaitForExit(int* exit_code) const {
  return WaitForExitWithTimeout(TimeDelta::Max(), exit_code);
}

bool Process::WaitForExitWithTimeout(TimeDelta timeout, int* exit_code) const {
  if (is_current_process_)
    return false;

  base::debug::ScopedProcessWaitActivity process_activity(this);

  zx_time_t deadline = timeout == TimeDelta::Max()
                           ? ZX_TIME_INFINITE
                           : (TimeTicks::Now() + timeout).ToZxTime();
  zx_signals_t signals_observed = 0;
  zx_status_t status = zx_object_wait_one(process_.get(), ZX_TASK_TERMINATED,
                                          deadline, &signals_observed);
  if (status != ZX_OK) {
    ZX_DLOG(ERROR, status) << "zx_object_wait_one";
    return false;
  }

  zx_info_process_t proc_info;
  status = zx_object_get_info(process_.get(), ZX_INFO_PROCESS, &proc_info,
                              sizeof(proc_info), nullptr, nullptr);
  if (status != ZX_OK) {
    ZX_DLOG(ERROR, status) << "zx_object_get_info";
    if (exit_code)
      *exit_code = -1;
    return false;
  }

  if (exit_code)
    *exit_code = proc_info.return_code;

  return true;
}

void Process::Exited(int exit_code) const {}

bool Process::IsProcessBackgrounded() const {

  DCHECK(IsValid());
  return false;
}

bool Process::SetProcessBackgrounded(bool value) {


  return false;
}

int Process::GetPriority() const {
  DCHECK(IsValid());

  return 0;
}

}  // namespace base
