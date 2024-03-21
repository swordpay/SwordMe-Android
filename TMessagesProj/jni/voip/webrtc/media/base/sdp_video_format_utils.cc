/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "media/base/sdp_video_format_utils.h"

#include <cstring>
#include <map>
#include <utility>

#include "api/video_codecs/h264_profile_level_id.h"
#include "rtc_base/checks.h"
#include "rtc_base/string_to_number.h"

namespace webrtc {
namespace {
const char kProfileLevelId[] = "profile-level-id";
const char kH264LevelAsymmetryAllowed[] = "level-asymmetry-allowed";
// Max frame rate for VP8 and VP9 video.
const char kVPxFmtpMaxFrameRate[] = "max-fr";
// Max frame size for VP8 and VP9 video.
const char kVPxFmtpMaxFrameSize[] = "max-fs";
const int kVPxFmtpFrameSizeSubBlockPixels = 256;

bool IsH264LevelAsymmetryAllowed(const SdpVideoFormat::Parameters& params) {
  const auto it = params.find(kH264LevelAsymmetryAllowed);
  return it != params.end() && strcmp(it->second.c_str(), "1") == 0;
}

bool H264LevelIsLess(H264Level a, H264Level b) {
  if (a == H264Level::kLevel1_b)
    return b != H264Level::kLevel1 && b != H264Level::kLevel1_b;
  if (b == H264Level::kLevel1_b)
    return a == H264Level::kLevel1;
  return a < b;
}

H264Level H264LevelMin(H264Level a, H264Level b) {
  return H264LevelIsLess(a, b) ? a : b;
}

absl::optional<int> ParsePositiveNumberFromParams(
    const SdpVideoFormat::Parameters& params,
    const char* parameter_name) {
  const auto max_frame_rate_it = params.find(parameter_name);
  if (max_frame_rate_it == params.end())
    return absl::nullopt;

  const absl::optional<int> i =
      rtc::StringToNumber<int>(max_frame_rate_it->second);
  if (!i.has_value() || i.value() <= 0)
    return absl::nullopt;
  return i;
}

}  // namespace

void H264GenerateProfileLevelIdForAnswer(
    const SdpVideoFormat::Parameters& local_supported_params,
    const SdpVideoFormat::Parameters& remote_offered_params,
    SdpVideoFormat::Parameters* answer_params) {



  if (!local_supported_params.count(kProfileLevelId) &&
      !remote_offered_params.count(kProfileLevelId)) {
    return;
  }

  const absl::optional<H264ProfileLevelId> local_profile_level_id =
      ParseSdpForH264ProfileLevelId(local_supported_params);
  const absl::optional<H264ProfileLevelId> remote_profile_level_id =
      ParseSdpForH264ProfileLevelId(remote_offered_params);

  RTC_DCHECK(local_profile_level_id);
  RTC_DCHECK(remote_profile_level_id);
  RTC_DCHECK_EQ(local_profile_level_id->profile,
                remote_profile_level_id->profile);

  const bool level_asymmetry_allowed =
      IsH264LevelAsymmetryAllowed(local_supported_params) &&
      IsH264LevelAsymmetryAllowed(remote_offered_params);
  const H264Level local_level = local_profile_level_id->level;
  const H264Level remote_level = remote_profile_level_id->level;
  const H264Level min_level = H264LevelMin(local_level, remote_level);



  const H264Level answer_level =
      level_asymmetry_allowed ? local_level : min_level;

  (*answer_params)[kProfileLevelId] = *H264ProfileLevelIdToString(
      H264ProfileLevelId(local_profile_level_id->profile, answer_level));
}

absl::optional<int> ParseSdpForVPxMaxFrameRate(
    const SdpVideoFormat::Parameters& params) {
  return ParsePositiveNumberFromParams(params, kVPxFmtpMaxFrameRate);
}

absl::optional<int> ParseSdpForVPxMaxFrameSize(
    const SdpVideoFormat::Parameters& params) {
  const absl::optional<int> i =
      ParsePositiveNumberFromParams(params, kVPxFmtpMaxFrameSize);
  return i ? absl::make_optional(i.value() * kVPxFmtpFrameSizeSubBlockPixels)
           : absl::nullopt;
}

}  // namespace webrtc
