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
// -----------------------------------------------------------------------------
// mutex.h
// -----------------------------------------------------------------------------
//
// This header file defines a `Mutex` -- a mutually exclusive lock -- and the
// most common type of synchronization primitive for facilitating locks on
// shared resources. A mutex is used to prevent multiple threads from accessing
// and/or writing to a shared resource concurrently.
//
// Unlike a `std::mutex`, the Abseil `Mutex` provides the following additional
// features:
//   * Conditional predicates intrinsic to the `Mutex` object
//   * Shared/reader locks, in addition to standard exclusive/writer locks
//   * Deadlock detection and debug support.
//
// The following helper classes are also defined within this file:
//
//  MutexLock - An RAII wrapper to acquire and release a `Mutex` for exclusive/
//              write access within the current scope.
//
//  ReaderMutexLock
//            - An RAII wrapper to acquire and release a `Mutex` for shared/read
//              access within the current scope.
//
//  WriterMutexLock
//            - Effectively an alias for `MutexLock` above, designed for use in
//              distinguishing reader and writer locks within code.
//
// In addition to simple mutex locks, this file also defines ways to perform
// locking under certain conditions.
//
//  Condition - (Preferred) Used to wait for a particular predicate that
//              depends on state protected by the `Mutex` to become true.
//  CondVar   - A lower-level variant of `Condition` that relies on
//              application code to explicitly signal the `CondVar` when
//              a condition has been met.
//
// See below for more information on using `Condition` or `CondVar`.
//
// Mutexes and mutex behavior can be quite complicated. The information within
// this header file is limited, as a result. Please consult the Mutex guide for
// more complete information and examples.

#ifndef ABSL_SYNCHRONIZATION_MUTEX_H_
#define ABSL_SYNCHRONIZATION_MUTEX_H_

#include <atomic>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>

#include "absl/base/const_init.h"
#include "absl/base/internal/identity.h"
#include "absl/base/internal/low_level_alloc.h"
#include "absl/base/internal/thread_identity.h"
#include "absl/base/internal/tsan_mutex_interface.h"
#include "absl/base/port.h"
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/internal/kernel_timeout.h"
#include "absl/synchronization/internal/per_thread_sem.h"
#include "absl/time/time.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

class Condition;
struct SynchWaitParams;

// Mutex
// -----------------------------------------------------------------------------
//
// A `Mutex` is a non-reentrant (aka non-recursive) Mutually Exclusive lock
// on some resource, typically a variable or data structure with associated
// invariants. Proper usage of mutexes prevents concurrent access by different
// threads to the same resource.
//
// A `Mutex` has two basic operations: `Mutex::Lock()` and `Mutex::Unlock()`.
// The `Lock()` operation *acquires* a `Mutex` (in a state known as an
// *exclusive* -- or write -- lock), while the `Unlock()` operation *releases* a
// Mutex. During the span of time between the Lock() and Unlock() operations,
// a mutex is said to be *held*. By design all mutexes support exclusive/write
// locks, as this is the most common way to use a mutex.
//
// The `Mutex` state machine for basic lock/unlock operations is quite simple:
//
// |                | Lock()     | Unlock() |
// |----------------+------------+----------|
// | Free           | Exclusive  | invalid  |
// | Exclusive      | blocks     | Free     |
//
// Attempts to `Unlock()` must originate from the thread that performed the
// corresponding `Lock()` operation.
//
// An "invalid" operation is disallowed by the API. The `Mutex` implementation
// is allowed to do anything on an invalid call, including but not limited to
// crashing with a useful error message, silently succeeding, or corrupting
// data structures. In debug mode, the implementation attempts to crash with a
// useful error message.
//
// `Mutex` is not guaranteed to be "fair" in prioritizing waiting threads; it
// is, however, approximately fair over long periods, and starvation-free for
// threads at the same priority.
//
// The lock/unlock primitives are now annotated with lock annotations
// defined in (base/thread_annotations.h). When writing multi-threaded code,
// you should use lock annotations whenever possible to document your lock
// synchronization policy. Besides acting as documentation, these annotations
// also help compilers or static analysis tools to identify and warn about
// issues that could potentially result in race conditions and deadlocks.
//
// For more information about the lock annotations, please see
// [Thread Safety Analysis](http://clang.llvm.org/docs/ThreadSafetyAnalysis.html)
// in the Clang documentation.
//
// See also `MutexLock`, below, for scoped `Mutex` acquisition.

