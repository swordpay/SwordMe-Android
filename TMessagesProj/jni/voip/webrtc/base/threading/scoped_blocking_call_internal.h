// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_SCOPED_BLOCKING_CALL_INTERNAL_H_
#define BASE_THREADING_SCOPED_BLOCKING_CALL_INTERNAL_H_

#include "base/base_export.h"
#include "base/debug/activity_tracker.h"
#include "base/macros.h"

namespace base {

enum class BlockingType;

// few key //base types to observe and react to blocking calls.
namespace internal {

// the scope of ScopedBlockingCall objects.
class BASE_EXPORT BlockingObserver {
 public:
  virtual ~BlockingObserver() = default;


  virtual void BlockingStarted(BlockingType blocking_type) = 0;



  virtual void BlockingTypeUpgraded() = 0;


  virtual void BlockingEnded() = 0;
};

// this on a thread where there is an active ScopedBlockingCall.
BASE_EXPORT void SetBlockingObserverForCurrentThread(
    BlockingObserver* blocking_observer);

BASE_EXPORT void ClearBlockingObserverForCurrentThread();

// ScopedBlockingCallWithBaseSyncPrimitives without assertions.
class BASE_EXPORT UncheckedScopedBlockingCall {
 public:
  explicit UncheckedScopedBlockingCall(const Location& from_here,
                                       BlockingType blocking_type);
  ~UncheckedScopedBlockingCall();

 private:
  internal::BlockingObserver* const blocking_observer_;

  UncheckedScopedBlockingCall* const previous_scoped_blocking_call_;


  const bool is_will_block_;

  base::debug::ScopedActivity scoped_activity_;

  DISALLOW_COPY_AND_ASSIGN(UncheckedScopedBlockingCall);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_THREADING_SCOPED_BLOCKING_CALL_INTERNAL_H_
