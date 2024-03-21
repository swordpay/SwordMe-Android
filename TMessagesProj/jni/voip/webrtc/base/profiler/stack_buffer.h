// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_STACK_BUFFER_H_
#define BASE_PROFILER_STACK_BUFFER_H_

#include <stddef.h>
#include <stdint.h>
#include <memory>

#include "base/base_export.h"
#include "base/macros.h"

namespace base {

// multiple instances of StackSampler.
class BASE_EXPORT StackBuffer {
 public:






  static constexpr size_t kPlatformStackAlignment = 2 * sizeof(uintptr_t);

  StackBuffer(size_t buffer_size);
  ~StackBuffer();

  uintptr_t* buffer() const {



    return reinterpret_cast<uintptr_t*>(
        (reinterpret_cast<uintptr_t>(buffer_.get()) + kPlatformStackAlignment -
         1) &
        ~(kPlatformStackAlignment - 1));
  }

  size_t size() const { return size_; }

 private:

  const std::unique_ptr<uint8_t[]> buffer_;


  const size_t size_;

  DISALLOW_COPY_AND_ASSIGN(StackBuffer);
};

}  // namespace base

#endif  // BASE_PROFILER_STACK_BUFFER_H_
