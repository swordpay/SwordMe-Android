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

#include "absl/base/internal/spinlock.h"

#include <algorithm>
#include <atomic>
#include <limits>

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/internal/atomic_hook.h"
#include "absl/base/internal/cycleclock.h"
#include "absl/base/internal/spinlock_wait.h"
#include "absl/base/internal/sysinfo.h" /* For NumCPUs() */
#include "absl/base/call_once.h"

//  31..00: [............................3][2][1][0]
//
//     [0]: kSpinLockHeld
//     [1]: kSpinLockCooperative
//     [2]: kSpinLockDisabledScheduling
// [31..3]: ONLY kSpinLockSleeper OR
//          Wait time in cycles >> PROFILE_TIMESTAMP_SHIFT
//
// Detailed descriptions:
//
// Bit [0]: The lock is considered held iff kSpinLockHeld is set.
//
// Bit [1]: Eligible waiters (e.g. Fibers) may co-operatively reschedule when
//          contended iff kSpinLockCooperative is set.
//
// Bit [2]: This bit is exclusive from bit [1].  It is used only by a
//          non-cooperative lock.  When set, indicates that scheduling was
//          successfully disabled when the lock was acquired.  May be unset,
//          even if non-cooperative, if a ThreadIdentity did not yet exist at
//          time of acquisition.
//
// Bit [3]: If this is the only upper bit ([31..3]) set then this lock was
//          acquired without contention, however, at least one waiter exists.
//
//          Otherwise, bits [31..3] represent the time spent by the current lock
//          holder to acquire the lock.  There may be outstanding waiter(s).

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace base_internal {

ABSL_INTERNAL_ATOMIC_HOOK_ATTRIBUTES static base_internal::AtomicHook<void (*)(
    const void *lock, int64_t wait_cycles)>
    submit_profile_data;

void RegisterSpinLockProfiler(void (*fn)(const void *contendedlock,
                                         int64_t wait_cycles)) {
  submit_profile_data.Store(fn);
}

#ifdef ABSL_INTERNAL_NEED_REDUNDANT_CONSTEXPR_DECL
// Static member variable definitions.
constexpr uint32_t SpinLock::kSpinLockHeld;
constexpr uint32_t SpinLock::kSpinLockCooperative;
constexpr uint32_t SpinLock::kSpinLockDisabledScheduling;
constexpr uint32_t SpinLock::kSpinLockSleeper;
constexpr uint32_t SpinLock::kWaitTimeMask;
#endif

SpinLock::SpinLock(base_internal::SchedulingMode mode)
    : lockword_(IsCooperative(mode) ? kSpinLockCooperative : 0) {
  ABSL_TSAN_MUTEX_CREATE(this, __tsan_mutex_not_static);
}

// (adaptive_spin_count loop iterations). The last value read from the lock
// is returned from the method.
uint32_t SpinLock::SpinLoop() {


  ABSL_CONST_INIT static absl::once_flag init_adaptive_spin_count;
  ABSL_CONST_INIT static int adaptive_spin_count = 0;
  base_internal::LowLevelCallOnce(&init_adaptive_spin_count, []() {
    adaptive_spin_count = base_internal::NumCPUs() > 1 ? 1000 : 1;
  });

  int c = adaptive_spin_count;
  uint32_t lock_value;
  do {
    lock_value = lockword_.load(std::memory_order_relaxed);
  } while ((lock_value & kSpinLockHeld) != 0 && --c > 0);
  return lock_value;
}

void SpinLock::SlowLock() {
  uint32_t lock_value = SpinLoop();
  lock_value = TryLockInternal(lock_value, 0);
  if ((lock_value & kSpinLockHeld) == 0) {
    return;
  }

  base_internal::SchedulingMode scheduling_mode;
  if ((lock_value & kSpinLockCooperative) != 0) {
    scheduling_mode = base_internal::SCHEDULE_COOPERATIVE_AND_KERNEL;
  } else {
    scheduling_mode = base_internal::SCHEDULE_KERNEL_ONLY;
  }




  int64_t wait_start_time = CycleClock::Now();
  uint32_t wait_cycles = 0;
  int lock_wait_call_count = 0;
  while ((lock_value & kSpinLockHeld) != 0) {


    if ((lock_value & kWaitTimeMask) == 0) {




      if (lockword_.compare_exchange_strong(
              lock_value, lock_value | kSpinLockSleeper,
              std::memory_order_relaxed, std::memory_order_relaxed)) {



        lock_value |= kSpinLockSleeper;
      } else if ((lock_value & kSpinLockHeld) == 0) {



        lock_value = TryLockInternal(lock_value, wait_cycles);
        continue;   // Skip the delay at the end of the loop.
      } else if ((lock_value & kWaitTimeMask) == 0) {






        continue;
      }
    }


    ABSL_TSAN_MUTEX_PRE_DIVERT(this, 0);

    base_internal::SpinLockDelay(&lockword_, lock_value, ++lock_wait_call_count,
                                 scheduling_mode);
    ABSL_TSAN_MUTEX_POST_DIVERT(this, 0);


    lock_value = SpinLoop();
    wait_cycles = EncodeWaitCycles(wait_start_time, CycleClock::Now());
    lock_value = TryLockInternal(lock_value, wait_cycles);
  }
}

void SpinLock::SlowUnlock(uint32_t lock_value) {
  base_internal::SpinLockWake(&lockword_,
                              false);  // wake waiter if necessary



  if ((lock_value & kWaitTimeMask) != kSpinLockSleeper) {
    const int64_t wait_cycles = DecodeWaitCycles(lock_value);
    ABSL_TSAN_MUTEX_PRE_DIVERT(this, 0);
    submit_profile_data(this, wait_cycles);
    ABSL_TSAN_MUTEX_POST_DIVERT(this, 0);
  }
}

// acquire this lock.  This is reported by contentionz profiling.  Since the
// lower bits of the cycle counter wrap very quickly on high-frequency
// processors we divide to reduce the granularity to 2^kProfileTimestampShift
// sized units.  On a 4Ghz machine this will lose track of wait times greater
// than (2^29/4 Ghz)*128 =~ 17.2 seconds.  Such waits should be extremely rare.
static constexpr int kProfileTimestampShift = 7;

static constexpr int kLockwordReservedShift = 3;

uint32_t SpinLock::EncodeWaitCycles(int64_t wait_start_time,
                                    int64_t wait_end_time) {
  static const int64_t kMaxWaitTime =
      std::numeric_limits<uint32_t>::max() >> kLockwordReservedShift;
  int64_t scaled_wait_time =
      (wait_end_time - wait_start_time) >> kProfileTimestampShift;


  uint32_t clamped = static_cast<uint32_t>(
      std::min(scaled_wait_time, kMaxWaitTime) << kLockwordReservedShift);

  if (clamped == 0) {
    return kSpinLockSleeper;  // Just wake waiters, but don't record contention.
  }

  const uint32_t kMinWaitTime =
      kSpinLockSleeper + (1 << kLockwordReservedShift);
  if (clamped == kSpinLockSleeper) {
    return kMinWaitTime;
  }
  return clamped;
}

int64_t SpinLock::DecodeWaitCycles(uint32_t lock_value) {

  const int64_t scaled_wait_time =
      static_cast<uint32_t>(lock_value & kWaitTimeMask);
  return scaled_wait_time << (kProfileTimestampShift - kLockwordReservedShift);
}

}  // namespace base_internal
ABSL_NAMESPACE_END
}  // namespace absl
