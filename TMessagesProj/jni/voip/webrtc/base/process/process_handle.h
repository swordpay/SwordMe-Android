// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROCESS_PROCESS_HANDLE_H_
#define BASE_PROCESS_PROCESS_HANDLE_H_

#include <stdint.h>
#include <sys/types.h>

#include "base/base_export.h"
#include "base/files/file_path.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/windows_types.h"
#endif

#if defined(OS_FUCHSIA)
#include <zircon/types.h>
#endif

namespace base {

// handle to a process.
// ProcessId is a number which identifies the process in the OS.
#if defined(OS_WIN)
typedef HANDLE ProcessHandle;
typedef DWORD ProcessId;
typedef HANDLE UserTokenHandle;
const ProcessHandle kNullProcessHandle = NULL;
const ProcessId kNullProcessId = 0;
#elif defined(OS_FUCHSIA)
typedef zx_handle_t ProcessHandle;
typedef zx_koid_t ProcessId;
const ProcessHandle kNullProcessHandle = ZX_HANDLE_INVALID;
const ProcessId kNullProcessId = ZX_KOID_INVALID;
#elif defined(OS_POSIX)
// On POSIX, our ProcessHandle will just be the PID.
typedef pid_t ProcessHandle;
typedef pid_t ProcessId;
const ProcessHandle kNullProcessHandle = 0;
const ProcessId kNullProcessId = 0;
#endif  // defined(OS_WIN)

// C99 and format_macros.h) like this:
// base::StringPrintf("PID is %" CrPRIdPid ".\n", pid);
#if defined(OS_WIN) || defined(OS_FUCHSIA)
#define CrPRIdPid "ld"
#else
#define CrPRIdPid "d"
#endif

class UniqueProcId {
 public:
  explicit UniqueProcId(ProcessId value) : value_(value) {}
  UniqueProcId(const UniqueProcId& other) = default;
  UniqueProcId& operator=(const UniqueProcId& other) = default;


  ProcessId GetUnsafeValue() const { return value_; }

  bool operator==(const UniqueProcId& other) const {
    return value_ == other.value_;
  }

  bool operator!=(const UniqueProcId& other) const {
    return value_ != other.value_;
  }

  bool operator<(const UniqueProcId& other) const {
    return value_ < other.value_;
  }

  bool operator<=(const UniqueProcId& other) const {
    return value_ <= other.value_;
  }

  bool operator>(const UniqueProcId& other) const {
    return value_ > other.value_;
  }

  bool operator>=(const UniqueProcId& other) const {
    return value_ >= other.value_;
  }

 private:
  ProcessId value_;
};

std::ostream& operator<<(std::ostream& os, const UniqueProcId& obj);

// Note that on some platforms, this is not guaranteed to be unique across
// processes (use GetUniqueIdForProcess if uniqueness is required).
BASE_EXPORT ProcessId GetCurrentProcId();

// currently running processes within the chrome session, but IDs of terminated
// processes may be reused.
BASE_EXPORT UniqueProcId GetUniqueIdForProcess();

#if defined(OS_LINUX)
// When a process is started in a different PID namespace from the browser
// process, this function must be called with the process's PID in the browser's
// PID namespace in order to initialize its unique ID. Not thread safe.
// WARNING: To avoid inconsistent results from GetUniqueIdForProcess, this
// should only be called very early after process startup - ideally as soon
// after process creation as possible.
BASE_EXPORT void InitUniqueIdForProcessInPidNamespace(
    ProcessId pid_outside_of_namespace);
#endif

BASE_EXPORT ProcessHandle GetCurrentProcessHandle();

// same as Windows' GetProcessId(), but works on versions of Windows before Win
// XP SP1 as well.
// DEPRECATED. New code should be using Process::Pid() instead.
// Note that on some platforms, this is not guaranteed to be unique across
// processes.
BASE_EXPORT ProcessId GetProcId(ProcessHandle process);

#if !defined(OS_FUCHSIA)
// Returns the ID for the parent of the given process. Not available on Fuchsia.
// Returning a negative value indicates an error, such as if the |process| does
// not exist. Returns 0 when |process| has no parent process.
BASE_EXPORT ProcessId GetParentProcessId(ProcessHandle process);
#endif  // !defined(OS_FUCHSIA)

#if defined(OS_POSIX)
// Returns the path to the executable of the given process.
BASE_EXPORT FilePath GetProcessExecutablePath(ProcessHandle process);
#endif

}  // namespace base

#endif  // BASE_PROCESS_PROCESS_HANDLE_H_
