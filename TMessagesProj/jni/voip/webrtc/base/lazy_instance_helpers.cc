// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/lazy_instance_helpers.h"

#include "base/at_exit.h"
#include "base/atomicops.h"
#include "base/threading/platform_thread.h"

namespace base {
namespace internal {

bool NeedsLazyInstance(subtle::AtomicWord* state) {





  if (subtle::NoBarrier_CompareAndSwap(state, 0, kLazyInstanceStateCreating) ==
      0) {

    return true;
  }





  if (subtle::Acquire_Load(state) == kLazyInstanceStateCreating) {
    const base::TimeTicks start = base::TimeTicks::Now();
    do {
      const base::TimeDelta elapsed = base::TimeTicks::Now() - start;




      if (elapsed < TimeDelta::FromMilliseconds(1))
        PlatformThread::YieldCurrentThread();
      else
        PlatformThread::Sleep(TimeDelta::FromMilliseconds(1));
    } while (subtle::Acquire_Load(state) == kLazyInstanceStateCreating);
  }

  return false;
}

void CompleteLazyInstance(subtle::AtomicWord* state,
                          subtle::AtomicWord new_instance,
                          void (*destructor)(void*),
                          void* destructor_arg) {



  subtle::Release_Store(state, new_instance);

  if (new_instance && destructor)
    AtExitManager::RegisterCallback(destructor, destructor_arg);
}

}  // namespace internal
}  // namespace base
