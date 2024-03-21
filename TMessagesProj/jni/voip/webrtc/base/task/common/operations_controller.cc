// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/common/operations_controller.h"
#include "base/logging.h"

namespace base {
namespace internal {

OperationsController::OperationsController() = default;

OperationsController::~OperationsController() {
#if DCHECK_IS_ON()



  auto value = state_and_count_.load();
  DCHECK(
      ExtractState(value) == State::kRejectingOperations ||
      (ExtractState(value) == State::kShuttingDown && ExtractCount(value) == 0))
      << value;
#endif
}

bool OperationsController::StartAcceptingOperations() {



  auto prev_value = state_and_count_.fetch_or(kAcceptingOperationsBitMask,
                                              std::memory_order_release);

  DCHECK_EQ(ExtractState(prev_value), State::kRejectingOperations);

  auto num_rejected = ExtractCount(prev_value);
  DecrementBy(num_rejected);
  return num_rejected != 0;
}

OperationsController::OperationToken OperationsController::TryBeginOperation() {




  auto prev_value = state_and_count_.fetch_add(1, std::memory_order_acquire);

  switch (ExtractState(prev_value)) {
    case State::kRejectingOperations:
      return OperationToken(nullptr);
    case State::kAcceptingOperations:
      return OperationToken(this);
    case State::kShuttingDown:
      DecrementBy(1);
      return OperationToken(nullptr);
  }
}

void OperationsController::ShutdownAndWaitForZeroOperations() {



  auto prev_value = state_and_count_.fetch_or(kShuttingDownBitMask,
                                              std::memory_order_acquire);

  switch (ExtractState(prev_value)) {
    case State::kRejectingOperations:

      DecrementBy(ExtractCount(prev_value));
      break;
    case State::kAcceptingOperations:
      if (ExtractCount(prev_value) != 0) {
        shutdown_complete_.Wait();
      }
      break;
    case State::kShuttingDown:
      DCHECK(false) << "Multiple calls to ShutdownAndWaitForZeroOperations()";
      break;
  }
}

OperationsController::State OperationsController::ExtractState(uint32_t value) {
  if (value & kShuttingDownBitMask) {
    return State::kShuttingDown;
  } else if (value & kAcceptingOperationsBitMask) {
    return State::kAcceptingOperations;
  } else {
    return State::kRejectingOperations;
  }
}

void OperationsController::DecrementBy(uint32_t n) {


  auto prev_value = state_and_count_.fetch_sub(n, std::memory_order_release);
  DCHECK_LE(n, ExtractCount(prev_value)) << "Decrement underflow";

  if (ExtractState(prev_value) == State::kShuttingDown &&
      ExtractCount(prev_value) == n) {
    shutdown_complete_.Signal();
  }
}

}  // namespace internal
}  // namespace base