/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_OPERATIONS_CHAIN_H_
#define RTC_BASE_OPERATIONS_CHAIN_H_

#include <functional>
#include <memory>
#include <queue>
#include <set>
#include <type_traits>
#include <utility>

#include "absl/types/optional.h"
#include "api/ref_counted_base.h"
#include "api/scoped_refptr.h"
#include "api/sequence_checker.h"
#include "rtc_base/checks.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/system/no_unique_address.h"

namespace rtc {

namespace rtc_operations_chain_internal {

// invoked exactly once during the Operation's lifespan.
class Operation {
 public:
  virtual ~Operation() {}

  virtual void Run() = 0;
};

// passed on to the `functor_` and is used to inform the OperationsChain that
// the operation completed. The functor is responsible for invoking the
// callback when the operation has completed.
template <typename FunctorT>
class OperationWithFunctor final : public Operation {
 public:
  OperationWithFunctor(FunctorT&& functor, std::function<void()> callback)
      : functor_(std::forward<FunctorT>(functor)),
        callback_(std::move(callback)) {}

  ~OperationWithFunctor() override {
#if RTC_DCHECK_IS_ON
    RTC_DCHECK(has_run_);
#endif  // RTC_DCHECK_IS_ON
  }

  void Run() override {
#if RTC_DCHECK_IS_ON
    RTC_DCHECK(!has_run_);
    has_run_ = true;
#endif  // RTC_DCHECK_IS_ON





    auto functor = std::move(functor_);
    functor(std::move(callback_));

  }

 private:
  typename std::remove_reference<FunctorT>::type functor_;
  std::function<void()> callback_;
#if RTC_DCHECK_IS_ON
  bool has_run_ = false;
#endif  // RTC_DCHECK_IS_ON
};

}  // namespace rtc_operations_chain_internal

// ensure that asynchronous tasks are executed in-order with at most one task
// running at a time. The notion of an operation chain is defined in
// https://w3c.github.io/webrtc-pc/#dfn-operations-chain, though unlike this
// implementation, the referenced definition is coupled with a peer connection.
//
// An operation is an asynchronous task. The operation starts when its functor
// is invoked, and completes when the callback that is passed to functor is
// invoked by the operation. The operation must start and complete on the same
// sequence that the operation was "chained" on. As such, the OperationsChain
// operates in a "single-threaded" fashion, but the asynchronous operations may
// use any number of threads to achieve "in parallel" behavior.
//
// When an operation is chained onto the OperationsChain, it is enqueued to be
// executed. Operations are executed in FIFO order, where the next operation
// does not start until the previous operation has completed. OperationsChain
// guarantees that:
// - If the operations chain is empty when an operation is chained, the
//   operation starts immediately, inside ChainOperation().
// - If the operations chain is not empty when an operation is chained, the
//   operation starts upon the previous operation completing, inside the
//   callback.
//
// An operation is contractually obligated to invoke the completion callback
// exactly once. Cancelling a chained operation is not supported by the
// OperationsChain; an operation that wants to be cancellable is responsible for
// aborting its own steps. The callback must still be invoked.
//
// The OperationsChain is kept-alive through reference counting if there are
// operations pending. This, together with the contract, guarantees that all
// operations that are chained get executed.
class OperationsChain final : public RefCountedNonVirtual<OperationsChain> {
 public:
  static scoped_refptr<OperationsChain> Create();
  ~OperationsChain();

  OperationsChain(const OperationsChain&) = delete;
  OperationsChain& operator=(const OperationsChain&) = delete;

  void SetOnChainEmptyCallback(std::function<void()> on_chain_empty_callback);
  bool IsEmpty() const;



















  template <typename FunctorT>
  void ChainOperation(FunctorT&& functor) {
    RTC_DCHECK_RUN_ON(&sequence_checker_);
    chained_operations_.push(
        std::make_unique<
            rtc_operations_chain_internal::OperationWithFunctor<FunctorT>>(
            std::forward<FunctorT>(functor), CreateOperationsChainCallback()));



    if (chained_operations_.size() == 1) {
      chained_operations_.front()->Run();
    }
  }

 private:
  friend class CallbackHandle;





  class CallbackHandle final : public RefCountedNonVirtual<CallbackHandle> {
   public:
    explicit CallbackHandle(scoped_refptr<OperationsChain> operations_chain);
    ~CallbackHandle();

    CallbackHandle(const CallbackHandle&) = delete;
    CallbackHandle& operator=(const CallbackHandle&) = delete;

    void OnOperationComplete();

   private:
    scoped_refptr<OperationsChain> operations_chain_;
#if RTC_DCHECK_IS_ON
    bool has_run_ = false;
#endif  // RTC_DCHECK_IS_ON
  };

  OperationsChain();

  std::function<void()> CreateOperationsChainCallback();
  void OnOperationComplete();

  RTC_NO_UNIQUE_ADDRESS webrtc::SequenceChecker sequence_checker_;



  std::queue<std::unique_ptr<rtc_operations_chain_internal::Operation>>
      chained_operations_ RTC_GUARDED_BY(sequence_checker_);
  absl::optional<std::function<void()>> on_chain_empty_callback_
      RTC_GUARDED_BY(sequence_checker_);
};

}  // namespace rtc

#endif  // RTC_BASE_OPERATIONS_CHAIN_H_
