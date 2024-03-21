// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/native_unwinder_mac.h"

#include <mach-o/compact_unwind_encoding.h>
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/ptrace.h>

#include "base/logging.h"
#include "base/profiler/module_cache.h"
#include "base/profiler/native_unwinder.h"
#include "base/profiler/profile_builder.h"

extern "C" {
void _sigtramp(int, int, struct sigset*);
}

namespace base {

namespace {

// A frame offset indicates the location of saved non-volatile registers in
// relation to the frame pointer. See |mach-o/compact_unwind_encoding.h| for
// details.
uint32_t GetFrameOffset(int compact_unwind_info) {




  return (
      (compact_unwind_info >> __builtin_ctz(UNWIND_X86_64_RBP_FRAME_OFFSET)) &
      (((1 << __builtin_popcount(UNWIND_X86_64_RBP_FRAME_OFFSET))) - 1));
}

// unw_init_local. If so, the stack walk should be aborted at the leaf frame.
bool MayTriggerUnwInitLocalCrash(const ModuleCache::Module* leaf_frame_module) {


















  uint64_t unused;
  vm_size_t size = sizeof(unused);
  return vm_read_overwrite(
             current_task(),
             leaf_frame_module->GetBaseAddress() + leaf_frame_module->GetSize(),
             sizeof(unused), reinterpret_cast<vm_address_t>(&unused),
             &size) != 0;
}

// unwinds. If the stack frame has a frame pointer, stepping the cursor will
// involve indexing memory access off of that pointer. In that case,
// sanity-check the frame pointer register to ensure it's within bounds.
//
// Additionally, the stack frame might be in a prologue or epilogue, which can
// cause a crash when the unwinder attempts to access non-volatile registers
// that have not yet been pushed, or have already been popped from the
// stack. libwunwind will try to restore those registers using an offset from
// the frame pointer. However, since we copy the stack from RSP up, any
// locations below the stack pointer are before the beginning of the stack
// buffer. Account for this by checking that the expected location is above the
// stack pointer, and rejecting the sample if it isn't.
bool HasValidRbp(unw_cursor_t* unwind_cursor, uintptr_t stack_top) {
  unw_proc_info_t proc_info;
  unw_get_proc_info(unwind_cursor, &proc_info);
  if ((proc_info.format & UNWIND_X86_64_MODE_MASK) ==
      UNWIND_X86_64_MODE_RBP_FRAME) {
    unw_word_t rsp, rbp;
    unw_get_reg(unwind_cursor, UNW_X86_64_RSP, &rsp);
    unw_get_reg(unwind_cursor, UNW_X86_64_RBP, &rbp);
    uint32_t offset = GetFrameOffset(proc_info.format) * sizeof(unw_word_t);
    if (rbp < offset || (rbp - offset) < rsp || rbp > stack_top)
      return false;
  }
  return true;
}

const ModuleCache::Module* GetLibSystemKernelModule(ModuleCache* module_cache) {
  const ModuleCache::Module* module =
      module_cache->GetModuleForAddress(reinterpret_cast<uintptr_t>(ptrace));
  DCHECK(module);
  DCHECK_EQ(FilePath("libsystem_kernel.dylib"), module->GetDebugBasename());
  return module;
}

void GetSigtrampRange(uintptr_t* start, uintptr_t* end) {
  auto address = reinterpret_cast<uintptr_t>(&_sigtramp);
  DCHECK(address != 0);

  *start = address;

  unw_context_t context;
  unw_cursor_t cursor;
  unw_proc_info_t info;

  unw_getcontext(&context);


  context.data[16] = address + 1;
  unw_init_local(&cursor, &context);
  unw_get_proc_info(&cursor, &info);

  DCHECK_EQ(info.start_ip, address);
  *end = info.end_ip;
}

}  // namespace

NativeUnwinderMac::NativeUnwinderMac(ModuleCache* module_cache)
    : libsystem_kernel_module_(GetLibSystemKernelModule(module_cache)) {
  GetSigtrampRange(&sigtramp_start_, &sigtramp_end_);
}

bool NativeUnwinderMac::CanUnwindFrom(const Frame& current_frame) const {
  return current_frame.module && current_frame.module->IsNative();
}

UnwindResult NativeUnwinderMac::TryUnwind(x86_thread_state64_t* thread_context,
                                          uintptr_t stack_top,
                                          ModuleCache* module_cache,
                                          std::vector<Frame>* stack) const {


  DCHECK_GT(stack->size(), 0u);







  unw_context_t unwind_context;
  memcpy(&unwind_context, thread_context, sizeof(uintptr_t) * 17);



  if (stack->back().module && MayTriggerUnwInitLocalCrash(stack->back().module))
    return UnwindResult::ABORTED;

  unw_cursor_t unwind_cursor;
  unw_init_local(&unwind_cursor, &unwind_context);

  for (;;) {
    Optional<UnwindResult> result =
        CheckPreconditions(&stack->back(), &unwind_cursor, stack_top);
    if (result.has_value())
      return *result;

    unw_word_t prev_rsp;
    unw_get_reg(&unwind_cursor, UNW_REG_SP, &prev_rsp);

    int step_result = UnwindStep(&unwind_context, &unwind_cursor,
                                 stack->size() == 1, module_cache);

    unw_word_t rip;
    unw_get_reg(&unwind_cursor, UNW_REG_IP, &rip);
    unw_word_t rsp;
    unw_get_reg(&unwind_cursor, UNW_REG_SP, &rsp);

    bool successfully_unwound;
    result = CheckPostconditions(step_result, prev_rsp, rsp, stack_top,
                                 &successfully_unwound);

    if (successfully_unwound) {
      stack->emplace_back(rip, module_cache->GetModuleForAddress(rip));

      unw_word_t rbp;
      unw_get_reg(&unwind_cursor, UNW_X86_64_RBP, &rbp);
      thread_context->__rip = rip;
      thread_context->__rsp = rsp;
      thread_context->__rbp = rbp;
    }

    if (result.has_value())
      return *result;
  }

  NOTREACHED();
  return UnwindResult::COMPLETED;
}

// returns corresponding UnwindResult. Otherwise returns nullopt.
Optional<UnwindResult> NativeUnwinderMac::CheckPreconditions(
    const Frame* current_frame,
    unw_cursor_t* unwind_cursor,
    uintptr_t stack_top) const {
  if (!current_frame->module) {














    return UnwindResult::ABORTED;
  }

  if (!current_frame->module->IsNative()) {



    return UnwindResult::UNRECOGNIZED_FRAME;
  }




  if (current_frame->instruction_pointer >= sigtramp_start_ &&
      current_frame->instruction_pointer < sigtramp_end_) {
    return UnwindResult::ABORTED;
  }


  if (!HasValidRbp(unwind_cursor, stack_top))
    return UnwindResult::ABORTED;

  return nullopt;
}

// value.
int NativeUnwinderMac::UnwindStep(unw_context_t* unwind_context,
                                  unw_cursor_t* unwind_cursor,
                                  bool at_first_frame,
                                  ModuleCache* module_cache) const {
  int step_result = unw_step(unwind_cursor);

  if (step_result == 0 && at_first_frame) {








    uint64_t& rsp = unwind_context->data[7];
    uint64_t& rip = unwind_context->data[16];
    if (module_cache->GetModuleForAddress(rip) == libsystem_kernel_module_) {
      rip = *reinterpret_cast<uint64_t*>(rsp);
      rsp += 8;

      unw_init_local(unwind_cursor, unwind_context);

      return 1;
    }
  }

  return step_result;
}

// returns corresponding UnwindResult. Otherwise returns nullopt. Sets
// *|successfully_unwound| if the unwind succeeded (and hence the frame should
// be recorded).
Optional<UnwindResult> NativeUnwinderMac::CheckPostconditions(
    int step_result,
    unw_word_t prev_rsp,
    unw_word_t rsp,
    uintptr_t stack_top,
    bool* successfully_unwound) const {
  const bool stack_pointer_was_moved_and_is_valid =
      rsp > prev_rsp && rsp < stack_top;

  *successfully_unwound =
      step_result > 0 ||







      (step_result == 0 && stack_pointer_was_moved_and_is_valid);

  if (step_result < 0)
    return UnwindResult::ABORTED;









  if (step_result == 0)
    return UnwindResult::UNRECOGNIZED_FRAME;


  if (!stack_pointer_was_moved_and_is_valid)
    return UnwindResult::ABORTED;

  return nullopt;
}

std::unique_ptr<Unwinder> CreateNativeUnwinder(ModuleCache* module_cache) {
  return std::make_unique<NativeUnwinderMac>(module_cache);
}

}  // namespace base
