// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_OS_COMPAT_ANDROID_H_
#define BASE_OS_COMPAT_ANDROID_H_

#include <fcntl.h>
#include <sys/types.h>
#include <utime.h>

extern "C" int futimes(int fd, const struct timeval tv[2]);

extern "C" char* mkdtemp(char* path);

extern "C" time_t timegm(struct tm* const t);

#endif  // BASE_OS_COMPAT_ANDROID_H_
