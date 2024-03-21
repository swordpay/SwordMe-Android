// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/platform_thread.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>

#include "base/debug/activity_tracker.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/threading/platform_thread_internal_posix.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/thread_id_name_manager.h"
#include "build/build_config.h"

#if !defined(OS_MACOSX) && !defined(OS_FUCHSIA) && !defined(OS_NACL)
#include "base/posix/can_lower_nice_to.h"
#endif

#if defined(OS_LINUX)
#include <sys/syscall.h>
#endif

#if defined(OS_FUCHSIA)
#include <zircon/process.h>
#else
#include <sys/resource.h>
#endif

namespace base {

void InitThreading();
void TerminateOnThread();
size_t GetDefaultThreadStackSize(const pthread_attr_t& attributes);

namespace {

struct ThreadParams {
  ThreadParams()
      : delegate(nullptr), joinable(false), priority(ThreadPriority::NORMAL) {}

  PlatformThread::Delegate* delegate;
  bool joinable;
  ThreadPriority priority;
};

void* ThreadFunc(void* params) {
  PlatformThread::Delegate* delegate = nullptr;

  {
    std::unique_ptr<ThreadParams> thread_params(
        static_cast<ThreadParams*>(params));

    delegate = thread_params->delegate;
    if (!thread_params->joinable)
      base::ThreadRestrictions::SetSingletonAllowed(false);

#if !defined(OS_NACL)



    PlatformThread::SetCurrentThreadPriority(thread_params->priority);
#endif
  }

  ThreadIdNameManager::GetInstance()->RegisterThread(
      PlatformThread::CurrentHandle().platform_handle(),
      PlatformThread::CurrentId());

  delegate->ThreadMain();

  ThreadIdNameManager::GetInstance()->RemoveName(
      PlatformThread::CurrentHandle().platform_handle(),
      PlatformThread::CurrentId());

  base::TerminateOnThread();
  return nullptr;
}

bool CreateThread(size_t stack_size,
                  bool joinable,
                  PlatformThread::Delegate* delegate,
                  PlatformThreadHandle* thread_handle,
                  ThreadPriority priority) {
  DCHECK(thread_handle);
  base::InitThreading();

  pthread_attr_t attributes;
  pthread_attr_init(&attributes);


  if (!joinable)
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

  if (stack_size == 0)
    stack_size = base::GetDefaultThreadStackSize(attributes);

  if (stack_size > 0)
    pthread_attr_setstacksize(&attributes, stack_size);

  std::unique_ptr<ThreadParams> params(new ThreadParams);
  params->delegate = delegate;
  params->joinable = joinable;
  params->priority = priority;

  pthread_t handle;
  int err = pthread_create(&handle, &attributes, ThreadFunc, params.get());
  bool success = !err;
  if (success) {

    ignore_result(params.release());
  } else {

    handle = 0;
    errno = err;
    PLOG(ERROR) << "pthread_create";
  }
  *thread_handle = PlatformThreadHandle(handle);

  pthread_attr_destroy(&attributes);

  return success;
}

#if defined(OS_LINUX)

// expensive and PlatformThread::CurrentId is used liberally. Clear
// the stored value after a fork() because forking changes the thread
// id. Forking without going through fork() (e.g. clone()) is not
// supported, but there is no known usage. Using thread_local is
// fine here (despite being banned) since it is going to be allowed
// but is blocked on a clang bug for Mac (https://crbug.com/829078)
// and we can't use ThreadLocalStorage because of re-entrancy due to
// CHECK/DCHECKs.
thread_local pid_t g_thread_id = -1;

class InitAtFork {
 public:
  InitAtFork() { pthread_atfork(nullptr, nullptr, internal::ClearTidCache); }
};

#endif  // defined(OS_LINUX)

}  // namespace

#if defined(OS_LINUX)

namespace internal {

void ClearTidCache() {
  g_thread_id = -1;
}

}  // namespace internal

#endif  // defined(OS_LINUX)

PlatformThreadId PlatformThread::CurrentId() {


#if defined(OS_MACOSX)
  return pthread_mach_thread_np(pthread_self());
#elif defined(OS_LINUX)
  static NoDestructor<InitAtFork> init_at_fork;
  if (g_thread_id == -1) {
    g_thread_id = syscall(__NR_gettid);
  } else {
    DCHECK_EQ(g_thread_id, syscall(__NR_gettid))
        << "Thread id stored in TLS is different from thread id returned by "
           "the system. It is likely that the process was forked without going "
           "through fork().";
  }
  return g_thread_id;
#elif defined(OS_ANDROID)
  return gettid();
#elif defined(OS_FUCHSIA)
  return zx_thread_self();
#elif defined(OS_SOLARIS) || defined(OS_QNX)
  return pthread_self();
#elif defined(OS_NACL) && defined(__GLIBC__)
  return pthread_self();
#elif defined(OS_NACL) && !defined(__GLIBC__)

  return reinterpret_cast<int32_t>(pthread_self());
#elif defined(OS_POSIX) && defined(OS_AIX)
  return pthread_self();
#elif defined(OS_POSIX) && !defined(OS_AIX)
  return reinterpret_cast<int64_t>(pthread_self());
#endif
}

PlatformThreadRef PlatformThread::CurrentRef() {
  return PlatformThreadRef(pthread_self());
}

PlatformThreadHandle PlatformThread::CurrentHandle() {
  return PlatformThreadHandle(pthread_self());
}

void PlatformThread::YieldCurrentThread() {
  sched_yield();
}

void PlatformThread::Sleep(TimeDelta duration) {
  struct timespec sleep_time, remaining;



  sleep_time.tv_sec = duration.InSeconds();
  duration -= TimeDelta::FromSeconds(sleep_time.tv_sec);
  sleep_time.tv_nsec = duration.InMicroseconds() * 1000;  // nanoseconds

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
}

const char* PlatformThread::GetName() {
  return ThreadIdNameManager::GetInstance()->GetName(CurrentId());
}

bool PlatformThread::CreateWithPriority(size_t stack_size, Delegate* delegate,
                                        PlatformThreadHandle* thread_handle,
                                        ThreadPriority priority) {
  return CreateThread(stack_size, true /* joinable thread */, delegate,
                      thread_handle, priority);
}

bool PlatformThread::CreateNonJoinable(size_t stack_size, Delegate* delegate) {
  return CreateNonJoinableWithPriority(stack_size, delegate,
                                       ThreadPriority::NORMAL);
}

bool PlatformThread::CreateNonJoinableWithPriority(size_t stack_size,
                                                   Delegate* delegate,
                                                   ThreadPriority priority) {
  PlatformThreadHandle unused;

  bool result = CreateThread(stack_size, false /* non-joinable thread */,
                             delegate, &unused, priority);
  return result;
}

void PlatformThread::Join(PlatformThreadHandle thread_handle) {

  base::debug::ScopedThreadJoinActivity thread_activity(&thread_handle);



  base::internal::ScopedBlockingCallWithBaseSyncPrimitives scoped_blocking_call(
      FROM_HERE, base::BlockingType::MAY_BLOCK);
  CHECK_EQ(0, pthread_join(thread_handle.platform_handle(), nullptr));
}

void PlatformThread::Detach(PlatformThreadHandle thread_handle) {
  CHECK_EQ(0, pthread_detach(thread_handle.platform_handle()));
}

// implementations.
#if !defined(OS_MACOSX) && !defined(OS_FUCHSIA)

bool PlatformThread::CanIncreaseThreadPriority(ThreadPriority priority) {
#if defined(OS_NACL)
  return false;
#else
  auto platform_specific_ability =
      internal::CanIncreaseCurrentThreadPriorityForPlatform(priority);
  if (platform_specific_ability)
    return platform_specific_ability.value();

  return internal::CanLowerNiceTo(
      internal::ThreadPriorityToNiceValue(priority));
#endif  // defined(OS_NACL)
}

void PlatformThread::SetCurrentThreadPriorityImpl(ThreadPriority priority) {
#if defined(OS_NACL)
  NOTIMPLEMENTED();
#else
  if (internal::SetCurrentThreadPriorityForPlatform(priority))
    return;






  const int nice_setting = internal::ThreadPriorityToNiceValue(priority);
  if (setpriority(PRIO_PROCESS, 0, nice_setting)) {
    DVPLOG(1) << "Failed to set nice value of thread ("
              << PlatformThread::CurrentId() << ") to " << nice_setting;
  }
#endif  // defined(OS_NACL)
}

ThreadPriority PlatformThread::GetCurrentThreadPriority() {
#if defined(OS_NACL)
  NOTIMPLEMENTED();
  return ThreadPriority::NORMAL;
#else

  auto platform_specific_priority =
      internal::GetCurrentThreadPriorityForPlatform();
  if (platform_specific_priority)
    return platform_specific_priority.value();


  errno = 0;
  int nice_value = getpriority(PRIO_PROCESS, 0);
  if (errno != 0) {
    DVPLOG(1) << "Failed to get nice value of thread ("
              << PlatformThread::CurrentId() << ")";
    return ThreadPriority::NORMAL;
  }

  return internal::NiceValueToThreadPriority(nice_value);
#endif  // !defined(OS_NACL)
}

#endif  // !defined(OS_MACOSX) && !defined(OS_FUCHSIA)

size_t PlatformThread::GetDefaultThreadStackSize() {
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);
  return base::GetDefaultThreadStackSize(attributes);
}

}  // namespace base
