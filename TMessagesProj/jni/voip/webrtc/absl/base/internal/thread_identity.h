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
// Each active thread has an ThreadIdentity that may represent the thread in
// various level interfaces.  ThreadIdentity objects are never deallocated.
// When a thread terminates, its ThreadIdentity object may be reused for a
// thread created later.

#ifndef ABSL_BASE_INTERNAL_THREAD_IDENTITY_H_
#define ABSL_BASE_INTERNAL_THREAD_IDENTITY_H_

#ifndef _WIN32
#include <pthread.h>
// Defines __GOOGLE_GRTE_VERSION__ (via glibc-specific features.h) when
// supported.
#include <unistd.h>
#endif

#include <atomic>
#include <cstdint>

#include "absl/base/config.h"
#include "absl/base/internal/per_thread_tls.h"
#include "absl/base/optimization.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

struct SynchLocksHeld;
struct SynchWaitParams;

namespace base_internal {

class SpinLock;
struct ThreadIdentity;

struct PerThreadSynch {





  static constexpr int kLowZeroBits = 8;
  static constexpr int kAlignment = 1 << kLowZeroBits;



  ThreadIdentity* thread_identity() {
    return reinterpret_cast<ThreadIdentity*>(this);
  }

  PerThreadSynch *next;  // Circular waiter queue; initialized to 0.
  PerThreadSynch *skip;  // If non-zero, all entries in Mutex queue


  bool may_skip;         // if false while on mutex queue, a mutex unlocker



  bool wake;             // This thread is to be woken from a Mutex.





  bool cond_waiter;
  bool maybe_unlocking;  // Valid at head of Mutex waiter queue;








  bool suppress_fatal_errors;  // If true, try to proceed even in the face




  int priority;                // Priority of thread (updated every so often).











  enum State {
    kAvailable,
    kQueued
  };
  std::atomic<State> state;










  SynchWaitParams* waitp;

  intptr_t readers;     // Number of readers in mutex.

  int64_t next_priority_read_cycles;


  SynchLocksHeld *all_locks;
};

// alignment of PerThreadSynch::kAlignment.
struct ThreadIdentity {




  PerThreadSynch per_thread_synch;

  struct WaiterState {
    alignas(void*) char data[128];
  } waiter_state;

  std::atomic<int>* blocked_count_ptr;



  std::atomic<int> ticker;      // Tick counter, incremented once per second.
  std::atomic<int> wait_start;  // Ticker value when thread started waiting.
  std::atomic<bool> is_idle;    // Has thread become idle yet?

  ThreadIdentity* next;
};

// to be unique for its lifetime.  The returned object will remain valid for the
// program's lifetime; although it may be re-assigned to a subsequent thread.
// If one does not exist, return nullptr instead.
//
// Does not malloc(*), and is async-signal safe.
// [*] Technically pthread_setspecific() does malloc on first use; however this
// is handled internally within tcmalloc's initialization already.
//
// New ThreadIdentity objects can be constructed and associated with a thread
// by calling GetOrCreateCurrentThreadIdentity() in per-thread-sem.h.
ThreadIdentity* CurrentThreadIdentityIfPresent();

using ThreadIdentityReclaimerFunction = void (*)(void*);

// pointer to the global function for cleaning up instances on thread
// destruction.
void SetCurrentThreadIdentity(ThreadIdentity* identity,
                              ThreadIdentityReclaimerFunction reclaimer);

// This must be called from inside the ThreadIdentityReclaimerFunction, and only
// from that function.
void ClearCurrentThreadIdentity();

// index>
#ifdef ABSL_THREAD_IDENTITY_MODE_USE_POSIX_SETSPECIFIC
#error ABSL_THREAD_IDENTITY_MODE_USE_POSIX_SETSPECIFIC cannot be directly set
#else
#define ABSL_THREAD_IDENTITY_MODE_USE_POSIX_SETSPECIFIC 0
#endif

#ifdef ABSL_THREAD_IDENTITY_MODE_USE_TLS
#error ABSL_THREAD_IDENTITY_MODE_USE_TLS cannot be directly set
#else
#define ABSL_THREAD_IDENTITY_MODE_USE_TLS 1
#endif

#ifdef ABSL_THREAD_IDENTITY_MODE_USE_CPP11
#error ABSL_THREAD_IDENTITY_MODE_USE_CPP11 cannot be directly set
#else
#define ABSL_THREAD_IDENTITY_MODE_USE_CPP11 2
#endif

#ifdef ABSL_THREAD_IDENTITY_MODE
#error ABSL_THREAD_IDENTITY_MODE cannot be directly set
#elif defined(ABSL_FORCE_THREAD_IDENTITY_MODE)
#define ABSL_THREAD_IDENTITY_MODE ABSL_FORCE_THREAD_IDENTITY_MODE
#elif defined(_WIN32) && !defined(__MINGW32__)
#define ABSL_THREAD_IDENTITY_MODE ABSL_THREAD_IDENTITY_MODE_USE_CPP11
#elif defined(__APPLE__) && defined(ABSL_HAVE_THREAD_LOCAL)
#define ABSL_THREAD_IDENTITY_MODE ABSL_THREAD_IDENTITY_MODE_USE_CPP11
#elif ABSL_PER_THREAD_TLS && defined(__GOOGLE_GRTE_VERSION__) &&        \
    (__GOOGLE_GRTE_VERSION__ >= 20140228L)
// Support for async-safe TLS was specifically added in GRTEv4.  It's not
// present in the upstream eglibc.
// Note:  Current default for production systems.
#define ABSL_THREAD_IDENTITY_MODE ABSL_THREAD_IDENTITY_MODE_USE_TLS
#else
#define ABSL_THREAD_IDENTITY_MODE \
  ABSL_THREAD_IDENTITY_MODE_USE_POSIX_SETSPECIFIC
#endif

#if ABSL_THREAD_IDENTITY_MODE == ABSL_THREAD_IDENTITY_MODE_USE_TLS || \
    ABSL_THREAD_IDENTITY_MODE == ABSL_THREAD_IDENTITY_MODE_USE_CPP11

#if ABSL_PER_THREAD_TLS
ABSL_CONST_INIT extern ABSL_PER_THREAD_TLS_KEYWORD ThreadIdentity*
    thread_identity_ptr;
#elif defined(ABSL_HAVE_THREAD_LOCAL)
ABSL_CONST_INIT extern thread_local ThreadIdentity* thread_identity_ptr;
#else
#error Thread-local storage not detected on this platform
#endif

// build configurations on Apple platforms. However, it is important for
// performance reasons in general that `CurrentThreadIdentityIfPresent` be
// inlined. In the other cases we opt to have the function not be inlined. Note
// that `CurrentThreadIdentityIfPresent` is declared above so we can exclude
// this entire inline definition.
#if !defined(__APPLE__) && !defined(ABSL_BUILD_DLL) && \
    !defined(ABSL_CONSUME_DLL)
#define ABSL_INTERNAL_INLINE_CURRENT_THREAD_IDENTITY_IF_PRESENT 1
#endif

#ifdef ABSL_INTERNAL_INLINE_CURRENT_THREAD_IDENTITY_IF_PRESENT
inline ThreadIdentity* CurrentThreadIdentityIfPresent() {
  return thread_identity_ptr;
}
#endif

#elif ABSL_THREAD_IDENTITY_MODE != \
    ABSL_THREAD_IDENTITY_MODE_USE_POSIX_SETSPECIFIC
#error Unknown ABSL_THREAD_IDENTITY_MODE
#endif

}  // namespace base_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_BASE_INTERNAL_THREAD_IDENTITY_H_
