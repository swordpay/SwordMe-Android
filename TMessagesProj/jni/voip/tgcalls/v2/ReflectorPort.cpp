#include "v2/ReflectorPort.h"

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <random>
#include <sstream>

#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/types/optional.h"
#include "api/transport/stun.h"
#include "p2p/base/connection.h"
#include "p2p/base/p2p_constants.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/byte_order.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/net_helpers.h"
#include "rtc_base/socket_address.h"
#include "rtc_base/strings/string_builder.h"
#include "system_wrappers/include/field_trial.h"
#include "rtc_base/byte_order.h"

namespace tgcalls {

namespace {

rtc::CopyOnWriteBuffer parseHex(std::string const &string) {
    rtc::CopyOnWriteBuffer result;
    
    for (size_t i = 0; i < string.length(); i += 2) {
        std::string byteString = string.substr(i, 2);
        char byte = (char)strtol(byteString.c_str(), NULL, 16);
        result.AppendData(&byte, 1);
    }
    
    return result;
}

}

static int GetRelayPreference(cricket::ProtocolType proto) {
    switch (proto) {
        case cricket::PROTO_TCP:
            return cricket::ICE_TYPE_PREFERENCE_RELAY_TCP;
        case cricket::PROTO_TLS:
            return cricket::ICE_TYPE_PREFERENCE_RELAY_TLS;
        default:
            RTC_DCHECK(proto == cricket::PROTO_UDP);
            return cricket::ICE_TYPE_PREFERENCE_RELAY_UDP;
    }
}

ReflectorPort::ReflectorPort(const cricket::CreateRelayPortArgs& args,
                             rtc::AsyncPacketSocket* socket,
                             uint8_t serverId)
: Port(args.network_thread,
    cricket::RELAY_PORT_TYPE,
    args.socket_factory,
    args.network,
    args.username,
    args.password),
server_address_(*args.server_address),
credentials_(args.config->credentials),
socket_(socket),
error_(0),
stun_dscp_value_(rtc::DSCP_NO_CHANGE),
state_(STATE_CONNECTING),
server_priority_(args.config->priority) {
    serverId_ = serverId;
    
    auto rawPeerTag = parseHex(args.config->credentials.password);
    auto generator = std::mt19937(std::random_device()());
    auto distribution = std::uniform_int_distribution<uint32_t>();
    do {
        randomTag_ = distribution(generator);
    } while (!randomTag_);
    peer_tag_.AppendData(rawPeerTag.data(), rawPeerTag.size() - 4);
    peer_tag_.AppendData((uint8_t *)&randomTag_, 4);
}

ReflectorPort::ReflectorPort(const cricket::CreateRelayPortArgs& args,
                             uint16_t min_port,
                             uint16_t max_port,
                             uint8_t serverId)
: Port(args.network_thread,
       cricket::RELAY_PORT_TYPE,
       args.socket_factory,
       args.network,
       min_port,
       max_port,
       args.username,
       args.password),
server_address_(*args.server_address),
credentials_(args.config->credentials),
socket_(NULL),
error_(0),
stun_dscp_value_(rtc::DSCP_NO_CHANGE),
state_(STATE_CONNECTING),
server_priority_(args.config->priority) {
    serverId_ = serverId;
    
    auto rawPeerTag = parseHex(args.config->credentials.password);
    auto generator = std::mt19937(std::random_device()());
    auto distribution = std::uniform_int_distribution<uint32_t>();
    do {
        randomTag_ = distribution(generator);
    } while (!randomTag_);
    peer_tag_.AppendData(rawPeerTag.data(), rawPeerTag.size() - 4);
    peer_tag_.AppendData((uint8_t *)&randomTag_, 4);
}

ReflectorPort::~ReflectorPort() {



    if (ready()) {
        Release();
    }
    
    if (!SharedSocket()) {
        delete socket_;
    }
}

rtc::SocketAddress ReflectorPort::GetLocalAddress() const {
    return socket_ ? socket_->GetLocalAddress() : rtc::SocketAddress();
}

cricket::ProtocolType ReflectorPort::GetProtocol() const {
    return server_address_.proto;
}

void ReflectorPort::PrepareAddress() {
    if (peer_tag_.size() != 16) {
        RTC_LOG(LS_ERROR) << "Allocation can't be started without setting the"
        " peer tag.";
        OnAllocateError(cricket::STUN_ERROR_UNAUTHORIZED,
                        "Missing REFLECTOR server credentials.");
        return;
    }
    if (serverId_ == 0) {
        RTC_LOG(LS_ERROR) << "Allocation can't be started without setting the"
        " server id.";
        OnAllocateError(cricket::STUN_ERROR_UNAUTHORIZED,
                        "Missing REFLECTOR server id.");
        return;
    }
    
    if (!server_address_.address.port()) {

        server_address_.address.SetPort(599);
    }
    
    if (!AllowedReflectorPort(server_address_.address.port())) {


        RTC_LOG(LS_ERROR) << "Attempt to start allocation with disallowed port# "
        << server_address_.address.port();
        OnAllocateError(cricket::STUN_ERROR_SERVER_ERROR,
                        "Attempt to start allocation to a disallowed port");
        return;
    }
    if (server_address_.address.IsUnresolvedIP()) {
        ResolveTurnAddress(server_address_.address);
    } else {

        if (!IsCompatibleAddress(server_address_.address)) {
            RTC_LOG(LS_ERROR) << "IP address family does not match. server: "
            << server_address_.address.family()
            << " local: " << Network()->GetBestIP().family();
            OnAllocateError(cricket::STUN_ERROR_GLOBAL_FAILURE,
                            "IP address family does not match.");
            return;
        }

        attempted_server_addresses_.insert(server_address_.address);
        
        RTC_LOG(LS_INFO) << ToString() << ": Trying to connect to REFLECTOR server via "
        << ProtoToString(server_address_.proto) << " @ "
        << server_address_.address.ToSensitiveString();
        if (!CreateReflectorClientSocket()) {
            RTC_LOG(LS_ERROR) << "Failed to create REFLECTOR client socket";
            OnAllocateError(cricket::SERVER_NOT_REACHABLE_ERROR,
                            "Failed to create REFLECTOR client socket.");
            return;
        }
        if (server_address_.proto == cricket::PROTO_UDP) {
            SendReflectorHello();
        }
    }
}

void ReflectorPort::SendReflectorHello() {
    if (!(state_ == STATE_CONNECTED || state_ == STATE_READY)) {
        return;
    }
    
    RTC_LOG(LS_WARNING)
    << ToString()
    << ": REFLECTOR sending ping to " << server_address_.address.ToString();
    
    rtc::ByteBufferWriter bufferWriter;
    bufferWriter.WriteBytes((const char *)peer_tag_.data(), peer_tag_.size());
    for (int i = 0; i < 12; i++) {
        bufferWriter.WriteUInt8(0xffu);
    }
    bufferWriter.WriteUInt8(0xfeu);
    for (int i = 0; i < 3; i++) {
        bufferWriter.WriteUInt8(0xffu);
    }
    bufferWriter.WriteUInt64(123);
    
    while (bufferWriter.Length() % 4 != 0) {
        bufferWriter.WriteUInt8(0);
    }
    
    rtc::PacketOptions options;
    Send(bufferWriter.Data(), bufferWriter.Length(), options);
    
    if (!is_running_ping_task_) {
        is_running_ping_task_ = true;
        
        int timeoutMs = 10000;

        if (state_ == STATE_CONNECTED) {
            timeoutMs = 500;
        }
        
        thread()->PostDelayedTask(SafeTask(task_safety_.flag(), [this] {
            is_running_ping_task_ = false;
            SendReflectorHello();
        }), webrtc::TimeDelta::Millis(timeoutMs));
    }
}

bool ReflectorPort::CreateReflectorClientSocket() {
    RTC_DCHECK(!socket_ || SharedSocket());
    
    if (server_address_.proto == cricket::PROTO_UDP && !SharedSocket()) {
        socket_ = socket_factory()->CreateUdpSocket(
                                                    rtc::SocketAddress(Network()->GetBestIP(), 0), min_port(), max_port());
    } else if (server_address_.proto == cricket::PROTO_TCP) {
        RTC_DCHECK(!SharedSocket());
        int opts = rtc::PacketSocketFactory::OPT_STUN;
        
        rtc::PacketSocketTcpOptions tcp_options;
        tcp_options.opts = opts;
        socket_ = socket_factory()->CreateClientTcpSocket(
                                                          rtc::SocketAddress(Network()->GetBestIP(), 0), server_address_.address,
                                                          proxy(), user_agent(), tcp_options);
    }
    
    if (!socket_) {
        error_ = SOCKET_ERROR;
        return false;
    }

    for (SocketOptionsMap::iterator iter = socket_options_.begin();
         iter != socket_options_.end(); ++iter) {
        socket_->SetOption(iter->first, iter->second);
    }
    
    if (!SharedSocket()) {

        socket_->SignalReadPacket.connect(this, &ReflectorPort::OnReadPacket);
    }
    
    socket_->SignalReadyToSend.connect(this, &ReflectorPort::OnReadyToSend);
    
    socket_->SignalSentPacket.connect(this, &ReflectorPort::OnSentPacket);


    if (server_address_.proto == cricket::PROTO_TCP ||
        server_address_.proto == cricket::PROTO_TLS) {
        socket_->SignalConnect.connect(this, &ReflectorPort::OnSocketConnect);
        socket_->SubscribeClose(this, [this](rtc::AsyncPacketSocket* socket, int error) { OnSocketClose(socket, error); });
    } else {
        state_ = STATE_CONNECTED;
    }
    return true;
}

void ReflectorPort::OnSocketConnect(rtc::AsyncPacketSocket* socket) {


    RTC_DCHECK(server_address_.proto == cricket::PROTO_TCP ||
               server_address_.proto == cricket::PROTO_TLS);














    const rtc::SocketAddress& socket_address = socket->GetLocalAddress();
    if (absl::c_none_of(Network()->GetIPs(),
                        [socket_address](const rtc::InterfaceAddress& addr) {
        return socket_address.ipaddr() == addr;
    })) {
        if (socket->GetLocalAddress().IsLoopbackIP()) {
            RTC_LOG(LS_WARNING) << "Socket is bound to the address:"
            << socket_address.ipaddr().ToSensitiveString()
            << ", rather than an address associated with network:"
            << Network()->ToString()
            << ". Still allowing it since it's localhost.";
        } else if (IPIsAny(Network()->GetBestIP())) {
            RTC_LOG(LS_WARNING)
            << "Socket is bound to the address:"
            << socket_address.ipaddr().ToSensitiveString()
            << ", rather than an address associated with network:"
            << Network()->ToString()
            << ". Still allowing it since it's the 'any' address"
            ", possibly caused by multiple_routes being disabled.";
        } else {
            RTC_LOG(LS_WARNING) << "Socket is bound to the address:"
            << socket_address.ipaddr().ToSensitiveString()
            << ", rather than an address associated with network:"
            << Network()->ToString() << ". Discarding REFLECTOR port.";
            OnAllocateError(
                            cricket::STUN_ERROR_GLOBAL_FAILURE,
                            "Address not associated with the desired network interface.");
            return;
        }
    }
    
    state_ = STATE_CONNECTED;  // It is ready to send stun requests.
    if (server_address_.address.IsUnresolvedIP()) {
        server_address_.address = socket_->GetRemoteAddress();
    }
    
    RTC_LOG(LS_INFO) << "ReflectorPort connected to "
    << socket->GetRemoteAddress().ToSensitiveString()
    << " using tcp.";

}

void ReflectorPort::OnSocketClose(rtc::AsyncPacketSocket* socket, int error) {
    RTC_LOG(LS_WARNING) << ToString()
    << ": Connection with server failed with error: "
    << error;
    RTC_DCHECK(socket == socket_);
    Close();
}

cricket::Connection* ReflectorPort::CreateConnection(const cricket::Candidate& remote_candidate,
                                                     CandidateOrigin origin) {

    if (!SupportsProtocol(remote_candidate.protocol())) {
        return nullptr;
    }
    
    auto remoteHostname = remote_candidate.address().hostname();
    if (remoteHostname.empty()) {
        return nullptr;
    }
    std::ostringstream ipFormat;
    ipFormat << "reflector-" << (uint32_t)serverId_ << "-";
    if (!absl::StartsWith(remoteHostname, ipFormat.str()) || !absl::EndsWith(remoteHostname, ".reflector")) {
        return nullptr;
    }
    if (remote_candidate.address().port() != server_address_.address.port()) {
        return nullptr;
    }
    
    if (state_ == STATE_DISCONNECTED || state_ == STATE_RECEIVEONLY) {
        return nullptr;
    }
    
    cricket::ProxyConnection* conn = new cricket::ProxyConnection(NewWeakPtr(), 0, remote_candidate);
    AddOrReplaceConnection(conn);
    
    return conn;
}

bool ReflectorPort::FailAndPruneConnection(const rtc::SocketAddress& address) {
    cricket::Connection* conn = GetConnection(address);
    if (conn != nullptr) {
        conn->FailAndPrune();
        return true;
    }
    return false;
}

int ReflectorPort::SetOption(rtc::Socket::Option opt, int value) {

    if (opt == rtc::Socket::OPT_DSCP)
        stun_dscp_value_ = static_cast<rtc::DiffServCodePoint>(value);
    
    if (!socket_) {


        socket_options_[opt] = value;
        return 0;
    }
    return socket_->SetOption(opt, value);
}

int ReflectorPort::GetOption(rtc::Socket::Option opt, int* value) {
    if (!socket_) {
        SocketOptionsMap::const_iterator it = socket_options_.find(opt);
        if (it == socket_options_.end()) {
            return -1;
        }
        *value = it->second;
        return 0;
    }
    
    return socket_->GetOption(opt, value);
}

int ReflectorPort::GetError() {
    return error_;
}

int ReflectorPort::SendTo(const void* data,
                          size_t size,
                          const rtc::SocketAddress& addr,
                          const rtc::PacketOptions& options,
                          bool payload) {
    rtc::CopyOnWriteBuffer targetPeerTag;
    
    auto syntheticHostname = addr.hostname();
    
    uint32_t resolvedPeerTag = 0;
    auto resolvedPeerTagIt = resolved_peer_tags_by_hostname_.find(syntheticHostname);
    if (resolvedPeerTagIt != resolved_peer_tags_by_hostname_.end()) {
        resolvedPeerTag = resolvedPeerTagIt->second;
    } else {
        std::ostringstream prefixFormat;
        prefixFormat << "reflector-" << (uint32_t)serverId_ << "-";
        std::string suffixFormat = ".reflector";
        if (!absl::StartsWith(syntheticHostname, prefixFormat.str()) || !absl::EndsWith(syntheticHostname, suffixFormat)) {
            RTC_LOG(LS_ERROR) << ToString()
            << ": Discarding SendTo request with destination "
            << addr.ToString();
            
            return -1;
        }
        
        auto startPosition = prefixFormat.str().size();
        auto tagString = syntheticHostname.substr(startPosition, syntheticHostname.size() - suffixFormat.size() - startPosition);
        
        std::stringstream tagStringStream(tagString);
        tagStringStream >> resolvedPeerTag;
        
        if (resolvedPeerTag == 0) {
            RTC_LOG(LS_ERROR) << ToString()
            << ": Discarding SendTo request with destination "
            << addr.ToString() << " (could not parse peer tag)";
            
            return -1;
        }
        
        resolved_peer_tags_by_hostname_.insert(std::make_pair(syntheticHostname, resolvedPeerTag));
    }
    
    targetPeerTag.AppendData(peer_tag_.data(), peer_tag_.size() - 4);
    targetPeerTag.AppendData((uint8_t *)&resolvedPeerTag, 4);
    
    rtc::ByteBufferWriter bufferWriter;
    bufferWriter.WriteBytes((const char *)targetPeerTag.data(), targetPeerTag.size());
    
    bufferWriter.WriteBytes((const char *)&randomTag_, 4);
    
    bufferWriter.WriteUInt32((uint32_t)size);
    bufferWriter.WriteBytes((const char *)data, size);
    
    while (bufferWriter.Length() % 4 != 0) {
        bufferWriter.WriteUInt8(0);
    }
    
    rtc::PacketOptions modified_options(options);
    CopyPortInformationToPacketInfo(&modified_options.info_signaled_after_sent);
    
    modified_options.info_signaled_after_sent.turn_overhead_bytes = bufferWriter.Length() - size;
    
    Send(bufferWriter.Data(), bufferWriter.Length(), modified_options);
    
    return static_cast<int>(size);
}

bool ReflectorPort::CanHandleIncomingPacketsFrom(
                                                 const rtc::SocketAddress& addr) const {
                                                     return server_address_.address == addr;
                                                 }

bool ReflectorPort::HandleIncomingPacket(rtc::AsyncPacketSocket* socket,
                                         const char* data,
                                         size_t size,
                                         const rtc::SocketAddress& remote_addr,
                                         int64_t packet_time_us) {
    if (socket != socket_) {


        return false;
    }



    if (remote_addr != server_address_.address) {
        RTC_LOG(LS_WARNING) << ToString()
        << ": Discarding REFLECTOR message from unknown address: "
        << remote_addr.ToSensitiveString()
        << " server_address_: "
        << server_address_.address.ToSensitiveString();
        return false;
    }

    if (size < 16) {
        RTC_LOG(LS_WARNING) << ToString()
        << ": Received REFLECTOR message that was too short (" << size << ")";
        return false;
    }
    
    if (state_ == STATE_DISCONNECTED) {
        RTC_LOG(LS_WARNING)
        << ToString()
        << ": Received REFLECTOR message while the REFLECTOR port is disconnected";
        return false;
    }
    
    uint8_t receivedPeerTag[16];
    memcpy(receivedPeerTag, data, 16);
    
    if (memcmp(receivedPeerTag, peer_tag_.data(), 16 - 4) != 0) {
        RTC_LOG(LS_WARNING)
        << ToString()
        << ": Received REFLECTOR message with incorrect peer_tag";
        return false;
    }
    
    if (state_ != STATE_READY) {
        state_ = STATE_READY;
        
        RTC_LOG(LS_INFO)
        << ToString()
        << ": REFLECTOR " << server_address_.address.ToString() << " is now ready";
        
        std::ostringstream ipFormat;
        ipFormat << "reflector-" << (uint32_t)serverId_ << "-" << randomTag_ << ".reflector";
        rtc::SocketAddress candidateAddress(ipFormat.str(), server_address_.address.port());

        AddAddress(candidateAddress,          // Candidate address.
                   server_address_.address,          // Base address.
                   rtc::SocketAddress(),  // Related address.
                   cricket::UDP_PROTOCOL_NAME,
                   ProtoToString(server_address_.proto),  // The first hop protocol.
                   "",  // TCP canddiate type, empty for turn candidates.
                   cricket::RELAY_PORT_TYPE, GetRelayPreference(server_address_.proto),
                   server_priority_, ReconstructedServerUrl(false /* use_hostname */),
                   true);
    }
    
    if (size > 16 + 4 + 4) {
        bool isSpecialPacket = false;
        if (size >= 16 + 12) {
            uint8_t specialTag[12];
            memcpy(specialTag, data + 16, 12);
            
            uint8_t expectedSpecialTag[12];
            memset(expectedSpecialTag, 0xff, 12);
            
            if (memcmp(specialTag, expectedSpecialTag, 12) == 0) {
                isSpecialPacket = true;
            }
        }
        
        if (!isSpecialPacket) {
            uint32_t senderTag = 0;
            memcpy(&senderTag, data + 16, 4);
            
            uint32_t dataSize = 0;
            memcpy(&dataSize, data + 16 + 4, 4);
            dataSize = be32toh(dataSize);
            if (dataSize > size - 16 - 4 - 4) {
                RTC_LOG(LS_WARNING)
                << ToString()
                << ": Received data packet with invalid size tag";
            } else {
                std::ostringstream ipFormat;
                ipFormat << "reflector-" << (uint32_t)serverId_ << "-" << senderTag << ".reflector";
                rtc::SocketAddress candidateAddress(ipFormat.str(), server_address_.address.port());
                candidateAddress.SetResolvedIP(server_address_.address.ipaddr());
                
                DispatchPacket(data + 16 + 4 + 4, dataSize, candidateAddress, cricket::ProtocolType::PROTO_UDP, packet_time_us);
            }
        }
    }
    
    return true;
}

void ReflectorPort::OnReadPacket(rtc::AsyncPacketSocket* socket,
                                 const char* data,
                                 size_t size,
                                 const rtc::SocketAddress& remote_addr,
                                 const int64_t& packet_time_us) {
    HandleIncomingPacket(socket, data, size, remote_addr, packet_time_us);
}

void ReflectorPort::OnSentPacket(rtc::AsyncPacketSocket* socket,
                                 const rtc::SentPacket& sent_packet) {
    PortInterface::SignalSentPacket(sent_packet);
}

void ReflectorPort::OnReadyToSend(rtc::AsyncPacketSocket* socket) {
    if (ready()) {
        Port::OnReadyToSend();
    }
}

bool ReflectorPort::SupportsProtocol(absl::string_view protocol) const {

    return protocol == cricket::UDP_PROTOCOL_NAME;
}

void ReflectorPort::ResolveTurnAddress(const rtc::SocketAddress& address) {
    if (resolver_)
        return;
    
    RTC_LOG(LS_INFO) << ToString() << ": Starting TURN host lookup for "
    << address.ToSensitiveString();
    resolver_ = socket_factory()->CreateAsyncDnsResolver();
    resolver_->Start(address, [this] {





        auto& result = resolver_->result();
        if (result.GetError() != 0 && (server_address_.proto == cricket::PROTO_TCP ||
                                       server_address_.proto == cricket::PROTO_TLS)) {
            if (!CreateReflectorClientSocket()) {
                OnAllocateError(cricket::SERVER_NOT_REACHABLE_ERROR,
                                "TURN host lookup received error.");
            }
            return;
        }


        rtc::SocketAddress resolved_address = server_address_.address;
        if (result.GetError() != 0 ||
            !result.GetResolvedAddress(Network()->GetBestIP().family(),
                                       &resolved_address)) {
            RTC_LOG(LS_WARNING) << ToString() << ": TURN host lookup received error "
            << result.GetError();
            error_ = result.GetError();
            OnAllocateError(cricket::SERVER_NOT_REACHABLE_ERROR,
                            "TURN host lookup received error.");
            return;
        }


        SignalResolvedServerAddress(this, server_address_.address,
                                    resolved_address);
        server_address_.address = resolved_address;
        PrepareAddress();
    });
}

void ReflectorPort::OnSendStunPacket(const void* data,
                                     size_t size,
                                     cricket::StunRequest* request) {
    RTC_DCHECK(connected());
    rtc::PacketOptions options(StunDscpValue());
    options.info_signaled_after_sent.packet_type = rtc::PacketType::kTurnMessage;
    CopyPortInformationToPacketInfo(&options.info_signaled_after_sent);
    if (Send(data, size, options) < 0) {
        RTC_LOG(LS_ERROR) << ToString() << ": Failed to send TURN message, error: "
        << socket_->GetError();
    }
}

void ReflectorPort::OnAllocateError(int error_code, const std::string& reason) {



    thread()->PostTask(
      SafeTask(task_safety_.flag(), [this] { SignalPortError(this); }));
    std::string address = GetLocalAddress().HostAsSensitiveURIString();
    int port = GetLocalAddress().port();
    if (server_address_.proto == cricket::PROTO_TCP &&
        server_address_.address.IsPrivateIP()) {
        address.clear();
        port = 0;
    }
    SignalCandidateError(this, cricket::IceCandidateErrorEvent(address, port, ReconstructedServerUrl(true /* use_hostname */), error_code, reason));
}

void ReflectorPort::Release() {
    state_ = STATE_RECEIVEONLY;
}

void ReflectorPort::Close() {
    if (!ready()) {
        OnAllocateError(cricket::SERVER_NOT_REACHABLE_ERROR, "");
    }

    state_ = STATE_DISCONNECTED;

    for (auto kv : connections()) {
        kv.second->Destroy();
    }
    
    SignalReflectorPortClosed(this);
}

rtc::DiffServCodePoint ReflectorPort::StunDscpValue() const {
    return stun_dscp_value_;
}

bool ReflectorPort::AllowedReflectorPort(int port) {
    return true;
}

void ReflectorPort::DispatchPacket(const char* data,
                                   size_t size,
                                   const rtc::SocketAddress& remote_addr,
                                   cricket::ProtocolType proto,
                                   int64_t packet_time_us) {
    if (cricket::Connection* conn = GetConnection(remote_addr)) {
        conn->OnReadPacket(data, size, packet_time_us);
    } else {
        Port::OnReadPacket(data, size, remote_addr, proto);
    }
}

int ReflectorPort::Send(const void* data,
                        size_t len,
                        const rtc::PacketOptions& options) {
    return socket_->SendTo(data, len, server_address_.address, options);
}

void ReflectorPort::HandleConnectionDestroyed(cricket::Connection* conn) {
}

std::string ReflectorPort::ReconstructedServerUrl(bool use_hostname) {








    std::string scheme = "turn";
    std::string transport = "tcp";
    switch (server_address_.proto) {
        case cricket::PROTO_SSLTCP:
        case cricket::PROTO_TLS:
            scheme = "turns";
            break;
        case cricket::PROTO_UDP:
            transport = "udp";
            break;
        case cricket::PROTO_TCP:
            break;
    }
    rtc::StringBuilder url;
    url << scheme << ":"
    << (use_hostname ? server_address_.address.hostname()
        : server_address_.address.ipaddr().ToString())
    << ":" << server_address_.address.port() << "?transport=" << transport;
    return url.Release();
}

}  // namespace cricket
