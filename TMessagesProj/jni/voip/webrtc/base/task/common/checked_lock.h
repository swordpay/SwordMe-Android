// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_COMMON_CHECKED_LOCK_H_
#define BASE_TASK_COMMON_CHECKED_LOCK_H_

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/task/common/checked_lock_impl.h"
#include "base/thread_annotations.h"

namespace base {
namespace internal {

// impl. When DCHECK_IS_ON(), lock checking occurs. Otherwise, CheckedLock is
// equivalent to base::Lock.
//
// The shape of CheckedLock is as follows:
// CheckedLock()
//     Default constructor, no predecessor lock.
//     DCHECKs
//         On Acquisition if any CheckedLock is acquired on this thread.
//             Okay if a universal predecessor is acquired.
//
// CheckedLock(const CheckedLock* predecessor)
//     Constructor that specifies an allowed predecessor for that lock.
//     DCHECKs
//         On Construction if |predecessor| forms a predecessor lock cycle.
//         On Acquisition if the previous lock acquired on the thread is not
//             either |predecessor| or a universal predecessor. Okay if there
//             was no previous lock acquired.
//
// CheckedLock(UniversalPredecessor universal_predecessor)
//     Constructor for a lock that will allow the acquisition of any lock after
//     it, without needing to explicitly be named a predecessor. Can only be
//     acquired if no locks are currently held by this thread.
//     DCHECKs
//         On Acquisition if any CheckedLock is acquired on this thread.
//
// void Acquire()
//     Acquires the lock.
//
// void Release()
//     Releases the lock.
//
// void AssertAcquired().
//     DCHECKs if the lock is not acquired.
//
// std::unique_ptr<ConditionVariable> CreateConditionVariable()
//     Creates a condition variable using this as a lock.

#if DCHECK_IS_ON()
class LOCKABLE CheckedLock : public CheckedLockImpl {
 public:
  CheckedLock() = default;
  explicit CheckedLock(const CheckedLock* predecessor)
      : CheckedLockImpl(predecessor) {}
  explicit CheckedLock(UniversalPredecessor universal_predecessor)
      : CheckedLockImpl(universal_predecessor) {}
};
#else   // DCHECK_IS_ON()
class LOCKABLE CheckedLock : public Lock {
 public:
  CheckedLock() = default;
  explicit CheckedLock(const CheckedLock*) {}
  explicit CheckedLock(UniversalPredecessor) {}
  static void AssertNoLockHeldOnCurrentThread() {}

  std::unique_ptr<ConditionVariable> CreateConditionVariable() {
    return std::unique_ptr<ConditionVariable>(new ConditionVariable(this));
  }
};
#endif  // DCHECK_IS_ON()

using CheckedAutoLock = internal::BasicAutoLock<CheckedLock>;

using CheckedAutoUnlock = internal::BasicAutoUnlock<CheckedLock>;

using CheckedAutoLockMaybe = internal::BasicAutoLockMaybe<CheckedLock>;

// Because the clang thread safety analysis doesn't understand aliased locks
// [1], this code wouldn't compile without AnnotateAcquiredLockAlias:
//
// class Example {
//  public:
//    CheckedLock lock_;
//    int value = 0 GUARDED_BY(lock_);
// };
//
// Example example;
// CheckedLock* acquired = &example.lock_;
// CheckedAutoLock auto_lock(*acquired);
// AnnotateAcquiredLockAlias annotate(*acquired, example.lock_);
// example.value = 42;  // Doesn't compile without |annotate|.
//
// [1] https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#no-alias-analysis
class SCOPED_LOCKABLE AnnotateAcquiredLockAlias {
 public:


  AnnotateAcquiredLockAlias(const CheckedLock& acquired_lock,
                            const CheckedLock& lock_alias)
      EXCLUSIVE_LOCK_FUNCTION(lock_alias)
      : acquired_lock_(acquired_lock) {
    DCHECK_EQ(&acquired_lock, &lock_alias);
    acquired_lock_.AssertAcquired();
  }
  ~AnnotateAcquiredLockAlias() UNLOCK_FUNCTION() {
    acquired_lock_.AssertAcquired();
  }

 private:
  const CheckedLock& acquired_lock_;

  DISALLOW_COPY_AND_ASSIGN(AnnotateAcquiredLockAlias);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_COMMON_CHECKED_LOCK_H_
