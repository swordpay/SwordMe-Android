// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_COMMON_OPERATIONS_CONTROLLER_H_
#define BASE_TASK_COMMON_OPERATIONS_CONTROLLER_H_

#include <atomic>
#include <cstdint>

#include "base/synchronization/waitable_event.h"

namespace base {
namespace internal {

// operations without locks.
//
// The controller is used to determine if operations are allowed, and to keep
// track of how many are currently active. Users will call TryBeginOperation()
// before starting such operations. If the call succeeds the user can run the
// operation and the controller will keep track of it until the user signals
// that the operation is completed. No operations are allowed before
// StartAcceptingOperations() is called, or after
// ShutdownAndWaitForZeroOperations() is called.
//
// There is no explicit way of telling the controller when an operation is
// completed, instead for convenience TryBeginOperation() will return a RAII
// like object that will do so on destruction.
//
// For example:
//
// OperationsController controller_;
//
// void SetUp() {
//   controller_.StartAcceptingOperations();
// }
//
// void TearDown() {
//   controller_.ShutdownAndWaitForZeroOperations();
// }
//
// void MaybeRunOperation() {
//   auto operation_token = controller_.TryBeginOperation();
//   if (operation_token) {
//     Process();
//   }
// }
//
// This class is thread-safe.
// But note that StartAcceptingOperations can never be called after
// ShutdownAndWaitForZeroOperations.
class BASE_EXPORT OperationsController {
 public:







  class OperationToken {
   public:
    ~OperationToken() {
      if (outer_)
        outer_->DecrementBy(1);
    }
    OperationToken(const OperationToken&) = delete;
    OperationToken(OperationToken&& other) {
      this->outer_ = other.outer_;
      other.outer_ = nullptr;
    }

    operator bool() const { return !!outer_; }

   private:
    friend class OperationsController;
    explicit OperationToken(OperationsController* outer) : outer_(outer) {}
    OperationsController* outer_;
  };

  OperationsController();


  ~OperationsController();

  OperationsController(const OperationsController&) = delete;
  OperationsController& operator=(const OperationsController&) = delete;




  bool StartAcceptingOperations();





  OperationToken TryBeginOperation();




  void ShutdownAndWaitForZeroOperations();

 private:














  static constexpr uint32_t kShuttingDownBitMask = uint32_t{1} << 31;
  static constexpr uint32_t kAcceptingOperationsBitMask = uint32_t{1} << 30;
  static constexpr uint32_t kFlagsBitMask =
      (kShuttingDownBitMask | kAcceptingOperationsBitMask);
  static constexpr uint32_t kCountBitMask = ~kFlagsBitMask;
  enum class State {
    kRejectingOperations,
    kAcceptingOperations,
    kShuttingDown,
  };


  static uint32_t ExtractCount(uint32_t value) { return value & kCountBitMask; }
  static State ExtractState(uint32_t value);

  void DecrementBy(uint32_t n);

  std::atomic<uint32_t> state_and_count_{0};
  WaitableEvent shutdown_complete_;
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_COMMON_OPERATIONS_CONTROLLER_H_
