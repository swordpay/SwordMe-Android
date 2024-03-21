/*
 *  Copyright 2020 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H_
#define API_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H_

#include <utility>

#include "absl/functional/any_invocable.h"
#include "api/ref_counted_base.h"
#include "api/scoped_refptr.h"
#include "api/sequence_checker.h"
#include "rtc_base/checks.h"
#include "rtc_base/system/no_unique_address.h"

namespace webrtc {

// the issue where you have a task to be executed later that has references,
// but cannot guarantee that the referenced object is alive when the task is
// executed.

// on a single thread / task queue, and with tasks posted to the same
// thread/task queue, but tasks can be posted from any thread/TQ.

// When posting a task, post a copy (capture by-value in a lambda) of the flag
// reference and before performing the work, check the `alive()` state. Abort if
// alive() returns `false`:
//
// class ExampleClass {
// ....
//    rtc::scoped_refptr<PendingTaskSafetyFlag> flag = safety_flag_;
//    my_task_queue_->PostTask(
//        [flag = std::move(flag), this] {
//          // Now running on the main thread.
//          if (!flag->alive())
//            return;
//          MyMethod();
//        });
//   ....
//   ~ExampleClass() {
//     safety_flag_->SetNotAlive();
//   }
//   scoped_refptr<PendingTaskSafetyFlag> safety_flag_
//        = PendingTaskSafetyFlag::Create();
// }
//
// SafeTask makes this check automatic:
//
//   my_task_queue_->PostTask(SafeTask(safety_flag_, [this] { MyMethod(); }));
//
class PendingTaskSafetyFlag final
    : public rtc::RefCountedNonVirtual<PendingTaskSafetyFlag> {
 public:
  static rtc::scoped_refptr<PendingTaskSafetyFlag> Create();


  static rtc::scoped_refptr<PendingTaskSafetyFlag> CreateDetached();


  static rtc::scoped_refptr<PendingTaskSafetyFlag> CreateDetachedInactive();

  ~PendingTaskSafetyFlag() = default;

  void SetNotAlive();















  void SetAlive();
  bool alive() const;

 protected:
  explicit PendingTaskSafetyFlag(bool alive) : alive_(alive) {}

 private:
  static rtc::scoped_refptr<PendingTaskSafetyFlag> CreateInternal(bool alive);

  bool alive_ = true;
  RTC_NO_UNIQUE_ADDRESS SequenceChecker main_sequence_;
};

// It does automatic PTSF creation and signalling of destruction when the
// ScopedTaskSafety instance goes out of scope.
//
// Example usage:
//
//     my_task_queue->PostTask(SafeTask(scoped_task_safety.flag(),
//        [this] {
//             // task goes here
//        }
//
// This should be used by the class that wants tasks dropped after destruction.
// The requirement is that the instance has to be constructed and destructed on
// the same thread as the potentially dropped tasks would be running on.
class ScopedTaskSafety final {
 public:
  ScopedTaskSafety() = default;
  explicit ScopedTaskSafety(rtc::scoped_refptr<PendingTaskSafetyFlag> flag)
      : flag_(std::move(flag)) {}
  ~ScopedTaskSafety() { flag_->SetNotAlive(); }

  rtc::scoped_refptr<PendingTaskSafetyFlag> flag() const { return flag_; }

  void reset(rtc::scoped_refptr<PendingTaskSafetyFlag> new_flag =
                 PendingTaskSafetyFlag::Create()) {
    flag_->SetNotAlive();
    flag_ = std::move(new_flag);
  }

 private:
  rtc::scoped_refptr<PendingTaskSafetyFlag> flag_ =
      PendingTaskSafetyFlag::Create();
};

// where the flag will be used.
class ScopedTaskSafetyDetached final {
 public:
  ScopedTaskSafetyDetached() = default;
  ~ScopedTaskSafetyDetached() { flag_->SetNotAlive(); }

  rtc::scoped_refptr<PendingTaskSafetyFlag> flag() const { return flag_; }

 private:
  rtc::scoped_refptr<PendingTaskSafetyFlag> flag_ =
      PendingTaskSafetyFlag::CreateDetached();
};

inline absl::AnyInvocable<void() &&> SafeTask(
    rtc::scoped_refptr<PendingTaskSafetyFlag> flag,
    absl::AnyInvocable<void() &&> task) {
  return [flag = std::move(flag), task = std::move(task)]() mutable {
    if (flag->alive()) {
      std::move(task)();
    }
  };
}

}  // namespace webrtc

#endif  // API_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H_