class ABSL_LOCKABLE Mutex {
 public:






  Mutex();











  explicit constexpr Mutex(absl::ConstInitType);

  ~Mutex();




  void Lock() ABSL_EXCLUSIVE_LOCK_FUNCTION();




  void Unlock() ABSL_UNLOCK_FUNCTION();





  bool TryLock() ABSL_EXCLUSIVE_TRYLOCK_FUNCTION(true);








  void AssertHeld() const ABSL_ASSERT_EXCLUSIVE_LOCK();





































  void ReaderLock() ABSL_SHARED_LOCK_FUNCTION();





  void ReaderUnlock() ABSL_UNLOCK_FUNCTION();





  bool ReaderTryLock() ABSL_SHARED_TRYLOCK_FUNCTION(true);









  void AssertReaderHeld() const ABSL_ASSERT_SHARED_LOCK();









  void WriterLock() ABSL_EXCLUSIVE_LOCK_FUNCTION() { this->Lock(); }

  void WriterUnlock() ABSL_UNLOCK_FUNCTION() { this->Unlock(); }

  bool WriterTryLock() ABSL_EXCLUSIVE_TRYLOCK_FUNCTION(true) {
    return this->TryLock();
  }






































  void Await(const Condition &cond);








  void LockWhen(const Condition &cond) ABSL_EXCLUSIVE_LOCK_FUNCTION();

  void ReaderLockWhen(const Condition &cond) ABSL_SHARED_LOCK_FUNCTION();

  void WriterLockWhen(const Condition &cond) ABSL_EXCLUSIVE_LOCK_FUNCTION() {
    this->LockWhen(cond);
  }




















  bool AwaitWithTimeout(const Condition &cond, absl::Duration timeout);

  bool AwaitWithDeadline(const Condition &cond, absl::Time deadline);











  bool LockWhenWithTimeout(const Condition &cond, absl::Duration timeout)
      ABSL_EXCLUSIVE_LOCK_FUNCTION();
  bool ReaderLockWhenWithTimeout(const Condition &cond, absl::Duration timeout)
      ABSL_SHARED_LOCK_FUNCTION();
  bool WriterLockWhenWithTimeout(const Condition &cond, absl::Duration timeout)
      ABSL_EXCLUSIVE_LOCK_FUNCTION() {
    return this->LockWhenWithTimeout(cond, timeout);
  }











  bool LockWhenWithDeadline(const Condition &cond, absl::Time deadline)
      ABSL_EXCLUSIVE_LOCK_FUNCTION();
  bool ReaderLockWhenWithDeadline(const Condition &cond, absl::Time deadline)
      ABSL_SHARED_LOCK_FUNCTION();
  bool WriterLockWhenWithDeadline(const Condition &cond, absl::Time deadline)
      ABSL_EXCLUSIVE_LOCK_FUNCTION() {
    return this->LockWhenWithDeadline(cond, deadline);
  }
















  void EnableInvariantDebugging(void (*invariant)(void *), void *arg);







  void EnableDebugLog(const char *name);






  void ForgetDeadlockInfo();











  void AssertNotHeld() const;



  typedef const struct MuHowS *MuHow;













  static void InternalAttemptToUseMutexInFatalSignalHandler();

 private:
  std::atomic<intptr_t> mu_;  // The Mutex state.


  static void IncrementSynchSem(Mutex *mu, base_internal::PerThreadSynch *w);
  static bool DecrementSynchSem(Mutex *mu, base_internal::PerThreadSynch *w,
                                synchronization_internal::KernelTimeout t);

  void LockSlowLoop(SynchWaitParams *waitp, int flags);

  bool LockSlowWithDeadline(MuHow how, const Condition *cond,
                            synchronization_internal::KernelTimeout t,
                            int flags);
  void LockSlow(MuHow how, const Condition *cond,
                int flags) ABSL_ATTRIBUTE_COLD;

  void UnlockSlow(SynchWaitParams *waitp) ABSL_ATTRIBUTE_COLD;

  bool AwaitCommon(const Condition &cond,
                   synchronization_internal::KernelTimeout t);

  void TryRemove(base_internal::PerThreadSynch *s);

  void Block(base_internal::PerThreadSynch *s);

