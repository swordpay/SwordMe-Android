// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/sequence_manager/associated_thread_id.h"

namespace base {
namespace sequence_manager {
namespace internal {

AssociatedThreadId::AssociatedThreadId() = default;
AssociatedThreadId::~AssociatedThreadId() = default;

void AssociatedThreadId::BindToCurrentThread() {


  auto current_thread_id = PlatformThread::CurrentId();
  auto prev_thread_id =
      thread_id_.exchange(current_thread_id, std::memory_order_release);
  ANALYZER_ALLOW_UNUSED(prev_thread_id);
  DCHECK(prev_thread_id == current_thread_id ||
         prev_thread_id == kInvalidThreadId);

  DETACH_FROM_THREAD(thread_checker);
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker);

  DETACH_FROM_SEQUENCE(sequence_checker);
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker);
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base