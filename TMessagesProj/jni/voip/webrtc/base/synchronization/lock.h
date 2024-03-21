// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYNCHRONIZATION_LOCK_H_
#define BASE_SYNCHRONIZATION_LOCK_H_

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/synchronization/lock_impl.h"
#include "base/thread_annotations.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"

namespace base {

// intelligence in this class is in debug mode for the support for the
// AssertAcquired() method.
class LOCKABLE BASE_EXPORT Lock {
 public:
#if !DCHECK_IS_ON()

  Lock() : lock_() {}
  ~Lock() {}

  void Acquire() EXCLUSIVE_LOCK_FUNCTION() { lock_.Lock(); }
  void Release() UNLOCK_FUNCTION() { lock_.Unlock(); }




  bool Try() EXCLUSIVE_TRYLOCK_FUNCTION(true) { return lock_.Try(); }

  void AssertAcquired() const ASSERT_EXCLUSIVE_LOCK() {}
#else
  Lock();
  ~Lock();



  void Acquire() EXCLUSIVE_LOCK_FUNCTION() {
    lock_.Lock();
    CheckUnheldAndMark();
  }
  void Release() UNLOCK_FUNCTION() {
    CheckHeldAndUnmark();
    lock_.Unlock();
  }

  bool Try() EXCLUSIVE_TRYLOCK_FUNCTION(true) {
    bool rv = lock_.Try();
    if (rv) {
      CheckUnheldAndMark();
    }
    return rv;
  }

  void AssertAcquired() const ASSERT_EXCLUSIVE_LOCK();
#endif  // DCHECK_IS_ON()


  static bool HandlesMultipleThreadPriorities() {
#if defined(OS_WIN)



    return true;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)


    return internal::LockImpl::PriorityInheritanceAvailable();
#else
#error Unsupported platform
#endif
  }



  friend class ConditionVariable;

 private:
#if DCHECK_IS_ON()





  void CheckHeldAndUnmark();
  void CheckUnheldAndMark();


  base::PlatformThreadRef owning_thread_ref_;
#endif  // DCHECK_IS_ON()

  internal::LockImpl lock_;

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

using AutoLock = internal::BasicAutoLock<Lock>;

// constructor, and re-Acquire() it in the destructor.
using AutoUnlock = internal::BasicAutoUnlock<Lock>;

// absl::MutexLockMaybe. Use this instead of base::Optional<base::AutoLock> to
// get around -Wthread-safety-analysis warnings for conditional locking.
using AutoLockMaybe = internal::BasicAutoLockMaybe<Lock>;

// Release() may be called at most once. Inspired from
// absl::ReleasableMutexLock. Use this instead of base::Optional<base::AutoLock>
// to get around -Wthread-safety-analysis warnings for AutoLocks that are
// explicitly released early (prefer proper scoping to this).
using ReleasableAutoLock = internal::BasicReleasableAutoLock<Lock>;

}  // namespace base

#endif  // BASE_SYNCHRONIZATION_LOCK_H_
