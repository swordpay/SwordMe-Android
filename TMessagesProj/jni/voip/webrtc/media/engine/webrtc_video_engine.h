/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_ENGINE_WEBRTC_VIDEO_ENGINE_H_
#define MEDIA_ENGINE_WEBRTC_VIDEO_ENGINE_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "api/call/transport.h"
#include "api/sequence_checker.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/transport/field_trial_based_config.h"
#include "api/video/video_bitrate_allocator_factory.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "api/video/video_source_interface.h"
#include "api/video_codecs/sdp_video_format.h"
#include "call/call.h"
#include "call/flexfec_receive_stream.h"
#include "call/video_receive_stream.h"
#include "call/video_send_stream.h"
#include "media/base/media_engine.h"
#include "media/engine/unhandled_packets_buffer.h"
#include "rtc_base/network_route.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/system/no_unique_address.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {
class VideoDecoderFactory;
class VideoEncoderFactory;
}  // namespace webrtc

namespace cricket {

class WebRtcVideoChannel;

// Inputs StreamStats for all types of substreams (kMedia, kRtx, kFlexfec) and
// merges any non-kMedia substream stats object into its referenced kMedia-type
// substream. The resulting substreams are all kMedia. This means, for example,
// that packet and byte counters of RTX and FlexFEC streams are accounted for in
// the relevant RTP media stream's stats. This makes the resulting StreamStats
// objects ready to be turned into "outbound-rtp" stats objects for GetStats()
// which does not create separate stream stats objects for complementary
// streams.
std::map<uint32_t, webrtc::VideoSendStream::StreamStats>
MergeInfoAboutOutboundRtpSubstreamsForTesting(
    const std::map<uint32_t, webrtc::VideoSendStream::StreamStats>& substreams);

class UnsignalledSsrcHandler {
 public:
  enum Action {
    kDropPacket,
    kDeliverPacket,
  };
  virtual Action OnUnsignalledSsrc(WebRtcVideoChannel* channel,
                                   uint32_t ssrc,
                                   absl::optional<uint32_t> rtx_ssrc) = 0;
  virtual ~UnsignalledSsrcHandler() = default;
};

class DefaultUnsignalledSsrcHandler : public UnsignalledSsrcHandler {
 public:
  DefaultUnsignalledSsrcHandler();
  Action OnUnsignalledSsrc(WebRtcVideoChannel* channel,
                           uint32_t ssrc,
                           absl::optional<uint32_t> rtx_ssrc) override;

  rtc::VideoSinkInterface<webrtc::VideoFrame>* GetDefaultSink() const;
  void SetDefaultSink(WebRtcVideoChannel* channel,
                      rtc::VideoSinkInterface<webrtc::VideoFrame>* sink);

  virtual ~DefaultUnsignalledSsrcHandler() = default;

 private:
  rtc::VideoSinkInterface<webrtc::VideoFrame>* default_sink_;
};

class WebRtcVideoEngine : public VideoEngineInterface {
 public:


  WebRtcVideoEngine(
      std::unique_ptr<webrtc::VideoEncoderFactory> video_encoder_factory,
      std::unique_ptr<webrtc::VideoDecoderFactory> video_decoder_factory,
      const webrtc::FieldTrialsView& trials);

  ~WebRtcVideoEngine() override;

  VideoMediaChannel* CreateMediaChannel(
      webrtc::Call* call,
      const MediaConfig& config,
      const VideoOptions& options,
      const webrtc::CryptoOptions& crypto_options,
      webrtc::VideoBitrateAllocatorFactory* video_bitrate_allocator_factory)
      override;

  std::vector<VideoCodec> send_codecs() const override {
    return send_codecs(true);
  }
  std::vector<VideoCodec> recv_codecs() const override {
    return recv_codecs(true);
  }
  std::vector<VideoCodec> send_codecs(bool include_rtx) const override;
  std::vector<VideoCodec> recv_codecs(bool include_rtx) const override;
  std::vector<webrtc::RtpHeaderExtensionCapability> GetRtpHeaderExtensions()
      const override;

