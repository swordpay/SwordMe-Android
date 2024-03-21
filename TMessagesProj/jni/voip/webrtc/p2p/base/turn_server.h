/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_TURN_SERVER_H_
#define P2P_BASE_TURN_SERVER_H_

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "api/sequence_checker.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/task_queue/task_queue_base.h"
#include "api/units/time_delta.h"
#include "p2p/base/port_interface.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/socket_address.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace rtc {
class ByteBufferWriter;
class PacketSocketFactory;
}  // namespace rtc

namespace cricket {

class StunMessage;
class TurnMessage;
class TurnServer;

const int TURN_SERVER_PORT = 3478;

class TurnServerConnection {
 public:
  TurnServerConnection() : proto_(PROTO_UDP), socket_(NULL) {}
  TurnServerConnection(const rtc::SocketAddress& src,
                       ProtocolType proto,
                       rtc::AsyncPacketSocket* socket);
  const rtc::SocketAddress& src() const { return src_; }
  rtc::AsyncPacketSocket* socket() { return socket_; }
  bool operator==(const TurnServerConnection& t) const;
  bool operator<(const TurnServerConnection& t) const;
  std::string ToString() const;

 private:
  rtc::SocketAddress src_;
  rtc::SocketAddress dst_;
  cricket::ProtocolType proto_;
  rtc::AsyncPacketSocket* socket_;
};

// The object is created when an allocation request is received, and then
// handles TURN messages (via HandleTurnMessage) and channel data messages
// (via HandleChannelData) for this allocation when received by the server.
// The object informs the server when its lifetime timer expires.
class TurnServerAllocation : public sigslot::has_slots<> {
 public:
  TurnServerAllocation(TurnServer* server_,
                       webrtc::TaskQueueBase* thread,
                       const TurnServerConnection& conn,
                       rtc::AsyncPacketSocket* server_socket,
                       absl::string_view key);
  ~TurnServerAllocation() override;

  TurnServerConnection* conn() { return &conn_; }
  const std::string& key() const { return key_; }
  const std::string& transaction_id() const { return transaction_id_; }
  const std::string& username() const { return username_; }
  const std::string& last_nonce() const { return last_nonce_; }
  void set_last_nonce(absl::string_view nonce) {
    last_nonce_ = std::string(nonce);
  }

  std::string ToString() const;

  void HandleTurnMessage(const TurnMessage* msg);
  void HandleChannelData(const char* data, size_t size);

 private:
  struct Channel {
    webrtc::ScopedTaskSafety pending_delete;
    int id;
    rtc::SocketAddress peer;
  };
  struct Permission {
    webrtc::ScopedTaskSafety pending_delete;
    rtc::IPAddress peer;
  };
  using PermissionList = std::list<Permission>;
  using ChannelList = std::list<Channel>;

  void PostDeleteSelf(webrtc::TimeDelta delay);

  void HandleAllocateRequest(const TurnMessage* msg);
  void HandleRefreshRequest(const TurnMessage* msg);
  void HandleSendIndication(const TurnMessage* msg);
  void HandleCreatePermissionRequest(const TurnMessage* msg);
  void HandleChannelBindRequest(const TurnMessage* msg);

  void OnExternalPacket(rtc::AsyncPacketSocket* socket,
                        const char* data,
                        size_t size,
                        const rtc::SocketAddress& addr,
                        const int64_t& packet_time_us);

  static webrtc::TimeDelta ComputeLifetime(const TurnMessage& msg);
  bool HasPermission(const rtc::IPAddress& addr);
  void AddPermission(const rtc::IPAddress& addr);
  PermissionList::iterator FindPermission(const rtc::IPAddress& addr);
  ChannelList::iterator FindChannel(int channel_id);
  ChannelList::iterator FindChannel(const rtc::SocketAddress& addr);

  void SendResponse(TurnMessage* msg);
  void SendBadRequestResponse(const TurnMessage* req);
  void SendErrorResponse(const TurnMessage* req,
                         int code,
                         absl::string_view reason);
  void SendExternal(const void* data,
                    size_t size,
                    const rtc::SocketAddress& peer);

