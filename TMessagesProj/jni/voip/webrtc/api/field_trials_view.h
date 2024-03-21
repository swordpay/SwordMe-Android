/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef API_FIELD_TRIALS_VIEW_H_
#define API_FIELD_TRIALS_VIEW_H_

#include <string>

#include "absl/strings/string_view.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

//
// Note that there are no guarantess that the meaning of a particular key-value
// mapping will be preserved over time and no announcements will be made if they
// are changed. It's up to the library user to ensure that the behavior does not
// break.
class RTC_EXPORT FieldTrialsView {
 public:
  virtual ~FieldTrialsView() = default;


  virtual std::string Lookup(absl::string_view key) const = 0;

  bool IsEnabled(absl::string_view key) const {
    return Lookup(key).find("Enabled") == 0;
  }

  bool IsDisabled(absl::string_view key) const {
    return Lookup(key).find("Disabled") == 0;
  }
};

// api/field_trials_view.h
typedef FieldTrialsView WebRtcKeyValueConfig;

}  // namespace webrtc

#endif  // API_FIELD_TRIALS_VIEW_H_