 private:
  const std::unique_ptr<webrtc::VideoDecoderFactory> decoder_factory_;
  const std::unique_ptr<webrtc::VideoEncoderFactory> encoder_factory_;
  const std::unique_ptr<webrtc::VideoBitrateAllocatorFactory>
      bitrate_allocator_factory_;
  const webrtc::FieldTrialsView& trials_;
};

class WebRtcVideoChannel : public VideoMediaChannel,
                           public webrtc::Transport,
                           public webrtc::EncoderSwitchRequestCallback {
 public:
  WebRtcVideoChannel(
      webrtc::Call* call,
      const MediaConfig& config,
      const VideoOptions& options,
      const webrtc::CryptoOptions& crypto_options,
      webrtc::VideoEncoderFactory* encoder_factory,
      webrtc::VideoDecoderFactory* decoder_factory,
      webrtc::VideoBitrateAllocatorFactory* bitrate_allocator_factory);
  ~WebRtcVideoChannel() override;

  bool SetSendParameters(const VideoSendParameters& params) override;
  bool SetRecvParameters(const VideoRecvParameters& params) override;
  webrtc::RtpParameters GetRtpSendParameters(uint32_t ssrc) const override;
  webrtc::RTCError SetRtpSendParameters(
      uint32_t ssrc,
      const webrtc::RtpParameters& parameters) override;
  webrtc::RtpParameters GetRtpReceiveParameters(uint32_t ssrc) const override;
  webrtc::RtpParameters GetDefaultRtpReceiveParameters() const override;
  bool GetSendCodec(VideoCodec* send_codec) override;
  bool SetSend(bool send) override;
  bool SetVideoSend(
      uint32_t ssrc,
      const VideoOptions* options,
      rtc::VideoSourceInterface<webrtc::VideoFrame>* source) override;
  bool AddSendStream(const StreamParams& sp) override;
  bool RemoveSendStream(uint32_t ssrc) override;
  bool AddRecvStream(const StreamParams& sp) override;
  bool AddRecvStream(const StreamParams& sp, bool default_stream);
  bool RemoveRecvStream(uint32_t ssrc) override;
  void ResetUnsignaledRecvStream() override;
  void OnDemuxerCriteriaUpdatePending() override;
  void OnDemuxerCriteriaUpdateComplete() override;
  bool SetSink(uint32_t ssrc,
               rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
  void SetDefaultSink(
      rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
  void FillBitrateInfo(BandwidthEstimationInfo* bwe_info) override;
  bool GetStats(VideoMediaInfo* info) override;

  void OnPacketReceived(rtc::CopyOnWriteBuffer packet,
                        int64_t packet_time_us) override;
  void OnPacketSent(const rtc::SentPacket& sent_packet) override;
  void OnReadyToSend(bool ready) override;
  void OnNetworkRouteChanged(absl::string_view transport_name,
                             const rtc::NetworkRoute& network_route) override;
  void SetInterface(NetworkInterface* iface) override;




  void SetFrameDecryptor(uint32_t ssrc,
                         rtc::scoped_refptr<webrtc::FrameDecryptorInterface>
                             frame_decryptor) override;



  void SetFrameEncryptor(uint32_t ssrc,
                         rtc::scoped_refptr<webrtc::FrameEncryptorInterface>
                             frame_encryptor) override;


  void SetEncoderSelector(uint32_t ssrc,
                          webrtc::VideoEncoderFactory::EncoderSelectorInterface*
                              encoder_selector) override;

  void SetVideoCodecSwitchingEnabled(bool enabled) override;

  bool SetBaseMinimumPlayoutDelayMs(uint32_t ssrc, int delay_ms) override;

  absl::optional<int> GetBaseMinimumPlayoutDelayMs(
      uint32_t ssrc) const override;

  bool sending() const {
    RTC_DCHECK_RUN_ON(&thread_checker_);
    return sending_;
  }

  absl::optional<uint32_t> GetDefaultReceiveStreamSsrc();

  StreamParams unsignaled_stream_params() {
    RTC_DCHECK_RUN_ON(&thread_checker_);
    return unsignaled_stream_params_;
  }



  enum AdaptReason {
    ADAPTREASON_NONE = 0,
    ADAPTREASON_CPU = 1,
    ADAPTREASON_BANDWIDTH = 2,
  };

  static constexpr int kDefaultQpMax = 56;

  std::vector<webrtc::RtpSource> GetSources(uint32_t ssrc) const override;


  void BackfillBufferedPackets(rtc::ArrayView<const uint32_t> ssrcs);

  void RequestEncoderFallback() override;
  void RequestEncoderSwitch(const webrtc::SdpVideoFormat& format,
                            bool allow_default_fallback) override;

  void SetRecordableEncodedFrameCallback(
      uint32_t ssrc,
      std::function<void(const webrtc::RecordableEncodedFrame&)> callback)
      override;
  void ClearRecordableEncodedFrameCallback(uint32_t ssrc) override;
  void RequestRecvKeyFrame(uint32_t ssrc) override;
  void GenerateSendKeyFrame(uint32_t ssrc) override;

  void SetEncoderToPacketizerFrameTransformer(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer)
      override;
  void SetDepacketizerToDecoderFrameTransformer(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer)
      override;

 private:
  class WebRtcVideoReceiveStream;


  WebRtcVideoReceiveStream* FindReceiveStream(uint32_t ssrc)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);

  struct VideoCodecSettings {
    VideoCodecSettings();


    bool operator==(const VideoCodecSettings& other) const;
    bool operator!=(const VideoCodecSettings& other) const;


    static bool EqualsDisregardingFlexfec(const VideoCodecSettings& a,
                                          const VideoCodecSettings& b);

    VideoCodec codec;
    webrtc::UlpfecConfig ulpfec;
    int flexfec_payload_type;  // -1 if absent.
    int rtx_payload_type;      // -1 if absent.
    int rtx_time;              // -1 if absent.
  };

  struct ChangedSendParameters {

    absl::optional<VideoCodecSettings> send_codec;
    absl::optional<std::vector<VideoCodecSettings>> negotiated_codecs;
    absl::optional<std::vector<webrtc::RtpExtension>> rtp_header_extensions;
    absl::optional<std::string> mid;
    absl::optional<bool> extmap_allow_mixed;
    absl::optional<int> max_bandwidth_bps;
    absl::optional<bool> conference_mode;
    absl::optional<webrtc::RtcpMode> rtcp_mode;
  };

  struct ChangedRecvParameters {

    absl::optional<std::vector<VideoCodecSettings>> codec_settings;
    absl::optional<std::vector<webrtc::RtpExtension>> rtp_header_extensions;



    absl::optional<int> flexfec_payload_type;
  };

  bool GetChangedSendParameters(const VideoSendParameters& params,
                                ChangedSendParameters* changed_params) const
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  bool ApplyChangedParams(const ChangedSendParameters& changed_params);
  bool GetChangedRecvParameters(const VideoRecvParameters& params,
                                ChangedRecvParameters* changed_params) const
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);

  void ConfigureReceiverRtp(
      webrtc::VideoReceiveStreamInterface::Config* config,
      webrtc::FlexfecReceiveStream::Config* flexfec_config,
      const StreamParams& sp) const
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  bool ValidateSendSsrcAvailability(const StreamParams& sp) const
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  bool ValidateReceiveSsrcAvailability(const StreamParams& sp) const
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  void DeleteReceiveStream(WebRtcVideoReceiveStream* stream)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);

