/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_BASE_MEDIA_CHANNEL_H_
#define MEDIA_BASE_MEDIA_CHANNEL_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_encoder.h"
#include "api/audio_options.h"
#include "api/crypto/frame_decryptor_interface.h"
#include "api/crypto/frame_encryptor_interface.h"
#include "api/frame_transformer_interface.h"
#include "api/media_stream_interface.h"
#include "api/rtc_error.h"
#include "api/rtp_parameters.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/transport/data_channel_transport_interface.h"
#include "api/transport/rtp/rtp_source.h"
#include "api/units/time_delta.h"
#include "api/video/video_content_type.h"
#include "api/video/video_sink_interface.h"
#include "api/video/video_source_interface.h"
#include "api/video/video_timing.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "call/video_receive_stream.h"
#include "common_video/include/quality_limitation_reason.h"
#include "media/base/codec.h"
#include "media/base/delayable.h"
#include "media/base/media_constants.h"
#include "media/base/stream_params.h"
#include "modules/audio_processing/include/audio_processing_statistics.h"
#include "modules/rtp_rtcp/include/report_block_data.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/buffer.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/dscp.h"
#include "rtc_base/logging.h"
#include "rtc_base/network_route.h"
#include "rtc_base/socket.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/strings/string_builder.h"
#include "video/config/video_encoder_config.h"

namespace rtc {
class Timing;
}

namespace webrtc {
class AudioSinkInterface;
class VideoFrame;
}  // namespace webrtc

namespace cricket {

class AudioSource;
class VideoCapturer;
struct RtpHeader;
struct VideoFormat;

const int kScreencastDefaultFps = 5;

template <class T>
static std::string ToStringIfSet(const char* key,
                                 const absl::optional<T>& val) {
  std::string str;
  if (val) {
    str = key;
    str += ": ";
    str += val ? rtc::ToString(*val) : "";
    str += ", ";
  }
  return str;
}

template <class T>
static std::string VectorToString(const std::vector<T>& vals) {
  rtc::StringBuilder ost;  // no-presubmit-check TODO(webrtc:8982)
  ost << "[";
  for (size_t i = 0; i < vals.size(); ++i) {
    if (i > 0) {
      ost << ", ";
    }
    ost << vals[i].ToString();
  }
  ost << "]";
  return ost.Release();
}

// Used to be flags, but that makes it hard to selectively apply options.
// We are moving all of the setting of options to structs like this,
// but some things currently still use flags.
struct VideoOptions {
  VideoOptions();
  ~VideoOptions();

  void SetAll(const VideoOptions& change) {
    SetFrom(&video_noise_reduction, change.video_noise_reduction);
    SetFrom(&screencast_min_bitrate_kbps, change.screencast_min_bitrate_kbps);
    SetFrom(&is_screencast, change.is_screencast);
  }

  bool operator==(const VideoOptions& o) const {
    return video_noise_reduction == o.video_noise_reduction &&
           screencast_min_bitrate_kbps == o.screencast_min_bitrate_kbps &&
           is_screencast == o.is_screencast;
  }
  bool operator!=(const VideoOptions& o) const { return !(*this == o); }

  std::string ToString() const {
    rtc::StringBuilder ost;
    ost << "VideoOptions {";
    ost << ToStringIfSet("noise reduction", video_noise_reduction);
    ost << ToStringIfSet("screencast min bitrate kbps",
                         screencast_min_bitrate_kbps);
    ost << ToStringIfSet("is_screencast ", is_screencast);
    ost << "}";
    return ost.Release();
  }



  absl::optional<bool> video_noise_reduction;





  absl::optional<int> screencast_min_bitrate_kbps;




  absl::optional<bool> is_screencast;
  webrtc::VideoTrackInterface::ContentHint content_hint;

 private:
  template <typename T>
  static void SetFrom(absl::optional<T>* s, const absl::optional<T>& o) {
    if (o) {
      *s = o;
    }
  }
};

class MediaChannel {
 public:
  class NetworkInterface {
   public:
    enum SocketType { ST_RTP, ST_RTCP };
    virtual bool SendPacket(rtc::CopyOnWriteBuffer* packet,
                            const rtc::PacketOptions& options) = 0;
    virtual bool SendRtcp(rtc::CopyOnWriteBuffer* packet,
                          const rtc::PacketOptions& options) = 0;
    virtual int SetOption(SocketType type,
                          rtc::Socket::Option opt,
                          int option) = 0;
    virtual ~NetworkInterface() {}
  };

