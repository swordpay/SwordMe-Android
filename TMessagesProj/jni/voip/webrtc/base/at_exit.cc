// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"

#include <stddef.h>
#include <ostream>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"

namespace base {

// recent, and we should never have more than one outside of testing (for a
// statically linked version of this library).  Testing may use the shadow
// version of the constructor, and if we are building a dynamic library we may
// end up with multiple AtExitManagers on the same process.  We don't protect
// this for thread-safe access, since it will only be modified in testing.
static AtExitManager* g_top_manager = nullptr;

static bool g_disable_managers = false;

AtExitManager::AtExitManager() : next_manager_(g_top_manager) {
// If multiple modules instantiate AtExitManagers they'll end up living in this
// module... they have to coexist.
#if !defined(COMPONENT_BUILD)
  DCHECK(!g_top_manager);
#endif
  g_top_manager = this;
}

AtExitManager::~AtExitManager() {
  if (!g_top_manager) {
    NOTREACHED() << "Tried to ~AtExitManager without an AtExitManager";
    return;
  }
  DCHECK_EQ(this, g_top_manager);

  if (!g_disable_managers)
    ProcessCallbacksNow();
  g_top_manager = next_manager_;
}

void AtExitManager::RegisterCallback(AtExitCallbackType func, void* param) {
  DCHECK(func);
  RegisterTask(base::BindOnce(func, param));
}

void AtExitManager::RegisterTask(base::OnceClosure task) {
  if (!g_top_manager) {
    NOTREACHED() << "Tried to RegisterCallback without an AtExitManager";
    return;
  }

  AutoLock lock(g_top_manager->lock_);
#if DCHECK_IS_ON()
  DCHECK(!g_top_manager->processing_callbacks_);
#endif
  g_top_manager->stack_.push(std::move(task));
}

void AtExitManager::ProcessCallbacksNow() {
  if (!g_top_manager) {
    NOTREACHED() << "Tried to ProcessCallbacksNow without an AtExitManager";
    return;
  }



  base::stack<base::OnceClosure> tasks;
  {
    AutoLock lock(g_top_manager->lock_);
    tasks.swap(g_top_manager->stack_);
#if DCHECK_IS_ON()
    g_top_manager->processing_callbacks_ = true;
#endif
  }


  ScopedAllowCrossThreadRefCountAccess allow_cross_thread_ref_count_access;

  while (!tasks.empty()) {
    std::move(tasks.top()).Run();
    tasks.pop();
  }

#if DCHECK_IS_ON()
  AutoLock lock(g_top_manager->lock_);

  DCHECK(g_top_manager->stack_.empty());
  g_top_manager->processing_callbacks_ = false;
#endif
}

void AtExitManager::DisableAllAtExitManagers() {
  AutoLock lock(g_top_manager->lock_);
  g_disable_managers = true;
}

AtExitManager::AtExitManager(bool shadow) : next_manager_(g_top_manager) {
  DCHECK(shadow || !g_top_manager);
  g_top_manager = this;
}

}  // namespace base
