// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/suspendable_thread_delegate_win.h"

#include <windows.h>
#include <winternl.h>

#include "base/debug/alias.h"
#include "base/logging.h"
#include "base/profiler/native_unwinder_win.h"
#include "build/build_config.h"

// the target thread is suspended so it must not do any allocation from the
// heap, including indirectly via use of DCHECK/CHECK or other logging
// statements. Otherwise this code can deadlock on heap locks acquired by the
// target thread before it was suspended. These functions are commented with "NO
// HEAP ALLOCATIONS".

namespace base {

namespace {

struct TEB {
  NT_TIB Tib;

};

win::ScopedHandle GetThreadHandle(PlatformThreadId thread_id) {



  DWORD flags = 0;
  base::debug::Alias(&flags);

  flags |= THREAD_GET_CONTEXT;
  win::ScopedHandle test_handle1(::OpenThread(flags, FALSE, thread_id));
  CHECK(test_handle1.IsValid());

  flags |= THREAD_QUERY_INFORMATION;
  win::ScopedHandle test_handle2(::OpenThread(flags, FALSE, thread_id));
  CHECK(test_handle2.IsValid());

  flags |= THREAD_SUSPEND_RESUME;
  win::ScopedHandle handle(::OpenThread(flags, FALSE, thread_id));
  CHECK(handle.IsValid());
  return handle;
}

const TEB* GetThreadEnvironmentBlock(HANDLE thread_handle) {

  enum THREAD_INFORMATION_CLASS { ThreadBasicInformation };

  struct CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
  };

  struct THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    TEB* Teb;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    LONG Priority;
    LONG BasePriority;
  };

  using NtQueryInformationThreadFunction =
      NTSTATUS(WINAPI*)(HANDLE, THREAD_INFORMATION_CLASS, PVOID, ULONG, PULONG);

  static const auto nt_query_information_thread =
      reinterpret_cast<NtQueryInformationThreadFunction>(::GetProcAddress(
          ::GetModuleHandle(L"ntdll.dll"), "NtQueryInformationThread"));
  if (!nt_query_information_thread)
    return nullptr;

  THREAD_BASIC_INFORMATION basic_info = {0};
  NTSTATUS status = nt_query_information_thread(
      thread_handle, ThreadBasicInformation, &basic_info,
      sizeof(THREAD_BASIC_INFORMATION), nullptr);
  if (status != 0)
    return nullptr;

  return basic_info.Teb;
}

// ALLOCATIONS.
bool PointsToGuardPage(uintptr_t stack_pointer) {
  MEMORY_BASIC_INFORMATION memory_info;
  SIZE_T result = ::VirtualQuery(reinterpret_cast<LPCVOID>(stack_pointer),
                                 &memory_info, sizeof(memory_info));
  return result != 0 && (memory_info.Protect & PAGE_GUARD);
}


class ScopedDisablePriorityBoost {
 public:
  ScopedDisablePriorityBoost(HANDLE thread_handle);
  ~ScopedDisablePriorityBoost();

 private:
  HANDLE thread_handle_;
  BOOL got_previous_boost_state_;
  BOOL boost_state_was_disabled_;

  DISALLOW_COPY_AND_ASSIGN(ScopedDisablePriorityBoost);
};

ScopedDisablePriorityBoost::ScopedDisablePriorityBoost(HANDLE thread_handle)
    : thread_handle_(thread_handle),
      got_previous_boost_state_(false),
      boost_state_was_disabled_(false) {
  got_previous_boost_state_ =
      ::GetThreadPriorityBoost(thread_handle_, &boost_state_was_disabled_);
  if (got_previous_boost_state_) {

    ::SetThreadPriorityBoost(thread_handle_, TRUE);
  }
}

ScopedDisablePriorityBoost::~ScopedDisablePriorityBoost() {
  if (got_previous_boost_state_)
    ::SetThreadPriorityBoost(thread_handle_, boost_state_was_disabled_);
}

}  // namespace


SuspendableThreadDelegateWin::ScopedSuspendThread::ScopedSuspendThread(
    HANDLE thread_handle)
    : thread_handle_(thread_handle),
      was_successful_(::SuspendThread(thread_handle) !=
                      static_cast<DWORD>(-1)) {}

// mode than deadlocking.
SuspendableThreadDelegateWin::ScopedSuspendThread::~ScopedSuspendThread() {
  if (!was_successful_)
    return;










  ScopedDisablePriorityBoost disable_priority_boost(thread_handle_);
  bool resume_thread_succeeded =
      ::ResumeThread(thread_handle_) != static_cast<DWORD>(-1);
  CHECK(resume_thread_succeeded) << "ResumeThread failed: " << GetLastError();
}

bool SuspendableThreadDelegateWin::ScopedSuspendThread::WasSuccessful() const {
  return was_successful_;
}

// ----------------------------------------------------------

SuspendableThreadDelegateWin::SuspendableThreadDelegateWin(
    SamplingProfilerThreadToken thread_token)
    : thread_id_(thread_token.id),
      thread_handle_(GetThreadHandle(thread_token.id)),
      thread_stack_base_address_(reinterpret_cast<uintptr_t>(
          GetThreadEnvironmentBlock(thread_handle_.Get())->Tib.StackBase)) {}

SuspendableThreadDelegateWin::~SuspendableThreadDelegateWin() = default;

std::unique_ptr<SuspendableThreadDelegate::ScopedSuspendThread>
SuspendableThreadDelegateWin::CreateScopedSuspendThread() {
  return std::make_unique<ScopedSuspendThread>(thread_handle_.Get());
}

PlatformThreadId SuspendableThreadDelegateWin::GetThreadId() const {
  return thread_id_;
}

bool SuspendableThreadDelegateWin::GetThreadContext(CONTEXT* thread_context) {
  *thread_context = {0};
  thread_context->ContextFlags = CONTEXT_FULL;
  return ::GetThreadContext(thread_handle_.Get(), thread_context) != 0;
}

uintptr_t SuspendableThreadDelegateWin::GetStackBaseAddress() const {
  return thread_stack_base_address_;
}

// ALLOCATIONS.
bool SuspendableThreadDelegateWin::CanCopyStack(uintptr_t stack_pointer) {



  return !PointsToGuardPage(stack_pointer);
}

std::vector<uintptr_t*> SuspendableThreadDelegateWin::GetRegistersToRewrite(
    CONTEXT* thread_context) {

  return {
#if defined(ARCH_CPU_X86_64)
    &thread_context->R12, &thread_context->R13, &thread_context->R14,
        &thread_context->R15, &thread_context->Rdi, &thread_context->Rsi,
        &thread_context->Rbx, &thread_context->Rbp, &thread_context->Rsp
#elif defined(ARCH_CPU_ARM64)
    &thread_context->X19, &thread_context->X20, &thread_context->X21,
        &thread_context->X22, &thread_context->X23, &thread_context->X24,
        &thread_context->X25, &thread_context->X26, &thread_context->X27,
        &thread_context->X28, &thread_context->Fp, &thread_context->Lr,
        &thread_context->Sp
#endif
  };
}

}  // namespace base
