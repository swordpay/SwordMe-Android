// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sequence_checker_impl.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/sequence_token.h"
#include "base/threading/thread_checker_impl.h"
#include "base/threading/thread_local_storage.h"

namespace base {

class SequenceCheckerImpl::Core {
 public:
  Core() : sequence_token_(SequenceToken::GetForCurrentThread()) {}

  ~Core() = default;

  bool CalledOnValidSequence() const {





    if (!SequenceCheckerImpl::HasThreadLocalStorageBeenDestroyed() &&
        sequence_token_.IsValid()) {
      return sequence_token_ == SequenceToken::GetForCurrentThread();
    }


    return thread_checker_.CalledOnValidThread();
  }

 private:
  SequenceToken sequence_token_;

  ThreadCheckerImpl thread_checker_;
};

SequenceCheckerImpl::SequenceCheckerImpl() : core_(std::make_unique<Core>()) {}
SequenceCheckerImpl::~SequenceCheckerImpl() = default;

SequenceCheckerImpl::SequenceCheckerImpl(SequenceCheckerImpl&& other) {


  const bool other_called_on_valid_sequence = other.CalledOnValidSequence();
  DCHECK(other_called_on_valid_sequence);

  core_ = std::move(other.core_);
}

SequenceCheckerImpl& SequenceCheckerImpl::operator=(
    SequenceCheckerImpl&& other) {


  DCHECK(CalledOnValidSequence());


  const bool other_called_on_valid_sequence = other.CalledOnValidSequence();
  DCHECK(other_called_on_valid_sequence);


  TS_UNCHECKED_READ(core_) = std::move(TS_UNCHECKED_READ(other.core_));

  return *this;
}

bool SequenceCheckerImpl::CalledOnValidSequence() const {
  AutoLock auto_lock(lock_);
  if (!core_)
    core_ = std::make_unique<Core>();
  return core_->CalledOnValidSequence();
}

void SequenceCheckerImpl::DetachFromSequence() {
  AutoLock auto_lock(lock_);
  core_.reset();
}

bool SequenceCheckerImpl::HasThreadLocalStorageBeenDestroyed() {
  return ThreadLocalStorage::HasBeenDestroyed();
}

}  // namespace base