  static std::string CodecSettingsVectorToString(
      const std::vector<VideoCodecSettings>& codecs);



  static void ExtractCodecInformation(
      rtc::ArrayView<const VideoCodecSettings> recv_codecs,
      std::map<int, int>& rtx_associated_payload_types,
      std::set<int>& raw_payload_types,
      std::vector<webrtc::VideoReceiveStreamInterface::Decoder>& decoders);


  void SetReceiverReportSsrc(uint32_t ssrc) RTC_RUN_ON(&thread_checker_);

  class WebRtcVideoSendStream {
   public:
    WebRtcVideoSendStream(
        webrtc::Call* call,
        const StreamParams& sp,
        webrtc::VideoSendStream::Config config,
        const VideoOptions& options,
        bool enable_cpu_overuse_detection,
        int max_bitrate_bps,
        const absl::optional<VideoCodecSettings>& codec_settings,
        const absl::optional<std::vector<webrtc::RtpExtension>>& rtp_extensions,
        const VideoSendParameters& send_params);
    ~WebRtcVideoSendStream();

    void SetSendParameters(const ChangedSendParameters& send_params);
    webrtc::RTCError SetRtpParameters(const webrtc::RtpParameters& parameters);
    webrtc::RtpParameters GetRtpParameters() const;

    void SetFrameEncryptor(
        rtc::scoped_refptr<webrtc::FrameEncryptorInterface> frame_encryptor);

