// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POST_TASK_AND_REPLY_WITH_RESULT_INTERNAL_H_
#define BASE_POST_TASK_AND_REPLY_WITH_RESULT_INTERNAL_H_

#include <memory>
#include <utility>

#include "base/callback.h"

namespace base {

namespace internal {

// one that returns via an output parameter.
template <typename ReturnType>
void ReturnAsParamAdapter(OnceCallback<ReturnType()> func,
                          std::unique_ptr<ReturnType>* result) {
  result->reset(new ReturnType(std::move(func).Run()));
}

template <typename TaskReturnType, typename ReplyArgType>
void ReplyAdapter(OnceCallback<void(ReplyArgType)> callback,
                  std::unique_ptr<TaskReturnType>* result) {
  DCHECK(result->get());
  std::move(callback).Run(std::move(**result));
}

}  // namespace internal

}  // namespace base

#endif  // BASE_POST_TASK_AND_REPLY_WITH_RESULT_INTERNAL_H_