  TurnServer* const server_;
  webrtc::TaskQueueBase* const thread_;
  TurnServerConnection conn_;
  std::unique_ptr<rtc::AsyncPacketSocket> external_socket_;
  std::string key_;
  std::string transaction_id_;
  std::string username_;
  std::string last_nonce_;
  PermissionList perms_;
  ChannelList channels_;
  webrtc::ScopedTaskSafety safety_;
};

class TurnAuthInterface {
 public:



  virtual bool GetKey(absl::string_view username,
                      absl::string_view realm,
                      std::string* key) = 0;
  virtual ~TurnAuthInterface() = default;
};

class TurnRedirectInterface {
 public:
  virtual bool ShouldRedirect(const rtc::SocketAddress& address,
                              rtc::SocketAddress* out) = 0;
  virtual ~TurnRedirectInterface() {}
};

class StunMessageObserver {
 public:
  virtual void ReceivedMessage(const TurnMessage* msg) = 0;
  virtual void ReceivedChannelData(const char* data, size_t size) = 0;
  virtual ~StunMessageObserver() {}
};

// AddInternalServerSocket, and a factory to create external sockets via
// SetExternalSocketFactory, and it's ready to go.
// Not yet wired up: TCP support.
class TurnServer : public sigslot::has_slots<> {
 public:
  typedef std::map<TurnServerConnection, std::unique_ptr<TurnServerAllocation>>
      AllocationMap;

  explicit TurnServer(webrtc::TaskQueueBase* thread);
  ~TurnServer() override;

  const std::string& realm() const {
    RTC_DCHECK_RUN_ON(thread_);
    return realm_;
  }
  void set_realm(absl::string_view realm) {
    RTC_DCHECK_RUN_ON(thread_);
    realm_ = std::string(realm);
  }

  const std::string& software() const {
    RTC_DCHECK_RUN_ON(thread_);
    return software_;
  }
  void set_software(absl::string_view software) {
    RTC_DCHECK_RUN_ON(thread_);
    software_ = std::string(software);
  }

  const AllocationMap& allocations() const {
    RTC_DCHECK_RUN_ON(thread_);
    return allocations_;
  }

  void set_auth_hook(TurnAuthInterface* auth_hook) {
    RTC_DCHECK_RUN_ON(thread_);
    auth_hook_ = auth_hook;
  }

  void set_redirect_hook(TurnRedirectInterface* redirect_hook) {
    RTC_DCHECK_RUN_ON(thread_);
    redirect_hook_ = redirect_hook;
  }

  void set_enable_otu_nonce(bool enable) {
    RTC_DCHECK_RUN_ON(thread_);
    enable_otu_nonce_ = enable;
  }

  void set_reject_private_addresses(bool filter) {
    RTC_DCHECK_RUN_ON(thread_);
    reject_private_addresses_ = filter;
  }

  void set_enable_permission_checks(bool enable) {
    RTC_DCHECK_RUN_ON(thread_);
    enable_permission_checks_ = enable;
  }

  void AddInternalSocket(rtc::AsyncPacketSocket* socket, ProtocolType proto);



  void AddInternalServerSocket(
      rtc::Socket* socket,
      ProtocolType proto,
      std::unique_ptr<rtc::SSLAdapterFactory> ssl_adapter_factory = nullptr);

  void SetExternalSocketFactory(rtc::PacketSocketFactory* factory,
                                const rtc::SocketAddress& address);

  std::string SetTimestampForNextNonce(int64_t timestamp) {
    RTC_DCHECK_RUN_ON(thread_);
    ts_for_next_nonce_ = timestamp;
    return GenerateNonce(timestamp);
  }

  void SetStunMessageObserver(std::unique_ptr<StunMessageObserver> observer) {
    RTC_DCHECK_RUN_ON(thread_);
    stun_message_observer_ = std::move(observer);
  }

 private:




  std::string GenerateNonce(int64_t now) const RTC_RUN_ON(thread_);
  void OnInternalPacket(rtc::AsyncPacketSocket* socket,
                        const char* data,
                        size_t size,
                        const rtc::SocketAddress& address,
                        const int64_t& packet_time_us);

