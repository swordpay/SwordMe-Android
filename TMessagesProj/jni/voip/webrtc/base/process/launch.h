// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef BASE_PROCESS_LAUNCH_H_
#define BASE_PROCESS_LAUNCH_H_

#include <stddef.h>

#include <string>
#include <utility>
#include <vector>

#include "base/base_export.h"
#include "base/command_line.h"
#include "base/environment.h"
#include "base/macros.h"
#include "base/process/process.h"
#include "base/process/process_handle.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_FUCHSIA)
#include <lib/fdio/spawn.h>
#include <zircon/types.h>
#endif

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include "base/posix/file_descriptor_shuffle.h"
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include "base/mac/mach_port_rendezvous.h"
#endif

namespace base {

#if defined(OS_WIN)
typedef std::vector<HANDLE> HandlesToInheritVector;
#elif defined(OS_FUCHSIA)
struct PathToTransfer {
  base::FilePath path;
  zx_handle_t handle;
};
struct HandleToTransfer {
  uint32_t id;
  zx_handle_t handle;
};
typedef std::vector<HandleToTransfer> HandlesToTransferVector;
typedef std::vector<std::pair<int, int>> FileHandleMappingVector;
#elif defined(OS_POSIX)
typedef std::vector<std::pair<int, int>> FileHandleMappingVector;
#endif  // defined(OS_WIN)

// The default constructor constructs the object with default options.
struct BASE_EXPORT LaunchOptions {
#if (defined(OS_POSIX) || defined(OS_FUCHSIA)) && !defined(OS_MACOSX)


  class BASE_EXPORT PreExecDelegate {
   public:
    PreExecDelegate() = default;
    virtual ~PreExecDelegate() = default;



    virtual void RunAsyncSafe() = 0;

   private:
    DISALLOW_COPY_AND_ASSIGN(PreExecDelegate);
  };
#endif  // defined(OS_POSIX)

  LaunchOptions();
  LaunchOptions(const LaunchOptions&);
  ~LaunchOptions();

  bool wait = false;

  base::FilePath current_directory;

#if defined(OS_WIN)
  bool start_hidden = false;


  bool feedback_cursor_off = false;







  enum class Inherit {



    kSpecific,












    kAll
  };
  Inherit inherit_mode = Inherit::kSpecific;
  HandlesToInheritVector handles_to_inherit;







  UserTokenHandle as_user = nullptr;

  bool empty_desktop_name = false;



  HANDLE job_handle = nullptr;










  HANDLE stdin_handle = nullptr;
  HANDLE stdout_handle = nullptr;
  HANDLE stderr_handle = nullptr;



  bool force_breakaway_from_job_ = false;


  bool grant_foreground_privilege = false;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)


  FileHandleMappingVector fds_to_remap;
#endif  // defined(OS_WIN)

#if defined(OS_WIN) || defined(OS_POSIX) || defined(OS_FUCHSIA)



  EnvironmentMap environment;


  bool clear_environment = false;
#endif  // OS_WIN || OS_POSIX || OS_FUCHSIA

#if defined(OS_LINUX)




  int clone_flags = 0;


  bool allow_new_privs = false;

  bool kill_on_parent_death = false;
#endif  // defined(OS_LINUX)

#if defined(OS_MACOSX) && !defined(OS_IOS)








  MachPortsForRendezvous mach_ports_for_rendezvous;






  bool disclaim_responsibility = false;
#endif

#if defined(OS_FUCHSIA)

  zx_handle_t job_handle = ZX_HANDLE_INVALID;








  HandlesToTransferVector handles_to_transfer;


  static uint32_t AddHandleToTransfer(
      HandlesToTransferVector* handles_to_transfer,
      zx_handle_t handle);





  uint32_t spawn_flags = FDIO_SPAWN_CLONE_NAMESPACE | FDIO_SPAWN_CLONE_STDIO |
                         FDIO_SPAWN_CLONE_JOB;




  std::vector<FilePath> paths_to_clone;



  std::vector<PathToTransfer> paths_to_transfer;


  std::string process_name_suffix;
#endif  // defined(OS_FUCHSIA)

#if defined(OS_POSIX)



  base::FilePath real_path;

#if !defined(OS_MACOSX)






  PreExecDelegate* pre_exec_delegate = nullptr;
#endif  // !defined(OS_MACOSX)



  const std::vector<int>* maximize_rlimits = nullptr;



  bool new_process_group = false;
#endif  // defined(OS_POSIX)

#if defined(OS_CHROMEOS)


  int ctrl_terminal_fd = -1;
#endif  // defined(OS_CHROMEOS)
};

// See the documentation of LaunchOptions for details on |options|.
//
// Returns a valid Process upon success.
//
// Unix-specific notes:
// - All file descriptors open in the parent process will be closed in the
//   child process except for any preserved by options::fds_to_remap, and
//   stdin, stdout, and stderr. If not remapped by options::fds_to_remap,
//   stdin is reopened as /dev/null, and the child is allowed to inherit its
//   parent's stdout and stderr.
// - If the first argument on the command line does not contain a slash,
//   PATH will be searched.  (See man execvp.)
BASE_EXPORT Process LaunchProcess(const CommandLine& cmdline,
                                  const LaunchOptions& options);