  base_internal::PerThreadSynch *Wakeup(base_internal::PerThreadSynch *w);

  friend class CondVar;   // for access to Trans()/Fer().
  void Trans(MuHow how);  // used for CondVar->Mutex transfer
  void Fer(
      base_internal::PerThreadSynch *w);  // used for CondVar->Mutex transfer

  Mutex(const volatile Mutex * /*ignored*/) {}  // NOLINT(runtime/explicit)

  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
};

// Mutex RAII Wrappers
// -----------------------------------------------------------------------------

//
// `MutexLock` is a helper class, which acquires and releases a `Mutex` via
// RAII.
//
// Example:
//
// Class Foo {
//  public:
//   Foo::Bar* Baz() {
//     MutexLock lock(&mu_);
//     ...
//     return bar;
//   }
//
// private:
//   Mutex mu_;
// };
class ABSL_SCOPED_LOCKABLE MutexLock {
 public:




  explicit MutexLock(Mutex *mu) ABSL_EXCLUSIVE_LOCK_FUNCTION(mu) : mu_(mu) {
    this->mu_->Lock();
  }



  explicit MutexLock(Mutex *mu, const Condition &cond)
      ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    this->mu_->LockWhen(cond);
  }

  MutexLock(const MutexLock &) = delete;  // NOLINT(runtime/mutex)
  MutexLock(MutexLock&&) = delete;  // NOLINT(runtime/mutex)
  MutexLock& operator=(const MutexLock&) = delete;
  MutexLock& operator=(MutexLock&&) = delete;

  ~MutexLock() ABSL_UNLOCK_FUNCTION() { this->mu_->Unlock(); }

 private:
  Mutex *const mu_;
};

//
// The `ReaderMutexLock` is a helper class, like `MutexLock`, which acquires and
// releases a shared lock on a `Mutex` via RAII.
class ABSL_SCOPED_LOCKABLE ReaderMutexLock {
 public:
  explicit ReaderMutexLock(Mutex *mu) ABSL_SHARED_LOCK_FUNCTION(mu) : mu_(mu) {
    mu->ReaderLock();
  }

  explicit ReaderMutexLock(Mutex *mu, const Condition &cond)
      ABSL_SHARED_LOCK_FUNCTION(mu)
      : mu_(mu) {
    mu->ReaderLockWhen(cond);
  }

  ReaderMutexLock(const ReaderMutexLock&) = delete;
  ReaderMutexLock(ReaderMutexLock&&) = delete;
  ReaderMutexLock& operator=(const ReaderMutexLock&) = delete;
  ReaderMutexLock& operator=(ReaderMutexLock&&) = delete;

  ~ReaderMutexLock() ABSL_UNLOCK_FUNCTION() { this->mu_->ReaderUnlock(); }

 private:
  Mutex *const mu_;
};

//
// The `WriterMutexLock` is a helper class, like `MutexLock`, which acquires and
// releases a write (exclusive) lock on a `Mutex` via RAII.
class ABSL_SCOPED_LOCKABLE WriterMutexLock {
 public:
  explicit WriterMutexLock(Mutex *mu) ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    mu->WriterLock();
  }

  explicit WriterMutexLock(Mutex *mu, const Condition &cond)
      ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    mu->WriterLockWhen(cond);
  }

  WriterMutexLock(const WriterMutexLock&) = delete;
  WriterMutexLock(WriterMutexLock&&) = delete;
  WriterMutexLock& operator=(const WriterMutexLock&) = delete;
  WriterMutexLock& operator=(WriterMutexLock&&) = delete;

  ~WriterMutexLock() ABSL_UNLOCK_FUNCTION() { this->mu_->WriterUnlock(); }

 private:
  Mutex *const mu_;
};

