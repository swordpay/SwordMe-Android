// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process_iterator.h"

#include <errno.h>
#include <stddef.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/logging.h"
#include "base/stl_util.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"

namespace base {

ProcessIterator::ProcessIterator(const ProcessFilter* filter)
    : index_of_kinfo_proc_(0),
      filter_(filter) {




  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_UID,
                static_cast<int>(geteuid()) };


  bool done = false;
  int try_num = 1;
  const int max_tries = 10;
  do {

    size_t len = 0;
    if (sysctl(mib, base::size(mib), NULL, &len, NULL, 0) < 0) {
      DLOG(ERROR) << "failed to get the size needed for the process list";
      kinfo_procs_.resize(0);
      done = true;
    } else {
      size_t num_of_kinfo_proc = len / sizeof(struct kinfo_proc);


      num_of_kinfo_proc += 16;
      kinfo_procs_.resize(num_of_kinfo_proc);
      len = num_of_kinfo_proc * sizeof(struct kinfo_proc);

      if (sysctl(mib, base::size(mib), &kinfo_procs_[0], &len, NULL, 0) < 0) {


        if (errno != ENOMEM) {
          DLOG(ERROR) << "failed to get the process list";
          kinfo_procs_.resize(0);
          done = true;
        }
      } else {

        kinfo_procs_.resize(len / sizeof(struct kinfo_proc));
        done = true;
      }
    }
  } while (!done && (try_num++ < max_tries));

  if (!done) {
    DLOG(ERROR) << "failed to collect the process list in a few tries";
    kinfo_procs_.resize(0);
  }
}

ProcessIterator::~ProcessIterator() {
}

bool ProcessIterator::CheckForNextProcess() {
  std::string data;
  for (; index_of_kinfo_proc_ < kinfo_procs_.size(); ++index_of_kinfo_proc_) {
    kinfo_proc& kinfo = kinfo_procs_[index_of_kinfo_proc_];

    if ((kinfo.kp_proc.p_pid > 0) && (kinfo.kp_proc.p_stat == SZOMB))
      continue;

    int mib[] = { CTL_KERN, KERN_PROCARGS, kinfo.kp_proc.p_pid };

    size_t data_len = 0;
    if (sysctl(mib, base::size(mib), NULL, &data_len, NULL, 0) < 0) {
      DVPLOG(1) << "failed to figure out the buffer size for a commandline";
      continue;
    }

    data.resize(data_len);
    if (sysctl(mib, base::size(mib), &data[0], &data_len, NULL, 0) < 0) {
      DVPLOG(1) << "failed to fetch a commandline";
      continue;
    }




    std::string delimiters;
    delimiters.push_back('\0');
    entry_.cmd_line_args_ = SplitString(data, delimiters,
                                        KEEP_WHITESPACE, SPLIT_WANT_NONEMPTY);



    size_t exec_name_end = data.find('\0');
    if (exec_name_end == std::string::npos) {
      DLOG(ERROR) << "command line data didn't match expected format";
      continue;
    }

    entry_.pid_ = kinfo.kp_proc.p_pid;
    entry_.ppid_ = kinfo.kp_eproc.e_ppid;
    entry_.gid_ = kinfo.kp_eproc.e_pgid;
    size_t last_slash = data.rfind('/', exec_name_end);
    if (last_slash == std::string::npos)
      entry_.exe_file_.assign(data, 0, exec_name_end);
    else
      entry_.exe_file_.assign(data, last_slash + 1,
                              exec_name_end - last_slash - 1);

    ++index_of_kinfo_proc_;

    return true;
  }
  return false;
}

bool NamedProcessIterator::IncludeEntry() {
  return (executable_name_ == entry().exe_file() &&
          ProcessIterator::IncludeEntry());
}

}  // namespace base
