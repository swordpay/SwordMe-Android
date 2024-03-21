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

// runnability of a single thread, used internally by Mutex and CondVar.
//
// This is NOT a general-purpose synchronization mechanism, and should not be
// used directly by applications.  Applications should use Mutex and CondVar.
//
// The semantics of PerThreadSem are the same as that of a counting semaphore.
// Each thread maintains an abstract "count" value associated with its identity.

#ifndef ABSL_SYNCHRONIZATION_INTERNAL_PER_THREAD_SEM_H_
#define ABSL_SYNCHRONIZATION_INTERNAL_PER_THREAD_SEM_H_

#include <atomic>

#include "absl/base/internal/thread_identity.h"
#include "absl/synchronization/internal/create_thread_identity.h"
#include "absl/synchronization/internal/kernel_timeout.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

class Mutex;

namespace synchronization_internal {

class PerThreadSem {
 public:
  PerThreadSem() = delete;
  PerThreadSem(const PerThreadSem&) = delete;
  PerThreadSem& operator=(const PerThreadSem&) = delete;


  static void Tick(base_internal::ThreadIdentity* identity);










  static void SetThreadBlockedCounter(std::atomic<int> *counter);
  static std::atomic<int> *GetThreadBlockedCounter();

 private:


  static void Init(base_internal::ThreadIdentity* identity);

  static inline void Post(base_internal::ThreadIdentity* identity);



  static inline bool Wait(KernelTimeout t);

  friend class PerThreadSemTest;
  friend class absl::Mutex;
  friend void OneTimeInitThreadIdentity(absl::base_internal::ThreadIdentity*);
};

}  // namespace synchronization_internal
ABSL_NAMESPACE_END
}  // namespace absl

// gold linker.  This causes it to flag weak symbol overrides as ODR
// violations.  Because ODR only applies to C++ and not C,
// --detect-odr-violations ignores symbols not mangled with C++ names.
// By changing our extension points to be extern "C", we dodge this
// check.
extern "C" {
void ABSL_INTERNAL_C_SYMBOL(AbslInternalPerThreadSemPost)(
    absl::base_internal::ThreadIdentity* identity);
bool ABSL_INTERNAL_C_SYMBOL(AbslInternalPerThreadSemWait)(
    absl::synchronization_internal::KernelTimeout t);
}  // extern "C"

void absl::synchronization_internal::PerThreadSem::Post(
    absl::base_internal::ThreadIdentity* identity) {
  ABSL_INTERNAL_C_SYMBOL(AbslInternalPerThreadSemPost)(identity);
}

bool absl::synchronization_internal::PerThreadSem::Wait(
    absl::synchronization_internal::KernelTimeout t) {
  return ABSL_INTERNAL_C_SYMBOL(AbslInternalPerThreadSemWait)(t);
}

#endif  // ABSL_SYNCHRONIZATION_INTERNAL_PER_THREAD_SEM_H_
