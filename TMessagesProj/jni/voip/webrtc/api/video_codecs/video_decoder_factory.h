/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VIDEO_CODECS_VIDEO_DECODER_FACTORY_H_
#define API_VIDEO_CODECS_VIDEO_DECODER_FACTORY_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "api/video_codecs/sdp_video_format.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

class VideoDecoder;

// NOTE: This class is still under development and may change without notice.
class RTC_EXPORT VideoDecoderFactory {
 public:
  struct CodecSupport {
    bool is_supported = false;
    bool is_power_efficient = false;
  };


  virtual std::vector<SdpVideoFormat> GetSupportedFormats() const = 0;










  virtual CodecSupport QueryCodecSupport(const SdpVideoFormat& format,
                                         bool reference_scaling) const {



    CodecSupport codec_support;
    codec_support.is_supported =
        !reference_scaling && format.IsCodecInList(GetSupportedFormats());
    return codec_support;
  }

  virtual std::unique_ptr<VideoDecoder> CreateVideoDecoder(
      const SdpVideoFormat& format) = 0;

  virtual ~VideoDecoderFactory() {}
};

}  // namespace webrtc

#endif  // API_VIDEO_CODECS_VIDEO_DECODER_FACTORY_H_
