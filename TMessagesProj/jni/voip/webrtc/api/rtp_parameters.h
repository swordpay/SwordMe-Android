/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_RTP_PARAMETERS_H_
#define API_RTP_PARAMETERS_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/media_types.h"
#include "api/priority.h"
#include "api/rtp_transceiver_direction.h"
#include "api/video/resolution.h"
#include "api/video_codecs/scalability_mode.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

// http://draft.ortc.org/#rtcrtpdictionaries*
// Contains everything specified as of 2017 Jan 24.
//
// They are used when retrieving or modifying the parameters of an
// RtpSender/RtpReceiver, or retrieving capabilities.
//
// Note on conventions: Where ORTC may use "octet", "short" and "unsigned"
// types, we typically use "int", in keeping with our style guidelines. The
// parameter's actual valid range will be enforced when the parameters are set,
// rather than when the parameters struct is built. An exception is made for
// SSRCs, since they use the full unsigned 32-bit range, and aren't expected to
// be used for any numeric comparisons/operations.
//
// Additionally, where ORTC uses strings, we may use enums for things that have
// a fixed number of supported values. However, for things that can be extended
// (such as codecs, by providing an external encoder factory), a string
// identifier is used.

enum class FecMechanism {
  RED,
  RED_AND_ULPFEC,
  FLEXFEC,
};

enum class RtcpFeedbackType {
  CCM,
  LNTF,  // "goog-lntf"
  NACK,
  REMB,  // "goog-remb"
  TRANSPORT_CC,
};

enum class RtcpFeedbackMessageType {

  GENERIC_NACK,
  PLI,  // Usable with NACK.
  FIR,  // Usable with CCM.
};

enum class DtxStatus {
  DISABLED,
  ENABLED,
};

// https://w3c.github.io/webrtc-pc/#idl-def-rtcdegradationpreference.
// These options are enforced on a best-effort basis. For instance, all of
// these options may suffer some frame drops in order to avoid queuing.
// TODO(sprang): Look into possibility of more strictly enforcing the
// maintain-framerate option.
// TODO(deadbeef): Default to "balanced", as the spec indicates?
enum class DegradationPreference {


  DISABLED,

  MAINTAIN_FRAMERATE,

  MAINTAIN_RESOLUTION,

  BALANCED,
};

RTC_EXPORT const char* DegradationPreferenceToString(
    DegradationPreference degradation_preference);

RTC_EXPORT extern const double kDefaultBitratePriority;

struct RTC_EXPORT RtcpFeedback {
  RtcpFeedbackType type = RtcpFeedbackType::CCM;




  absl::optional<RtcpFeedbackMessageType> message_type;

  RtcpFeedback();
  explicit RtcpFeedback(RtcpFeedbackType type);
  RtcpFeedback(RtcpFeedbackType type, RtcpFeedbackMessageType message_type);
  RtcpFeedback(const RtcpFeedback&);
  ~RtcpFeedback();

  bool operator==(const RtcpFeedback& o) const {
    return type == o.type && message_type == o.message_type;
  }
  bool operator!=(const RtcpFeedback& o) const { return !(*this == o); }
};

// RtpParameters. This represents the static capabilities of an endpoint's
// implementation of a codec.
struct RTC_EXPORT RtpCodecCapability {
  RtpCodecCapability();
  ~RtpCodecCapability();

  std::string mime_type() const { return MediaTypeToString(kind) + "/" + name; }

  std::string name;

  cricket::MediaType kind = cricket::MEDIA_TYPE_AUDIO;

  absl::optional<int> clock_rate;


  absl::optional<int> preferred_payload_type;


  absl::optional<int> max_ptime;


  absl::optional<int> ptime;

  absl::optional<int> num_channels;

  std::vector<RtcpFeedback> rtcp_feedback;







  std::map<std::string, std::string> parameters;



  std::map<std::string, std::string> options;



