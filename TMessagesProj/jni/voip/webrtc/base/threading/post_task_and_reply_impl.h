// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef BASE_THREADING_POST_TASK_AND_REPLY_IMPL_H_
#define BASE_THREADING_POST_TASK_AND_REPLY_IMPL_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/location.h"

namespace base {
namespace internal {

// custom execution context.
//
// If you're looking for a concrete implementation of PostTaskAndReply, you
// probably want base::TaskRunner or base/task/post_task.h
class BASE_EXPORT PostTaskAndReplyImpl {
 public:
  virtual ~PostTaskAndReplyImpl() = default;









  bool PostTaskAndReply(const Location& from_here,
                        OnceClosure task,
                        OnceClosure reply);

 private:
  virtual bool PostTask(const Location& from_here, OnceClosure task) = 0;
};

}  // namespace internal
}  // namespace base

#endif  // BASE_THREADING_POST_TASK_AND_REPLY_IMPL_H_
