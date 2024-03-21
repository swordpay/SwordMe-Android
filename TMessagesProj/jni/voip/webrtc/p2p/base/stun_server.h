/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_STUN_SERVER_H_
#define P2P_BASE_STUN_SERVER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "absl/strings/string_view.h"
#include "api/transport/stun.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/async_udp_socket.h"
#include "rtc_base/socket_address.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace cricket {

const int STUN_SERVER_PORT = 3478;

class StunServer : public sigslot::has_slots<> {
 public:

  explicit StunServer(rtc::AsyncUDPSocket* socket);

  ~StunServer() override;

 protected:

  void OnPacket(rtc::AsyncPacketSocket* socket,
                const char* buf,
                size_t size,
                const rtc::SocketAddress& remote_addr,
                const int64_t& packet_time_us);

  virtual void OnBindingRequest(StunMessage* msg,
                                const rtc::SocketAddress& addr);
  void OnAllocateRequest(StunMessage* msg, const rtc::SocketAddress& addr);
  void OnSharedSecretRequest(StunMessage* msg, const rtc::SocketAddress& addr);
  void OnSendRequest(StunMessage* msg, const rtc::SocketAddress& addr);

  void SendErrorResponse(const StunMessage& msg,
                         const rtc::SocketAddress& addr,
                         int error_code,
                         absl::string_view error_desc);

  void SendResponse(const StunMessage& msg, const rtc::SocketAddress& addr);

  void GetStunBindResponse(StunMessage* message,
                           const rtc::SocketAddress& remote_addr,
                           StunMessage* response) const;

 private:
  std::unique_ptr<rtc::AsyncUDPSocket> socket_;
};

}  // namespace cricket

#endif  // P2P_BASE_STUN_SERVER_H_
