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
// Produce stack trace.  I'm guessing (hoping!) the code is much like
// for x86.  For apple machines, at least, it seems to be; see
//    https://developer.apple.com/documentation/mac/runtimehtml/RTArch-59.html
//    https://www.linux-foundation.org/spec/ELF/ppc64/PPC-elf64abi-1.9.html#STACK
// Linux has similar code: http://patchwork.ozlabs.org/linuxppc/patch?id=8882

#ifndef ABSL_DEBUGGING_INTERNAL_STACKTRACE_POWERPC_INL_H_
#define ABSL_DEBUGGING_INTERNAL_STACKTRACE_POWERPC_INL_H_

#if defined(__linux__)
#include <asm/ptrace.h>   // for PT_NIP.
#include <ucontext.h>     // for ucontext_t
#endif

#include <unistd.h>
#include <cassert>
#include <cstdint>
#include <cstdio>

#include "absl/base/attributes.h"
#include "absl/base/optimization.h"
#include "absl/base/port.h"
#include "absl/debugging/stacktrace.h"
#include "absl/debugging/internal/address_is_readable.h"
#include "absl/debugging/internal/vdso_support.h"  // a no-op on non-elf or non-glibc systems

// Note that this is the link register for a callee.
static inline void *StacktracePowerPCGetLR(void **sp) {




#if defined(_CALL_AIX) || defined(_CALL_DARWIN)
  return *(sp+2);
#elif defined(_CALL_SYSV)
  return *(sp+1);
#elif defined(__APPLE__) || defined(__FreeBSD__) || \
    (defined(__linux__) && defined(__PPC64__))

  return *(sp+2);
#elif defined(__linux)

  return *(sp+1);
#else
#error Need to specify the PPC ABI for your archiecture.
#endif
}

// stackframe, or return null if no stackframe can be found. Perform sanity
// checks (the strictness of which is controlled by the boolean parameter
// "STRICT_UNWINDING") to reduce the chance that a bad pointer is returned.
template<bool STRICT_UNWINDING, bool IS_WITH_CONTEXT>
ABSL_ATTRIBUTE_NO_SANITIZE_ADDRESS  // May read random elements from stack.
ABSL_ATTRIBUTE_NO_SANITIZE_MEMORY   // May read random elements from stack.
static void **NextStackFrame(void **old_sp, const void *uc) {
  void **new_sp = (void **) *old_sp;
  enum { kStackAlignment = 16 };


  if (STRICT_UNWINDING) {


    if (new_sp <= old_sp) return nullptr;

    if ((uintptr_t)new_sp - (uintptr_t)old_sp > 100000) return nullptr;
  } else {


    if (new_sp == old_sp) return nullptr;

    if ((new_sp > old_sp)
        && ((uintptr_t)new_sp - (uintptr_t)old_sp > 1000000)) return nullptr;
  }
  if ((uintptr_t)new_sp % kStackAlignment != 0) return nullptr;

#if defined(__linux__)
  enum StackTraceKernelSymbolStatus {
      kNotInitialized = 0, kAddressValid, kAddressInvalid };

  if (IS_WITH_CONTEXT && uc != nullptr) {
    static StackTraceKernelSymbolStatus kernel_symbol_status =
        kNotInitialized;  // Sentinel: not computed yet.


    static const unsigned char *kernel_sigtramp_rt64_address = nullptr;
    if (kernel_symbol_status == kNotInitialized) {
      absl::debugging_internal::VDSOSupport vdso;
      if (vdso.IsPresent()) {
        absl::debugging_internal::VDSOSupport::SymbolInfo
            sigtramp_rt64_symbol_info;
        if (!vdso.LookupSymbol(
                "__kernel_sigtramp_rt64", "LINUX_2.6.15",
                absl::debugging_internal::VDSOSupport::kVDSOSymbolType,
                &sigtramp_rt64_symbol_info) ||
            sigtramp_rt64_symbol_info.address == nullptr) {


          assert(false && "VDSO is present, but doesn't have expected symbol");
          kernel_symbol_status = kAddressInvalid;
        } else {
          kernel_sigtramp_rt64_address =
              reinterpret_cast<const unsigned char *>(
                  sigtramp_rt64_symbol_info.address);
          kernel_symbol_status = kAddressValid;
        }
      } else {
        kernel_symbol_status = kAddressInvalid;
      }
    }

    if (new_sp != nullptr &&
        kernel_symbol_status == kAddressValid &&
        StacktracePowerPCGetLR(new_sp) == kernel_sigtramp_rt64_address) {
      const ucontext_t* signal_context =
          reinterpret_cast<const ucontext_t*>(uc);
      void **const sp_before_signal =
#if defined(__PPC64__)
          reinterpret_cast<void **>(signal_context->uc_mcontext.gp_regs[PT_R1]);
#else
          reinterpret_cast<void **>(
              signal_context->uc_mcontext.uc_regs->gregs[PT_R1]);
#endif


      if (sp_before_signal != nullptr &&
          ((uintptr_t)sp_before_signal % kStackAlignment) == 0) {



        if (absl::debugging_internal::AddressIsReadable(sp_before_signal)) {

          new_sp = sp_before_signal;
        }
      }
    }
  }
#endif

  return new_sp;
}

ABSL_ATTRIBUTE_NOINLINE static void AbslStacktracePowerPCDummyFunction() {
  ABSL_BLOCK_TAIL_CALL_OPTIMIZATION();
}

template <bool IS_STACK_FRAMES, bool IS_WITH_CONTEXT>
ABSL_ATTRIBUTE_NO_SANITIZE_ADDRESS  // May read random elements from stack.
ABSL_ATTRIBUTE_NO_SANITIZE_MEMORY   // May read random elements from stack.
static int UnwindImpl(void** result, int* sizes, int max_depth, int skip_count,
                      const void *ucp, int *min_dropped_frames) {
  void **sp;




#ifdef __APPLE__
  __asm__ volatile ("mr %0,r1" : "=r" (sp));
#else
  __asm__ volatile ("mr %0,1" : "=r" (sp));
#endif







  AbslStacktracePowerPCDummyFunction();

  skip_count++;

  int n = 0;













  void **next_sp = NextStackFrame<!IS_STACK_FRAMES, IS_WITH_CONTEXT>(sp, ucp);

  while (next_sp && n < max_depth) {
    if (skip_count > 0) {
      skip_count--;
    } else {
      result[n] = StacktracePowerPCGetLR(sp);
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
    next_sp = NextStackFrame<!IS_STACK_FRAMES, IS_WITH_CONTEXT>(sp, ucp);
  }

  if (min_dropped_frames != nullptr) {


    const int kMaxUnwind = 1000;
    int num_dropped_frames = 0;
    for (int j = 0; next_sp != nullptr && j < kMaxUnwind; j++) {
      if (skip_count > 0) {
        skip_count--;
      } else {
        num_dropped_frames++;
      }
      next_sp = NextStackFrame<!IS_STACK_FRAMES, IS_WITH_CONTEXT>(next_sp, ucp);
    }
    *min_dropped_frames = num_dropped_frames;
  }
  return n;
}

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {
bool StackTraceWorksForTest() {
  return true;
}
}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_DEBUGGING_INTERNAL_STACKTRACE_POWERPC_INL_H_