  explicit MediaChannel(webrtc::TaskQueueBase* network_thread,
                        bool enable_dscp = false);
  virtual ~MediaChannel();

  virtual cricket::MediaType media_type() const = 0;

  virtual void SetInterface(NetworkInterface* iface);

  virtual void OnPacketReceived(rtc::CopyOnWriteBuffer packet,
                                int64_t packet_time_us) = 0;


  virtual void OnPacketSent(const rtc::SentPacket& sent_packet) = 0;

  virtual void OnReadyToSend(bool ready) = 0;

  virtual void OnNetworkRouteChanged(
      absl::string_view transport_name,
      const rtc::NetworkRoute& network_route) = 0;


  virtual bool AddSendStream(const StreamParams& sp) = 0;




  virtual bool RemoveSendStream(uint32_t ssrc) = 0;



  virtual bool AddRecvStream(const StreamParams& sp) = 0;



  virtual bool RemoveRecvStream(uint32_t ssrc) = 0;


  virtual void ResetUnsignaledRecvStream() = 0;












  virtual void OnDemuxerCriteriaUpdatePending() = 0;
  virtual void OnDemuxerCriteriaUpdateComplete() = 0;

  virtual int GetRtpSendTimeExtnId() const;




  virtual void SetFrameEncryptor(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameEncryptorInterface> frame_encryptor);




  virtual void SetFrameDecryptor(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameDecryptorInterface> frame_decryptor);

  virtual void SetVideoCodecSwitchingEnabled(bool enabled);


  virtual void SetEncoderSelector(
      uint32_t ssrc,
      webrtc::VideoEncoderFactory::EncoderSelectorInterface* encoder_selector) {
  }

  bool SendPacket(rtc::CopyOnWriteBuffer* packet,
                  const rtc::PacketOptions& options);

  bool SendRtcp(rtc::CopyOnWriteBuffer* packet,
                const rtc::PacketOptions& options);

  int SetOption(NetworkInterface::SocketType type,
                rtc::Socket::Option opt,
                int option);




  void SetExtmapAllowMixed(bool extmap_allow_mixed);
  bool ExtmapAllowMixed() const;


  bool HasNetworkInterface() const;

  virtual webrtc::RtpParameters GetRtpSendParameters(uint32_t ssrc) const = 0;
  virtual webrtc::RTCError SetRtpSendParameters(
      uint32_t ssrc,
      const webrtc::RtpParameters& parameters) = 0;

  virtual void SetEncoderToPacketizerFrameTransformer(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer);
  virtual void SetDepacketizerToDecoderFrameTransformer(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer);

 protected:
  int SetOptionLocked(NetworkInterface::SocketType type,
                      rtc::Socket::Option opt,
                      int option) RTC_RUN_ON(network_thread_);

  bool DscpEnabled() const;


  rtc::DiffServCodePoint PreferredDscp() const;
  void SetPreferredDscp(rtc::DiffServCodePoint new_dscp);

  rtc::scoped_refptr<webrtc::PendingTaskSafetyFlag> network_safety();


  void SendRtp(const uint8_t* data,
               size_t len,
               const webrtc::PacketOptions& options);

  void SendRtcp(const uint8_t* data, size_t len);

 private:


  void UpdateDscp() RTC_RUN_ON(network_thread_);

  bool DoSendPacket(rtc::CopyOnWriteBuffer* packet,
                    bool rtcp,
                    const rtc::PacketOptions& options);

  const bool enable_dscp_;
  const rtc::scoped_refptr<webrtc::PendingTaskSafetyFlag> network_safety_
      RTC_PT_GUARDED_BY(network_thread_);
  webrtc::TaskQueueBase* const network_thread_;
  NetworkInterface* network_interface_ RTC_GUARDED_BY(network_thread_) =
      nullptr;
  rtc::DiffServCodePoint preferred_dscp_ RTC_GUARDED_BY(network_thread_) =
      rtc::DSCP_DEFAULT;
  bool extmap_allow_mixed_ = false;
};

// Media are represented by either MediaSenderInfo or MediaReceiverInfo.
// Media contains a vector of SSRC infos that are exclusively used by this
// media. (SSRCs shared between media streams can't be represented.)

// This data may be locally recorded, or received in an RTCP SR or RR.
struct SsrcSenderInfo {
  uint32_t ssrc = 0;
  double timestamp = 0.0;  // NTP timestamp, represented as seconds since epoch.
};

struct SsrcReceiverInfo {
  uint32_t ssrc = 0;
  double timestamp = 0.0;
};

struct MediaSenderInfo {
  MediaSenderInfo();
  ~MediaSenderInfo();
  void add_ssrc(const SsrcSenderInfo& stat) { local_stats.push_back(stat); }


