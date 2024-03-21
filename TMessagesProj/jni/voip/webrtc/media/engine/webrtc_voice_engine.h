/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_ENGINE_WEBRTC_VOICE_ENGINE_H_
#define MEDIA_ENGINE_WEBRTC_VOICE_ENGINE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/field_trials_view.h"
#include "api/scoped_refptr.h"
#include "api/sequence_checker.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/task_queue/task_queue_factory.h"
#include "api/transport/rtp/rtp_source.h"
#include "call/audio_state.h"
#include "call/call.h"
#include "media/base/media_engine.h"
#include "media/base/rtp_utils.h"
#include "modules/async_audio_processing/async_audio_processing.h"
#include "rtc_base/buffer.h"
#include "rtc_base/network_route.h"
#include "rtc_base/task_queue.h"

namespace webrtc {
class AudioFrameProcessor;
}

namespace cricket {

class AudioSource;
class WebRtcVoiceMediaChannel;

// It uses the WebRtc VoiceEngine library for audio handling.
class WebRtcVoiceEngine final : public VoiceEngineInterface {
  friend class WebRtcVoiceMediaChannel;

 public:
  WebRtcVoiceEngine(
      webrtc::TaskQueueFactory* task_queue_factory,
      webrtc::AudioDeviceModule* adm,
      const rtc::scoped_refptr<webrtc::AudioEncoderFactory>& encoder_factory,
      const rtc::scoped_refptr<webrtc::AudioDecoderFactory>& decoder_factory,
      rtc::scoped_refptr<webrtc::AudioMixer> audio_mixer,
      rtc::scoped_refptr<webrtc::AudioProcessing> audio_processing,
      webrtc::AudioFrameProcessor* audio_frame_processor,
      const webrtc::FieldTrialsView& trials);

  WebRtcVoiceEngine() = delete;
  WebRtcVoiceEngine(const WebRtcVoiceEngine&) = delete;
  WebRtcVoiceEngine& operator=(const WebRtcVoiceEngine&) = delete;

  ~WebRtcVoiceEngine() override;

  void Init() override;

  rtc::scoped_refptr<webrtc::AudioState> GetAudioState() const override;
  VoiceMediaChannel* CreateMediaChannel(
      webrtc::Call* call,
      const MediaConfig& config,
      const AudioOptions& options,
      const webrtc::CryptoOptions& crypto_options) override;

  const std::vector<AudioCodec>& send_codecs() const override;
  const std::vector<AudioCodec>& recv_codecs() const override;
  std::vector<webrtc::RtpHeaderExtensionCapability> GetRtpHeaderExtensions()
      const override;




  bool StartAecDump(webrtc::FileWrapper file, int64_t max_size_bytes) override;

  void StopAecDump() override;

 private:



  void ApplyOptions(const AudioOptions& options);

  int CreateVoEChannel();

  webrtc::TaskQueueFactory* const task_queue_factory_;
  std::unique_ptr<rtc::TaskQueue> low_priority_worker_queue_;

  webrtc::AudioDeviceModule* adm();
  webrtc::AudioProcessing* apm() const;
  webrtc::AudioState* audio_state();

  std::vector<AudioCodec> CollectCodecs(
      const std::vector<webrtc::AudioCodecSpec>& specs) const;

  webrtc::SequenceChecker signal_thread_checker_;
  webrtc::SequenceChecker worker_thread_checker_;

  rtc::scoped_refptr<webrtc::AudioDeviceModule> adm_;
  rtc::scoped_refptr<webrtc::AudioEncoderFactory> encoder_factory_;
  rtc::scoped_refptr<webrtc::AudioDecoderFactory> decoder_factory_;
  rtc::scoped_refptr<webrtc::AudioMixer> audio_mixer_;

  rtc::scoped_refptr<webrtc::AudioProcessing> apm_;

  webrtc::AudioFrameProcessor* const audio_frame_processor_;

  rtc::scoped_refptr<webrtc::AudioState> audio_state_;
  std::vector<AudioCodec> send_codecs_;
  std::vector<AudioCodec> recv_codecs_;
  bool is_dumping_aec_ = false;
  bool initialized_ = false;

  size_t audio_jitter_buffer_max_packets_ = 200;
  bool audio_jitter_buffer_fast_accelerate_ = false;
  int audio_jitter_buffer_min_delay_ms_ = 0;

