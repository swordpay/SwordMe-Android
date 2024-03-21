// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROCESS_PROCESS_H_
#define BASE_PROCESS_PROCESS_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/process/process_handle.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/scoped_handle.h"
#endif

#if defined(OS_FUCHSIA)
#include <lib/zx/process.h>
#endif

#if defined(OS_MACOSX)
#include "base/feature_list.h"
#include "base/process/port_provider_mac.h"
#endif

namespace base {

#if defined(OS_MACOSX)
extern const Feature kMacAllowBackgroundingProcesses;
#endif

//
// This object is not tied to the lifetime of the underlying process: the
// process may be killed and this object may still around, and it will still
// claim to be valid. The actual behavior in that case is OS dependent like so:
//
// Windows: The underlying ProcessHandle will be valid after the process dies
// and can be used to gather some information about that process, but most
// methods will obviously fail.
//
// POSIX: The underlying ProcessHandle is not guaranteed to remain valid after
// the process dies, and it may be reused by the system, which means that it may
// end up pointing to the wrong process.
class BASE_EXPORT Process {
 public:


  explicit Process(ProcessHandle handle = kNullProcessHandle);

  Process(Process&& other);

  ~Process();

  Process& operator=(Process&& other);

  static Process Current();

  static Process Open(ProcessId pid);



  static Process OpenWithExtraPrivileges(ProcessId pid);

#if defined(OS_WIN)


  static Process OpenWithAccess(ProcessId pid, DWORD desired_access);
#endif




  static Process DeprecatedGetProcessFromHandle(ProcessHandle handle);

  static bool CanBackgroundProcesses();

  [[noreturn]] static void TerminateCurrentProcessImmediately(int exit_code);

  bool IsValid() const;


  ProcessHandle Handle() const;

  Process Duplicate() const;

  ProcessId Pid() const;

#if !defined(OS_ANDROID)






  Time CreationTime() const;
#endif  // !defined(OS_ANDROID)

  bool is_current() const;

  void Close();




#if defined(OS_WIN)
  bool IsRunning() const {
    return !WaitForExitWithTimeout(base::TimeDelta(), nullptr);
  }
#endif





  bool Terminate(int exit_code, bool wait) const;

#if defined(OS_WIN)
  enum class WaitExitStatus {
    PROCESS_EXITED,
    STOP_EVENT_SIGNALED,
    FAILED,
  };



  WaitExitStatus WaitForExitOrEvent(
      const base::win::ScopedHandle& stop_event_handle,
      int* exit_code) const;
#endif  // OS_WIN






  bool WaitForExit(int* exit_code) const;



  bool WaitForExitWithTimeout(TimeDelta timeout, int* exit_code) const;





  void Exited(int exit_code) const;

#if defined(OS_MACOSX)










  bool IsProcessBackgrounded(PortProvider* port_provider) const;







  bool SetProcessBackgrounded(PortProvider* port_provider, bool value);
#else


  bool IsProcessBackgrounded() const;




  bool SetProcessBackgrounded(bool value);
#endif  // defined(OS_MACOSX)


  int GetPriority() const;

#if defined(OS_CHROMEOS)



  ProcessId GetPidInNamespace() const;
#endif

 private:
#if defined(OS_WIN)
  win::ScopedHandle process_;
#elif defined(OS_FUCHSIA)
  zx::process process_;
#else
  ProcessHandle process_;
#endif

#if defined(OS_WIN) || defined(OS_FUCHSIA)
  bool is_current_process_;
#endif

  DISALLOW_COPY_AND_ASSIGN(Process);
};

#if defined(OS_CHROMEOS)
// Exposed for testing.
// Given the contents of the /proc/<pid>/cgroup file, determine whether the
// process is backgrounded or not.
BASE_EXPORT bool IsProcessBackgroundedCGroup(
    const StringPiece& cgroup_contents);
#endif  // defined(OS_CHROMEOS)

}  // namespace base

#endif  // BASE_PROCESS_PROCESS_H_