// Condition
// -----------------------------------------------------------------------------
//
// `Mutex` contains a number of member functions which take a `Condition` as an
// argument; clients can wait for conditions to become `true` before attempting
// to acquire the mutex. These sections are known as "condition critical"
// sections. To use a `Condition`, you simply need to construct it, and use
// within an appropriate `Mutex` member function; everything else in the
// `Condition` class is an implementation detail.
//
// A `Condition` is specified as a function pointer which returns a boolean.
// `Condition` functions should be pure functions -- their results should depend
// only on passed arguments, should not consult any external state (such as
// clocks), and should have no side-effects, aside from debug logging. Any
// objects that the function may access should be limited to those which are
// constant while the mutex is blocked on the condition (e.g. a stack variable),
// or objects of state protected explicitly by the mutex.
//
// No matter which construction is used for `Condition`, the underlying
// function pointer / functor / callable must not throw any
// exceptions. Correctness of `Mutex` / `Condition` is not guaranteed in
// the face of a throwing `Condition`. (When Abseil is allowed to depend
// on C++17, these function pointers will be explicitly marked
// `noexcept`; until then this requirement cannot be enforced in the
// type system.)
//
// Note: to use a `Condition`, you need only construct it and pass it to a
// suitable `Mutex' member function, such as `Mutex::Await()`, or to the
// constructor of one of the scope guard classes.
//
// Example using LockWhen/Unlock:
//
//   // assume count_ is not internal reference count
//   int count_ ABSL_GUARDED_BY(mu_);
//   Condition count_is_zero(+[](int *count) { return *count == 0; }, &count_);
//
//   mu_.LockWhen(count_is_zero);
//   // ...
//   mu_.Unlock();
//
// Example using a scope guard:
//
//   {
//     MutexLock lock(&mu_, count_is_zero);
//     // ...
//   }
//
// When multiple threads are waiting on exactly the same condition, make sure
// that they are constructed with the same parameters (same pointer to function
// + arg, or same pointer to object + method), so that the mutex implementation
// can avoid redundantly evaluating the same condition for each thread.
class Condition {
 public:

  Condition(bool (*func)(void *), void *arg);









  template<typename T>
  Condition(bool (*func)(T *), T *arg);








  template<typename T>
  Condition(T *object, bool (absl::internal::identity<T>::type::* method)());

  template<typename T>
  Condition(const T *object,
            bool (absl::internal::identity<T>::type::* method)() const);

  explicit Condition(const bool *cond);























  template <typename T, typename E = decltype(
      static_cast<bool (T::*)() const>(&T::operator()))>
  explicit Condition(const T *obj)
      : Condition(obj, static_cast<bool (T::*)() const>(&T::operator())) {}

  static const Condition kTrue;

  bool Eval() const;







  static bool GuaranteedEqual(const Condition *a, const Condition *b);

 private:







#ifndef _MSC_VER




  using MethodPtr = bool (Condition::*)();
  char callback_[sizeof(MethodPtr)] = {0};
#else



  char callback_[24] = {0};
#endif

  bool (*eval_)(const Condition*);

  void *arg_;

  static bool CallVoidPtrFunction(const Condition*);
  template <typename T> static bool CastAndCallFunction(const Condition* c);
  template <typename T> static bool CastAndCallMethod(const Condition* c);

  template <typename T>
  inline void StoreCallback(T callback) {
    static_assert(
        sizeof(callback) <= sizeof(callback_),
        "An overlarge pointer was passed as a callback to Condition.");
    std::memcpy(callback_, &callback, sizeof(callback));
  }

  template <typename T>
  inline void ReadCallback(T *callback) const {
    std::memcpy(callback, callback_, sizeof(*callback));
  }

  Condition();        // null constructor used only to create kTrue
};

// CondVar
// -----------------------------------------------------------------------------
//
// A condition variable, reflecting state evaluated separately outside of the
// `Mutex` object, which can be signaled to wake callers.
// This class is not normally needed; use `Mutex` member functions such as
// `Mutex::Await()` and intrinsic `Condition` abstractions. In rare cases
// with many threads and many conditions, `CondVar` may be faster.
//
// The implementation may deliver signals to any condition variable at
// any time, even when no call to `Signal()` or `SignalAll()` is made; as a
// result, upon being awoken, you must check the logical condition you have
// been waiting upon.
//
// Examples:
//
// Usage for a thread waiting for some condition C protected by mutex mu:
//       mu.Lock();
//       while (!C) { cv->Wait(&mu); }        // releases and reacquires mu
//       //  C holds; process data
//       mu.Unlock();
//
// Usage to wake T is:
//       mu.Lock();
//       // process data, possibly establishing C
//       if (C) { cv->Signal(); }
//       mu.Unlock();
//
// If C may be useful to more than one waiter, use `SignalAll()` instead of
// `Signal()`.
//
// With this implementation it is efficient to use `Signal()/SignalAll()` inside
// the locked region; this usage can make reasoning about your program easier.
//
class CondVar {
 public:


