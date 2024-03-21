// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_LAZY_INSTANCE_INTERNAL_H_
#define BASE_LAZY_INSTANCE_INTERNAL_H_

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/logging.h"

// lazy construction.

namespace base {
namespace internal {

// kLazyInstanceStateCreating means the spinlock is being held for creation.
constexpr subtle::AtomicWord kLazyInstanceStateCreating = 1;

// If so returns true otherwise if another thread has beat us, waits for
// instance to be created and returns false.
BASE_EXPORT bool NeedsLazyInstance(subtle::AtomicWord* state);

// called to register the dtor to be called at program exit and to update the
// atomic state to hold the |new_instance|
BASE_EXPORT void CompleteLazyInstance(subtle::AtomicWord* state,
                                      subtle::AtomicWord new_instance,
                                      void (*destructor)(void*),
                                      void* destructor_arg);

}  // namespace internal

namespace subtle {

// |creator_func(creator_arg)|, stores it into |state| and registers
// |destructor(destructor_arg)| to be called when the current AtExitManager goes
// out of scope. Then, returns the value stored in |state|. It is safe to have
// concurrent calls to this function with the same |state|. |creator_func| may
// return nullptr if it doesn't want to create an instance anymore (e.g. on
// shutdown), it is from then on required to return nullptr to all callers (ref.
// StaticMemorySingletonTraits). In that case, callers need to synchronize
// before |creator_func| may return a non-null instance again (ref.
// StaticMemorySingletonTraits::ResurectForTesting()).
// Implementation note on |creator_func/creator_arg|. It makes for ugly adapters
// but it avoids redundant template instantiations (e.g. saves 27KB in
// chrome.dll) because linker is able to fold these for multiple Types but
// couldn't with the more advanced CreatorFunc template type which in turn
// improves code locality (and application startup) -- ref.
// https://chromium-review.googlesource.com/c/chromium/src/+/530984/5/base/lazy_instance.h#140,
// worsened by https://chromium-review.googlesource.com/c/chromium/src/+/868013
// and caught then as https://crbug.com/804034.
template <typename Type>
Type* GetOrCreateLazyPointer(subtle::AtomicWord* state,
                             Type* (*creator_func)(void*),
                             void* creator_arg,
                             void (*destructor)(void*),
                             void* destructor_arg) {
  DCHECK(state);
  DCHECK(creator_func);


  constexpr subtle::AtomicWord kLazyInstanceCreatedMask =
      ~internal::kLazyInstanceStateCreating;






  subtle::AtomicWord instance = subtle::Acquire_Load(state);
  if (!(instance & kLazyInstanceCreatedMask)) {
    if (internal::NeedsLazyInstance(state)) {


      instance =
          reinterpret_cast<subtle::AtomicWord>((*creator_func)(creator_arg));
      internal::CompleteLazyInstance(state, instance, destructor,
                                     destructor_arg);
    } else {



      instance = subtle::Acquire_Load(state);
      DCHECK(instance & kLazyInstanceCreatedMask);
    }
  }
  return reinterpret_cast<Type*>(instance);
}

}  // namespace subtle

}  // namespace base

#endif  // BASE_LAZY_INSTANCE_INTERNAL_H_
