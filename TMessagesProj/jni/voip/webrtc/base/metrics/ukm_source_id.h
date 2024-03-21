// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_UKM_SOURCE_ID_H_
#define BASE_METRICS_UKM_SOURCE_ID_H_

#include <stdint.h>

#include "base/base_export.h"

namespace base {

// These objects are copyable, assignable, and occupy 64-bits per instance.
// Prefer passing them by value.
class BASE_EXPORT UkmSourceId {
 public:
  enum class Type : int64_t {




    UKM = 0,



    NAVIGATION_ID = 1,



    APP_ID = 2,





    HISTORY_ID = 3,



    WEBAPK_ID = 4,



    PAYMENT_APP_ID = 5,
    kMaxValue = PAYMENT_APP_ID,
  };

  constexpr UkmSourceId() : value_(0) {}

  constexpr UkmSourceId& operator=(UkmSourceId other) {
    value_ = other.value_;
    return *this;
  }

  constexpr bool operator==(UkmSourceId other) const {
    return value_ == other.value_;
  }
  constexpr bool operator!=(UkmSourceId other) const {
    return value_ != other.value_;
  }


  constexpr bool operator==(int64_t other) const { return value_ == other; }
  constexpr bool operator!=(int64_t other) const { return value_ == other; }

  Type GetType() const;

  constexpr int64_t ToInt64() const { return value_; }

  static constexpr UkmSourceId FromInt64(int64_t internal_value) {
    return UkmSourceId(internal_value);
  }


  static UkmSourceId New();

  static UkmSourceId FromOtherId(int64_t value, Type type);

 private:
  constexpr explicit UkmSourceId(int64_t value) : value_(value) {}
  int64_t value_;
};

constexpr UkmSourceId kInvalidUkmSourceId = UkmSourceId();

}  // namespace base

#endif  // BASE_METRICS_UKM_SOURCE_ID_H_
