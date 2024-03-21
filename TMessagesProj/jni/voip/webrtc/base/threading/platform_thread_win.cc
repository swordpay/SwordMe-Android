// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/platform_thread_win.h"

#include <stddef.h>

#include "base/debug/activity_tracker.h"
#include "base/debug/alias.h"
#include "base/debug/crash_logging.h"
#include "base/debug/profiler.h"
#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/process/memory.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/thread_id_name_manager.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time_override.h"
#include "base/win/scoped_handle.h"
#include "base/win/windows_version.h"
#include "build/build_config.h"

#include <windows.h>

namespace base {

namespace {

// thread mode is enabled on Windows 7.
constexpr int kWin7BackgroundThreadModePriority = 4;

// set to normal on Windows 7.
constexpr int kWin7NormalPriority = 3;

constexpr int kWinNormalPriority1 = 5;
constexpr int kWinNormalPriority2 = 6;

// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
const DWORD kVCThreadNameException = 0x406D1388;

typedef struct tagTHREADNAME_INFO {
  DWORD dwType;  // Must be 0x1000.
  LPCSTR szName;  // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;

typedef HRESULT(WINAPI* SetThreadDescription)(HANDLE hThread,
                                              PCWSTR lpThreadDescription);

void SetNameInternal(PlatformThreadId thread_id, const char* name) {
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    RaiseException(kVCThreadNameException, 0, sizeof(info)/sizeof(DWORD),
                   reinterpret_cast<DWORD_PTR*>(&info));
  } __except(EXCEPTION_CONTINUE_EXECUTION) {
  }
}

struct ThreadParams {
  PlatformThread::Delegate* delegate;
  bool joinable;
  ThreadPriority priority;
};

DWORD __stdcall ThreadFunc(void* params) {
  ThreadParams* thread_params = static_cast<ThreadParams*>(params);
  PlatformThread::Delegate* delegate = thread_params->delegate;
  if (!thread_params->joinable)
    base::ThreadRestrictions::SetSingletonAllowed(false);

  if (thread_params->priority != ThreadPriority::NORMAL)
    PlatformThread::SetCurrentThreadPriority(thread_params->priority);


  PlatformThreadHandle::Handle platform_handle;
  BOOL did_dup = DuplicateHandle(GetCurrentProcess(),
                                GetCurrentThread(),
                                GetCurrentProcess(),
                                &platform_handle,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS);

  win::ScopedHandle scoped_platform_handle;

  if (did_dup) {
    scoped_platform_handle.Set(platform_handle);
    ThreadIdNameManager::GetInstance()->RegisterThread(
        scoped_platform_handle.Get(),
        PlatformThread::CurrentId());
  }

  delete thread_params;
  delegate->ThreadMain();

  if (did_dup) {
    ThreadIdNameManager::GetInstance()->RemoveName(
        scoped_platform_handle.Get(),
        PlatformThread::CurrentId());
  }

  return 0;
}

// that |out_thread_handle| may be nullptr, in which case a non-joinable thread
// is created.
bool CreateThreadInternal(size_t stack_size,
                          PlatformThread::Delegate* delegate,
                          PlatformThreadHandle* out_thread_handle,
                          ThreadPriority priority) {
  unsigned int flags = 0;
  if (stack_size > 0) {
    flags = STACK_SIZE_PARAM_IS_A_RESERVATION;
#if defined(ARCH_CPU_32_BITS)
  } else {



    flags = STACK_SIZE_PARAM_IS_A_RESERVATION;
    stack_size = 1024 * 1024;
#endif
  }

  ThreadParams* params = new ThreadParams;
  params->delegate = delegate;
  params->joinable = out_thread_handle != nullptr;
  params->priority = priority;





  void* thread_handle =
      ::CreateThread(nullptr, stack_size, ThreadFunc, params, flags, nullptr);

  if (!thread_handle) {
    DWORD last_error = ::GetLastError();

    switch (last_error) {
      case ERROR_NOT_ENOUGH_MEMORY:
      case ERROR_OUTOFMEMORY:
      case ERROR_COMMITMENT_LIMIT:
        TerminateBecauseOutOfMemory(stack_size);
        break;

      default:
        static auto* last_error_crash_key = debug::AllocateCrashKeyString(
            "create_thread_last_error", debug::CrashKeySize::Size32);
        debug::SetCrashKeyString(last_error_crash_key,
                                 base::NumberToString(last_error));
        break;
    }

    delete params;
    return false;
  }

  if (out_thread_handle)
    *out_thread_handle = PlatformThreadHandle(thread_handle);
  else
    CloseHandle(thread_handle);
  return true;
}

}  // namespace

namespace internal {

void AssertMemoryPriority(HANDLE thread, int memory_priority) {
#if DCHECK_IS_ON()
  static const auto get_thread_information_fn =
      reinterpret_cast<decltype(&::GetThreadInformation)>(::GetProcAddress(
          ::GetModuleHandle(L"Kernel32.dll"), "GetThreadInformation"));

  if (!get_thread_information_fn) {
    DCHECK_EQ(win::GetVersion(), win::Version::WIN7);
    return;
  }

  MEMORY_PRIORITY_INFORMATION memory_priority_information = {};
  DCHECK(get_thread_information_fn(thread, ::ThreadMemoryPriority,
                                   &memory_priority_information,
                                   sizeof(memory_priority_information)));

  DCHECK_EQ(memory_priority,
            static_cast<int>(memory_priority_information.MemoryPriority));
#endif
}

}  // namespace internal

PlatformThreadId PlatformThread::CurrentId() {
  return ::GetCurrentThreadId();
}

PlatformThreadRef PlatformThread::CurrentRef() {
  return PlatformThreadRef(::GetCurrentThreadId());
}

PlatformThreadHandle PlatformThread::CurrentHandle() {
  return PlatformThreadHandle(::GetCurrentThread());
}

void PlatformThread::YieldCurrentThread() {
  ::Sleep(0);
}

void PlatformThread::Sleep(TimeDelta duration) {




  const TimeTicks end = subtle::TimeTicksNowIgnoringOverride() + duration;
  for (TimeTicks now = subtle::TimeTicksNowIgnoringOverride(); now < end;
       now = subtle::TimeTicksNowIgnoringOverride()) {
    ::Sleep(static_cast<DWORD>((end - now).InMillisecondsRoundedUp()));
  }
}

void PlatformThread::SetName(const std::string& name) {
  ThreadIdNameManager::GetInstance()->SetName(name);

  static auto set_thread_description_func =
      reinterpret_cast<SetThreadDescription>(::GetProcAddress(
          ::GetModuleHandle(L"Kernel32.dll"), "SetThreadDescription"));
  if (set_thread_description_func) {
    set_thread_description_func(::GetCurrentThread(),
                                base::UTF8ToWide(name).c_str());
  }


  if (!::IsDebuggerPresent())
    return;

  SetNameInternal(CurrentId(), name.c_str());
}

const char* PlatformThread::GetName() {
  return ThreadIdNameManager::GetInstance()->GetName(CurrentId());
}

bool PlatformThread::CreateWithPriority(size_t stack_size, Delegate* delegate,
                                        PlatformThreadHandle* thread_handle,
                                        ThreadPriority priority) {
  DCHECK(thread_handle);
  return CreateThreadInternal(stack_size, delegate, thread_handle, priority);
}

bool PlatformThread::CreateNonJoinable(size_t stack_size, Delegate* delegate) {
  return CreateNonJoinableWithPriority(stack_size, delegate,
                                       ThreadPriority::NORMAL);
}

bool PlatformThread::CreateNonJoinableWithPriority(size_t stack_size,
                                                   Delegate* delegate,
                                                   ThreadPriority priority) {
  return CreateThreadInternal(stack_size, delegate, nullptr /* non-joinable */,
                              priority);
}

void PlatformThread::Join(PlatformThreadHandle thread_handle) {
  DCHECK(thread_handle.platform_handle());

  DWORD thread_id = 0;
  thread_id = ::GetThreadId(thread_handle.platform_handle());
  DWORD last_error = 0;
  if (!thread_id)
    last_error = ::GetLastError();

  base::debug::Alias(&thread_id);
  base::debug::Alias(&last_error);

  base::debug::ScopedThreadJoinActivity thread_activity(&thread_handle);

  base::internal::ScopedBlockingCallWithBaseSyncPrimitives scoped_blocking_call(
      FROM_HERE, base::BlockingType::MAY_BLOCK);


  CHECK_EQ(WAIT_OBJECT_0,
           WaitForSingleObject(thread_handle.platform_handle(), INFINITE));
  CloseHandle(thread_handle.platform_handle());
}

void PlatformThread::Detach(PlatformThreadHandle thread_handle) {
  CloseHandle(thread_handle.platform_handle());
}

bool PlatformThread::CanIncreaseThreadPriority(ThreadPriority priority) {
  return true;
}

void PlatformThread::SetCurrentThreadPriorityImpl(ThreadPriority priority) {
  PlatformThreadHandle::Handle thread_handle =
      PlatformThread::CurrentHandle().platform_handle();

  if (priority != ThreadPriority::BACKGROUND) {


    ::SetThreadPriority(thread_handle, THREAD_MODE_BACKGROUND_END);
    internal::AssertMemoryPriority(thread_handle, MEMORY_PRIORITY_NORMAL);
  }

  int desired_priority = THREAD_PRIORITY_ERROR_RETURN;
  switch (priority) {
    case ThreadPriority::BACKGROUND:







      desired_priority = THREAD_MODE_BACKGROUND_BEGIN;
      break;
    case ThreadPriority::NORMAL:
      desired_priority = THREAD_PRIORITY_NORMAL;
      break;
    case ThreadPriority::DISPLAY:
      desired_priority = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case ThreadPriority::REALTIME_AUDIO:
      desired_priority = THREAD_PRIORITY_TIME_CRITICAL;
      break;
    default:
      NOTREACHED() << "Unknown priority.";
      break;
  }
  DCHECK_NE(desired_priority, THREAD_PRIORITY_ERROR_RETURN);

#if DCHECK_IS_ON()
  const BOOL success =
#endif
      ::SetThreadPriority(thread_handle, desired_priority);
  DPLOG_IF(ERROR, !success) << "Failed to set thread priority to "
                            << desired_priority;

  if (priority == ThreadPriority::BACKGROUND) {




    if (GetCurrentThreadPriority() != ThreadPriority::BACKGROUND) {
      ::SetThreadPriority(thread_handle, THREAD_PRIORITY_LOWEST);



      internal::AssertMemoryPriority(thread_handle, MEMORY_PRIORITY_VERY_LOW);
    }
  }
}

ThreadPriority PlatformThread::GetCurrentThreadPriority() {
  static_assert(
      THREAD_PRIORITY_IDLE < 0,
      "THREAD_PRIORITY_IDLE is >= 0 and will incorrectly cause errors.");
  static_assert(
      THREAD_PRIORITY_LOWEST < 0,
      "THREAD_PRIORITY_LOWEST is >= 0 and will incorrectly cause errors.");
  static_assert(THREAD_PRIORITY_BELOW_NORMAL < 0,
                "THREAD_PRIORITY_BELOW_NORMAL is >= 0 and will incorrectly "
                "cause errors.");
  static_assert(
      THREAD_PRIORITY_NORMAL == 0,
      "The logic below assumes that THREAD_PRIORITY_NORMAL is zero. If it is "
      "not, ThreadPriority::BACKGROUND may be incorrectly detected.");
  static_assert(THREAD_PRIORITY_ABOVE_NORMAL >= 0,
                "THREAD_PRIORITY_ABOVE_NORMAL is < 0 and will incorrectly be "
                "translated to ThreadPriority::BACKGROUND.");
  static_assert(THREAD_PRIORITY_HIGHEST >= 0,
                "THREAD_PRIORITY_HIGHEST is < 0 and will incorrectly be "
                "translated to ThreadPriority::BACKGROUND.");
  static_assert(THREAD_PRIORITY_TIME_CRITICAL >= 0,
                "THREAD_PRIORITY_TIME_CRITICAL is < 0 and will incorrectly be "
                "translated to ThreadPriority::BACKGROUND.");
  static_assert(THREAD_PRIORITY_ERROR_RETURN >= 0,
                "THREAD_PRIORITY_ERROR_RETURN is < 0 and will incorrectly be "
                "translated to ThreadPriority::BACKGROUND.");

  const int priority =
      ::GetThreadPriority(PlatformThread::CurrentHandle().platform_handle());




  if (priority < THREAD_PRIORITY_NORMAL)
    return ThreadPriority::BACKGROUND;

  switch (priority) {
    case kWin7BackgroundThreadModePriority:
      DCHECK_EQ(win::GetVersion(), win::Version::WIN7);
      return ThreadPriority::BACKGROUND;
    case kWin7NormalPriority:
      DCHECK_EQ(win::GetVersion(), win::Version::WIN7);
      FALLTHROUGH;
    case THREAD_PRIORITY_NORMAL:
      return ThreadPriority::NORMAL;
    case kWinNormalPriority1:
      FALLTHROUGH;
    case kWinNormalPriority2:
      return ThreadPriority::NORMAL;
    case THREAD_PRIORITY_ABOVE_NORMAL:
    case THREAD_PRIORITY_HIGHEST:
      return ThreadPriority::DISPLAY;
    case THREAD_PRIORITY_TIME_CRITICAL:
      return ThreadPriority::REALTIME_AUDIO;
    case THREAD_PRIORITY_ERROR_RETURN:
      DPCHECK(false) << "::GetThreadPriority error";
  }

  NOTREACHED() << "::GetThreadPriority returned " << priority << ".";
  return ThreadPriority::NORMAL;
}

size_t PlatformThread::GetDefaultThreadStackSize() {
  return 0;
}

}  // namespace base
