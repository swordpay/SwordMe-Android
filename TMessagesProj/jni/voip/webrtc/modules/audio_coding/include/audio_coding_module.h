/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_INCLUDE_AUDIO_CODING_MODULE_H_
#define MODULES_AUDIO_CODING_INCLUDE_AUDIO_CODING_MODULE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder.h"
#include "api/function_view.h"
#include "api/neteq/neteq.h"
#include "api/neteq/neteq_factory.h"
#include "modules/audio_coding/include/audio_coding_module_typedefs.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {

class AudioDecoder;
class AudioEncoder;
class AudioFrame;
struct RTPHeader;

class AudioPacketizationCallback {
 public:
  virtual ~AudioPacketizationCallback() {}

  virtual int32_t SendData(AudioFrameType frame_type,
                           uint8_t payload_type,
                           uint32_t timestamp,
                           const uint8_t* payload_data,
                           size_t payload_len_bytes,
                           int64_t absolute_capture_timestamp_ms) {


    return SendData(frame_type, payload_type, timestamp, payload_data,
                    payload_len_bytes);
  }
  virtual int32_t SendData(AudioFrameType frame_type,
                           uint8_t payload_type,
                           uint32_t timestamp,
                           const uint8_t* payload_data,
                           size_t payload_len_bytes) {
    RTC_DCHECK_NOTREACHED() << "This method must be overridden, or not used.";
    return -1;
  }
};

class AudioCodingModule {
 protected:
  AudioCodingModule() {}

 public:
  struct Config {
    explicit Config(
        rtc::scoped_refptr<AudioDecoderFactory> decoder_factory = nullptr);
    Config(const Config&);
    ~Config();

    NetEq::Config neteq_config;
    Clock* clock;
    rtc::scoped_refptr<AudioDecoderFactory> decoder_factory;
    NetEqFactory* neteq_factory = nullptr;
  };

  static AudioCodingModule* Create(const Config& config);
  virtual ~AudioCodingModule() = default;








  virtual void ModifyEncoder(
      rtc::FunctionView<void(std::unique_ptr<AudioEncoder>*)> modifier) = 0;

  void SetEncoder(std::unique_ptr<AudioEncoder> new_encoder) {
    ModifyEncoder([&](std::unique_ptr<AudioEncoder>* encoder) {
      *encoder = std::move(new_encoder);
    });
  }















  virtual int32_t RegisterTransportCallback(
      AudioPacketizationCallback* transport) = 0;

















  virtual int32_t Add10MsData(const AudioFrame& audio_frame) = 0;















  virtual int SetPacketLossRate(int packet_loss_rate) = 0;
















  virtual int32_t InitializeReceiver() = 0;

  virtual void SetReceiveCodecs(
      const std::map<int, SdpAudioFormat>& codecs) = 0;














  virtual int32_t IncomingPacket(const uint8_t* incoming_payload,
                                 size_t payload_len_bytes,
                                 const RTPHeader& rtp_header) = 0;




















  virtual int32_t PlayoutData10Ms(int32_t desired_freq_hz,
                                  AudioFrame* audio_frame,
                                  bool* muted) = 0;















  virtual int32_t GetNetworkStatistics(
      NetworkStatistics* network_statistics) = 0;

  virtual ANAStats GetANAStats() const = 0;

  virtual int GetTargetBitrate() const = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_CODING_INCLUDE_AUDIO_CODING_MODULE_H_
