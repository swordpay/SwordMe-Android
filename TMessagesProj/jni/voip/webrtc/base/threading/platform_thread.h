// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// the low-level platform-specific abstraction to the OS's threading interface.
// You should instead be using a message-loop driven Thread, see thread.h.

#ifndef BASE_THREADING_PLATFORM_THREAD_H_
#define BASE_THREADING_PLATFORM_THREAD_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/windows_types.h"
#elif defined(OS_FUCHSIA)
#include <zircon/types.h>
#elif defined(OS_MACOSX)
#include <mach/mach_types.h>
#elif defined(OS_POSIX)
#include <pthread.h>
#include <unistd.h>
#endif

namespace base {

#if defined(OS_WIN)
typedef DWORD PlatformThreadId;
#elif defined(OS_FUCHSIA)
typedef zx_handle_t PlatformThreadId;
#elif defined(OS_MACOSX)
typedef mach_port_t PlatformThreadId;
#elif defined(OS_POSIX)
typedef pid_t PlatformThreadId;
#endif

// Meant to be as fast as possible.
// These are produced by PlatformThread::CurrentRef(), and used to later
// check if we are on the same thread or not by using ==. These are safe
// to copy between threads, but can't be copied to another process as they
// have no meaning there. Also, the internal identifier can be re-used
// after a thread dies, so a PlatformThreadRef cannot be reliably used
// to distinguish a new thread from an old, dead thread.
class PlatformThreadRef {
 public:
#if defined(OS_WIN)
  typedef DWORD RefType;
#else  //  OS_POSIX
  typedef pthread_t RefType;
#endif
  constexpr PlatformThreadRef() : id_(0) {}

  explicit constexpr PlatformThreadRef(RefType id) : id_(id) {}

  bool operator==(PlatformThreadRef other) const {
    return id_ == other.id_;
  }

  bool operator!=(PlatformThreadRef other) const { return id_ != other.id_; }

  bool is_null() const {
    return id_ == 0;
  }
 private:
  RefType id_;
};

class PlatformThreadHandle {
 public:
#if defined(OS_WIN)
  typedef void* Handle;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  typedef pthread_t Handle;
#endif

  constexpr PlatformThreadHandle() : handle_(0) {}

  explicit constexpr PlatformThreadHandle(Handle handle) : handle_(handle) {}

  bool is_equal(const PlatformThreadHandle& other) const {
    return handle_ == other.handle_;
  }

  bool is_null() const {
    return !handle_;
  }

  Handle platform_handle() const {
    return handle_;
  }

 private:
  Handle handle_;
};

const PlatformThreadId kInvalidThreadId(0);

// SetCurrentThreadPriority(), listed in increasing order of importance.
enum class ThreadPriority : int {

  BACKGROUND,

  NORMAL,

  DISPLAY,

  REALTIME_AUDIO,
};

class BASE_EXPORT PlatformThread {
 public:


  class BASE_EXPORT Delegate {
   public:
    virtual void ThreadMain() = 0;

   protected:
    virtual ~Delegate() = default;
  };

  static PlatformThreadId CurrentId();


  static PlatformThreadRef CurrentRef();




  static PlatformThreadHandle CurrentHandle();

  static void YieldCurrentThread();





  static void Sleep(base::TimeDelta duration);


  static void SetName(const std::string& name);

  static const char* GetName();








  static bool Create(size_t stack_size,
                     Delegate* delegate,
                     PlatformThreadHandle* thread_handle) {
    return CreateWithPriority(stack_size, delegate, thread_handle,
                              ThreadPriority::NORMAL);
  }


  static bool CreateWithPriority(size_t stack_size, Delegate* delegate,
                                 PlatformThreadHandle* thread_handle,
                                 ThreadPriority priority);



  static bool CreateNonJoinable(size_t stack_size, Delegate* delegate);


  static bool CreateNonJoinableWithPriority(size_t stack_size,
                                            Delegate* delegate,
                                            ThreadPriority priority);



  static void Join(PlatformThreadHandle thread_handle);


  static void Detach(PlatformThreadHandle thread_handle);


  static bool CanIncreaseThreadPriority(ThreadPriority priority);













  static void SetCurrentThreadPriority(ThreadPriority priority);

  static ThreadPriority GetCurrentThreadPriority();

#if defined(OS_LINUX)









  static void SetThreadPriority(PlatformThreadId thread_id,
                                ThreadPriority priority);
#endif


  static size_t GetDefaultThreadStackSize();

 private:
  static void SetCurrentThreadPriorityImpl(ThreadPriority priority);

  DISALLOW_IMPLICIT_CONSTRUCTORS(PlatformThread);
};

namespace internal {

// into account after this initialization. This initialization must be
// synchronized with calls to PlatformThread::SetCurrentThreadPriority().
void InitializeThreadPrioritiesFeature();

}  // namespace internal

}  // namespace base

#endif  // BASE_THREADING_PLATFORM_THREAD_H_
