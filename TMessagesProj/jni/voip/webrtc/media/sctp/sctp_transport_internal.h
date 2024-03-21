/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MEDIA_SCTP_SCTP_TRANSPORT_INTERNAL_H_
#define MEDIA_SCTP_SCTP_TRANSPORT_INTERNAL_H_

// anything in media/.

#include <memory>
#include <string>
#include <vector>

#include "api/transport/data_channel_transport_interface.h"
// For SendDataParams/ReceiveDataParams.
// TODO(deadbeef): Use something else for SCTP. It's confusing that we use an
// SSRC field for SID.
#include "media/base/media_channel.h"
#include "p2p/base/packet_transport_internal.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/thread.h"

namespace cricket {

// The size of the SCTP association send buffer. 256kB, the usrsctp default.
constexpr int kSctpSendBufferSize = 256 * 1024;

// are 0-based, the highest usable SID is 1023.
//
// It's recommended to use the maximum of 65535 in:
// https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13#section-6.2
// However, we use 1024 in order to save memory. usrsctp allocates 104 bytes
// for each pair of incoming/outgoing streams (on a 64-bit system), so 65535
// streams would waste ~6MB.
//
// Note: "max" and "min" here are inclusive.
constexpr uint16_t kMaxSctpStreams = 1024;
constexpr uint16_t kMaxSctpSid = kMaxSctpStreams - 1;
constexpr uint16_t kMinSctpSid = 0;

// connectee and connector must be using the same port. It is not related to the
// ports at the IP level. (Corresponds to: sockaddr_conn.sconn_port in
// usrsctp.h)
const int kSctpDefaultPort = 5000;

// https://www.iana.org/assignments/sctp-parameters/sctp-parameters.xhtml#sctp-parameters-24
enum class SctpErrorCauseCode : uint16_t {
  kInvalidStreamIdentifier = 1,
  kMissingMandatoryParameter = 2,
  kStaleCookieError = 3,
  kOutOfResource = 4,
  kUnresolvableAddress = 5,
  kUnrecognizedChunkType = 6,
  kInvalidMandatoryParameter = 7,
  kUnrecognizedParameters = 8,
  kNoUserData = 9,
  kCookieReceivedWhileShuttingDown = 10,
  kRestartWithNewAddresses = 11,
  kUserInitiatedAbort = 12,
  kProtocolViolation = 13,
};

// Exists to allow mock/fake SctpTransports to be created.
class SctpTransportInternal {
 public:
  virtual ~SctpTransportInternal() {}

  virtual void SetOnConnectedCallback(std::function<void()> callback) = 0;
  virtual void SetDataChannelSink(webrtc::DataChannelSink* sink) = 0;


  virtual void SetDtlsTransport(rtc::PacketTransportInternal* transport) = 0;
















  virtual bool Start(int local_sctp_port,
                     int remote_sctp_port,
                     int max_message_size) = 0;








  virtual bool OpenStream(int sid) = 0;




  virtual bool ResetStream(int sid) = 0;




  virtual bool SendData(int sid,
                        const webrtc::SendDataParams& params,
                        const rtc::CopyOnWriteBuffer& payload,
                        SendDataResult* result = nullptr) = 0;






  virtual bool ReadyToSendData() = 0;

  virtual int max_message_size() const = 0;


  virtual absl::optional<int> max_outbound_streams() const = 0;

  virtual absl::optional<int> max_inbound_streams() const = 0;

  virtual void set_debug_name_for_testing(const char* debug_name) = 0;
};

}  // namespace cricket

#endif  // MEDIA_SCTP_SCTP_TRANSPORT_INTERNAL_H_
