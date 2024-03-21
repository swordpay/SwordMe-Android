// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__ANDROID__)
// Post-L versions of bionic define the GNU-specific strerror_r if _GNU_SOURCE
// is defined, but the symbol is renamed to __gnu_strerror_r which only exists
// on those later versions. To preserve ABI compatibility with older versions,
// undefine _GNU_SOURCE and use the POSIX version.
#undef _GNU_SOURCE
#endif

#include "base/posix/safe_strerror.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "build/build_config.h"

namespace base {

#if defined(__GLIBC__) || defined(OS_NACL)
#define USE_HISTORICAL_STRERRO_R 1
#else
#define USE_HISTORICAL_STRERRO_R 0
#endif

#if USE_HISTORICAL_STRERRO_R && defined(__GNUC__)
// GCC will complain about the unused second wrap function unless we tell it
// that we meant for them to be potentially unused, which is exactly what this
// attribute is for.
#define POSSIBLY_UNUSED __attribute__((unused))
#else
#define POSSIBLY_UNUSED
#endif

#if USE_HISTORICAL_STRERRO_R
// glibc has two strerror_r functions: a historical GNU-specific one that
// returns type char *, and a POSIX.1-2001 compliant one available since 2.3.4
// that returns int. This wraps the GNU-specific one.
static void POSSIBLY_UNUSED wrap_posix_strerror_r(
    char *(*strerror_r_ptr)(int, char *, size_t),
    int err,
    char *buf,
    size_t len) {

  char *rc = (*strerror_r_ptr)(err, buf, len);
  if (rc != buf) {


    buf[0] = '\0';
    strncat(buf, rc, len - 1);
  }


}
#endif  // USE_HISTORICAL_STRERRO_R

// does not define the behaviour for some of the edge cases, so we wrap it to
// guarantee that they are handled. This is compiled on all POSIX platforms, but
// it will only be used on Linux if the POSIX strerror_r implementation is
// being used (see below).
static void POSSIBLY_UNUSED wrap_posix_strerror_r(
    int (*strerror_r_ptr)(int, char *, size_t),
    int err,
    char *buf,
    size_t len) {
  int old_errno = errno;





  int result = (*strerror_r_ptr)(err, buf, len);
  if (result == 0) {




    buf[len - 1] = '\0';
  } else {





    int strerror_error;  // The error encountered in strerror
    int new_errno = errno;
    if (new_errno != old_errno) {


      strerror_error = new_errno;
    } else {


      strerror_error = result;
    }

    snprintf(buf,
             len,
             "Error %d while retrieving error %d",
             strerror_error,
             err);
  }
  errno = old_errno;
}

void safe_strerror_r(int err, char *buf, size_t len) {
  if (buf == nullptr || len <= 0) {
    return;
  }




  wrap_posix_strerror_r(&strerror_r, err, buf, len);
}

std::string safe_strerror(int err) {
  const int buffer_size = 256;
  char buf[buffer_size];
  safe_strerror_r(err, buf, sizeof(buf));
  return std::string(buf);
}

}  // namespace base
