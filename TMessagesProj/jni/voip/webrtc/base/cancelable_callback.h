// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CancelableCallback is a wrapper around base::Callback that allows
// cancellation of a callback. CancelableCallback takes a reference on the
// wrapped callback until this object is destroyed or Reset()/Cancel() are
// called.
//
// NOTE:
//
// Calling CancelableCallback::Cancel() brings the object back to its natural,
// default-constructed state, i.e., CancelableCallback::callback() will return
// a null callback.
//
// THREAD-SAFETY:
//
// CancelableCallback objects must be created on, posted to, cancelled on, and
// destroyed on the same thread.
//
//
// EXAMPLE USAGE:
//
// In the following example, the test is verifying that RunIntensiveTest()
// Quit()s the message loop within 4 seconds. The cancelable callback is posted
// to the message loop, the intensive test runs, the message loop is run,
// then the callback is cancelled.
//
// RunLoop run_loop;
//
// void TimeoutCallback(const std::string& timeout_message) {
//   FAIL() << timeout_message;
//   run_loop.QuitWhenIdle();
// }
//
// CancelableOnceClosure timeout(
//     base::BindOnce(&TimeoutCallback, "Test timed out."));
// ThreadTaskRunnerHandle::Get()->PostDelayedTask(FROM_HERE, timeout.callback(),
//                                                TimeDelta::FromSeconds(4));
// RunIntensiveTest();
// run_loop.Run();
// timeout.Cancel();  // Hopefully this is hit before the timeout callback runs.
//

#ifndef BASE_CANCELABLE_CALLBACK_H_
#define BASE_CANCELABLE_CALLBACK_H_

#include <utility>

#include "base/base_export.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_internal.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"

namespace base {
namespace internal {

template <typename CallbackType>
class CancelableCallbackImpl {
 public:
  CancelableCallbackImpl() {}

  explicit CancelableCallbackImpl(CallbackType callback)
      : callback_(std::move(callback)) {
    DCHECK(callback_);
  }

  ~CancelableCallbackImpl() = default;

  void Cancel() {
    weak_ptr_factory_.InvalidateWeakPtrs();
    callback_.Reset();
  }

  bool IsCancelled() const {
    return callback_.is_null();
  }


  void Reset(CallbackType callback) {
    DCHECK(callback);

    Cancel();
    callback_ = std::move(callback);
  }

  CallbackType callback() const {
    if (!callback_)
      return CallbackType();
    CallbackType forwarder;
    MakeForwarder(&forwarder);
    return forwarder;
  }

 private:
  template <typename... Args>
  void MakeForwarder(RepeatingCallback<void(Args...)>* out) const {
    using ForwarderType = void (CancelableCallbackImpl::*)(Args...);
    ForwarderType forwarder = &CancelableCallbackImpl::ForwardRepeating;
    *out = BindRepeating(forwarder, weak_ptr_factory_.GetWeakPtr());
  }

  template <typename... Args>
  void MakeForwarder(OnceCallback<void(Args...)>* out) const {
    using ForwarderType = void (CancelableCallbackImpl::*)(Args...);
    ForwarderType forwarder = &CancelableCallbackImpl::ForwardOnce;
    *out = BindOnce(forwarder, weak_ptr_factory_.GetWeakPtr());
  }

  template <typename... Args>
  void ForwardRepeating(Args... args) {
    callback_.Run(std::forward<Args>(args)...);
  }

  template <typename... Args>
  void ForwardOnce(Args... args) {
    weak_ptr_factory_.InvalidateWeakPtrs();
    std::move(callback_).Run(std::forward<Args>(args)...);
  }

  CallbackType callback_;
  mutable base::WeakPtrFactory<CancelableCallbackImpl> weak_ptr_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(CancelableCallbackImpl);
};

}  // namespace internal

// the task cancellation.
template <typename Signature>
using CancelableOnceCallback =
    internal::CancelableCallbackImpl<OnceCallback<Signature>>;
using CancelableOnceClosure = CancelableOnceCallback<void()>;

template <typename Signature>
using CancelableRepeatingCallback =
    internal::CancelableCallbackImpl<RepeatingCallback<Signature>>;
using CancelableRepeatingClosure = CancelableRepeatingCallback<void()>;

template <typename Signature>
using CancelableCallback = CancelableRepeatingCallback<Signature>;
using CancelableClosure = CancelableCallback<void()>;

}  // namespace base

#endif  // BASE_CANCELABLE_CALLBACK_H_
