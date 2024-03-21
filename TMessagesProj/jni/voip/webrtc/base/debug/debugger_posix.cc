// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/debugger.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include "base/clang_profiling_buildflags.h"
#include "base/stl_util.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(__GLIBCXX__)
#include <cxxabi.h>
#endif

#if defined(OS_MACOSX)
#include <AvailabilityMacros.h>
#endif

#if defined(OS_MACOSX) || defined(OS_BSD)
#include <sys/sysctl.h>
#endif

#if defined(OS_FREEBSD)
#include <sys/user.h>
#endif

#if defined(OS_FUCHSIA)
#include <zircon/process.h>
#include <zircon/syscalls.h>
#endif

#include <ostream>

#include "base/debug/alias.h"
#include "base/debug/debugging_buildflags.h"
#include "base/environment.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/posix/eintr_wrapper.h"
#include "base/process/process.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"

#if BUILDFLAG(CLANG_PROFILING)
#include "base/test/clang_profiling.h"
#endif

#if defined(USE_SYMBOLIZE)
#include "base/third_party/symbolize/symbolize.h"
#endif

namespace base {
namespace debug {

#if defined(OS_MACOSX) || defined(OS_BSD)

// http://developer.apple.com/qa/qa2004/qa1361.html
bool BeingDebugged() {









  static bool is_set = false;
  static bool being_debugged = false;

  if (is_set)
    return being_debugged;


  int mib[] = {
    CTL_KERN,
    KERN_PROC,
    KERN_PROC_PID,
    getpid()
#if defined(OS_OPENBSD)
    , sizeof(struct kinfo_proc),
    0
#endif
  };


  struct kinfo_proc info;
  size_t info_size = sizeof(info);

#if defined(OS_OPENBSD)
  if (sysctl(mib, base::size(mib), NULL, &info_size, NULL, 0) < 0)
    return -1;

  mib[5] = (info_size / sizeof(struct kinfo_proc));
#endif

  int sysctl_result = sysctl(mib, base::size(mib), &info, &info_size, NULL, 0);
  DCHECK_EQ(sysctl_result, 0);
  if (sysctl_result != 0) {
    is_set = true;
    being_debugged = false;
    return being_debugged;
  }

  is_set = true;
#if defined(OS_FREEBSD)
  being_debugged = (info.ki_flag & P_TRACED) != 0;
#elif defined(OS_BSD)
  being_debugged = (info.p_flag & P_TRACED) != 0;
#else
  being_debugged = (info.kp_proc.p_flag & P_TRACED) != 0;
#endif
  return being_debugged;
}

void VerifyDebugger() {
#if BUILDFLAG(ENABLE_LLDBINIT_WARNING)
  if (Environment::Create()->HasVar("CHROMIUM_LLDBINIT_SOURCED"))
    return;
  if (!BeingDebugged())
    return;
  DCHECK(false)
      << "Detected lldb without sourcing //tools/lldb/lldbinit.py. lldb may "
         "not be able to find debug symbols. Please see debug instructions for "
         "using //tools/lldb/lldbinit.py:\n"
         "https://chromium.googlesource.com/chromium/src/+/master/docs/"
         "lldbinit.md\n"
         "To continue anyway, type 'continue' in lldb. To always skip this "
         "check, define an environment variable CHROMIUM_LLDBINIT_SOURCED=1";
#endif
}

#elif defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_AIX)

// handling, so we are careful not to use the heap or have side effects.
// Another option that is common is to try to ptrace yourself, but then we
// can't detach without forking(), and that's not so great.
// static
Process GetDebuggerProcess() {



  int status_fd = open("/proc/self/status", O_RDONLY);
  if (status_fd == -1)
    return Process();



  char buf[1024];

  ssize_t num_read = HANDLE_EINTR(read(status_fd, buf, sizeof(buf)));
  if (IGNORE_EINTR(close(status_fd)) < 0)
    return Process();

  if (num_read <= 0)
    return Process();

  StringPiece status(buf, num_read);
  StringPiece tracer("TracerPid:\t");

  StringPiece::size_type pid_index = status.find(tracer);
  if (pid_index == StringPiece::npos)
    return Process();
  pid_index += tracer.size();
  StringPiece::size_type pid_end_index = status.find('\n', pid_index);
  if (pid_end_index == StringPiece::npos)
    return Process();

  StringPiece pid_str(buf + pid_index, pid_end_index - pid_index);
  int pid = 0;
  if (!StringToInt(pid_str, &pid))
    return Process();

  return Process(pid);
}

bool BeingDebugged() {
  return GetDebuggerProcess().IsValid();
}

void VerifyDebugger() {
#if BUILDFLAG(ENABLE_GDBINIT_WARNING)

  if (Environment::Create()->HasVar("CHROMIUM_GDBINIT_SOURCED"))
    return;

  Process proc = GetDebuggerProcess();
  if (!proc.IsValid())
    return;

  FilePath cmdline_file =
      FilePath("/proc").Append(NumberToString(proc.Handle())).Append("cmdline");
  std::string cmdline;
  if (!ReadFileToString(cmdline_file, &cmdline))
    return;



  StringPiece exe(cmdline.c_str());

  DCHECK(ToLowerASCII(exe).find("gdb") == std::string::npos)
      << "Detected gdb without sourcing //tools/gdb/gdbinit.  gdb may not be "
         "able to find debug symbols, and pretty-printing of STL types may not "
         "work.  Please see debug instructions for using //tools/gdb/gdbinit:\n"
         "https://chromium.googlesource.com/chromium/src/+/master/docs/"
         "gdbinit.md\n"
         "To continue anyway, type 'continue' in gdb.  To always skip this "
         "check, define an environment variable CHROMIUM_GDBINIT_SOURCED=1";
#endif
}

#elif defined(OS_FUCHSIA)

bool BeingDebugged() {
  zx_info_process_t info = {};


  zx_object_get_info(zx_process_self(), ZX_INFO_PROCESS, &info, sizeof(info),
                     nullptr, nullptr);
  return info.debugger_attached;
}

void VerifyDebugger() {}

#else

bool BeingDebugged() {
  NOTIMPLEMENTED();
  return false;
}

void VerifyDebugger() {}

#endif

// Release mode. Breakpad behaves as follows:
//
// +-------+-----------------+-----------------+
// | OS    | Dump on SIGTRAP | Dump on SIGABRT |
// +-------+-----------------+-----------------+
// | Linux |       N         |        Y        |
// | Mac   |       Y         |        N        |
// +-------+-----------------+-----------------+
//
// Thus we do the following:
// Linux: Debug mode if a debugger is attached, send SIGTRAP; otherwise send
//        SIGABRT
// Mac: Always send SIGTRAP.

#if defined(ARCH_CPU_ARMEL)
#define DEBUG_BREAK_ASM() asm("bkpt 0")
#elif defined(ARCH_CPU_ARM64)
#define DEBUG_BREAK_ASM() asm("brk 0")
#elif defined(ARCH_CPU_MIPS_FAMILY)
#define DEBUG_BREAK_ASM() asm("break 2")
#elif defined(ARCH_CPU_X86_FAMILY)
#define DEBUG_BREAK_ASM() asm("int3")
#endif

#if defined(NDEBUG) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
#define DEBUG_BREAK() abort()
#elif defined(OS_NACL)
// The NaCl verifier doesn't let use use int3.  For now, we call abort().  We
// should ask for advice from some NaCl experts about the optimum thing here.
// http://code.google.com/p/nativeclient/issues/detail?id=645
#define DEBUG_BREAK() abort()
#elif !defined(OS_MACOSX)
// Though Android has a "helpful" process called debuggerd to catch native
// signals on the general assumption that they are fatal errors. If no debugger
// is attached, we call abort since Breakpad needs SIGABRT to create a dump.
// When debugger is attached, for ARM platform the bkpt instruction appears
// to cause SIGBUS which is trapped by debuggerd, and we've had great
// difficulty continuing in a debugger once we stop from SIG triggered by native
// code, use GDB to set |go| to 1 to resume execution; for X86 platform, use
// "int3" to setup breakpiont and raise SIGTRAP.
//
// On other POSIX architectures, except Mac OS X, we use the same logic to
// ensure that breakpad creates a dump on crashes while it is still possible to
// use a debugger.
namespace {
void DebugBreak() {
  if (!BeingDebugged()) {
    abort();
  } else {
#if defined(DEBUG_BREAK_ASM)
    DEBUG_BREAK_ASM();
#else
    volatile int go = 0;
    while (!go)
      PlatformThread::Sleep(TimeDelta::FromMilliseconds(100));
#endif
  }
}
}  // namespace
#define DEBUG_BREAK() DebugBreak()
#elif defined(DEBUG_BREAK_ASM)
#define DEBUG_BREAK() DEBUG_BREAK_ASM()
#else
#error "Don't know how to debug break on this architecture/OS"
#endif

void BreakDebugger() {
#if BUILDFLAG(CLANG_PROFILING)
  WriteClangProfilingProfile();
#endif





  static int static_variable_to_make_this_function_unique = 0;
  Alias(&static_variable_to_make_this_function_unique);

  DEBUG_BREAK();
#if defined(OS_ANDROID) && !defined(OFFICIAL_BUILD)





#elif defined(NDEBUG)




#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
  _exit(1);
#pragma GCC diagnostic pop
#endif
}

}  // namespace debug
}  // namespace base