  void add_ssrc(uint32_t ssrc) {
    SsrcSenderInfo stat;
    stat.ssrc = ssrc;
    add_ssrc(stat);
  }

  std::vector<uint32_t> ssrcs() const {
    std::vector<uint32_t> retval;
    for (std::vector<SsrcSenderInfo>::const_iterator it = local_stats.begin();
         it != local_stats.end(); ++it) {
      retval.push_back(it->ssrc);
    }
    return retval;
  }

  bool connected() const { return local_stats.size() > 0; }





  uint32_t ssrc() const {
    if (connected()) {
      return local_stats[0].ssrc;
    } else {
      return 0;
    }
  }

  int64_t payload_bytes_sent = 0;

  int64_t header_and_padding_bytes_sent = 0;

  uint64_t retransmitted_bytes_sent = 0;
  int packets_sent = 0;

  uint64_t retransmitted_packets_sent = 0;

  uint32_t nacks_rcvd = 0;

  absl::optional<double> target_bitrate;
  int packets_lost = 0;
  float fraction_lost = 0.0f;
  int64_t rtt_ms = 0;
  std::string codec_name;
  absl::optional<int> codec_payload_type;
  std::vector<SsrcSenderInfo> local_stats;
  std::vector<SsrcReceiverInfo> remote_stats;




  std::vector<webrtc::ReportBlockData> report_block_datas;
  absl::optional<bool> active;

  webrtc::TimeDelta total_packet_send_delay = webrtc::TimeDelta::Zero();
};

struct MediaReceiverInfo {
  MediaReceiverInfo();
  ~MediaReceiverInfo();
  void add_ssrc(const SsrcReceiverInfo& stat) { local_stats.push_back(stat); }


  void add_ssrc(uint32_t ssrc) {
    SsrcReceiverInfo stat;
    stat.ssrc = ssrc;
    add_ssrc(stat);
  }
  std::vector<uint32_t> ssrcs() const {
    std::vector<uint32_t> retval;
    for (std::vector<SsrcReceiverInfo>::const_iterator it = local_stats.begin();
         it != local_stats.end(); ++it) {
      retval.push_back(it->ssrc);
    }
    return retval;
  }

  bool connected() const { return local_stats.size() > 0; }





  uint32_t ssrc() const {
    if (connected()) {
      return local_stats[0].ssrc;
    } else {
      return 0;
    }
  }

  int64_t payload_bytes_rcvd = 0;

  int64_t header_and_padding_bytes_rcvd = 0;
  int packets_rcvd = 0;
  int packets_lost = 0;
  absl::optional<uint32_t> nacks_sent;


  double jitter_buffer_delay_seconds = 0.0;




  absl::optional<double> jitter_buffer_target_delay_seconds;




  absl::optional<double> jitter_buffer_minimum_delay_seconds;


  uint64_t jitter_buffer_emitted_count = 0;



  absl::optional<int64_t> last_packet_received_timestamp_ms;

  absl::optional<int64_t> estimated_playout_ntp_timestamp_ms;
  std::string codec_name;
  absl::optional<int> codec_payload_type;
  std::vector<SsrcReceiverInfo> local_stats;
  std::vector<SsrcSenderInfo> remote_stats;
};

struct VoiceSenderInfo : public MediaSenderInfo {
  VoiceSenderInfo();
  ~VoiceSenderInfo();
  int jitter_ms = 0;

  int audio_level = 0;


  double total_input_energy = 0.0;
  double total_input_duration = 0.0;
  webrtc::ANAStats ana_statistics;
  webrtc::AudioProcessingStats apm_statistics;
};

struct VoiceReceiverInfo : public MediaReceiverInfo {
  VoiceReceiverInfo();
  ~VoiceReceiverInfo();
  int jitter_ms = 0;
  int jitter_buffer_ms = 0;
  int jitter_buffer_preferred_ms = 0;
  int delay_estimate_ms = 0;
  int audio_level = 0;


