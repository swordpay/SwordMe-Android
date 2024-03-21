/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_PORT_INTERFACE_H_
#define P2P_BASE_PORT_INTERFACE_H_

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/candidate.h"
#include "p2p/base/transport_description.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/callback_list.h"
#include "rtc_base/socket_address.h"

namespace rtc {
class Network;
struct PacketOptions;
}  // namespace rtc

namespace cricket {
class Connection;
class IceMessage;
class StunMessage;
class StunStats;

enum ProtocolType {
  PROTO_UDP,
  PROTO_TCP,
  PROTO_SSLTCP,  // Pseudo-TLS.
  PROTO_TLS,
  PROTO_LAST = PROTO_TLS
};

// mechanism that can be used to create connections to similar mechanisms of
// the other client. Various types of ports will implement this interface.
class PortInterface {
 public:
  virtual ~PortInterface();

  virtual const std::string& Type() const = 0;
  virtual const rtc::Network* Network() const = 0;

  virtual void SetIceRole(IceRole role) = 0;
  virtual IceRole GetIceRole() const = 0;

  virtual void SetIceTiebreaker(uint64_t tiebreaker) = 0;
  virtual uint64_t IceTiebreaker() const = 0;

  virtual bool SharedSocket() const = 0;

  virtual bool SupportsProtocol(absl::string_view protocol) const = 0;





  virtual void PrepareAddress() = 0;

  virtual Connection* GetConnection(const rtc::SocketAddress& remote_addr) = 0;

  enum CandidateOrigin { ORIGIN_THIS_PORT, ORIGIN_OTHER_PORT, ORIGIN_MESSAGE };
  virtual Connection* CreateConnection(const Candidate& remote_candidate,
                                       CandidateOrigin origin) = 0;

  virtual int SetOption(rtc::Socket::Option opt, int value) = 0;
  virtual int GetOption(rtc::Socket::Option opt, int* value) = 0;
  virtual int GetError() = 0;

  virtual ProtocolType GetProtocol() const = 0;

  virtual const std::vector<Candidate>& Candidates() const = 0;


  virtual int SendTo(const void* data,
                     size_t size,
                     const rtc::SocketAddress& addr,
                     const rtc::PacketOptions& options,
                     bool payload) = 0;



  sigslot::signal6<PortInterface*,
                   const rtc::SocketAddress&,
                   ProtocolType,
                   IceMessage*,
                   const std::string&,
                   bool>
      SignalUnknownAddress;


  virtual void SendBindingErrorResponse(StunMessage* message,
                                        const rtc::SocketAddress& addr,
                                        int error_code,
                                        absl::string_view reason) = 0;


  virtual void SubscribePortDestroyed(
      std::function<void(PortInterface*)> callback) = 0;

  sigslot::signal1<PortInterface*> SignalRoleConflict;




  virtual void EnablePortPackets() = 0;
  sigslot::
      signal4<PortInterface*, const char*, size_t, const rtc::SocketAddress&>
          SignalReadPacket;

  sigslot::signal1<const rtc::SentPacket&> SignalSentPacket;

  virtual std::string ToString() const = 0;

  virtual void GetStunStats(absl::optional<StunStats>* stats) = 0;

 protected:
  PortInterface();
};

}  // namespace cricket

#endif  // P2P_BASE_PORT_INTERFACE_H_
