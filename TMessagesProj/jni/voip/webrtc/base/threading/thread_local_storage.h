// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_THREAD_LOCAL_STORAGE_H_
#define BASE_THREADING_THREAD_LOCAL_STORAGE_H_

#include <stdint.h>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/macros.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include "base/win/windows_types.h"
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <pthread.h>
#endif

namespace ui {
class TLSDestructionCheckerForX11;
}

namespace base {

class SamplingHeapProfiler;

namespace debug {
class GlobalActivityTracker;
}  // namespace debug

namespace trace_event {
class MallocDumpProvider;
}  // namespace trace_event

namespace internal {

class ThreadLocalStorageTestInternal;

// PlatformThreadLocalStorage is a low-level abstraction of the OS's TLS
// interface. Instead, you should use one of the following:
// * ThreadLocalBoolean (from thread_local.h) for booleans.
// * ThreadLocalPointer (from thread_local.h) for pointers.
// * ThreadLocalStorage::StaticSlot/Slot for more direct control of the slot.
class BASE_EXPORT PlatformThreadLocalStorage {
 public:

#if defined(OS_WIN)
  typedef unsigned long TLSKey;
  enum : unsigned { TLS_KEY_OUT_OF_INDEXES = TLS_OUT_OF_INDEXES };
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  typedef pthread_key_t TLSKey;




  enum { TLS_KEY_OUT_OF_INDEXES = 0x7FFFFFFF };
#endif







  static bool AllocTLS(TLSKey* key);



  static void FreeTLS(TLSKey key);
  static void SetTLSValue(TLSKey key, void* value);
  static void* GetTLSValue(TLSKey key) {
#if defined(OS_WIN)
    return TlsGetValue(key);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
    return pthread_getspecific(key);
#endif
  }








#if defined(OS_WIN)


  static void OnThreadExit();
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)



  static void OnThreadExit(void* value);
#endif
};

}  // namespace internal

// an API for portability.
class BASE_EXPORT ThreadLocalStorage {
 public:



  typedef void (*TLSDestructorFunc)(void* value);









  class BASE_EXPORT Slot final {
   public:


    explicit Slot(TLSDestructorFunc destructor = nullptr);


    ~Slot();


    void* Get() const;


    void Set(void* value);

   private:
    void Initialize(TLSDestructorFunc destructor);
    void Free();

    static constexpr int kInvalidSlotValue = -1;
    int slot_ = kInvalidSlotValue;
    uint32_t version_ = 0;

    DISALLOW_COPY_AND_ASSIGN(Slot);
  };

 private:









  friend class SequenceCheckerImpl;
  friend class SamplingHeapProfiler;
  friend class ThreadCheckerImpl;
  friend class internal::ThreadLocalStorageTestInternal;
  friend class trace_event::MallocDumpProvider;
  friend class debug::GlobalActivityTracker;
  friend class ui::TLSDestructionCheckerForX11;
  static bool HasBeenDestroyed();

  DISALLOW_COPY_AND_ASSIGN(ThreadLocalStorage);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_LOCAL_STORAGE_H_