    bool SetVideoSend(const VideoOptions* options,
                      rtc::VideoSourceInterface<webrtc::VideoFrame>* source);


    void SetEncoderSelector(
        webrtc::VideoEncoderFactory::EncoderSelectorInterface*
            encoder_selector);

    void SetSend(bool send);

    const std::vector<uint32_t>& GetSsrcs() const;

    std::vector<VideoSenderInfo> GetPerLayerVideoSenderInfos(bool log_stats);


    VideoSenderInfo GetAggregatedVideoSenderInfo(
        const std::vector<VideoSenderInfo>& infos) const;
    void FillBitrateInfo(BandwidthEstimationInfo* bwe_info);

    void SetEncoderToPacketizerFrameTransformer(
        rtc::scoped_refptr<webrtc::FrameTransformerInterface>
            frame_transformer);
    void GenerateKeyFrame();

   private:




    struct VideoSendStreamParameters {
      VideoSendStreamParameters(
          webrtc::VideoSendStream::Config config,
          const VideoOptions& options,
          int max_bitrate_bps,
          const absl::optional<VideoCodecSettings>& codec_settings);
      webrtc::VideoSendStream::Config config;
      VideoOptions options;
      int max_bitrate_bps;
      bool conference_mode;
      absl::optional<VideoCodecSettings> codec_settings;



      webrtc::VideoEncoderConfig encoder_config;
    };

    rtc::scoped_refptr<webrtc::VideoEncoderConfig::EncoderSpecificSettings>
    ConfigureVideoEncoderSettings(const VideoCodec& codec);
    void SetCodec(const VideoCodecSettings& codec);
    void RecreateWebRtcStream();
    webrtc::VideoEncoderConfig CreateVideoEncoderConfig(
        const VideoCodec& codec) const;
    void ReconfigureEncoder();


    void UpdateSendState();

    webrtc::DegradationPreference GetDegradationPreference() const
        RTC_EXCLUSIVE_LOCKS_REQUIRED(&thread_checker_);

    RTC_NO_UNIQUE_ADDRESS webrtc::SequenceChecker thread_checker_;
    webrtc::TaskQueueBase* const worker_thread_;
    const std::vector<uint32_t> ssrcs_ RTC_GUARDED_BY(&thread_checker_);
    const std::vector<SsrcGroup> ssrc_groups_ RTC_GUARDED_BY(&thread_checker_);
    webrtc::Call* const call_;
    const bool enable_cpu_overuse_detection_;
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source_
        RTC_GUARDED_BY(&thread_checker_);

    webrtc::VideoSendStream* stream_ RTC_GUARDED_BY(&thread_checker_);



    VideoSendStreamParameters parameters_ RTC_GUARDED_BY(&thread_checker_);





    webrtc::RtpParameters rtp_parameters_ RTC_GUARDED_BY(&thread_checker_);

    bool sending_ RTC_GUARDED_BY(&thread_checker_);



    const bool disable_automatic_resize_;
  };


  class WebRtcVideoReceiveStream
      : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
   public:
    WebRtcVideoReceiveStream(
        WebRtcVideoChannel* channel,
        webrtc::Call* call,
        const StreamParams& sp,
        webrtc::VideoReceiveStreamInterface::Config config,
        bool default_stream,
        const std::vector<VideoCodecSettings>& recv_codecs,
        const webrtc::FlexfecReceiveStream::Config& flexfec_config);
    ~WebRtcVideoReceiveStream();

