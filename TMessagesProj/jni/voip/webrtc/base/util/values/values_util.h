// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_UTIL_VALUES_VALUES_UTIL_H_
#define BASE_UTIL_VALUES_VALUES_UTIL_H_

#include "base/optional.h"
#include "base/time/time.h"
#include "base/values.h"

namespace util {

// base::Time to numeric string base::Values.
// Because base::TimeDelta and base::Time share the same internal representation
// as int64_t they are stored using the exact same numeric string format.

base::Value Int64ToValue(int64_t integer);
base::Optional<int64_t> ValueToInt64(const base::Value* value);
base::Optional<int64_t> ValueToInt64(const base::Value& value);

base::Value TimeDeltaToValue(base::TimeDelta time_delta);
base::Optional<base::TimeDelta> ValueToTimeDelta(const base::Value* value);
base::Optional<base::TimeDelta> ValueToTimeDelta(const base::Value& value);

base::Value TimeToValue(base::Time time);
base::Optional<base::Time> ValueToTime(const base::Value* value);
base::Optional<base::Time> ValueToTime(const base::Value& value);

}  // namespace util

#endif  // BASE_UTIL_VALUES_VALUES_UTIL_H_
