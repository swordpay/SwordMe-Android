/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_BASE_CODEC_H_
#define MEDIA_BASE_CODEC_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/field_trials_view.h"
#include "api/rtp_parameters.h"
#include "api/video_codecs/sdp_video_format.h"
#include "media/base/media_constants.h"
#include "rtc_base/system/rtc_export.h"

namespace cricket {

typedef std::map<std::string, std::string> CodecParameterMap;

class FeedbackParam {
 public:
  FeedbackParam() = default;
  FeedbackParam(absl::string_view id, const std::string& param)
      : id_(id), param_(param) {}
  explicit FeedbackParam(absl::string_view id)
      : id_(id), param_(kParamValueEmpty) {}

  bool operator==(const FeedbackParam& other) const;

  const std::string& id() const { return id_; }
  const std::string& param() const { return param_; }

 private:
  std::string id_;     // e.g. "nack", "ccm"
  std::string param_;  // e.g. "", "rpsi", "fir"
};

class FeedbackParams {
 public:
  FeedbackParams();
  ~FeedbackParams();
  bool operator==(const FeedbackParams& other) const;

  bool Has(const FeedbackParam& param) const;
  void Add(const FeedbackParam& param);

  void Intersect(const FeedbackParams& from);

  const std::vector<FeedbackParam>& params() const { return params_; }

 private:
  bool HasDuplicateEntries() const;

  std::vector<FeedbackParam> params_;
};

struct RTC_EXPORT Codec {
  int id;
  std::string name;
  int clockrate;


  CodecParameterMap params;
  FeedbackParams feedback_params;

  virtual ~Codec();

  bool Matches(const Codec& codec,
               const webrtc::FieldTrialsView* field_trials = nullptr) const;
  bool MatchesCapability(const webrtc::RtpCodecCapability& capability) const;

  bool GetParam(const std::string& name, std::string* out) const;
  bool GetParam(const std::string& name, int* out) const;

  void SetParam(const std::string& name, const std::string& value);
  void SetParam(const std::string& name, int value);


  bool RemoveParam(const std::string& name);

  bool HasFeedbackParam(const FeedbackParam& param) const;
  void AddFeedbackParam(const FeedbackParam& param);


  void IntersectFeedbackParams(const Codec& other);

  virtual webrtc::RtpCodecParameters ToCodecParameters() const;

  Codec& operator=(const Codec& c);
  Codec& operator=(Codec&& c);

  bool operator==(const Codec& c) const;

  bool operator!=(const Codec& c) const { return !(*this == c); }

 protected:


  Codec(int id, const std::string& name, int clockrate);

  Codec();
  Codec(const Codec& c);
  Codec(Codec&& c);
};

struct AudioCodec : public Codec {
  int bitrate;
  size_t channels;

  AudioCodec(int id,
             const std::string& name,
             int clockrate,
             int bitrate,
             size_t channels);

  AudioCodec();
  AudioCodec(const AudioCodec& c);
  AudioCodec(AudioCodec&& c);
  ~AudioCodec() override = default;

  bool Matches(const AudioCodec& codec,
               const webrtc::FieldTrialsView* field_trials = nullptr) const;

  std::string ToString() const;

  webrtc::RtpCodecParameters ToCodecParameters() const override;

  AudioCodec& operator=(const AudioCodec& c);
  AudioCodec& operator=(AudioCodec&& c);

  bool operator==(const AudioCodec& c) const;

  bool operator!=(const AudioCodec& c) const { return !(*this == c); }
};

struct RTC_EXPORT VideoCodec : public Codec {
  absl::optional<std::string> packetization;
  absl::InlinedVector<webrtc::ScalabilityMode, webrtc::kScalabilityModeCount>
      scalability_modes;

  VideoCodec(int id, const std::string& name);

  explicit VideoCodec(const std::string& name);

  VideoCodec();
  VideoCodec(const VideoCodec& c);
  explicit VideoCodec(const webrtc::SdpVideoFormat& c);
  VideoCodec(VideoCodec&& c);
  ~VideoCodec() override = default;



  bool Matches(const VideoCodec& codec,
               const webrtc::FieldTrialsView* field_trials = nullptr) const;

  std::string ToString() const;

  webrtc::RtpCodecParameters ToCodecParameters() const override;

  VideoCodec& operator=(const VideoCodec& c);
  VideoCodec& operator=(VideoCodec&& c);

  bool operator==(const VideoCodec& c) const;

  bool operator!=(const VideoCodec& c) const { return !(*this == c); }

  static absl::optional<std::string> IntersectPacketization(
      const VideoCodec& local_codec,
      const VideoCodec& remote_codec);

  static VideoCodec CreateRtxCodec(int rtx_payload_type,
                                   int associated_payload_type);

  enum CodecType {
    CODEC_VIDEO,
    CODEC_RED,
    CODEC_ULPFEC,
    CODEC_FLEXFEC,
    CODEC_RTX,
  };

  CodecType GetCodecType() const;



  bool ValidateCodecFormat() const;

 private:
  void SetDefaultParameters();
};

// is no codec associated with that payload type it returns nullptr.
template <class Codec>
const Codec* FindCodecById(const std::vector<Codec>& codecs, int payload_type) {
  for (const auto& codec : codecs) {
    if (codec.id == payload_type)
      return &codec;
  }
  return nullptr;
}

bool HasLntf(const Codec& codec);
bool HasNack(const Codec& codec);
bool HasRemb(const Codec& codec);
bool HasRrtr(const Codec& codec);
bool HasTransportCc(const Codec& codec);
// Returns the first codec in `supported_codecs` that matches `codec`, or
// nullptr if no codec matches.
const VideoCodec* FindMatchingCodec(
    const std::vector<VideoCodec>& supported_codecs,
    const VideoCodec& codec);

RTC_EXPORT void AddH264ConstrainedBaselineProfileToSupportedFormats(
    std::vector<webrtc::SdpVideoFormat>* supported_formats);

}  // namespace cricket

#endif  // MEDIA_BASE_CODEC_H_
