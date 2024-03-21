/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VIDEO_CODECS_VP9_PROFILE_H_
#define API_VIDEO_CODECS_VP9_PROFILE_H_

#include <string>

#include "absl/types/optional.h"
#include "api/video_codecs/sdp_video_format.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

extern RTC_EXPORT const char kVP9FmtpProfileId[];

enum class VP9Profile {
  kProfile0,
  kProfile1,
  kProfile2,
  kProfile3,
};

// default.
RTC_EXPORT std::string VP9ProfileToString(VP9Profile profile);

// an invalid profile string.
absl::optional<VP9Profile> StringToVP9Profile(const std::string& str);

// SDP key-value map. A default profile(kProfile0) will be returned if the
// profile key is missing. Nothing will be returned if the key is present but
// the string is invalid.
RTC_EXPORT absl::optional<VP9Profile> ParseSdpForVP9Profile(
    const SdpVideoFormat::Parameters& params);

// VP9 profile.
bool VP9IsSameProfile(const SdpVideoFormat::Parameters& params1,
                      const SdpVideoFormat::Parameters& params2);

}  // namespace webrtc

#endif  // API_VIDEO_CODECS_VP9_PROFILE_H_
