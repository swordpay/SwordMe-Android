/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_CONFIG_VIDEO_ENCODER_CONFIG_H_
#define VIDEO_CONFIG_VIDEO_ENCODER_CONFIG_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "api/scoped_refptr.h"
#include "api/video/resolution.h"
#include "api/video_codecs/scalability_mode.h"
#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_codec.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

struct VideoStream {
  VideoStream();
  ~VideoStream();
  VideoStream(const VideoStream& other);
  std::string ToString() const;




  size_t width;
  size_t height;

  int max_framerate;

  int min_bitrate_bps;
  int target_bitrate_bps;
  int max_bitrate_bps;


  double scale_resolution_down_by;

  int max_qp;






  absl::optional<size_t> num_temporal_layers;


  absl::optional<double> bitrate_priority;

  absl::optional<ScalabilityMode> scalability_mode;

  bool active;









  absl::optional<Resolution> requested_resolution;
};

class VideoEncoderConfig {
 public:




  class EncoderSpecificSettings : public rtc::RefCountInterface {
   public:



    void FillEncoderSpecificSettings(VideoCodec* codec_struct) const;

    virtual void FillVideoCodecVp8(VideoCodecVP8* vp8_settings) const;
    virtual void FillVideoCodecVp9(VideoCodecVP9* vp9_settings) const;
    virtual void FillVideoCodecH264(VideoCodecH264* h264_settings) const;
#ifndef DISABLE_H265
    virtual void FillVideoCodecH265(VideoCodecH265* h265_settings) const;
#endif

   private:
    ~EncoderSpecificSettings() override {}
    friend class VideoEncoderConfig;
  };

  class H264EncoderSpecificSettings : public EncoderSpecificSettings {
   public:
    explicit H264EncoderSpecificSettings(const VideoCodecH264& specifics);
    void FillVideoCodecH264(VideoCodecH264* h264_settings) const override;

   private:
    VideoCodecH264 specifics_;
  };

#ifndef DISABLE_H265
  class H265EncoderSpecificSettings : public EncoderSpecificSettings {
   public:
    explicit H265EncoderSpecificSettings(const VideoCodecH265& specifics);
    void FillVideoCodecH265(VideoCodecH265* h265_settings) const override;

   private:
    VideoCodecH265 specifics_;
  };
#endif
  class Vp8EncoderSpecificSettings : public EncoderSpecificSettings {
   public:
    explicit Vp8EncoderSpecificSettings(const VideoCodecVP8& specifics);
    void FillVideoCodecVp8(VideoCodecVP8* vp8_settings) const override;

   private:
    VideoCodecVP8 specifics_;
  };

  class Vp9EncoderSpecificSettings : public EncoderSpecificSettings {
   public:
    explicit Vp9EncoderSpecificSettings(const VideoCodecVP9& specifics);
    void FillVideoCodecVp9(VideoCodecVP9* vp9_settings) const override;

   private:
    VideoCodecVP9 specifics_;
  };

  enum class ContentType {
    kRealtimeVideo,
    kScreen,
  };

  class VideoStreamFactoryInterface : public rtc::RefCountInterface {
   public:




    virtual std::vector<VideoStream> CreateEncoderStreams(
        int frame_width,
        int frame_height,
        const VideoEncoderConfig& encoder_config) = 0;

   protected:
    ~VideoStreamFactoryInterface() override {}
  };

  VideoEncoderConfig& operator=(VideoEncoderConfig&&) = default;
  VideoEncoderConfig& operator=(const VideoEncoderConfig&) = delete;

  VideoEncoderConfig Copy() const { return VideoEncoderConfig(*this); }

  VideoEncoderConfig();
  VideoEncoderConfig(VideoEncoderConfig&&);
  ~VideoEncoderConfig();
  std::string ToString() const;

  VideoCodecType codec_type;
  SdpVideoFormat video_format;



  rtc::scoped_refptr<VideoStreamFactoryInterface> video_stream_factory;
  std::vector<SpatialLayer> spatial_layers;
  ContentType content_type;
  bool frame_drop_enabled;
  rtc::scoped_refptr<const EncoderSpecificSettings> encoder_specific_settings;




  int min_transmit_bitrate_bps;
  int max_bitrate_bps;

  double bitrate_priority;





  std::vector<VideoStream> simulcast_layers;

  size_t number_of_streams;

  bool legacy_conference_mode;

  bool is_quality_scaling_allowed;



  int max_qp;

 private:


  VideoEncoderConfig(const VideoEncoderConfig&);
};

}  // namespace webrtc

#endif  // VIDEO_CONFIG_VIDEO_ENCODER_CONFIG_H_