  double total_output_energy = 0.0;
  uint64_t total_samples_received = 0;
  double total_output_duration = 0.0;
  uint64_t concealed_samples = 0;
  uint64_t silent_concealed_samples = 0;
  uint64_t concealment_events = 0;
  uint64_t inserted_samples_for_deceleration = 0;
  uint64_t removed_samples_for_acceleration = 0;
  uint64_t fec_packets_received = 0;
  uint64_t fec_packets_discarded = 0;


  uint64_t packets_discarded = 0;


  float expand_rate = 0.0f;

  float speech_expand_rate = 0.0f;

  float secondary_decoded_rate = 0.0f;





  float secondary_discarded_rate = 0.0f;

  float accelerate_rate = 0.0f;

  float preemptive_expand_rate = 0.0f;
  int decoding_calls_to_silence_generator = 0;
  int decoding_calls_to_neteq = 0;
  int decoding_normal = 0;

  int decoding_plc = 0;
  int decoding_codec_plc = 0;
  int decoding_cng = 0;
  int decoding_plc_cng = 0;
  int decoding_muted_output = 0;

  int64_t capture_start_ntp_time_ms = -1;

  uint64_t jitter_buffer_flushes = 0;

  uint64_t delayed_packet_outage_samples = 0;

  double relative_packet_arrival_delay_seconds = 0.0;


  int32_t interruption_count = 0;
  int32_t total_interruption_duration_ms = 0;


  absl::optional<int64_t> last_sender_report_timestamp_ms;
  absl::optional<int64_t> last_sender_report_remote_timestamp_ms;
  uint32_t sender_reports_packets_sent = 0;
  uint64_t sender_reports_bytes_sent = 0;
  uint64_t sender_reports_reports_count = 0;
  absl::optional<webrtc::TimeDelta> round_trip_time;
  webrtc::TimeDelta total_round_trip_time = webrtc::TimeDelta::Zero();
  int round_trip_time_measurements = 0;
};

struct VideoSenderInfo : public MediaSenderInfo {
  VideoSenderInfo();
  ~VideoSenderInfo();
  std::vector<SsrcGroup> ssrc_groups;
  std::string encoder_implementation_name;
  int firs_rcvd = 0;
  int plis_rcvd = 0;
  int send_frame_width = 0;
  int send_frame_height = 0;
  int frames = 0;
  double framerate_input = 0;
  int framerate_sent = 0;
  int aggregated_framerate_sent = 0;
  int nominal_bitrate = 0;
  int adapt_reason = 0;
  int adapt_changes = 0;

  webrtc::QualityLimitationReason quality_limitation_reason =
      webrtc::QualityLimitationReason::kNone;

  std::map<webrtc::QualityLimitationReason, int64_t>
      quality_limitation_durations_ms;

  uint32_t quality_limitation_resolution_changes = 0;
  int avg_encode_ms = 0;
  int encode_usage_percent = 0;
  uint32_t frames_encoded = 0;
  uint32_t key_frames_encoded = 0;

  uint64_t total_encode_time_ms = 0;

  uint64_t total_encoded_bytes_target = 0;
  bool has_entered_low_resolution = false;
  absl::optional<uint64_t> qp_sum;
  webrtc::VideoContentType content_type = webrtc::VideoContentType::UNSPECIFIED;
  uint32_t frames_sent = 0;

  uint32_t huge_frames_sent = 0;
  uint32_t aggregated_huge_frames_sent = 0;
  absl::optional<std::string> rid;
  absl::optional<bool> power_efficient_encoder;
};

struct VideoReceiverInfo : public MediaReceiverInfo {
  VideoReceiverInfo();
  ~VideoReceiverInfo();
  std::vector<SsrcGroup> ssrc_groups;
  std::string decoder_implementation_name;
  absl::optional<bool> power_efficient_decoder;
  int packets_concealed = 0;
  int firs_sent = 0;
  int plis_sent = 0;
  int frame_width = 0;
  int frame_height = 0;
  int framerate_rcvd = 0;
  int framerate_decoded = 0;
  int framerate_output = 0;

  int framerate_render_input = 0;

  int framerate_render_output = 0;
  uint32_t frames_received = 0;
  uint32_t frames_dropped = 0;
  uint32_t frames_decoded = 0;
  uint32_t key_frames_decoded = 0;
  uint32_t frames_rendered = 0;
  absl::optional<uint64_t> qp_sum;

  webrtc::TimeDelta total_decode_time = webrtc::TimeDelta::Zero();

