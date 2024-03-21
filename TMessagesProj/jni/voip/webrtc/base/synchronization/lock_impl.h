// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYNCHRONIZATION_LOCK_IMPL_H_
#define BASE_SYNCHRONIZATION_LOCK_IMPL_H_

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/thread_annotations.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/windows_types.h"
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <errno.h>
#include <pthread.h>
#endif

namespace base {
namespace internal {

// used for the Lock class.  Most users should not use LockImpl directly, but
// should instead use Lock.
class BASE_EXPORT LockImpl {
 public:
#if defined(OS_WIN)
  using NativeHandle = CHROME_SRWLOCK;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  using NativeHandle = pthread_mutex_t;
#endif

  LockImpl();
  ~LockImpl();


  bool Try();

  void Lock();


  inline void Unlock();



  NativeHandle* native_handle() { return &native_handle_; }

#if defined(OS_POSIX) || defined(OS_FUCHSIA)

  static bool PriorityInheritanceAvailable();
#endif

 private:
  NativeHandle native_handle_;

  DISALLOW_COPY_AND_ASSIGN(LockImpl);
};

#if defined(OS_WIN)
void LockImpl::Unlock() {
  ::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&native_handle_));
}
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
void LockImpl::Unlock() {
  int rv = pthread_mutex_unlock(&native_handle_);
  DCHECK_EQ(rv, 0) << ". " << strerror(rv);
}
#endif

template <class LockType>
class SCOPED_LOCKABLE BasicAutoLock {
 public:
  struct AlreadyAcquired {};

  explicit BasicAutoLock(LockType& lock) EXCLUSIVE_LOCK_FUNCTION(lock)
      : lock_(lock) {
    lock_.Acquire();
  }

  BasicAutoLock(LockType& lock, const AlreadyAcquired&)
      EXCLUSIVE_LOCKS_REQUIRED(lock)
      : lock_(lock) {
    lock_.AssertAcquired();
  }

  ~BasicAutoLock() UNLOCK_FUNCTION() {
    lock_.AssertAcquired();
    lock_.Release();
  }

 private:
  LockType& lock_;
  DISALLOW_COPY_AND_ASSIGN(BasicAutoLock);
};

template <class LockType>
class BasicAutoUnlock {
 public:
  explicit BasicAutoUnlock(LockType& lock) : lock_(lock) {

    lock_.AssertAcquired();
    lock_.Release();
  }

  ~BasicAutoUnlock() { lock_.Acquire(); }

 private:
  LockType& lock_;
  DISALLOW_COPY_AND_ASSIGN(BasicAutoUnlock);
};

template <class LockType>
class SCOPED_LOCKABLE BasicAutoLockMaybe {
 public:
  explicit BasicAutoLockMaybe(LockType* lock) EXCLUSIVE_LOCK_FUNCTION(lock)
      : lock_(lock) {
    if (lock_)
      lock_->Acquire();
  }

  ~BasicAutoLockMaybe() UNLOCK_FUNCTION() {
    if (lock_) {
      lock_->AssertAcquired();
      lock_->Release();
    }
  }

 private:
  LockType* const lock_;
  DISALLOW_COPY_AND_ASSIGN(BasicAutoLockMaybe);
};

// type.
template <class LockType>
class SCOPED_LOCKABLE BasicReleasableAutoLock {
 public:
  explicit BasicReleasableAutoLock(LockType* lock) EXCLUSIVE_LOCK_FUNCTION(lock)
      : lock_(lock) {
    DCHECK(lock_);
    lock_->Acquire();
  }

  ~BasicReleasableAutoLock() UNLOCK_FUNCTION() {
    if (lock_) {
      lock_->AssertAcquired();
      lock_->Release();
    }
  }

  void Release() UNLOCK_FUNCTION() {
    DCHECK(lock_);
    lock_->AssertAcquired();
    lock_->Release();
    lock_ = nullptr;
  }

 private:
  LockType* lock_;
  DISALLOW_COPY_AND_ASSIGN(BasicReleasableAutoLock);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_SYNCHRONIZATION_LOCK_IMPL_H_