#if defined(OS_WIN)
// Windows-specific LaunchProcess that takes the command line as a
// string.  Useful for situations where you need to control the
// command line arguments directly, but prefer the CommandLine version
// if launching Chrome itself.
//
// The first command line argument should be the path to the process,
// and don't forget to quote it.
//
// Example (including literal quotes)
//  cmdline = "c:\windows\explorer.exe" -foo "c:\bar\"
BASE_EXPORT Process LaunchProcess(const CommandLine::StringType& cmdline,
                                  const LaunchOptions& options);

// like LaunchProcess as it uses ShellExecuteEx instead of CreateProcess to
// create the process.  This means the process will have elevated privileges
// and thus some common operations like OpenProcess will fail. Currently the
// only supported LaunchOptions are |start_hidden| and |wait|.
BASE_EXPORT Process LaunchElevatedProcess(const CommandLine& cmdline,
                                          const LaunchOptions& options);

#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
// A POSIX-specific version of LaunchProcess that takes an argv array
// instead of a CommandLine.  Useful for situations where you need to
// control the command line arguments directly, but prefer the
// CommandLine version if launching Chrome itself.
BASE_EXPORT Process LaunchProcess(const std::vector<std::string>& argv,
                                  const LaunchOptions& options);

#if !defined(OS_MACOSX)
// Close all file descriptors, except those which are a destination in the
// given multimap. Only call this function in a child process where you know
// that there aren't any other threads.
BASE_EXPORT void CloseSuperfluousFds(const InjectiveMultimap& saved_map);
#endif  // defined(OS_MACOSX)
#endif  // defined(OS_WIN)

#if defined(OS_WIN)
// Set |job_object|'s JOBOBJECT_EXTENDED_LIMIT_INFORMATION
// BasicLimitInformation.LimitFlags to |limit_flags|.
BASE_EXPORT bool SetJobObjectLimitFlags(HANDLE job_object, DWORD limit_flags);

// chrome. This is not thread-safe: only call from main thread.
BASE_EXPORT void RouteStdioToConsole(bool create_console_if_not_found);
#endif  // defined(OS_WIN)

// the output (stdout) in |output|. Redirects stderr to /dev/null. Returns true
// on success (application launched and exited cleanly, with exit code
// indicating success).
BASE_EXPORT bool GetAppOutput(const CommandLine& cl, std::string* output);

BASE_EXPORT bool GetAppOutputAndError(const CommandLine& cl,
                                      std::string* output);

// executed command. Returns true if the application runs and exits cleanly. If
// this is the case the exit code of the application is available in
// |*exit_code|.
BASE_EXPORT bool GetAppOutputWithExitCode(const CommandLine& cl,
                                          std::string* output, int* exit_code);

#if defined(OS_WIN)
// A Windows-specific version of GetAppOutput that takes a command line string
// instead of a CommandLine object. Useful for situations where you need to
// control the command line arguments directly.
BASE_EXPORT bool GetAppOutput(CommandLine::StringPieceType cl,
                              std::string* output);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
// A POSIX-specific version of GetAppOutput that takes an argv array
// instead of a CommandLine.  Useful for situations where you need to
// control the command line arguments directly.
BASE_EXPORT bool GetAppOutput(const std::vector<std::string>& argv,
                              std::string* output);

// stderr.
BASE_EXPORT bool GetAppOutputAndError(const std::vector<std::string>& argv,
                                      std::string* output);
#endif  // defined(OS_WIN)

// the current process's scheduling priority to a high priority.
BASE_EXPORT void RaiseProcessToHighPriority();

// binary. This should not be called in production/released code.
BASE_EXPORT LaunchOptions LaunchOptionsForTest();

#if defined(OS_LINUX) || defined(OS_NACL_NONSFI)
// A wrapper for clone with fork-like behavior, meaning that it returns the
// child's pid in the parent and 0 in the child. |flags|, |ptid|, and |ctid| are
// as in the clone system call (the CLONE_VM flag is not supported).
//
// This function uses the libc clone wrapper (which updates libc's pid cache)
// internally, so callers may expect things like getpid() to work correctly
// after in both the child and parent.
//
// As with fork(), callers should be extremely careful when calling this while
// multiple threads are running, since at the time the fork happened, the
// threads could have been in any state (potentially holding locks, etc.).
// Callers should most likely call execve() in the child soon after calling
// this.
//
// It is unsafe to use any pthread APIs after ForkWithFlags().
// However, performing an exec() will lift this restriction.
BASE_EXPORT pid_t ForkWithFlags(unsigned long flags, pid_t* ptid, pid_t* ctid);
#endif

}  // namespace base

#endif  // BASE_PROCESS_LAUNCH_H_