  webrtc::TimeDelta total_processing_delay = webrtc::TimeDelta::Zero();
  webrtc::TimeDelta total_assembly_time = webrtc::TimeDelta::Zero();
  uint32_t frames_assembled_from_multiple_packets = 0;
  double total_inter_frame_delay = 0;
  double total_squared_inter_frame_delay = 0;
  int64_t interframe_delay_max_ms = -1;
  uint32_t freeze_count = 0;
  uint32_t pause_count = 0;
  uint32_t total_freezes_duration_ms = 0;
  uint32_t total_pauses_duration_ms = 0;
  uint32_t total_frames_duration_ms = 0;
  double sum_squared_frame_durations = 0.0;
  uint32_t jitter_ms = 0;

  webrtc::VideoContentType content_type = webrtc::VideoContentType::UNSPECIFIED;




  int decode_ms = 0;

  int max_decode_ms = 0;

  int jitter_buffer_ms = 0;

  int min_playout_delay_ms = 0;

  int render_delay_ms = 0;


  int target_delay_ms = 0;

  int current_delay_ms = 0;

  int64_t capture_start_ntp_time_ms = -1;

  int64_t first_frame_received_to_decoded_ms = -1;


  absl::optional<webrtc::TimingFrameInfo> timing_frame_info;
};

struct BandwidthEstimationInfo {
  int available_send_bandwidth = 0;
  int available_recv_bandwidth = 0;
  int target_enc_bitrate = 0;
  int actual_enc_bitrate = 0;
  int retransmit_bitrate = 0;
  int transmit_bitrate = 0;
  int64_t bucket_delay = 0;
};

typedef std::map<int, webrtc::RtpCodecParameters> RtpCodecParametersMap;

struct VoiceMediaInfo {
  VoiceMediaInfo();
  ~VoiceMediaInfo();
  void Clear() {
    senders.clear();
    receivers.clear();
    send_codecs.clear();
    receive_codecs.clear();
  }
  std::vector<VoiceSenderInfo> senders;
  std::vector<VoiceReceiverInfo> receivers;
  RtpCodecParametersMap send_codecs;
  RtpCodecParametersMap receive_codecs;
  int32_t device_underrun_count = 0;
};

struct VideoMediaInfo {
  VideoMediaInfo();
  ~VideoMediaInfo();
  void Clear() {
    senders.clear();
    aggregated_senders.clear();
    receivers.clear();
    send_codecs.clear();
    receive_codecs.clear();
  }



  std::vector<VideoSenderInfo> senders;




  std::vector<VideoSenderInfo> aggregated_senders;
  std::vector<VideoReceiverInfo> receivers;
  RtpCodecParametersMap send_codecs;
  RtpCodecParametersMap receive_codecs;
};

struct RtcpParameters {
  bool reduced_size = false;
  bool remote_estimate = false;
};

template <class Codec>
struct RtpParameters {
  virtual ~RtpParameters() = default;

  std::vector<Codec> codecs;
  std::vector<webrtc::RtpExtension> extensions;


  bool is_stream_active = true;

  RtcpParameters rtcp;

  std::string ToString() const {
    rtc::StringBuilder ost;
    ost << "{";
    const char* separator = "";
    for (const auto& entry : ToStringMap()) {
      ost << separator << entry.first << ": " << entry.second;
      separator = ", ";
    }
    ost << "}";
    return ost.Release();
  }

 protected:
  virtual std::map<std::string, std::string> ToStringMap() const {
    return {{"codecs", VectorToString(codecs)},
            {"extensions", VectorToString(extensions)}};
  }
};

// encapsulate all the parameters needed for an RtpSender.
template <class Codec>
struct RtpSendParameters : RtpParameters<Codec> {
  int max_bandwidth_bps = -1;


  std::string mid;
  bool extmap_allow_mixed = false;

 protected:
  std::map<std::string, std::string> ToStringMap() const override {
    auto params = RtpParameters<Codec>::ToStringMap();
    params["max_bandwidth_bps"] = rtc::ToString(max_bandwidth_bps);
    params["mid"] = (mid.empty() ? "<not set>" : mid);
    params["extmap-allow-mixed"] = extmap_allow_mixed ? "true" : "false";
    return params;
  }
};

struct AudioSendParameters : RtpSendParameters<AudioCodec> {
  AudioSendParameters();
  ~AudioSendParameters() override;
  AudioOptions options;

 protected:
  std::map<std::string, std::string> ToStringMap() const override;
};

struct AudioRecvParameters : RtpParameters<AudioCodec> {};

class VoiceMediaChannel : public MediaChannel, public Delayable {
 public:
  VoiceMediaChannel(webrtc::TaskQueueBase* network_thread,
                    bool enable_dscp = false)
      : MediaChannel(network_thread, enable_dscp) {}
  ~VoiceMediaChannel() override {}

