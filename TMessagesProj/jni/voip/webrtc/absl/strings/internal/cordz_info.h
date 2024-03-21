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

#ifndef ABSL_STRINGS_INTERNAL_CORDZ_INFO_H_
#define ABSL_STRINGS_INTERNAL_CORDZ_INFO_H_

#include <atomic>
#include <cstdint>
#include <functional>

#include "absl/base/config.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/base/internal/spinlock.h"
#include "absl/base/thread_annotations.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cordz_functions.h"
#include "absl/strings/internal/cordz_handle.h"
#include "absl/strings/internal/cordz_statistics.h"
#include "absl/strings/internal/cordz_update_tracker.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// If a Cord is alive, the CordzInfo will be in the global_cordz_infos map, and
// can also be retrieved via the linked list starting with
// global_cordz_infos_head and continued via the cordz_info_next() method. When
// a Cord has reached the end of its lifespan, the CordzInfo object will be
// migrated out of the global_cordz_infos list and the global_cordz_infos_map,
// and will either be deleted or appended to the global_delete_queue. If it is
// placed on the global_delete_queue, the CordzInfo object will be cleaned in
// the destructor of a CordzSampleToken object.
class ABSL_LOCKABLE CordzInfo : public CordzHandle {
 public:
  using MethodIdentifier = CordzUpdateTracker::MethodIdentifier;












  static void TrackCord(InlineData& cord, MethodIdentifier method);






  static void TrackCord(InlineData& cord, const InlineData& src,
                        MethodIdentifier method);



  static void MaybeTrackCord(InlineData& cord, MethodIdentifier method);


























  static void MaybeTrackCord(InlineData& cord, const InlineData& src,
                             MethodIdentifier method);





  void Untrack();

  static void MaybeUntrackCord(CordzInfo* info);

  CordzInfo() = delete;
  CordzInfo(const CordzInfo&) = delete;
  CordzInfo& operator=(const CordzInfo&) = delete;

  static CordzInfo* Head(const CordzSnapshot& snapshot)
      ABSL_NO_THREAD_SAFETY_ANALYSIS;

  CordzInfo* Next(const CordzSnapshot& snapshot) const
      ABSL_NO_THREAD_SAFETY_ANALYSIS;


  void Lock(MethodIdentifier method) ABSL_EXCLUSIVE_LOCK_FUNCTION(mutex_);



  void Unlock() ABSL_UNLOCK_FUNCTION(mutex_);

  void AssertHeld() ABSL_ASSERT_EXCLUSIVE_LOCK(mutex_);







  void SetCordRep(CordRep* rep);



  CordRep* RefCordRep() const ABSL_LOCKS_EXCLUDED(mutex_);

  CordRep* GetCordRepForTesting() const ABSL_NO_THREAD_SAFETY_ANALYSIS {
    return rep_;
  }

  void SetCordRepForTesting(CordRep* rep) ABSL_NO_THREAD_SAFETY_ANALYSIS {
    rep_ = rep;
  }






  absl::Span<void* const> GetStack() const;



  absl::Span<void* const> GetParentStack() const;



  CordzStatistics GetCordzStatistics() const;

 private:
  using SpinLock = absl::base_internal::SpinLock;
  using SpinLockHolder = ::absl::base_internal::SpinLockHolder;


  struct List {
    constexpr explicit List(absl::ConstInitType)
        : mutex(absl::kConstInit,
                absl::base_internal::SCHEDULE_COOPERATIVE_AND_KERNEL) {}

    SpinLock mutex;
    std::atomic<CordzInfo*> head ABSL_GUARDED_BY(mutex){nullptr};
  };

  static constexpr size_t kMaxStackDepth = 64;

  explicit CordzInfo(CordRep* rep, const CordzInfo* src,
                     MethodIdentifier method);
  ~CordzInfo() override;

  void UnsafeSetCordRep(CordRep* rep) ABSL_NO_THREAD_SAFETY_ANALYSIS;

  void Track();



  static MethodIdentifier GetParentMethod(const CordzInfo* src);




  static size_t FillParentStack(const CordzInfo* src, void** stack);

  void ODRCheck() const {
#ifndef NDEBUG
    ABSL_RAW_CHECK(list_ == &global_list_, "ODR violation in Cord");
#endif
  }



  static void MaybeTrackCordImpl(InlineData& cord, const InlineData& src,
                                 MethodIdentifier method);

  ABSL_CONST_INIT static List global_list_;
  List* const list_ = &global_list_;



  std::atomic<CordzInfo*> ci_prev_{nullptr};
  std::atomic<CordzInfo*> ci_next_{nullptr};

  mutable absl::Mutex mutex_;
  CordRep* rep_ ABSL_GUARDED_BY(mutex_);

  void* stack_[kMaxStackDepth];
  void* parent_stack_[kMaxStackDepth];
  const size_t stack_depth_;
  const size_t parent_stack_depth_;
  const MethodIdentifier method_;
  const MethodIdentifier parent_method_;
  CordzUpdateTracker update_tracker_;
  const absl::Time create_time_;
};

inline ABSL_ATTRIBUTE_ALWAYS_INLINE void CordzInfo::MaybeTrackCord(
    InlineData& cord, MethodIdentifier method) {
  if (ABSL_PREDICT_FALSE(cordz_should_profile())) {
    TrackCord(cord, method);
  }
}

inline ABSL_ATTRIBUTE_ALWAYS_INLINE void CordzInfo::MaybeTrackCord(
    InlineData& cord, const InlineData& src, MethodIdentifier method) {
  if (ABSL_PREDICT_FALSE(InlineData::is_either_profiled(cord, src))) {
    MaybeTrackCordImpl(cord, src, method);
  }
}

inline ABSL_ATTRIBUTE_ALWAYS_INLINE void CordzInfo::MaybeUntrackCord(
    CordzInfo* info) {
  if (ABSL_PREDICT_FALSE(info)) {
    info->Untrack();
  }
}

inline void CordzInfo::AssertHeld() ABSL_ASSERT_EXCLUSIVE_LOCK(mutex_) {
#ifndef NDEBUG
  mutex_.AssertHeld();
#endif
}

inline void CordzInfo::SetCordRep(CordRep* rep) {
  AssertHeld();
  rep_ = rep;
}

inline void CordzInfo::UnsafeSetCordRep(CordRep* rep) { rep_ = rep; }

inline CordRep* CordzInfo::RefCordRep() const ABSL_LOCKS_EXCLUDED(mutex_) {
  MutexLock lock(&mutex_);
  return rep_ ? CordRep::Ref(rep_) : nullptr;
}

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORDZ_INFO_H_
