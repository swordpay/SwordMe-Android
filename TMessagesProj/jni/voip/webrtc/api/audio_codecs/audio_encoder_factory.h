/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_AUDIO_CODECS_AUDIO_ENCODER_FACTORY_H_
#define API_AUDIO_CODECS_AUDIO_ENCODER_FACTORY_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_encoder.h"
#include "api/audio_codecs/audio_format.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

class AudioEncoderFactory : public rtc::RefCountInterface {
 public:

  virtual std::vector<AudioCodecSpec> GetSupportedEncoders() = 0;



  virtual absl::optional<AudioCodecInfo> QueryAudioEncoder(
      const SdpAudioFormat& format) = 0;

















  virtual std::unique_ptr<AudioEncoder> MakeAudioEncoder(
      int payload_type,
      const SdpAudioFormat& format,
      absl::optional<AudioCodecPairId> codec_pair_id) = 0;
};

}  // namespace webrtc

#endif  // API_AUDIO_CODECS_AUDIO_ENCODER_FACTORY_H_
