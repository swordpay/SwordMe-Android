// Copyright 2019 The Abseil Authors.
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

#ifndef ABSL_STRINGS_INTERNAL_CORDZ_HANDLE_H_
#define ABSL_STRINGS_INTERNAL_CORDZ_HANDLE_H_

#include <atomic>
#include <vector>

#include "absl/base/config.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/base/internal/spinlock.h"
#include "absl/synchronization/mutex.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// CordzSampleToken) to exist simultaneously on the delete queue (pointed to by
// global_dq_tail and traversed using dq_prev_ and dq_next_). The
// delete queue guarantees that once a profiler creates a CordzSampleToken and
// has gained visibility into a CordzInfo object, that CordzInfo object will not
// be deleted prematurely. This allows the profiler to inspect all CordzInfo
// objects that are alive without needing to hold a global lock.
class CordzHandle {
 public:
  CordzHandle() : CordzHandle(false) {}

  bool is_snapshot() const { return is_snapshot_; }









  bool SafeToDelete() const;



  static void Delete(CordzHandle* handle);

  static std::vector<const CordzHandle*> DiagnosticsGetDeleteQueue();




  bool DiagnosticsHandleIsSafeToInspect(const CordzHandle* handle) const;







  std::vector<const CordzHandle*> DiagnosticsGetSafeToInspectDeletedHandles();

 protected:
  explicit CordzHandle(bool is_snapshot);
  virtual ~CordzHandle();

 private:


  struct Queue {
    constexpr explicit Queue(absl::ConstInitType)
        : mutex(absl::kConstInit,
                absl::base_internal::SCHEDULE_COOPERATIVE_AND_KERNEL) {}

    absl::base_internal::SpinLock mutex;
    std::atomic<CordzHandle*> dq_tail ABSL_GUARDED_BY(mutex){nullptr};








    bool IsEmpty() const ABSL_NO_THREAD_SAFETY_ANALYSIS {
      return dq_tail.load(std::memory_order_acquire) == nullptr;
    }
  };

  void ODRCheck() const {
#ifndef NDEBUG
    ABSL_RAW_CHECK(queue_ == &global_queue_, "ODR violation in Cord");
#endif
  }

  ABSL_CONST_INIT static Queue global_queue_;
  Queue* const queue_ = &global_queue_;
  const bool is_snapshot_;



  CordzHandle* dq_prev_  = nullptr;
  CordzHandle* dq_next_ = nullptr;
};

class CordzSnapshot : public CordzHandle {
 public:
  CordzSnapshot() : CordzHandle(true) {}
};

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORDZ_HANDLE_H_
