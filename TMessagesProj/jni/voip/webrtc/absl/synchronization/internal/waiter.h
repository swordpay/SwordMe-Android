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

#ifndef ABSL_SYNCHRONIZATION_INTERNAL_WAITER_H_
#define ABSL_SYNCHRONIZATION_INTERNAL_WAITER_H_

#include "absl/base/config.h"

#ifdef _WIN32
#include <sdkddkver.h>
#else
#include <pthread.h>
#endif

#ifdef __linux__
#include <linux/futex.h>
#endif

#ifdef ABSL_HAVE_SEMAPHORE_H
#include <semaphore.h>
#endif

#include <atomic>
#include <cstdint>

#include "absl/base/internal/thread_identity.h"
#include "absl/synchronization/internal/futex.h"
#include "absl/synchronization/internal/kernel_timeout.h"

#define ABSL_WAITER_MODE_FUTEX 0
#define ABSL_WAITER_MODE_SEM 1
#define ABSL_WAITER_MODE_CONDVAR 2
#define ABSL_WAITER_MODE_WIN32 3

#if defined(ABSL_FORCE_WAITER_MODE)
#define ABSL_WAITER_MODE ABSL_FORCE_WAITER_MODE
#elif defined(_WIN32) && _WIN32_WINNT >= _WIN32_WINNT_VISTA
#define ABSL_WAITER_MODE ABSL_WAITER_MODE_WIN32
#elif defined(ABSL_INTERNAL_HAVE_FUTEX)
#define ABSL_WAITER_MODE ABSL_WAITER_MODE_FUTEX
#elif defined(ABSL_HAVE_SEMAPHORE_H)
#define ABSL_WAITER_MODE ABSL_WAITER_MODE_SEM
#else
#define ABSL_WAITER_MODE ABSL_WAITER_MODE_CONDVAR
#endif

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace synchronization_internal {

class Waiter {
 public:

  Waiter();

  Waiter(const Waiter&) = delete;
  Waiter& operator=(const Waiter&) = delete;



  bool Wait(KernelTimeout t);

  void Post();



  void Poke();

  static Waiter* GetWaiter(base_internal::ThreadIdentity* identity) {
    static_assert(
        sizeof(Waiter) <= sizeof(base_internal::ThreadIdentity::WaiterState),
        "Insufficient space for Waiter");
    return reinterpret_cast<Waiter*>(identity->waiter_state.data);
  }

#ifndef ABSL_HAVE_THREAD_SANITIZER
  static constexpr int kIdlePeriods = 60;
#else



  static const int kIdlePeriods = 1;
#endif

 private:




  ~Waiter() = delete;

#if ABSL_WAITER_MODE == ABSL_WAITER_MODE_FUTEX


  std::atomic<int32_t> futex_;
  static_assert(sizeof(int32_t) == sizeof(futex_), "Wrong size for futex");

#elif ABSL_WAITER_MODE == ABSL_WAITER_MODE_CONDVAR

  void InternalCondVarPoke();

  pthread_mutex_t mu_;
  pthread_cond_t cv_;
  int waiter_count_;
  int wakeup_count_;  // Unclaimed wakeups.

#elif ABSL_WAITER_MODE == ABSL_WAITER_MODE_SEM
  sem_t sem_;



  std::atomic<int> wakeups_;

#elif ABSL_WAITER_MODE == ABSL_WAITER_MODE_WIN32


  class WinHelper;

  void InternalCondVarPoke();





  alignas(void*) unsigned char mu_storage_[sizeof(void*)];
  alignas(void*) unsigned char cv_storage_[sizeof(void*)];
  int waiter_count_;
  int wakeup_count_;

#else
  #error Unknown ABSL_WAITER_MODE
#endif
};

}  // namespace synchronization_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_SYNCHRONIZATION_INTERNAL_WAITER_H_
