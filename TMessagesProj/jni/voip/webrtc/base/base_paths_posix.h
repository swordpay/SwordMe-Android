// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_BASE_PATHS_POSIX_H_
#define BASE_BASE_PATHS_POSIX_H_

// These can be used with the PathService to access various special
// directories and files.

namespace base {

enum {
  PATH_POSIX_START = 400,

  DIR_CACHE,    // Directory where to put cache data.  Note this is




  PATH_POSIX_END
};

}  // namespace base

#endif  // BASE_BASE_PATHS_POSIX_H_