  int max_temporal_layer_extensions = 0;



  int max_spatial_layer_extensions = 0;




  bool svc_multi_stream_support = false;

  absl::InlinedVector<ScalabilityMode, kScalabilityModeCount> scalability_modes;

  bool operator==(const RtpCodecCapability& o) const {
    return name == o.name && kind == o.kind && clock_rate == o.clock_rate &&
           preferred_payload_type == o.preferred_payload_type &&
           max_ptime == o.max_ptime && ptime == o.ptime &&
           num_channels == o.num_channels && rtcp_feedback == o.rtcp_feedback &&
           parameters == o.parameters && options == o.options &&
           max_temporal_layer_extensions == o.max_temporal_layer_extensions &&
           max_spatial_layer_extensions == o.max_spatial_layer_extensions &&
           svc_multi_stream_support == o.svc_multi_stream_support &&
           scalability_modes == o.scalability_modes;
  }
  bool operator!=(const RtpCodecCapability& o) const { return !(*this == o); }
};

// and setup methods; represents the capabilities/preferences of an
// implementation for a header extension.
//
// Just called "RtpHeaderExtension" in ORTC, but the "Capability" suffix was
// added here for consistency and to avoid confusion with
// RtpHeaderExtensionParameters.
//
// Note that ORTC includes a "kind" field, but we omit this because it's
// redundant; if you call "RtpReceiver::GetCapabilities(MEDIA_TYPE_AUDIO)",
// you know you're getting audio capabilities.
struct RTC_EXPORT RtpHeaderExtensionCapability {

  std::string uri;

  absl::optional<int> preferred_id;


  bool preferred_encrypt = false;



  RtpTransceiverDirection direction = RtpTransceiverDirection::kSendRecv;

  RtpHeaderExtensionCapability();
  explicit RtpHeaderExtensionCapability(absl::string_view uri);
  RtpHeaderExtensionCapability(absl::string_view uri, int preferred_id);
  RtpHeaderExtensionCapability(absl::string_view uri,
                               int preferred_id,
                               RtpTransceiverDirection direction);
  ~RtpHeaderExtensionCapability();

  bool operator==(const RtpHeaderExtensionCapability& o) const {
    return uri == o.uri && preferred_id == o.preferred_id &&
           preferred_encrypt == o.preferred_encrypt && direction == o.direction;
  }
  bool operator!=(const RtpHeaderExtensionCapability& o) const {
    return !(*this == o);
  }
};

struct RTC_EXPORT RtpExtension {
  enum Filter {


    kDiscardEncryptedExtension,


    kPreferEncryptedExtension,


    kRequireEncryptedExtension,
  };

  RtpExtension();
  RtpExtension(absl::string_view uri, int id);
  RtpExtension(absl::string_view uri, int id, bool encrypt);
  ~RtpExtension();

  std::string ToString() const;
  bool operator==(const RtpExtension& rhs) const {
    return uri == rhs.uri && id == rhs.id && encrypt == rhs.encrypt;
  }
  static bool IsSupportedForAudio(absl::string_view uri);
  static bool IsSupportedForVideo(absl::string_view uri);

  static bool IsEncryptionSupported(absl::string_view uri);

  static const RtpExtension* FindHeaderExtensionByUri(
      const std::vector<RtpExtension>& extensions,
      absl::string_view uri,
      Filter filter);


  static const RtpExtension* FindHeaderExtensionByUriAndEncryption(
      const std::vector<RtpExtension>& extensions,
      absl::string_view uri,
      bool encrypt);




  static const std::vector<RtpExtension> DeduplicateHeaderExtensions(
      const std::vector<RtpExtension>& extensions,
      Filter filter);


  static constexpr char kEncryptHeaderExtensionsUri[] =
      "urn:ietf:params:rtp-hdrext:encrypt";


  static constexpr char kAudioLevelUri[] =
      "urn:ietf:params:rtp-hdrext:ssrc-audio-level";


