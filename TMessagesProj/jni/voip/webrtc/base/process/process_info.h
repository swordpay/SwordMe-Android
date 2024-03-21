// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROCESS_PROCESS_INFO_H_
#define BASE_PROCESS_PROCESS_INFO_H_

#include "base/base_export.h"
#include "build/build_config.h"

namespace base {

#if defined(OS_WIN)
enum IntegrityLevel {
  INTEGRITY_UNKNOWN,
  UNTRUSTED_INTEGRITY,
  LOW_INTEGRITY,
  MEDIUM_INTEGRITY,
  HIGH_INTEGRITY,
};

// case of an underlying system failure.
BASE_EXPORT IntegrityLevel GetCurrentProcessIntegrityLevel();

BASE_EXPORT bool IsCurrentProcessElevated();

#endif  // defined(OS_WIN)

}  // namespace base

#endif  // BASE_PROCESS_PROCESS_INFO_H_
