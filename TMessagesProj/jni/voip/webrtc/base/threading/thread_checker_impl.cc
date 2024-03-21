// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/thread_checker_impl.h"

#include "base/logging.h"
#include "base/threading/thread_local.h"
#include "base/threading/thread_task_runner_handle.h"

namespace base {

ThreadCheckerImpl::ThreadCheckerImpl() {
  AutoLock auto_lock(lock_);
  EnsureAssignedLockRequired();
}

ThreadCheckerImpl::~ThreadCheckerImpl() = default;

ThreadCheckerImpl::ThreadCheckerImpl(ThreadCheckerImpl&& other) {


  const bool other_called_on_valid_thread = other.CalledOnValidThread();
  DCHECK(other_called_on_valid_thread);


  thread_id_ = other.thread_id_;
  task_token_ = other.task_token_;
  sequence_token_ = other.sequence_token_;

  other.thread_id_ = PlatformThreadRef();
  other.task_token_ = TaskToken();
  other.sequence_token_ = SequenceToken();
}

ThreadCheckerImpl& ThreadCheckerImpl::operator=(ThreadCheckerImpl&& other) {
  DCHECK(CalledOnValidThread());


  const bool other_called_on_valid_thread = other.CalledOnValidThread();
  DCHECK(other_called_on_valid_thread);

  TS_UNCHECKED_READ(thread_id_) = TS_UNCHECKED_READ(other.thread_id_);
  TS_UNCHECKED_READ(task_token_) = TS_UNCHECKED_READ(other.task_token_);
  TS_UNCHECKED_READ(sequence_token_) = TS_UNCHECKED_READ(other.sequence_token_);

  TS_UNCHECKED_READ(other.thread_id_) = PlatformThreadRef();
  TS_UNCHECKED_READ(other.task_token_) = TaskToken();
  TS_UNCHECKED_READ(other.sequence_token_) = SequenceToken();

  return *this;
}

bool ThreadCheckerImpl::CalledOnValidThread() const {
  const bool has_thread_been_destroyed = ThreadLocalStorage::HasBeenDestroyed();

  AutoLock auto_lock(lock_);



  if (!has_thread_been_destroyed) {
    EnsureAssignedLockRequired();


    if (task_token_ == TaskToken::GetForCurrentThread())
      return true;




    if (sequence_token_.IsValid() &&
        (sequence_token_ != SequenceToken::GetForCurrentThread() ||
         !ThreadTaskRunnerHandle::IsSet())) {
      return false;
    }
  } else if (thread_id_.is_null()) {



    thread_id_ = PlatformThread::CurrentRef();
  }

  return thread_id_ == PlatformThread::CurrentRef();
}

void ThreadCheckerImpl::DetachFromThread() {
  AutoLock auto_lock(lock_);
  thread_id_ = PlatformThreadRef();
  task_token_ = TaskToken();
  sequence_token_ = SequenceToken();
}

void ThreadCheckerImpl::EnsureAssignedLockRequired() const {
  if (!thread_id_.is_null())
    return;

  thread_id_ = PlatformThread::CurrentRef();
  task_token_ = TaskToken::GetForCurrentThread();
  sequence_token_ = SequenceToken::GetForCurrentThread();
}

}  // namespace base
