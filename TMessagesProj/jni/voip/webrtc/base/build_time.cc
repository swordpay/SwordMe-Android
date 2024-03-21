// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/build_time.h"

#include "base/generated_build_date.h"

#include "base/logging.h"
#include "base/time/time.h"

namespace base {

Time GetBuildTime() {
  Time integral_build_time;



  bool result = Time::FromUTCString(BUILD_DATE, &integral_build_time);
  DCHECK(result);
  return integral_build_time;
}

}  // namespace base