  const bool minimized_remsampling_on_mobile_trial_enabled_;
};

// WebRtc Voice Engine.
class WebRtcVoiceMediaChannel final : public VoiceMediaChannel,
                                      public webrtc::Transport {
 public:
  WebRtcVoiceMediaChannel(WebRtcVoiceEngine* engine,
                          const MediaConfig& config,
                          const AudioOptions& options,
                          const webrtc::CryptoOptions& crypto_options,
                          webrtc::Call* call);

  WebRtcVoiceMediaChannel() = delete;
  WebRtcVoiceMediaChannel(const WebRtcVoiceMediaChannel&) = delete;
  WebRtcVoiceMediaChannel& operator=(const WebRtcVoiceMediaChannel&) = delete;

  ~WebRtcVoiceMediaChannel() override;

  const AudioOptions& options() const { return options_; }

  bool SetSendParameters(const AudioSendParameters& params) override;
  bool SetRecvParameters(const AudioRecvParameters& params) override;
  webrtc::RtpParameters GetRtpSendParameters(uint32_t ssrc) const override;
  webrtc::RTCError SetRtpSendParameters(
      uint32_t ssrc,
      const webrtc::RtpParameters& parameters) override;
  webrtc::RtpParameters GetRtpReceiveParameters(uint32_t ssrc) const override;
  webrtc::RtpParameters GetDefaultRtpReceiveParameters() const override;

  void SetPlayout(bool playout) override;
  void SetSend(bool send) override;
  bool SetAudioSend(uint32_t ssrc,
                    bool enable,
                    const AudioOptions* options,
                    AudioSource* source) override;
  bool AddSendStream(const StreamParams& sp) override;
  bool RemoveSendStream(uint32_t ssrc) override;
  bool AddRecvStream(const StreamParams& sp) override;
  bool RemoveRecvStream(uint32_t ssrc) override;
  void ResetUnsignaledRecvStream() override;
  void OnDemuxerCriteriaUpdatePending() override;
  void OnDemuxerCriteriaUpdateComplete() override;




  void SetFrameDecryptor(uint32_t ssrc,
                         rtc::scoped_refptr<webrtc::FrameDecryptorInterface>
                             frame_decryptor) override;



  void SetFrameEncryptor(uint32_t ssrc,
                         rtc::scoped_refptr<webrtc::FrameEncryptorInterface>
                             frame_encryptor) override;

  bool SetOutputVolume(uint32_t ssrc, double volume) override;

  bool SetDefaultOutputVolume(double volume) override;

  bool SetBaseMinimumPlayoutDelayMs(uint32_t ssrc, int delay_ms) override;
  absl::optional<int> GetBaseMinimumPlayoutDelayMs(
      uint32_t ssrc) const override;

  bool CanInsertDtmf() override;
  bool InsertDtmf(uint32_t ssrc, int event, int duration) override;

  void OnPacketReceived(rtc::CopyOnWriteBuffer packet,
                        int64_t packet_time_us) override;
  void OnPacketSent(const rtc::SentPacket& sent_packet) override;
  void OnNetworkRouteChanged(absl::string_view transport_name,
                             const rtc::NetworkRoute& network_route) override;
  void OnReadyToSend(bool ready) override;
  bool GetStats(VoiceMediaInfo* info, bool get_and_clear_legacy_stats) override;

  void SetRawAudioSink(
      uint32_t ssrc,
      std::unique_ptr<webrtc::AudioSinkInterface> sink) override;


  void SetDefaultRawAudioSink(
      std::unique_ptr<webrtc::AudioSinkInterface> sink) override;

  std::vector<webrtc::RtpSource> GetSources(uint32_t ssrc) const override;


  void SetEncoderToPacketizerFrameTransformer(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer)
      override;
  void SetDepacketizerToDecoderFrameTransformer(
      uint32_t ssrc,
      rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer)
      override;

  bool SendRtp(const uint8_t* data,
               size_t len,
               const webrtc::PacketOptions& options) override;

  bool SendRtcp(const uint8_t* data, size_t len) override;

 private:
  bool SetOptions(const AudioOptions& options);
  bool SetRecvCodecs(const std::vector<AudioCodec>& codecs);
  bool SetSendCodecs(const std::vector<AudioCodec>& codecs);
  bool SetLocalSource(uint32_t ssrc, AudioSource* source);
  bool MuteStream(uint32_t ssrc, bool mute);

  WebRtcVoiceEngine* engine() { return engine_; }
  int CreateVoEChannel();
  bool DeleteVoEChannel(int channel);
  bool SetMaxSendBitrate(int bps);
  void SetupRecording();


  bool MaybeDeregisterUnsignaledRecvStream(uint32_t ssrc);

  webrtc::TaskQueueBase* const worker_thread_;
  webrtc::ScopedTaskSafety task_safety_;
  webrtc::SequenceChecker network_thread_checker_;

  WebRtcVoiceEngine* const engine_ = nullptr;
  std::vector<AudioCodec> send_codecs_;


  std::map<int, webrtc::SdpAudioFormat> decoder_map_;
  std::vector<AudioCodec> recv_codecs_;

  int max_send_bitrate_bps_ = 0;
  AudioOptions options_;
  absl::optional<int> dtmf_payload_type_;
  int dtmf_payload_freq_ = -1;
  bool recv_transport_cc_enabled_ = false;
  bool recv_nack_enabled_ = false;
  bool enable_non_sender_rtt_ = false;
  bool playout_ = false;
  bool send_ = false;
  webrtc::Call* const call_ = nullptr;

  const MediaConfig::Audio audio_config_;

  std::vector<uint32_t> unsignaled_recv_ssrcs_;




  StreamParams unsignaled_stream_params_;

  double default_recv_volume_ = 1.0;

  int default_recv_base_minimum_delay_ms_ = 0;

  std::unique_ptr<webrtc::AudioSinkInterface> default_sink_;



  uint32_t receiver_reports_ssrc_ = 0xFA17FA17u;

  class WebRtcAudioSendStream;
  std::map<uint32_t, WebRtcAudioSendStream*> send_streams_;
  std::vector<webrtc::RtpExtension> send_rtp_extensions_;
  std::string mid_;

  class WebRtcAudioReceiveStream;
  std::map<uint32_t, WebRtcAudioReceiveStream*> recv_streams_;
  std::vector<webrtc::RtpExtension> recv_rtp_extensions_;

  absl::optional<webrtc::AudioSendStream::Config::SendCodecSpec>
      send_codec_spec_;

  const webrtc::AudioCodecPairId codec_pair_id_ =
      webrtc::AudioCodecPairId::Create();


  const webrtc::CryptoOptions crypto_options_;

  rtc::scoped_refptr<webrtc::FrameDecryptorInterface>
      unsignaled_frame_decryptor_;
  rtc::scoped_refptr<webrtc::FrameTransformerInterface>
      unsignaled_frame_transformer_;
};
}  // namespace cricket

#endif  // MEDIA_ENGINE_WEBRTC_VOICE_ENGINE_H_
