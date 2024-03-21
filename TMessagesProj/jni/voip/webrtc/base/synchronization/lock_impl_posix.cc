// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/lock_impl.h"

#include <string>

#include "base/debug/activity_tracker.h"
#include "base/logging.h"
#include "base/posix/safe_strerror.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/synchronization_buildflags.h"
#include "build/build_config.h"

namespace base {
namespace internal {

namespace {

#if DCHECK_IS_ON()
const char* AdditionalHintForSystemErrorCode(int error_code) {
  switch (error_code) {
    case EINVAL:
      return "Hint: This is often related to a use-after-free.";
    default:
      return "";
  }
}
#endif  // DCHECK_IS_ON()

std::string SystemErrorCodeToString(int error_code) {
#if DCHECK_IS_ON()
  return base::safe_strerror(error_code) + ". " +
         AdditionalHintForSystemErrorCode(error_code);
#else   // DCHECK_IS_ON()
  return std::string();
#endif  // DCHECK_IS_ON()
}

}  // namespace

// this define for platform code that may not compile if priority inheritance
// locks aren't available. For this platform code,
// PRIORITY_INHERITANCE_LOCKS_POSSIBLE() is a necessary but insufficient check.
// Lock::PriorityInheritanceAvailable still must be checked as the code may
// compile but the underlying platform still may not correctly support priority
// inheritance locks.
#if defined(OS_NACL) || defined(OS_ANDROID) || defined(OS_FUCHSIA)
#define PRIORITY_INHERITANCE_LOCKS_POSSIBLE() 0
#else
#define PRIORITY_INHERITANCE_LOCKS_POSSIBLE() 1
#endif

LockImpl::LockImpl() {
  pthread_mutexattr_t mta;
  int rv = pthread_mutexattr_init(&mta);
  DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
#if PRIORITY_INHERITANCE_LOCKS_POSSIBLE()
  if (PriorityInheritanceAvailable()) {
    rv = pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_INHERIT);
    DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
  }
#endif
#ifndef NDEBUG

  rv = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
  DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
#endif
  rv = pthread_mutex_init(&native_handle_, &mta);
  DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
  rv = pthread_mutexattr_destroy(&mta);
  DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
}

LockImpl::~LockImpl() {
  int rv = pthread_mutex_destroy(&native_handle_);
  DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
}

bool LockImpl::Try() {
  int rv = pthread_mutex_trylock(&native_handle_);
  DCHECK(rv == 0 || rv == EBUSY) << ". " << SystemErrorCodeToString(rv);
  return rv == 0;
}

void LockImpl::Lock() {







  if (base::debug::GlobalActivityTracker::IsEnabled())
    if (Try())
      return;

  base::debug::ScopedLockAcquireActivity lock_activity(this);
  int rv = pthread_mutex_lock(&native_handle_);
  DCHECK_EQ(rv, 0) << ". " << SystemErrorCodeToString(rv);
}

bool LockImpl::PriorityInheritanceAvailable() {
#if BUILDFLAG(ENABLE_MUTEX_PRIORITY_INHERITANCE)
  return true;
#elif PRIORITY_INHERITANCE_LOCKS_POSSIBLE() && defined(OS_MACOSX)
  return true;
#else















  return false;
#endif
}

}  // namespace internal
}  // namespace base