  void OnNewInternalConnection(rtc::Socket* socket);

  void AcceptConnection(rtc::Socket* server_socket) RTC_RUN_ON(thread_);
  void OnInternalSocketClose(rtc::AsyncPacketSocket* socket, int err);

  void HandleStunMessage(TurnServerConnection* conn,
                         const char* data,
                         size_t size) RTC_RUN_ON(thread_);
  void HandleBindingRequest(TurnServerConnection* conn, const StunMessage* msg)
      RTC_RUN_ON(thread_);
  void HandleAllocateRequest(TurnServerConnection* conn,
                             const TurnMessage* msg,
                             absl::string_view key) RTC_RUN_ON(thread_);

  bool GetKey(const StunMessage* msg, std::string* key) RTC_RUN_ON(thread_);
  bool CheckAuthorization(TurnServerConnection* conn,
                          StunMessage* msg,
                          const char* data,
                          size_t size,
                          absl::string_view key) RTC_RUN_ON(thread_);
  bool ValidateNonce(absl::string_view nonce) const RTC_RUN_ON(thread_);

  TurnServerAllocation* FindAllocation(TurnServerConnection* conn)
      RTC_RUN_ON(thread_);
  TurnServerAllocation* CreateAllocation(TurnServerConnection* conn,
                                         int proto,
                                         absl::string_view key)
      RTC_RUN_ON(thread_);

  void SendErrorResponse(TurnServerConnection* conn,
                         const StunMessage* req,
                         int code,
                         absl::string_view reason);

  void SendErrorResponseWithRealmAndNonce(TurnServerConnection* conn,
                                          const StunMessage* req,
                                          int code,
                                          absl::string_view reason)
      RTC_RUN_ON(thread_);

  void SendErrorResponseWithAlternateServer(TurnServerConnection* conn,
                                            const StunMessage* req,
                                            const rtc::SocketAddress& addr)
      RTC_RUN_ON(thread_);

  void SendStun(TurnServerConnection* conn, StunMessage* msg);
  void Send(TurnServerConnection* conn, const rtc::ByteBufferWriter& buf);

  void DestroyAllocation(TurnServerAllocation* allocation) RTC_RUN_ON(thread_);
  void DestroyInternalSocket(rtc::AsyncPacketSocket* socket)
      RTC_RUN_ON(thread_);

  typedef std::map<rtc::AsyncPacketSocket*, ProtocolType> InternalSocketMap;
  struct ServerSocketInfo {
    ProtocolType proto;

    std::unique_ptr<rtc::SSLAdapterFactory> ssl_adapter_factory;
  };
  typedef std::map<rtc::Socket*, ServerSocketInfo> ServerSocketMap;

  webrtc::TaskQueueBase* const thread_;
  const std::string nonce_key_;
  std::string realm_ RTC_GUARDED_BY(thread_);
  std::string software_ RTC_GUARDED_BY(thread_);
  TurnAuthInterface* auth_hook_ RTC_GUARDED_BY(thread_);
  TurnRedirectInterface* redirect_hook_ RTC_GUARDED_BY(thread_);


  bool enable_otu_nonce_ RTC_GUARDED_BY(thread_);
  bool reject_private_addresses_ = false;

  bool enable_permission_checks_ = true;

  InternalSocketMap server_sockets_ RTC_GUARDED_BY(thread_);
  ServerSocketMap server_listen_sockets_ RTC_GUARDED_BY(thread_);
  std::unique_ptr<rtc::PacketSocketFactory> external_socket_factory_
      RTC_GUARDED_BY(thread_);
  rtc::SocketAddress external_addr_ RTC_GUARDED_BY(thread_);

  AllocationMap allocations_ RTC_GUARDED_BY(thread_);


  int64_t ts_for_next_nonce_ RTC_GUARDED_BY(thread_) = 0;

  std::unique_ptr<StunMessageObserver> stun_message_observer_
      RTC_GUARDED_BY(thread_);

  friend class TurnServerAllocation;
};

}  // namespace cricket

#endif  // P2P_BASE_TURN_SERVER_H_