  static constexpr char kTimestampOffsetUri[] =
      "urn:ietf:params:rtp-hdrext:toffset";


  static constexpr char kAbsSendTimeUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";


  static constexpr char kAbsoluteCaptureTimeUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/abs-capture-time";



  static constexpr char kVideoRotationUri[] = "urn:3gpp:video-orientation";

  static constexpr char kVideoContentTypeUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/video-content-type";

  static constexpr char kVideoTimingUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/video-timing";

  static constexpr char kGenericFrameDescriptorUri00[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/"
      "generic-frame-descriptor-00";
  static constexpr char kDependencyDescriptorUri[] =
      "https://aomediacodec.github.io/av1-rtp-spec/"
      "#dependency-descriptor-rtp-header-extension";

  static constexpr char kVideoLayersAllocationUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/video-layers-allocation00";


  static constexpr char kTransportSequenceNumberUri[] =
      "http://www.ietf.org/id/"
      "draft-holmer-rmcat-transport-wide-cc-extensions-01";
  static constexpr char kTransportSequenceNumberV2Uri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02";




  static constexpr char kPlayoutDelayUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/playout-delay";

  static constexpr char kColorSpaceUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/color-space";


  static constexpr char kMidUri[] = "urn:ietf:params:rtp-hdrext:sdes:mid";



  static constexpr char kRidUri[] =
      "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id";
  static constexpr char kRepairedRidUri[] =
      "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id";

  static constexpr char kVideoFrameTrackingIdUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/video-frame-tracking-id";


  static constexpr char kCsrcAudioLevelsUri[] =
      "urn:ietf:params:rtp-hdrext:csrc-audio-level";


  static constexpr int kMinId = 1;
  static constexpr int kMaxId = 255;
  static constexpr int kMaxValueSize = 255;
  static constexpr int kOneByteHeaderExtensionMaxId = 14;
  static constexpr int kOneByteHeaderExtensionMaxValueSize = 16;

  std::string uri;
  int id = 0;
  bool encrypt = false;
};

struct RTC_EXPORT RtpFecParameters {


  absl::optional<uint32_t> ssrc;

  FecMechanism mechanism = FecMechanism::RED;

  RtpFecParameters();
  explicit RtpFecParameters(FecMechanism mechanism);
  RtpFecParameters(FecMechanism mechanism, uint32_t ssrc);
  RtpFecParameters(const RtpFecParameters&);
  ~RtpFecParameters();

  bool operator==(const RtpFecParameters& o) const {
    return ssrc == o.ssrc && mechanism == o.mechanism;
  }
  bool operator!=(const RtpFecParameters& o) const { return !(*this == o); }
};

struct RTC_EXPORT RtpRtxParameters {


  absl::optional<uint32_t> ssrc;

  RtpRtxParameters();
  explicit RtpRtxParameters(uint32_t ssrc);
  RtpRtxParameters(const RtpRtxParameters&);
  ~RtpRtxParameters();

  bool operator==(const RtpRtxParameters& o) const { return ssrc == o.ssrc; }
  bool operator!=(const RtpRtxParameters& o) const { return !(*this == o); }
};

struct RTC_EXPORT RtpEncodingParameters {
  RtpEncodingParameters();
  RtpEncodingParameters(const RtpEncodingParameters&);
  ~RtpEncodingParameters();






  absl::optional<uint32_t> ssrc;












  double bitrate_priority = kDefaultBitratePriority;






  Priority network_priority = Priority::kLow;











  absl::optional<int> max_bitrate_bps;

  absl::optional<int> min_bitrate_bps;

  absl::optional<double> max_framerate;



  absl::optional<int> num_temporal_layers;

  absl::optional<double> scale_resolution_down_by;

  absl::optional<std::string> scalability_mode;
















  absl::optional<Resolution> requested_resolution;






  bool active = true;


