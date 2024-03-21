// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/win32_stack_frame_unwinder.h"

#include <windows.h>

#include <utility>

#include "base/macros.h"
#include "build/build_config.h"

namespace base {


namespace {

// functions.
class Win32UnwindFunctions : public Win32StackFrameUnwinder::UnwindFunctions {
 public:
  Win32UnwindFunctions();
  ~Win32UnwindFunctions() override;

  PRUNTIME_FUNCTION LookupFunctionEntry(DWORD64 program_counter,
                                        PDWORD64 image_base) override;

  void VirtualUnwind(DWORD64 image_base,
                     DWORD64 program_counter,
                     PRUNTIME_FUNCTION runtime_function,
                     CONTEXT* context) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(Win32UnwindFunctions);
};

Win32UnwindFunctions::Win32UnwindFunctions() {}
Win32UnwindFunctions::~Win32UnwindFunctions() {}

PRUNTIME_FUNCTION Win32UnwindFunctions::LookupFunctionEntry(
    DWORD64 program_counter,
    PDWORD64 image_base) {
#ifdef _WIN64
  return ::RtlLookupFunctionEntry(program_counter, image_base, nullptr);
#else
  NOTREACHED();
  return nullptr;
#endif
}

void Win32UnwindFunctions::VirtualUnwind(DWORD64 image_base,
                                         DWORD64 program_counter,
                                         PRUNTIME_FUNCTION runtime_function,
                                         CONTEXT* context) {
#ifdef _WIN64
  void* handler_data = nullptr;
  ULONG64 establisher_frame;
  KNONVOLATILE_CONTEXT_POINTERS nvcontext = {};
  ::RtlVirtualUnwind(UNW_FLAG_NHANDLER, image_base, program_counter,
                     runtime_function, context, &handler_data,
                     &establisher_frame, &nvcontext);
#else
  NOTREACHED();
#endif
}

}  // namespace


Win32StackFrameUnwinder::UnwindFunctions::~UnwindFunctions() = default;
Win32StackFrameUnwinder::UnwindFunctions::UnwindFunctions() = default;

Win32StackFrameUnwinder::Win32StackFrameUnwinder()
    : Win32StackFrameUnwinder(std::make_unique<Win32UnwindFunctions>()) {}

Win32StackFrameUnwinder::~Win32StackFrameUnwinder() {}

bool Win32StackFrameUnwinder::TryUnwind(
    bool at_top_frame,
    CONTEXT* context,







    const ModuleCache::Module* module) {
#ifdef _WIN64

  DCHECK(module);
  ULONG64 image_base;

  PRUNTIME_FUNCTION runtime_function =
      unwind_functions_->LookupFunctionEntry(ContextPC(context), &image_base);
  DCHECK_EQ(module->GetBaseAddress(), image_base);

  if (runtime_function) {
    unwind_functions_->VirtualUnwind(image_base, ContextPC(context),
                                     runtime_function, context);
    return true;
  }

  if (at_top_frame) {


#if defined(ARCH_CPU_X86_64)

    context->Rip = *reinterpret_cast<DWORD64*>(context->Rsp);
    context->Rsp += 8;
#elif defined(ARCH_CPU_ARM64)



    context->Pc = context->Lr;
    context->ContextFlags |= CONTEXT_UNWOUND_TO_CALL;
#else
#error Unsupported Windows 64-bit Arch
#endif
    return true;
  }







  return false;
#else
  NOTREACHED();
  return false;
#endif
}

Win32StackFrameUnwinder::Win32StackFrameUnwinder(
    std::unique_ptr<UnwindFunctions> unwind_functions)
    : unwind_functions_(std::move(unwind_functions)) {}

}  // namespace base
