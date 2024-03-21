//
// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//  SpinLock is provided for use in two situations:
//   - for use by Abseil internal code that Mutex itself depends on
//   - for async signal safety (see below)

// handler, all code that acquires the lock must ensure that the signal cannot
// arrive while they are holding the lock.  Typically, this is done by blocking
// the signal.
//
// Threads waiting on a SpinLock may be woken in an arbitrary order.

#ifndef ABSL_BASE_INTERNAL_SPINLOCK_H_
#define ABSL_BASE_INTERNAL_SPINLOCK_H_

#include <atomic>
#include <cstdint>

#include "absl/base/attributes.h"
#include "absl/base/const_init.h"
#include "absl/base/dynamic_annotations.h"
#include "absl/base/internal/low_level_scheduling.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/base/internal/scheduling_mode.h"
#include "absl/base/internal/tsan_mutex_interface.h"
#include "absl/base/thread_annotations.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace base_internal {

class ABSL_LOCKABLE SpinLock {
 public:
  SpinLock() : lockword_(kSpinLockCooperative) {
    ABSL_TSAN_MUTEX_CREATE(this, __tsan_mutex_not_static);
  }


  explicit SpinLock(base_internal::SchedulingMode mode);

  constexpr SpinLock(absl::ConstInitType, base_internal::SchedulingMode mode)
      : lockword_(IsCooperative(mode) ? kSpinLockCooperative : 0) {}



#ifdef ABSL_INTERNAL_HAVE_TSAN_INTERFACE
  ~SpinLock() { ABSL_TSAN_MUTEX_DESTROY(this, __tsan_mutex_not_static); }
#else
  ~SpinLock() = default;
#endif

  inline void Lock() ABSL_EXCLUSIVE_LOCK_FUNCTION() {
    ABSL_TSAN_MUTEX_PRE_LOCK(this, 0);
    if (!TryLockImpl()) {
      SlowLock();
    }
    ABSL_TSAN_MUTEX_POST_LOCK(this, 0, 0);
  }




  inline bool TryLock() ABSL_EXCLUSIVE_TRYLOCK_FUNCTION(true) {
    ABSL_TSAN_MUTEX_PRE_LOCK(this, __tsan_mutex_try_lock);
    bool res = TryLockImpl();
    ABSL_TSAN_MUTEX_POST_LOCK(
        this, __tsan_mutex_try_lock | (res ? 0 : __tsan_mutex_try_lock_failed),
        0);
    return res;
  }

  inline void Unlock() ABSL_UNLOCK_FUNCTION() {
    ABSL_TSAN_MUTEX_PRE_UNLOCK(this, 0);
    uint32_t lock_value = lockword_.load(std::memory_order_relaxed);
    lock_value = lockword_.exchange(lock_value & kSpinLockCooperative,
                                    std::memory_order_release);

    if ((lock_value & kSpinLockDisabledScheduling) != 0) {
      base_internal::SchedulingGuard::EnableRescheduling(true);
    }
    if ((lock_value & kWaitTimeMask) != 0) {



      SlowUnlock(lock_value);
    }
    ABSL_TSAN_MUTEX_POST_UNLOCK(this, 0);
  }



  inline bool IsHeld() const {
    return (lockword_.load(std::memory_order_relaxed) & kSpinLockHeld) != 0;
  }


  inline void AssertHeld() const ABSL_ASSERT_EXCLUSIVE_LOCK() {
    if (!IsHeld()) {
      ABSL_RAW_LOG(FATAL, "thread should hold the lock on SpinLock");
    }
  }

 protected:



  static uint32_t EncodeWaitCycles(int64_t wait_start_time,
                                   int64_t wait_end_time);

  static int64_t DecodeWaitCycles(uint32_t lock_value);

  friend struct SpinLockTest;

 private:


















  static constexpr uint32_t kSpinLockHeld = 1;
  static constexpr uint32_t kSpinLockCooperative = 2;
  static constexpr uint32_t kSpinLockDisabledScheduling = 4;
  static constexpr uint32_t kSpinLockSleeper = 8;

  static constexpr uint32_t kWaitTimeMask =
      ~(kSpinLockHeld | kSpinLockCooperative | kSpinLockDisabledScheduling);

  static constexpr bool IsCooperative(
      base_internal::SchedulingMode scheduling_mode) {
    return scheduling_mode == base_internal::SCHEDULE_COOPERATIVE_AND_KERNEL;
  }

  uint32_t TryLockInternal(uint32_t lock_value, uint32_t wait_cycles);
  void SlowLock() ABSL_ATTRIBUTE_COLD;
  void SlowUnlock(uint32_t lock_value) ABSL_ATTRIBUTE_COLD;
  uint32_t SpinLoop();

  inline bool TryLockImpl() {
    uint32_t lock_value = lockword_.load(std::memory_order_relaxed);
    return (TryLockInternal(lock_value, 0) & kSpinLockHeld) == 0;
  }

  std::atomic<uint32_t> lockword_;

  SpinLock(const SpinLock&) = delete;
  SpinLock& operator=(const SpinLock&) = delete;
};

// the duration of a C++ scope.
class ABSL_SCOPED_LOCKABLE SpinLockHolder {
 public:
  inline explicit SpinLockHolder(SpinLock* l) ABSL_EXCLUSIVE_LOCK_FUNCTION(l)
      : lock_(l) {
    l->Lock();
  }
  inline ~SpinLockHolder() ABSL_UNLOCK_FUNCTION() { lock_->Unlock(); }

  SpinLockHolder(const SpinLockHolder&) = delete;
  SpinLockHolder& operator=(const SpinLockHolder&) = delete;

 private:
  SpinLock* lock_;
};

//
// The function pointer registered here will be called whenever a spinlock is
// contended.  The callback is given an opaque handle to the contended spinlock
// and the number of wait cycles.  This is thread-safe, but only a single
// profiler can be registered.  It is an error to call this function multiple
// times with different arguments.
void RegisterSpinLockProfiler(void (*fn)(const void* lock,
                                         int64_t wait_cycles));

// Public interface ends here.
//------------------------------------------------------------------------------

// Otherwise, returns last observed value for lockword_.
inline uint32_t SpinLock::TryLockInternal(uint32_t lock_value,
                                          uint32_t wait_cycles) {
  if ((lock_value & kSpinLockHeld) != 0) {
    return lock_value;
  }

  uint32_t sched_disabled_bit = 0;
  if ((lock_value & kSpinLockCooperative) == 0) {


    if (base_internal::SchedulingGuard::DisableRescheduling()) {
      sched_disabled_bit = kSpinLockDisabledScheduling;
    }
  }

  if (!lockword_.compare_exchange_strong(
          lock_value,
          kSpinLockHeld | lock_value | wait_cycles | sched_disabled_bit,
          std::memory_order_acquire, std::memory_order_relaxed)) {
    base_internal::SchedulingGuard::EnableRescheduling(sched_disabled_bit != 0);
  }

  return lock_value;
}

}  // namespace base_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_BASE_INTERNAL_SPINLOCK_H_