  std::string rid;


  bool adaptive_ptime = false;

  bool operator==(const RtpEncodingParameters& o) const {
    return ssrc == o.ssrc && bitrate_priority == o.bitrate_priority &&
           network_priority == o.network_priority &&
           max_bitrate_bps == o.max_bitrate_bps &&
           min_bitrate_bps == o.min_bitrate_bps &&
           max_framerate == o.max_framerate &&
           num_temporal_layers == o.num_temporal_layers &&
           scale_resolution_down_by == o.scale_resolution_down_by &&
           active == o.active && rid == o.rid &&
           adaptive_ptime == o.adaptive_ptime &&
           requested_resolution == o.requested_resolution;
  }
  bool operator!=(const RtpEncodingParameters& o) const {
    return !(*this == o);
  }
};

struct RTC_EXPORT RtpCodecParameters {
  RtpCodecParameters();
  RtpCodecParameters(const RtpCodecParameters&);
  ~RtpCodecParameters();

  std::string mime_type() const { return MediaTypeToString(kind) + "/" + name; }

  std::string name;

  cricket::MediaType kind = cricket::MEDIA_TYPE_AUDIO;



  int payload_type = 0;

  absl::optional<int> clock_rate;





  absl::optional<int> num_channels;



  absl::optional<int> max_ptime;



  absl::optional<int> ptime;


  std::vector<RtcpFeedback> rtcp_feedback;







  std::map<std::string, std::string> parameters;

  bool operator==(const RtpCodecParameters& o) const {
    return name == o.name && kind == o.kind && payload_type == o.payload_type &&
           clock_rate == o.clock_rate && num_channels == o.num_channels &&
           max_ptime == o.max_ptime && ptime == o.ptime &&
           rtcp_feedback == o.rtcp_feedback && parameters == o.parameters;
  }
  bool operator!=(const RtpCodecParameters& o) const { return !(*this == o); }
};

// An application can use these capabilities to construct an RtpParameters.
struct RTC_EXPORT RtpCapabilities {
  RtpCapabilities();
  ~RtpCapabilities();

  std::vector<RtpCodecCapability> codecs;

  std::vector<RtpHeaderExtensionCapability> header_extensions;



  std::vector<FecMechanism> fec;

  bool operator==(const RtpCapabilities& o) const {
    return codecs == o.codecs && header_extensions == o.header_extensions &&
           fec == o.fec;
  }
  bool operator!=(const RtpCapabilities& o) const { return !(*this == o); }
};

struct RtcpParameters final {
  RtcpParameters();
  RtcpParameters(const RtcpParameters&);
  ~RtcpParameters();



  absl::optional<uint32_t> ssrc;









  std::string cname;

  bool reduced_size = false;


  bool mux = true;

  bool operator==(const RtcpParameters& o) const {
    return ssrc == o.ssrc && cname == o.cname &&
           reduced_size == o.reduced_size && mux == o.mux;
  }
  bool operator!=(const RtcpParameters& o) const { return !(*this == o); }
};

struct RTC_EXPORT RtpParameters {
  RtpParameters();
  RtpParameters(const RtpParameters&);
  ~RtpParameters();



  std::string transaction_id;



  std::string mid;

  std::vector<RtpCodecParameters> codecs;

  std::vector<RtpExtension> header_extensions;

  std::vector<RtpEncodingParameters> encodings;



  RtcpParameters rtcp;



  absl::optional<DegradationPreference> degradation_preference;

  bool operator==(const RtpParameters& o) const {
    return mid == o.mid && codecs == o.codecs &&
           header_extensions == o.header_extensions &&
           encodings == o.encodings && rtcp == o.rtcp &&
           degradation_preference == o.degradation_preference;
  }
  bool operator!=(const RtpParameters& o) const { return !(*this == o); }
};

}  // namespace webrtc

#endif  // API_RTP_PARAMETERS_H_
