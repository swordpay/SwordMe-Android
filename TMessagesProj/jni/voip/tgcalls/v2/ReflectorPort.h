#ifndef TGCALLS_REFLECTOR_PORT_H_
#define TGCALLS_REFLECTOR_PORT_H_

#include <stdio.h>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "absl/memory/memory.h"
#include "api/async_dns_resolver.h"
#include "p2p/base/port.h"
#include "p2p/client/basic_port_allocator.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/ssl_certificate.h"

namespace webrtc {
class TurnCustomizer;
}

namespace tgcalls {

extern const int STUN_ATTR_TURN_LOGGING_ID;
extern const char TURN_PORT_TYPE[];
class TurnAllocateRequest;
class TurnEntry;

class ReflectorPort : public cricket::Port {
public:
    enum PortState {
        STATE_CONNECTING,    // Initial state, cannot send any packets.
        STATE_CONNECTED,     // Socket connected, ready to send stun requests.
        STATE_READY,         // Received allocate success, can send any packets.
        STATE_RECEIVEONLY,   // Had REFRESH_REQUEST error, cannot send any packets.
        STATE_DISCONNECTED,  // TCP connection died, cannot send/receive any

    };

    static std::unique_ptr<ReflectorPort> Create(
                                                 const cricket::CreateRelayPortArgs& args,
                                                 rtc::AsyncPacketSocket* socket,
                                                 uint8_t serverId) {

        if (args.config->credentials.username.size() > 32) {
            RTC_LOG(LS_ERROR) << "Attempt to use REFLECTOR with a too long username "
            << "of length " << args.config->credentials.username.size();
            return nullptr;
        }

        if (!AllowedReflectorPort(args.server_address->address.port())) {
            RTC_LOG(LS_ERROR) << "Attempt to use REFLECTOR to connect to port "
            << args.server_address->address.port();
            return nullptr;
        }

        return absl::WrapUnique(new ReflectorPort(args, socket, serverId));
    }


    static std::unique_ptr<ReflectorPort> Create(
                                                 const cricket::CreateRelayPortArgs& args,
                                                 uint16_t min_port,
                                                 uint16_t max_port,
                                                 uint8_t serverId) {

        if (args.config->credentials.username.size() > 32) {
            RTC_LOG(LS_ERROR) << "Attempt to use TURN with a too long username "
            << "of length " << args.config->credentials.username.size();
            return nullptr;
        }

        if (!AllowedReflectorPort(args.server_address->address.port())) {
            RTC_LOG(LS_ERROR) << "Attempt to use TURN to connect to port "
            << args.server_address->address.port();
            return nullptr;
        }

        return absl::WrapUnique(new ReflectorPort(args, min_port, max_port, serverId));
    }
    
    ~ReflectorPort() override;
    
    const cricket::ProtocolAddress& server_address() const { return server_address_; }

    rtc::SocketAddress GetLocalAddress() const;
    
    bool ready() const { return state_ == STATE_READY; }
    bool connected() const {
        return state_ == STATE_READY || state_ == STATE_CONNECTED;
    }
    const cricket::RelayCredentials& credentials() const { return credentials_; }
    
    cricket::ProtocolType GetProtocol() const override;

    void Release();
    
    void PrepareAddress() override;
    cricket::Connection* CreateConnection(const cricket::Candidate& c,
                                          PortInterface::CandidateOrigin origin) override;
    int SendTo(const void* data,
               size_t size,
               const rtc::SocketAddress& addr,
               const rtc::PacketOptions& options,
               bool payload) override;
    int SetOption(rtc::Socket::Option opt, int value) override;
    int GetOption(rtc::Socket::Option opt, int* value) override;
    int GetError() override;
    
