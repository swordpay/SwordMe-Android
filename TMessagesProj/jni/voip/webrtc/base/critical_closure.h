// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CRITICAL_CLOSURE_H_
#define BASE_CRITICAL_CLOSURE_H_

#include <utility>

#include "base/callback.h"
#include "base/macros.h"
#include "build/build_config.h"

#if defined(OS_IOS)
#include "base/bind.h"
#include "base/ios/scoped_critical_action.h"
#endif

namespace base {

namespace internal {

#if defined(OS_IOS)
// Returns true if multi-tasking is supported on this iOS device.
bool IsMultiTaskingSupported();

// when the application goes to the background by using
// |ios::ScopedCriticalAction|.
class CriticalClosure {
 public:
  explicit CriticalClosure(OnceClosure closure);
  ~CriticalClosure();
  void Run();

 private:
  ios::ScopedCriticalAction critical_action_;
  OnceClosure closure_;

  DISALLOW_COPY_AND_ASSIGN(CriticalClosure);
};
#endif  // defined(OS_IOS)

}  // namespace internal

// application goes to the background if possible on platforms where
// applications don't execute while backgrounded, otherwise the original task is
// returned.
//
// Example:
//   file_task_runner_->PostTask(
//       FROM_HERE,
//       MakeCriticalClosure(base::BindOnce(&WriteToDiskTask, path_, data)));
//
// Note new closures might be posted in this closure. If the new closures need
// background running time, |MakeCriticalClosure| should be applied on them
// before posting.
#if defined(OS_IOS)
inline OnceClosure MakeCriticalClosure(OnceClosure closure) {
  DCHECK(internal::IsMultiTaskingSupported());
  return base::BindOnce(
      &internal::CriticalClosure::Run,
      Owned(new internal::CriticalClosure(std::move(closure))));
}
#else  // defined(OS_IOS)
inline OnceClosure MakeCriticalClosure(OnceClosure closure) {


  return closure;
}
#endif  // defined(OS_IOS)

}  // namespace base

#endif  // BASE_CRITICAL_CLOSURE_H_
