/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_RTP_TRANSPORT_INTERNAL_H_
#define PC_RTP_TRANSPORT_INTERNAL_H_

#include <string>

#include "call/rtp_demuxer.h"
#include "p2p/base/ice_transport_internal.h"
#include "pc/session_description.h"
#include "rtc_base/network_route.h"
#include "rtc_base/ssl_stream_adapter.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace rtc {
class CopyOnWriteBuffer;
struct PacketOptions;
}  // namespace rtc

namespace webrtc {

// but is accessible to internal classes in order to send and receive RTP and
// RTCP packets belonging to a single RTP session. Additional convenience and
// configuration methods are also provided.
class RtpTransportInternal : public sigslot::has_slots<> {
 public:
  virtual ~RtpTransportInternal() = default;

  virtual void SetRtcpMuxEnabled(bool enable) = 0;

  virtual const std::string& transport_name() const = 0;

  virtual int SetRtpOption(rtc::Socket::Option opt, int value) = 0;
  virtual int SetRtcpOption(rtc::Socket::Option opt, int value) = 0;

  virtual bool rtcp_mux_enabled() const = 0;

  virtual bool IsReadyToSend() const = 0;



  sigslot::signal1<bool> SignalReadyToSend;



  sigslot::signal2<rtc::CopyOnWriteBuffer*, int64_t> SignalRtcpPacketReceived;


  sigslot::signal1<absl::optional<rtc::NetworkRoute>> SignalNetworkRouteChanged;

  sigslot::signal3<rtc::CopyOnWriteBuffer*, int64_t, bool> SignalRtpPacketReceived;


  sigslot::signal1<bool> SignalWritableState;

  sigslot::signal1<const rtc::SentPacket&> SignalSentPacket;

  virtual bool IsWritable(bool rtcp) const = 0;


  virtual bool SendRtpPacket(rtc::CopyOnWriteBuffer* packet,
                             const rtc::PacketOptions& options,
                             int flags) = 0;

  virtual bool SendRtcpPacket(rtc::CopyOnWriteBuffer* packet,
                              const rtc::PacketOptions& options,
                              int flags) = 0;










  virtual void UpdateRtpHeaderExtensionMap(
      const cricket::RtpHeaderExtensions& header_extensions) = 0;

  virtual bool IsSrtpActive() const = 0;

  virtual bool RegisterRtpDemuxerSink(const RtpDemuxerCriteria& criteria,
                                      RtpPacketSinkInterface* sink) = 0;

  virtual bool UnregisterRtpDemuxerSink(RtpPacketSinkInterface* sink) = 0;
};

}  // namespace webrtc

#endif  // PC_RTP_TRANSPORT_INTERNAL_H_
