/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_BASE_MEDIA_ENGINE_H_
#define MEDIA_BASE_MEDIA_ENGINE_H_

#include <memory>
#include <string>
#include <vector>

#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/crypto/crypto_options.h"
#include "api/field_trials_view.h"
#include "api/rtp_parameters.h"
#include "api/video/video_bitrate_allocator_factory.h"
#include "call/audio_state.h"
#include "media/base/codec.h"
#include "media/base/media_channel.h"
#include "media/base/media_config.h"
#include "media/base/video_common.h"
#include "rtc_base/system/file_wrapper.h"

namespace webrtc {
class AudioDeviceModule;
class AudioMixer;
class AudioProcessing;
class Call;
}  // namespace webrtc

namespace cricket {

// least one video codec of the list. If the list is empty, no check is done.
webrtc::RTCError CheckScalabilityModeValues(
    const webrtc::RtpParameters& new_parameters,
    rtc::ArrayView<cricket::VideoCodec> codecs);

// with CheckScalabilityModeValues().
webrtc::RTCError CheckRtpParametersValues(
    const webrtc::RtpParameters& new_parameters,
    rtc::ArrayView<cricket::VideoCodec> codecs);

// checks all parameters with CheckRtpParametersValues().
webrtc::RTCError CheckRtpParametersInvalidModificationAndValues(
    const webrtc::RtpParameters& old_parameters,
    const webrtc::RtpParameters& new_parameters,
    rtc::ArrayView<cricket::VideoCodec> codecs);

// checks parameters (except SVC) with CheckRtpParametersValues(). It should
// usually be paired with a call to CheckScalabilityModeValues().
webrtc::RTCError CheckRtpParametersInvalidModificationAndValues(
    const webrtc::RtpParameters& old_parameters,
    const webrtc::RtpParameters& new_parameters);

struct RtpCapabilities {
  RtpCapabilities();
  ~RtpCapabilities();
  std::vector<webrtc::RtpExtension> header_extensions;
};

class RtpHeaderExtensionQueryInterface {
 public:
  virtual ~RtpHeaderExtensionQueryInterface() = default;


  virtual std::vector<webrtc::RtpHeaderExtensionCapability>
  GetRtpHeaderExtensions() const = 0;
};

class VoiceEngineInterface : public RtpHeaderExtensionQueryInterface {
 public:
  VoiceEngineInterface() = default;
  virtual ~VoiceEngineInterface() = default;

  VoiceEngineInterface(const VoiceEngineInterface&) = delete;
  VoiceEngineInterface& operator=(const VoiceEngineInterface&) = delete;


  virtual void Init() = 0;

  virtual rtc::scoped_refptr<webrtc::AudioState> GetAudioState() const = 0;


  virtual VoiceMediaChannel* CreateMediaChannel(
      webrtc::Call* call,
      const MediaConfig& config,
      const AudioOptions& options,
      const webrtc::CryptoOptions& crypto_options) = 0;

  virtual const std::vector<AudioCodec>& send_codecs() const = 0;
  virtual const std::vector<AudioCodec>& recv_codecs() const = 0;



  virtual bool StartAecDump(webrtc::FileWrapper file,
                            int64_t max_size_bytes) = 0;

  virtual void StopAecDump() = 0;
};

class VideoEngineInterface : public RtpHeaderExtensionQueryInterface {
 public:
  VideoEngineInterface() = default;
  virtual ~VideoEngineInterface() = default;

  VideoEngineInterface(const VideoEngineInterface&) = delete;
  VideoEngineInterface& operator=(const VideoEngineInterface&) = delete;


  virtual VideoMediaChannel* CreateMediaChannel(
      webrtc::Call* call,
      const MediaConfig& config,
      const VideoOptions& options,
      const webrtc::CryptoOptions& crypto_options,
      webrtc::VideoBitrateAllocatorFactory*
          video_bitrate_allocator_factory) = 0;

  virtual std::vector<VideoCodec> send_codecs() const = 0;
  virtual std::vector<VideoCodec> recv_codecs() const = 0;



  virtual std::vector<VideoCodec> send_codecs(bool include_rtx) const {
    RTC_DCHECK(include_rtx);
    return send_codecs();
  }
  virtual std::vector<VideoCodec> recv_codecs(bool include_rtx) const {
    RTC_DCHECK(include_rtx);
    return recv_codecs();
  }
};

// subclassed to support different media componentry backends.
// It supports voice and video operations in the same class to facilitate
// proper synchronization between both media types.
class MediaEngineInterface {
 public:
  virtual ~MediaEngineInterface() {}

  virtual bool Init() = 0;

  virtual VoiceEngineInterface& voice() = 0;
  virtual VideoEngineInterface& video() = 0;
  virtual const VoiceEngineInterface& voice() const = 0;
  virtual const VideoEngineInterface& video() const = 0;
};

// voice and video engine classes.
// Optionally owns a FieldTrialsView trials map.
class CompositeMediaEngine : public MediaEngineInterface {
 public:
  CompositeMediaEngine(std::unique_ptr<webrtc::FieldTrialsView> trials,
                       std::unique_ptr<VoiceEngineInterface> audio_engine,
                       std::unique_ptr<VideoEngineInterface> video_engine);
  CompositeMediaEngine(std::unique_ptr<VoiceEngineInterface> audio_engine,
                       std::unique_ptr<VideoEngineInterface> video_engine);
  ~CompositeMediaEngine() override;

  bool Init() override;

  VoiceEngineInterface& voice() override;
  VideoEngineInterface& video() override;
  const VoiceEngineInterface& voice() const override;
  const VideoEngineInterface& video() const override;

 private:
  const std::unique_ptr<webrtc::FieldTrialsView> trials_;
  const std::unique_ptr<VoiceEngineInterface> voice_engine_;
  const std::unique_ptr<VideoEngineInterface> video_engine_;
};

webrtc::RtpParameters CreateRtpParametersWithOneEncoding();
webrtc::RtpParameters CreateRtpParametersWithEncodings(StreamParams sp);

// GetCapabilities(). The returned vector only shows what will definitely be
// offered by default, i.e. the list of extensions returned from
// GetRtpHeaderExtensions() that are not kStopped.
std::vector<webrtc::RtpExtension> GetDefaultEnabledRtpHeaderExtensions(
    const RtpHeaderExtensionQueryInterface& query_interface);

}  // namespace cricket

#endif  // MEDIA_BASE_MEDIA_ENGINE_H_
