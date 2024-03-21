// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_AT_EXIT_H_
#define BASE_AT_EXIT_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/containers/stack.h"
#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"

namespace base {

// we control when the callbacks are executed. Under Windows for a DLL they
// happen at a really bad time and under the loader lock. This facility is
// mostly used by base::Singleton.
//
// The usage is simple. Early in the main() or WinMain() scope create an
// AtExitManager object on the stack:
// int main(...) {
//    base::AtExitManager exit_manager;
//
// }
// When the exit_manager object goes out of scope, all the registered
// callbacks and singleton destructors will be called.

class BASE_EXPORT AtExitManager {
 public:
  typedef void (*AtExitCallbackType)(void*);

  AtExitManager();


  ~AtExitManager();


  static void RegisterCallback(AtExitCallbackType func, void* param);

  static void RegisterTask(base::OnceClosure task);


  static void ProcessCallbacksNow();


  static void DisableAllAtExitManagers();

 protected:




  explicit AtExitManager(bool shadow);

 private:
  base::Lock lock_;

  base::stack<base::OnceClosure> stack_ GUARDED_BY(lock_);

#if DCHECK_IS_ON()
  bool processing_callbacks_ GUARDED_BY(lock_) = false;
#endif

  AtExitManager* const next_manager_;

  DISALLOW_COPY_AND_ASSIGN(AtExitManager);
};

#if defined(UNIT_TEST)
class ShadowingAtExitManager : public AtExitManager {
 public:
  ShadowingAtExitManager() : AtExitManager(true) {}
};
#endif  // defined(UNIT_TEST)

}  // namespace base

#endif  // BASE_AT_EXIT_H_
