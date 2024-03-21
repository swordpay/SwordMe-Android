// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_FRAME_H_
#define BASE_PROFILER_FRAME_H_

#include <memory>

#include "base/profiler/module_cache.h"

namespace base {

// information.
struct BASE_EXPORT Frame {
  Frame(uintptr_t instruction_pointer, const ModuleCache::Module* module);
  ~Frame();

  uintptr_t instruction_pointer;

  const ModuleCache::Module* module;
};

}  // namespace base

#endif  // BASE_PROFILER_FRAME_H_