    bool HandleIncomingPacket(rtc::AsyncPacketSocket* socket,
                              const char* data,
                              size_t size,
                              const rtc::SocketAddress& remote_addr,
                              int64_t packet_time_us) override;
    bool CanHandleIncomingPacketsFrom(
                                      const rtc::SocketAddress& addr) const override;
    virtual void OnReadPacket(rtc::AsyncPacketSocket* socket,
                              const char* data,
                              size_t size,
                              const rtc::SocketAddress& remote_addr,
                              const int64_t& packet_time_us);
    
    void OnSentPacket(rtc::AsyncPacketSocket* socket,
                      const rtc::SentPacket& sent_packet) override;
    virtual void OnReadyToSend(rtc::AsyncPacketSocket* socket);
    bool SupportsProtocol(absl::string_view protocol) const override;
    
    void OnSocketConnect(rtc::AsyncPacketSocket* socket);
    void OnSocketClose(rtc::AsyncPacketSocket* socket, int error);
    
    int error() const { return error_; }
    
    rtc::AsyncPacketSocket* socket() const { return socket_; }



    sigslot::
    signal3<ReflectorPort*, const rtc::SocketAddress&, const rtc::SocketAddress&>
    SignalResolvedServerAddress;



    sigslot::signal1<ReflectorPort*> SignalReflectorPortClosed;

    sigslot::signal2<ReflectorPort*, int> SignalTurnRefreshResult;
    sigslot::signal3<ReflectorPort*, const rtc::SocketAddress&, int>
    SignalCreatePermissionResult;


    void Close();
    
    void HandleConnectionDestroyed(cricket::Connection* conn) override;
    
protected:
    ReflectorPort(const cricket::CreateRelayPortArgs& args,
                  rtc::AsyncPacketSocket* socket,
                  uint8_t serverId);
    
    ReflectorPort(const cricket::CreateRelayPortArgs& args,
                  uint16_t min_port,
                  uint16_t max_port,
                  uint8_t serverId);
    
    rtc::DiffServCodePoint StunDscpValue() const override;
    
private:
    typedef std::map<rtc::Socket::Option, int> SocketOptionsMap;
    typedef std::set<rtc::SocketAddress> AttemptedServerSet;
    
    static bool AllowedReflectorPort(int port);
    
    bool CreateReflectorClientSocket();
    
    void ResolveTurnAddress(const rtc::SocketAddress& address);
    void OnResolveResult(rtc::AsyncResolverInterface* resolver);
    
    void OnSendStunPacket(const void* data, size_t size, cricket::StunRequest* request);
    
    void OnAllocateError(int error_code, const std::string& reason);
    
    void DispatchPacket(const char* data,
                        size_t size,
                        const rtc::SocketAddress& remote_addr,
                        cricket::ProtocolType proto,
                        int64_t packet_time_us);
    
    int Send(const void* data, size_t size, const rtc::PacketOptions& options);


    bool FailAndPruneConnection(const rtc::SocketAddress& address);

    std::string ReconstructedServerUrl(bool use_hostname);
    
    void SendReflectorHello();
    
    rtc::CopyOnWriteBuffer peer_tag_;
    uint32_t randomTag_ = 0;
    
    cricket::ProtocolAddress server_address_;
    uint8_t serverId_ = 0;
    
    std::map<std::string, uint32_t> resolved_peer_tags_by_hostname_;
    
    cricket::RelayCredentials credentials_;
    AttemptedServerSet attempted_server_addresses_;
    
    rtc::AsyncPacketSocket* socket_;
    SocketOptionsMap socket_options_;
    std::unique_ptr<webrtc::AsyncDnsResolverInterface> resolver_;
    int error_;
    rtc::DiffServCodePoint stun_dscp_value_;
    
    PortState state_;


    int server_priority_;


    webrtc::TurnCustomizer* turn_customizer_ = nullptr;
    
    webrtc::ScopedTaskSafety task_safety_;
    
    bool is_running_ping_task_ = false;
};

}  // namespace tgcalls

#endif  // TGCALLS_REFLECTOR_PORT_H_
