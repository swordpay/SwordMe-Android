// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// base/process/.

#ifndef BASE_PROCESS_INTERNAL_LINUX_H_
#define BASE_PROCESS_INTERNAL_LINUX_H_

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "base/files/file_path.h"

namespace base {

class Time;
class TimeDelta;

namespace internal {

extern const char kProcDir[];

extern const char kStatFile[];

base::FilePath GetProcPidDir(pid_t pid);

// a process, convert it to a pid_t.
// Returns 0 on failure.
// e.g. /proc/self/ will return 0, whereas /proc/1234 will return 1234.
pid_t ProcDirSlotToPid(const char* d_name);

// and is non-empty.
bool ReadProcStats(pid_t pid, std::string* buffer);

// spaces. Taking into account the 2nd field may, in itself, contain spaces.
// Returns true if successful.
bool ParseProcStats(const std::string& stats_data,
                    std::vector<std::string>* proc_stats);

// If the ordering ever changes, carefully review functions that use these
// values.
enum ProcStatsFields {
  VM_COMM = 1,         // Filename of executable, without parentheses.
  VM_STATE = 2,        // Letter indicating the state of the process.
  VM_PPID = 3,         // PID of the parent.
  VM_PGRP = 4,         // Process group id.
  VM_MINFLT = 9,       // Minor page fault count excluding children.
  VM_MAJFLT = 11,      // Major page fault count excluding children.
  VM_UTIME = 13,       // Time scheduled in user mode in clock ticks.
  VM_STIME = 14,       // Time scheduled in kernel mode in clock ticks.
  VM_NUMTHREADS = 19,  // Number of threads.
  VM_STARTTIME = 21,   // The time the process started in clock ticks.
  VM_VSIZE = 22,       // Virtual memory size in bytes.
  VM_RSS = 23,         // Resident Set Size in pages.
};

// This version does not handle the first 3 values, since the first value is
// simply |pid|, and the next two values are strings.
int64_t GetProcStatsFieldAsInt64(const std::vector<std::string>& proc_stats,
                                 ProcStatsFields field_num);

size_t GetProcStatsFieldAsSizeT(const std::vector<std::string>& proc_stats,
                                ProcStatsFields field_num);

// ReadProcStats(). See GetProcStatsFieldAsInt64() for details.
int64_t ReadStatsFilendGetFieldAsInt64(const FilePath& stat_file,
                                       ProcStatsFields field_num);
int64_t ReadProcStatsAndGetFieldAsInt64(pid_t pid, ProcStatsFields field_num);
int64_t ReadProcSelfStatsAndGetFieldAsInt64(ProcStatsFields field_num);

size_t ReadProcStatsAndGetFieldAsSizeT(pid_t pid,
                                       ProcStatsFields field_num);

Time GetBootTime();

TimeDelta GetUserCpuTimeSinceBoot();

TimeDelta ClockTicksToTimeDelta(int clock_ticks);

}  // namespace internal
}  // namespace base

#endif  // BASE_PROCESS_INTERNAL_LINUX_H_