  CondVar();
  ~CondVar();







  void Wait(Mutex *mu);













  bool WaitWithTimeout(Mutex *mu, absl::Duration timeout);















  bool WaitWithDeadline(Mutex *mu, absl::Time deadline);



  void Signal();



  void SignalAll();





  void EnableDebugLog(const char *name);

 private:
  bool WaitCommon(Mutex *mutex, synchronization_internal::KernelTimeout t);
  void Remove(base_internal::PerThreadSynch *s);
  void Wakeup(base_internal::PerThreadSynch *w);
  std::atomic<intptr_t> cv_;  // Condition variable state.
  CondVar(const CondVar&) = delete;
  CondVar& operator=(const CondVar&) = delete;
};

//
// If you find yourself using one of these, consider instead using
// Mutex::Unlock() and/or if-statements for clarity.

//
// MutexLockMaybe is like MutexLock, but is a no-op when mu is null.
class ABSL_SCOPED_LOCKABLE MutexLockMaybe {
 public:
  explicit MutexLockMaybe(Mutex *mu) ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    if (this->mu_ != nullptr) {
      this->mu_->Lock();
    }
  }

  explicit MutexLockMaybe(Mutex *mu, const Condition &cond)
      ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    if (this->mu_ != nullptr) {
      this->mu_->LockWhen(cond);
    }
  }

  ~MutexLockMaybe() ABSL_UNLOCK_FUNCTION() {
    if (this->mu_ != nullptr) { this->mu_->Unlock(); }
  }

 private:
  Mutex *const mu_;
  MutexLockMaybe(const MutexLockMaybe&) = delete;
  MutexLockMaybe(MutexLockMaybe&&) = delete;
  MutexLockMaybe& operator=(const MutexLockMaybe&) = delete;
  MutexLockMaybe& operator=(MutexLockMaybe&&) = delete;
};

//
// ReleasableMutexLock is like MutexLock, but permits `Release()` of its
// mutex before destruction. `Release()` may be called at most once.
class ABSL_SCOPED_LOCKABLE ReleasableMutexLock {
 public:
  explicit ReleasableMutexLock(Mutex *mu) ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    this->mu_->Lock();
  }

  explicit ReleasableMutexLock(Mutex *mu, const Condition &cond)
      ABSL_EXCLUSIVE_LOCK_FUNCTION(mu)
      : mu_(mu) {
    this->mu_->LockWhen(cond);
  }

  ~ReleasableMutexLock() ABSL_UNLOCK_FUNCTION() {
    if (this->mu_ != nullptr) { this->mu_->Unlock(); }
  }

  void Release() ABSL_UNLOCK_FUNCTION();

 private:
  Mutex *mu_;
  ReleasableMutexLock(const ReleasableMutexLock&) = delete;
  ReleasableMutexLock(ReleasableMutexLock&&) = delete;
  ReleasableMutexLock& operator=(const ReleasableMutexLock&) = delete;
  ReleasableMutexLock& operator=(ReleasableMutexLock&&) = delete;
};

inline Mutex::Mutex() : mu_(0) {
  ABSL_TSAN_MUTEX_CREATE(this, __tsan_mutex_not_static);
}

inline constexpr Mutex::Mutex(absl::ConstInitType) : mu_(0) {}

inline CondVar::CondVar() : cv_(0) {}

template <typename T>
bool Condition::CastAndCallMethod(const Condition *c) {
  T *object = static_cast<T *>(c->arg_);
  bool (T::*method_pointer)();
  c->ReadCallback(&method_pointer);
  return (object->*method_pointer)();
}

template <typename T>
bool Condition::CastAndCallFunction(const Condition *c) {
  bool (*function)(T *);
  c->ReadCallback(&function);
  T *argument = static_cast<T *>(c->arg_);
  return (*function)(argument);
}

template <typename T>
inline Condition::Condition(bool (*func)(T *), T *arg)
    : eval_(&CastAndCallFunction<T>),
      arg_(const_cast<void *>(static_cast<const void *>(arg))) {
  static_assert(sizeof(&func) <= sizeof(callback_),
                "An overlarge function pointer was passed to Condition.");
  StoreCallback(func);
}

