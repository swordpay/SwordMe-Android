// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/posix/can_lower_nice_to.h"

#include <limits.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#include "build/build_config.h"

#if defined(OS_AIX)
#if defined(RLIMIT_NICE)
#error Assumption about OS_AIX is incorrect
#endif
#define RLIMIT_NICE 20
#endif

namespace base {
namespace internal {

bool CanLowerNiceTo(int nice_value) {




  if (geteuid() == 0)
    return true;














  struct rlimit rlim;
  if (getrlimit(RLIMIT_NICE, &rlim) != 0)
    return false;
  const int lowest_nice_allowed = NZERO - static_cast<int>(rlim.rlim_cur);


  return nice_value >= lowest_nice_allowed;
}

}  // namespace internal
}  // namespace base
