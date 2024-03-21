// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef BASE_PROCESS_PROCESS_ITERATOR_H_
#define BASE_PROCESS_PROCESS_ITERATOR_H_

#include <stddef.h>

#include <list>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/process/process.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#include <tlhelp32.h>
#elif defined(OS_MACOSX) || defined(OS_OPENBSD)
#include <sys/sysctl.h>
#elif defined(OS_FREEBSD)
#include <sys/user.h>
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <dirent.h>
#endif

namespace base {

#if defined(OS_WIN)
struct ProcessEntry : public PROCESSENTRY32 {
  ProcessId pid() const { return th32ProcessID; }
  ProcessId parent_pid() const { return th32ParentProcessID; }
  const wchar_t* exe_file() const { return szExeFile; }
};
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
struct BASE_EXPORT ProcessEntry {
  ProcessEntry();
  ProcessEntry(const ProcessEntry& other);
  ~ProcessEntry();

  ProcessId pid() const { return pid_; }
  ProcessId parent_pid() const { return ppid_; }
  ProcessId gid() const { return gid_; }
  const char* exe_file() const { return exe_file_.c_str(); }
  const std::vector<std::string>& cmd_line_args() const {
    return cmd_line_args_;
  }

  ProcessId pid_;
  ProcessId ppid_;
  ProcessId gid_;
  std::string exe_file_;
  std::vector<std::string> cmd_line_args_;
};
#endif  // defined(OS_WIN)

class ProcessFilter {
 public:


  virtual bool Includes(const ProcessEntry& entry) const = 0;

 protected:
  virtual ~ProcessFilter() = default;
};

// current machine with a specified filter.
// To use, create an instance and then call NextProcessEntry() until it returns
// false.
class BASE_EXPORT ProcessIterator {
 public:
  typedef std::list<ProcessEntry> ProcessEntries;

  explicit ProcessIterator(const ProcessFilter* filter);
  virtual ~ProcessIterator();





  const ProcessEntry* NextProcessEntry();

  ProcessEntries Snapshot();

 protected:
  virtual bool IncludeEntry();
  const ProcessEntry& entry() { return entry_; }

 private:



  bool CheckForNextProcess();


  void InitProcessEntry(ProcessEntry* entry);

#if defined(OS_WIN)
  HANDLE snapshot_;
  bool started_iteration_;
#elif defined(OS_MACOSX) || defined(OS_BSD)
  std::vector<kinfo_proc> kinfo_procs_;
  size_t index_of_kinfo_proc_;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  DIR* procfs_dir_;
#endif
  ProcessEntry entry_;
  const ProcessFilter* filter_;

  DISALLOW_COPY_AND_ASSIGN(ProcessIterator);
};

// on the current machine that were started from the given executable
// name.  To use, create an instance and then call NextProcessEntry()
// until it returns false.
class BASE_EXPORT NamedProcessIterator : public ProcessIterator {
 public:
  NamedProcessIterator(const FilePath::StringType& executable_name,
                       const ProcessFilter* filter);
  ~NamedProcessIterator() override;

 protected:
  bool IncludeEntry() override;

 private:
  FilePath::StringType executable_name_;

  DISALLOW_COPY_AND_ASSIGN(NamedProcessIterator);
};

// given executable name.  If filter is non-null, then only processes selected
// by the filter will be counted.
BASE_EXPORT int GetProcessCount(const FilePath::StringType& executable_name,
                                const ProcessFilter* filter);

}  // namespace base

#endif  // BASE_PROCESS_PROCESS_ITERATOR_H_
