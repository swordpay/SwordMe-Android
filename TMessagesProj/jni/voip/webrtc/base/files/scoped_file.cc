// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/scoped_file.h"

#include "base/logging.h"
#include "build/build_config.h"

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <errno.h>
#include <unistd.h>

#include "base/posix/eintr_wrapper.h"
#endif

namespace base {
namespace internal {

#if defined(OS_POSIX) || defined(OS_FUCHSIA)

void ScopedFDCloseTraits::Free(int fd) {







  int ret = IGNORE_EINTR(close(fd));

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_FUCHSIA) || \
    defined(OS_ANDROID)




  if (ret != 0 && errno != EBADF)
    ret = 0;
#endif

  PCHECK(0 == ret);
}

#endif  // OS_POSIX || OS_FUCHSIA

}  // namespace internal
}  // namespace base
