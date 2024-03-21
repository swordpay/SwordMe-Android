// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_STACK_COPIER_H_
#define BASE_PROFILER_STACK_COPIER_H_

#include <stdint.h>

#include "base/base_export.h"
#include "base/profiler/register_context.h"
#include "base/time/time.h"

namespace base {

class StackBuffer;

// the thread's execution. It's intended to provide an abstraction over stack
// copying techniques where the thread suspension is performed directly by the
// profiler thread (Windows and Mac platforms) vs. where the thread suspension
// is performed by the OS through signals (Android).
class BASE_EXPORT StackCopier {
 public:


  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() {}






    virtual void OnStackCopy() = 0;

    virtual void OnThreadResume() = 0;
  };

  virtual ~StackCopier();




  virtual bool CopyStack(StackBuffer* stack_buffer,
                         uintptr_t* stack_top,
                         TimeTicks* timestamp,
                         RegisterContext* thread_context,
                         Delegate* delegate) = 0;

 protected:




  static uintptr_t RewritePointerIfInOriginalStack(
      const uint8_t* original_stack_bottom,
      const uintptr_t* original_stack_top,
      const uint8_t* stack_copy_bottom,
      uintptr_t pointer);
























  static const uint8_t* CopyStackContentsAndRewritePointers(
      const uint8_t* original_stack_bottom,
      const uintptr_t* original_stack_top,
      int platform_stack_alignment,
      uintptr_t* stack_buffer_bottom);
};

}  // namespace base

#endif  // BASE_PROFILER_STACK_COPIER_H_