    webrtc::VideoReceiveStreamInterface& stream();

    webrtc::FlexfecReceiveStream* flexfec_stream();

    const std::vector<uint32_t>& GetSsrcs() const;

    std::vector<webrtc::RtpSource> GetSources();

    webrtc::RtpParameters GetRtpParameters() const;

    void SetFeedbackParameters(bool lntf_enabled,
                               bool nack_enabled,
                               bool transport_cc_enabled,
                               webrtc::RtcpMode rtcp_mode,
                               int rtx_time);
    void SetRecvParameters(const ChangedRecvParameters& recv_params);

    void OnFrame(const webrtc::VideoFrame& frame) override;
    bool IsDefaultStream() const;

    void SetFrameDecryptor(
        rtc::scoped_refptr<webrtc::FrameDecryptorInterface> frame_decryptor);

    bool SetBaseMinimumPlayoutDelayMs(int delay_ms);

    int GetBaseMinimumPlayoutDelayMs() const;

    void SetSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink);

    VideoReceiverInfo GetVideoReceiverInfo(bool log_stats);

    void SetRecordableEncodedFrameCallback(
        std::function<void(const webrtc::RecordableEncodedFrame&)> callback);
    void ClearRecordableEncodedFrameCallback();
    void GenerateKeyFrame();

    void SetDepacketizerToDecoderFrameTransformer(
        rtc::scoped_refptr<webrtc::FrameTransformerInterface>
            frame_transformer);

    void SetLocalSsrc(uint32_t local_ssrc);

   private:



    void SetFlexFecPayload(int payload_type);

    void RecreateReceiveStream();
    void CreateReceiveStream();
    void StartReceiveStream();



    bool ReconfigureCodecs(const std::vector<VideoCodecSettings>& recv_codecs);

    WebRtcVideoChannel* const channel_;
    webrtc::Call* const call_;
    const StreamParams stream_params_;



    webrtc::VideoReceiveStreamInterface* stream_;
    const bool default_stream_;
    webrtc::VideoReceiveStreamInterface::Config config_;
    webrtc::FlexfecReceiveStream::Config flexfec_config_;
    webrtc::FlexfecReceiveStream* flexfec_stream_;

    webrtc::Mutex sink_lock_;
    rtc::VideoSinkInterface<webrtc::VideoFrame>* sink_
        RTC_GUARDED_BY(sink_lock_);


    rtc::TimestampWrapAroundHandler timestamp_wraparound_handler_
        RTC_GUARDED_BY(sink_lock_);
    int64_t first_frame_timestamp_ RTC_GUARDED_BY(sink_lock_);


    int64_t estimated_remote_start_ntp_time_ms_ RTC_GUARDED_BY(sink_lock_);
  };

  void Construct(webrtc::Call* call, WebRtcVideoEngine* engine);

  bool SendRtp(const uint8_t* data,
               size_t len,
               const webrtc::PacketOptions& options) override;
  bool SendRtcp(const uint8_t* data, size_t len) override;




  static std::vector<VideoCodecSettings> MapCodecs(
      const std::vector<VideoCodec>& codecs);

  std::vector<VideoCodecSettings> SelectSendVideoCodecs(
      const std::vector<VideoCodecSettings>& remote_mapped_codecs) const
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);

  static bool NonFlexfecReceiveCodecsHaveChanged(
      std::vector<VideoCodecSettings> before,
      std::vector<VideoCodecSettings> after);

  void FillSenderStats(VideoMediaInfo* info, bool log_stats)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  void FillReceiverStats(VideoMediaInfo* info, bool log_stats)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  void FillBandwidthEstimationStats(const webrtc::Call::Stats& stats,
                                    VideoMediaInfo* info)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);
  void FillSendAndReceiveCodecStats(VideoMediaInfo* video_media_info)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(thread_checker_);

  webrtc::TaskQueueBase* const worker_thread_;
  webrtc::ScopedTaskSafety task_safety_;
  RTC_NO_UNIQUE_ADDRESS webrtc::SequenceChecker network_thread_checker_;
  RTC_NO_UNIQUE_ADDRESS webrtc::SequenceChecker thread_checker_;

  uint32_t rtcp_receiver_report_ssrc_ RTC_GUARDED_BY(thread_checker_);
  bool sending_ RTC_GUARDED_BY(thread_checker_);
  webrtc::Call* const call_;

  DefaultUnsignalledSsrcHandler default_unsignalled_ssrc_handler_
      RTC_GUARDED_BY(thread_checker_);
  UnsignalledSsrcHandler* const unsignalled_ssrc_handler_
      RTC_GUARDED_BY(thread_checker_);

  int default_recv_base_minimum_delay_ms_ RTC_GUARDED_BY(thread_checker_) = 0;

  const MediaConfig::Video video_config_ RTC_GUARDED_BY(thread_checker_);

  std::map<uint32_t, WebRtcVideoSendStream*> send_streams_
      RTC_GUARDED_BY(thread_checker_);
  std::map<uint32_t, WebRtcVideoReceiveStream*> receive_streams_
      RTC_GUARDED_BY(thread_checker_);














  uint32_t demuxer_criteria_id_ RTC_GUARDED_BY(thread_checker_) = 0;
  uint32_t demuxer_criteria_completed_id_ RTC_GUARDED_BY(thread_checker_) = 0;
  absl::optional<int64_t> last_unsignalled_ssrc_creation_time_ms_
      RTC_GUARDED_BY(thread_checker_);
  std::set<uint32_t> send_ssrcs_ RTC_GUARDED_BY(thread_checker_);
  std::set<uint32_t> receive_ssrcs_ RTC_GUARDED_BY(thread_checker_);

  absl::optional<VideoCodecSettings> send_codec_
      RTC_GUARDED_BY(thread_checker_);
  std::vector<VideoCodecSettings> negotiated_codecs_
      RTC_GUARDED_BY(thread_checker_);

  std::vector<webrtc::RtpExtension> send_rtp_extensions_
      RTC_GUARDED_BY(thread_checker_);

  webrtc::VideoEncoderFactory* const encoder_factory_
      RTC_GUARDED_BY(thread_checker_);
  webrtc::VideoDecoderFactory* const decoder_factory_
      RTC_GUARDED_BY(thread_checker_);
  webrtc::VideoBitrateAllocatorFactory* const bitrate_allocator_factory_
      RTC_GUARDED_BY(thread_checker_);
  std::vector<VideoCodecSettings> recv_codecs_ RTC_GUARDED_BY(thread_checker_);
  std::vector<webrtc::RtpExtension> recv_rtp_extensions_
      RTC_GUARDED_BY(thread_checker_);


  int recv_flexfec_payload_type_ RTC_GUARDED_BY(thread_checker_);
  webrtc::BitrateConstraints bitrate_config_ RTC_GUARDED_BY(thread_checker_);


  VideoSendParameters send_params_ RTC_GUARDED_BY(thread_checker_);
  VideoOptions default_send_options_ RTC_GUARDED_BY(thread_checker_);
  VideoRecvParameters recv_params_ RTC_GUARDED_BY(thread_checker_);
  int64_t last_stats_log_ms_ RTC_GUARDED_BY(thread_checker_);
  const bool discard_unknown_ssrc_packets_ RTC_GUARDED_BY(thread_checker_);




  StreamParams unsignaled_stream_params_ RTC_GUARDED_BY(thread_checker_);


  const webrtc::CryptoOptions crypto_options_ RTC_GUARDED_BY(thread_checker_);

  rtc::scoped_refptr<webrtc::FrameTransformerInterface>
      unsignaled_frame_transformer_ RTC_GUARDED_BY(thread_checker_);

  std::unique_ptr<UnhandledPacketsBuffer> unknown_ssrc_packet_buffer_
      RTC_GUARDED_BY(thread_checker_);



  bool allow_codec_switching_ = false;
};

}  // namespace cricket

#endif  // MEDIA_ENGINE_WEBRTC_VIDEO_ENGINE_H_
