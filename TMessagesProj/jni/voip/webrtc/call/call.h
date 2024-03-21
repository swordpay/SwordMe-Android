/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef CALL_CALL_H_
#define CALL_CALL_H_

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "api/adaptation/resource.h"
#include "api/media_types.h"
#include "api/task_queue/task_queue_base.h"
#include "call/audio_receive_stream.h"
#include "call/audio_send_stream.h"
#include "call/call_config.h"
#include "call/flexfec_receive_stream.h"
#include "call/packet_receiver.h"
#include "call/rtp_transport_controller_send_interface.h"
#include "call/video_receive_stream.h"
#include "call/video_send_stream.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/network/sent_packet.h"
#include "rtc_base/network_route.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

// and incoming media streams, transported over one or more RTP transports.

// are assumed to have the same remote endpoint and will share bitrate estimates
// etc.

// between the PeerConnection and the Call.

class Call {
 public:
  using Config = CallConfig;

  struct Stats {
    std::string ToString(int64_t time_ms) const;

    int send_bandwidth_bps = 0;       // Estimated available send bandwidth.
    int max_padding_bitrate_bps = 0;  // Cumulative configured max padding.
    int recv_bandwidth_bps = 0;       // Estimated available receive bandwidth.
    int64_t pacer_delay_ms = 0;
    int64_t rtt_ms = -1;
  };

  static Call* Create(const Call::Config& config);
  static Call* Create(const Call::Config& config,
                      Clock* clock,
                      std::unique_ptr<RtpTransportControllerSendInterface>
                          transportControllerSend);

  virtual AudioSendStream* CreateAudioSendStream(
      const AudioSendStream::Config& config) = 0;

  virtual void DestroyAudioSendStream(AudioSendStream* send_stream) = 0;

  virtual AudioReceiveStreamInterface* CreateAudioReceiveStream(
      const AudioReceiveStreamInterface::Config& config) = 0;
  virtual void DestroyAudioReceiveStream(
      AudioReceiveStreamInterface* receive_stream) = 0;

  virtual VideoSendStream* CreateVideoSendStream(
      VideoSendStream::Config config,
      VideoEncoderConfig encoder_config) = 0;
  virtual VideoSendStream* CreateVideoSendStream(
      VideoSendStream::Config config,
      VideoEncoderConfig encoder_config,
      std::unique_ptr<FecController> fec_controller);
  virtual void DestroyVideoSendStream(VideoSendStream* send_stream) = 0;

  virtual VideoReceiveStreamInterface* CreateVideoReceiveStream(
      VideoReceiveStreamInterface::Config configuration) = 0;
  virtual void DestroyVideoReceiveStream(
      VideoReceiveStreamInterface* receive_stream) = 0;



  virtual FlexfecReceiveStream* CreateFlexfecReceiveStream(
      const FlexfecReceiveStream::Config config) = 0;
  virtual void DestroyFlexfecReceiveStream(
      FlexfecReceiveStream* receive_stream) = 0;



  virtual void AddAdaptationResource(rtc::scoped_refptr<Resource> resource) = 0;



  virtual PacketReceiver* Receiver() = 0;





  virtual RtpTransportControllerSendInterface* GetTransportControllerSend() = 0;


  virtual Stats GetStats() const = 0;



  virtual void SignalChannelNetworkState(MediaType media,
                                         NetworkState state) = 0;

  virtual void OnAudioTransportOverheadChanged(
      int transport_overhead_per_packet) = 0;


  virtual void OnLocalSsrcUpdated(AudioReceiveStreamInterface& stream,
                                  uint32_t local_ssrc) = 0;
  virtual void OnLocalSsrcUpdated(VideoReceiveStreamInterface& stream,
                                  uint32_t local_ssrc) = 0;
  virtual void OnLocalSsrcUpdated(FlexfecReceiveStream& stream,
                                  uint32_t local_ssrc) = 0;

  virtual void OnUpdateSyncGroup(AudioReceiveStreamInterface& stream,
                                 absl::string_view sync_group) = 0;

  virtual void OnSentPacket(const rtc::SentPacket& sent_packet) = 0;

  virtual void SetClientBitratePreferences(
      const BitrateSettings& preferences) = 0;

  virtual const FieldTrialsView& trials() const = 0;

  virtual TaskQueueBase* network_thread() const = 0;
  virtual TaskQueueBase* worker_thread() const = 0;

  virtual ~Call() {}
};

}  // namespace webrtc

#endif  // CALL_CALL_H_