template <typename T>
inline Condition::Condition(T *object,
                            bool (absl::internal::identity<T>::type::*method)())
    : eval_(&CastAndCallMethod<T>),
      arg_(object) {
  static_assert(sizeof(&method) <= sizeof(callback_),
                "An overlarge method pointer was passed to Condition.");
  StoreCallback(method);
}

template <typename T>
inline Condition::Condition(const T *object,
                            bool (absl::internal::identity<T>::type::*method)()
                                const)
    : eval_(&CastAndCallMethod<T>),
      arg_(reinterpret_cast<void *>(const_cast<T *>(object))) {
  StoreCallback(method);
}

//
// The function pointer registered here will be called whenever a mutex is
// contended.  The callback is given the cycles for which waiting happened (as
// measured by //absl/base/internal/cycleclock.h, and which may not
// be real "cycle" counts.)
//
// Calls to this function do not race or block, but there is no ordering
// guaranteed between calls to this function and call to the provided hook.
// In particular, the previously registered hook may still be called for some
// time after this function returns.
void RegisterMutexProfiler(void (*fn)(int64_t wait_cycles));

//
// The function pointer registered here will be called whenever a mutex is
// contended.  The callback is given an opaque handle to the contended mutex,
// an event name, and the number of wait cycles (as measured by
// //absl/base/internal/cycleclock.h, and which may not be real
// "cycle" counts.)
//
// The only event name currently sent is "slow release".
//
// This has the same memory ordering concerns as RegisterMutexProfiler() above.
void RegisterMutexTracer(void (*fn)(const char *msg, const void *obj,
                                    int64_t wait_cycles));

// into a single interface, since they are only ever called in pairs.

//
// The function pointer registered here will be called here on various CondVar
// events.  The callback is given an opaque handle to the CondVar object and
// a string identifying the event.  This is thread-safe, but only a single
// tracer can be registered.
//
// Events that can be sent are "Wait", "Unwait", "Signal wakeup", and
// "SignalAll wakeup".
//
// This has the same memory ordering concerns as RegisterMutexProfiler() above.
void RegisterCondVarTracer(void (*fn)(const char *msg, const void *cv));

//
// 'pc' is the program counter being symbolized, 'out' is the buffer to write
// into, and 'out_size' is the size of the buffer.  This function can return
// false if symbolizing failed, or true if a NUL-terminated symbol was written
// to 'out.'
//
// This has the same memory ordering concerns as RegisterMutexProfiler() above.
//
// DEPRECATED: The default symbolizer function is absl::Symbolize() and the
// ability to register a different hook for symbolizing stack traces will be
// removed on or after 2023-05-01.
ABSL_DEPRECATED("absl::RegisterSymbolizer() is deprecated and will be removed "
                "on or after 2023-05-01")
void RegisterSymbolizer(bool (*fn)(const void *pc, char *out, int out_size));

//
// Enable or disable global support for Mutex invariant debugging.  If enabled,
// then invariant predicates can be registered per-Mutex for debug checking.
// See Mutex::EnableInvariantDebugging().
void EnableMutexInvariantDebugging(bool enabled);

// implementation will keep track of lock ordering and complain (or optionally
// crash) if a cycle is detected in the acquired-before graph.

enum class OnDeadlockCycle {
  kIgnore,  // Neither report on nor attempt to track cycles in lock ordering
  kReport,  // Report lock cycles to stderr when detected
  kAbort,  // Report lock cycles to stderr when detected, then abort
};

//
// Enable or disable global support for detection of potential deadlocks
// due to Mutex lock ordering inversions.  When set to 'kIgnore', tracking of
// lock ordering is disabled.  Otherwise, in debug builds, a lock ordering graph
// will be maintained internally, and detected cycles will be reported in
// the manner chosen here.
void SetMutexDeadlockDetectionMode(OnDeadlockCycle mode);

ABSL_NAMESPACE_END
}  // namespace absl

// gold linker.  This causes it to flag weak symbol overrides as ODR
// violations.  Because ODR only applies to C++ and not C,
// --detect-odr-violations ignores symbols not mangled with C++ names.
// By changing our extension points to be extern "C", we dodge this
// check.
extern "C" {
void ABSL_INTERNAL_C_SYMBOL(AbslInternalMutexYield)();
}  // extern "C"

#endif  // ABSL_SYNCHRONIZATION_MUTEX_H_