  cricket::MediaType media_type() const override;
  virtual bool SetSendParameters(const AudioSendParameters& params) = 0;
  virtual bool SetRecvParameters(const AudioRecvParameters& params) = 0;

  virtual webrtc::RtpParameters GetRtpReceiveParameters(
      uint32_t ssrc) const = 0;


  virtual webrtc::RtpParameters GetDefaultRtpReceiveParameters() const = 0;

  virtual void SetPlayout(bool playout) = 0;

  virtual void SetSend(bool send) = 0;

  virtual bool SetAudioSend(uint32_t ssrc,
                            bool enable,
                            const AudioOptions* options,
                            AudioSource* source) = 0;

  virtual bool SetOutputVolume(uint32_t ssrc, double volume) = 0;

  virtual bool SetDefaultOutputVolume(double volume) = 0;

  virtual bool CanInsertDtmf() = 0;




  virtual bool InsertDtmf(uint32_t ssrc, int event, int duration) = 0;

  virtual bool GetStats(VoiceMediaInfo* info,
                        bool get_and_clear_legacy_stats) = 0;

  virtual void SetRawAudioSink(
      uint32_t ssrc,
      std::unique_ptr<webrtc::AudioSinkInterface> sink) = 0;
  virtual void SetDefaultRawAudioSink(
      std::unique_ptr<webrtc::AudioSinkInterface> sink) = 0;

  virtual std::vector<webrtc::RtpSource> GetSources(uint32_t ssrc) const = 0;
};

// encapsulate all the parameters needed for a video RtpSender.
struct VideoSendParameters : RtpSendParameters<VideoCodec> {
  VideoSendParameters();
  ~VideoSendParameters() override;






  bool conference_mode = false;

 protected:
  std::map<std::string, std::string> ToStringMap() const override;
};

// encapsulate all the parameters needed for a video RtpReceiver.
struct VideoRecvParameters : RtpParameters<VideoCodec> {};

class VideoMediaChannel : public MediaChannel, public Delayable {
 public:
  explicit VideoMediaChannel(webrtc::TaskQueueBase* network_thread,
                             bool enable_dscp = false)
      : MediaChannel(network_thread, enable_dscp) {}
  ~VideoMediaChannel() override {}

  cricket::MediaType media_type() const override;
  virtual bool SetSendParameters(const VideoSendParameters& params) = 0;
  virtual bool SetRecvParameters(const VideoRecvParameters& params) = 0;

  virtual webrtc::RtpParameters GetRtpReceiveParameters(
      uint32_t ssrc) const = 0;


  virtual webrtc::RtpParameters GetDefaultRtpReceiveParameters() const = 0;

  virtual bool GetSendCodec(VideoCodec* send_codec) = 0;

  virtual bool SetSend(bool send) = 0;


  virtual bool SetVideoSend(
      uint32_t ssrc,
      const VideoOptions* options,
      rtc::VideoSourceInterface<webrtc::VideoFrame>* source) = 0;

  virtual bool SetSink(uint32_t ssrc,
                       rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) = 0;

  virtual void SetDefaultSink(
      rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) = 0;








  virtual void FillBitrateInfo(BandwidthEstimationInfo* bwe_info) = 0;

  virtual bool GetStats(VideoMediaInfo* info) = 0;

  virtual void SetRecordableEncodedFrameCallback(
      uint32_t ssrc,
      std::function<void(const webrtc::RecordableEncodedFrame&)> callback) = 0;

  virtual void ClearRecordableEncodedFrameCallback(uint32_t ssrc) = 0;


  virtual void RequestRecvKeyFrame(uint32_t ssrc) = 0;

  virtual void GenerateSendKeyFrame(uint32_t ssrc) = 0;

  virtual std::vector<webrtc::RtpSource> GetSources(uint32_t ssrc) const = 0;
};

// DataMediaChannel::SignalDataReceived and in all of the signals that
// signal fires, on up the chain.
struct ReceiveDataParams {


  int sid = 0;

  webrtc::DataMessageType type = webrtc::DataMessageType::kText;

  int seq_num = 0;
};

enum SendDataResult { SDR_SUCCESS, SDR_ERROR, SDR_BLOCK };

}  // namespace cricket

#endif  // MEDIA_BASE_MEDIA_CHANNEL_H_
