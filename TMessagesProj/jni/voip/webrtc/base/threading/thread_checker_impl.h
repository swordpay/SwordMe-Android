// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_THREAD_CHECKER_IMPL_H_
#define BASE_THREADING_THREAD_CHECKER_IMPL_H_

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/sequence_token.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"
#include "base/threading/platform_thread.h"

namespace base {

// use in release mode (e.g. to CHECK on a threading issue seen only in the
// wild).
//
// Note: You should almost always use the ThreadChecker class to get the right
// version for your build configuration.
// Note: This is only a check, not a "lock". It is marked "LOCKABLE" only in
// order to support thread_annotations.h.
class LOCKABLE BASE_EXPORT ThreadCheckerImpl {
 public:
  ThreadCheckerImpl();
  ~ThreadCheckerImpl();






  ThreadCheckerImpl(ThreadCheckerImpl&& other);
  ThreadCheckerImpl& operator=(ThreadCheckerImpl&& other);

  bool CalledOnValidThread() const WARN_UNUSED_RESULT;



  void DetachFromThread();

 private:
  void EnsureAssignedLockRequired() const EXCLUSIVE_LOCKS_REQUIRED(lock_);


  mutable base::Lock lock_;

  mutable PlatformThreadRef thread_id_ GUARDED_BY(lock_);






  mutable TaskToken task_token_ GUARDED_BY(lock_);




  mutable SequenceToken sequence_token_ GUARDED_BY(lock_);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_CHECKER_IMPL_H_
