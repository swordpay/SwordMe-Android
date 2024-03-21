// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This is inspired by Craig Silverstein's PowerPC stacktrace code.

#ifndef ABSL_DEBUGGING_INTERNAL_STACKTRACE_ARM_INL_H_
#define ABSL_DEBUGGING_INTERNAL_STACKTRACE_ARM_INL_H_

#include <cstdint>

#include "absl/debugging/stacktrace.h"

// This only works if all your code is in either ARM or THUMB mode.  With
// interworking, the frame pointer of the caller can either be in r11 (ARM
// mode) or r7 (THUMB mode).  A callee only saves the frame pointer of its
// mode in a fixed location on its stack frame.  If the caller is a different
// mode, there is no easy way to find the frame pointer.  It can either be
// still in the designated register or saved on stack along with other callee
// saved registers.

// stackframe, or return nullptr if no stackframe can be found. Perform sanity
// checks (the strictness of which is controlled by the boolean parameter
// "STRICT_UNWINDING") to reduce the chance that a bad pointer is returned.
template<bool STRICT_UNWINDING>
static void **NextStackFrame(void **old_sp) {
  void **new_sp = (void**) old_sp[-1];


  if (STRICT_UNWINDING) {


    if (new_sp <= old_sp) return nullptr;

    if ((uintptr_t)new_sp - (uintptr_t)old_sp > 100000) return nullptr;
  } else {


    if (new_sp == old_sp) return nullptr;

    if ((new_sp > old_sp)
        && ((uintptr_t)new_sp - (uintptr_t)old_sp > 1000000)) return nullptr;
  }
  if ((uintptr_t)new_sp & (sizeof(void *) - 1)) return nullptr;
  return new_sp;
}

#ifdef __GNUC__
void StacktraceArmDummyFunction() __attribute__((noinline));
void StacktraceArmDummyFunction() { __asm__ volatile(""); }
#else
# error StacktraceArmDummyFunction() needs to be ported to this platform.
#endif

template <bool IS_STACK_FRAMES, bool IS_WITH_CONTEXT>
static int UnwindImpl(void** result, int* sizes, int max_depth, int skip_count,
                      const void * /* ucp */, int *min_dropped_frames) {
#ifdef __GNUC__
  void **sp = reinterpret_cast<void**>(__builtin_frame_address(0));
#else
# error reading stack point not yet supported on this platform.
#endif





  StacktraceArmDummyFunction();

  int n = 0;
  while (sp && n < max_depth) {





    void **next_sp = NextStackFrame<!IS_STACK_FRAMES>(sp);

    if (skip_count > 0) {
      skip_count--;
    } else {
      result[n] = *sp;

      if (IS_STACK_FRAMES) {
        if (next_sp > sp) {
          sizes[n] = (uintptr_t)next_sp - (uintptr_t)sp;
        } else {

          sizes[n] = 0;
        }
      }
      n++;
    }
    sp = next_sp;
  }
  if (min_dropped_frames != nullptr) {


    const int kMaxUnwind = 200;
    int num_dropped_frames = 0;
    for (int j = 0; sp != nullptr && j < kMaxUnwind; j++) {
      if (skip_count > 0) {
        skip_count--;
      } else {
        num_dropped_frames++;
      }
      sp = NextStackFrame<!IS_STACK_FRAMES>(sp);
    }
    *min_dropped_frames = num_dropped_frames;
  }
  return n;
}

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {
bool StackTraceWorksForTest() {
  return false;
}
}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_DEBUGGING_INTERNAL_STACKTRACE_ARM_INL_H_
