/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "p2p/base/stun_port.h"

#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "api/transport/stun.h"
#include "p2p/base/connection.h"
#include "p2p/base/p2p_constants.h"
#include "p2p/base/port_allocator.h"
#include "rtc_base/async_resolver_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/helpers.h"
#include "rtc_base/ip_address.h"
#include "rtc_base/logging.h"
#include "rtc_base/strings/string_builder.h"

namespace cricket {

namespace {

bool ResolveStunHostnameForFamily(const webrtc::FieldTrialsView& field_trials) {


  static constexpr char field_trial_name[] =
      "WebRTC-IPv6NetworkResolutionFixes";
  if (!field_trials.IsEnabled(field_trial_name)) {
    return false;
  }

  webrtc::FieldTrialParameter<bool> resolve_stun_hostname_for_family(
      "ResolveStunHostnameForFamily", /*default_value=*/false);
  webrtc::ParseFieldTrial({&resolve_stun_hostname_for_family},
                          field_trials.Lookup(field_trial_name));
  return resolve_stun_hostname_for_family;
}

}  // namespace

const int RETRY_TIMEOUT = 50 * 1000;  // 50 seconds

// `kSendErrorLogLimit` messages. Start again after a successful send.
const int kSendErrorLogLimit = 5;

class StunBindingRequest : public StunRequest {
 public:
  StunBindingRequest(UDPPort* port,
                     const rtc::SocketAddress& addr,
                     int64_t start_time)
      : StunRequest(port->request_manager(),
                    std::make_unique<StunMessage>(STUN_BINDING_REQUEST)),
        port_(port),
        server_addr_(addr),
        start_time_(start_time) {}

  const rtc::SocketAddress& server_addr() const { return server_addr_; }

  void OnResponse(StunMessage* response) override {
    const StunAddressAttribute* addr_attr =
        response->GetAddress(STUN_ATTR_MAPPED_ADDRESS);
    if (!addr_attr) {
      RTC_LOG(LS_ERROR) << "Binding response missing mapped address.";
    } else if (addr_attr->family() != STUN_ADDRESS_IPV4 &&
               addr_attr->family() != STUN_ADDRESS_IPV6) {
      RTC_LOG(LS_ERROR) << "Binding address has bad family";
    } else {
      rtc::SocketAddress addr(addr_attr->ipaddr(), addr_attr->port());
      port_->OnStunBindingRequestSucceeded(this->Elapsed(), server_addr_, addr);
    }

    if (WithinLifetime(rtc::TimeMillis())) {
      port_->request_manager_.SendDelayed(
          new StunBindingRequest(port_, server_addr_, start_time_),
          port_->stun_keepalive_delay());
    }
  }

  void OnErrorResponse(StunMessage* response) override {
    const StunErrorCodeAttribute* attr = response->GetErrorCode();
    if (!attr) {
      RTC_LOG(LS_ERROR) << "Missing binding response error code.";
    } else {
      RTC_LOG(LS_ERROR) << "Binding error response:"
                           " class="
                        << attr->eclass() << " number=" << attr->number()
                        << " reason=" << attr->reason();
    }

    port_->OnStunBindingOrResolveRequestFailed(
        server_addr_, attr ? attr->number() : STUN_ERROR_GLOBAL_FAILURE,
        attr ? attr->reason()
             : "STUN binding response with no error code attribute.");

    int64_t now = rtc::TimeMillis();
    if (WithinLifetime(now) &&
        rtc::TimeDiff(now, start_time_) < RETRY_TIMEOUT) {
      port_->request_manager_.SendDelayed(
          new StunBindingRequest(port_, server_addr_, start_time_),
          port_->stun_keepalive_delay());
    }
  }
  void OnTimeout() override {
    RTC_LOG(LS_ERROR) << "Binding request timed out from "
                      << port_->GetLocalAddress().ToSensitiveString() << " ("
                      << port_->Network()->name() << ")";
    port_->OnStunBindingOrResolveRequestFailed(
        server_addr_, SERVER_NOT_REACHABLE_ERROR,
        "STUN binding request timed out.");
  }

 private:


  bool WithinLifetime(int64_t now) const {
    int lifetime = port_->stun_keepalive_lifetime();
    return lifetime < 0 || rtc::TimeDiff(now, start_time_) <= lifetime;
  }

  UDPPort* port_;
  const rtc::SocketAddress server_addr_;

  int64_t start_time_;
};

UDPPort::AddressResolver::AddressResolver(
    rtc::PacketSocketFactory* factory,
    std::function<void(const rtc::SocketAddress&, int)> done_callback)
    : socket_factory_(factory), done_(std::move(done_callback)) {}

void UDPPort::AddressResolver::Resolve(
    const rtc::SocketAddress& address,
    int family,
    const webrtc::FieldTrialsView& field_trials) {
  if (resolvers_.find(address) != resolvers_.end())
    return;

  auto resolver = socket_factory_->CreateAsyncDnsResolver();
  auto resolver_ptr = resolver.get();
  std::pair<rtc::SocketAddress,
            std::unique_ptr<webrtc::AsyncDnsResolverInterface>>
      pair = std::make_pair(address, std::move(resolver));

  resolvers_.insert(std::move(pair));
  auto callback = [this, address] {
    ResolverMap::const_iterator it = resolvers_.find(address);
    if (it != resolvers_.end()) {
      done_(it->first, it->second->result().GetError());
    }
  };
  if (ResolveStunHostnameForFamily(field_trials)) {
    resolver_ptr->Start(address, family, std::move(callback));
  } else {
    resolver_ptr->Start(address, std::move(callback));
  }
}

bool UDPPort::AddressResolver::GetResolvedAddress(
    const rtc::SocketAddress& input,
    int family,
    rtc::SocketAddress* output) const {
  ResolverMap::const_iterator it = resolvers_.find(input);
  if (it == resolvers_.end())
    return false;

  return it->second->result().GetResolvedAddress(family, output);
}

UDPPort::UDPPort(rtc::Thread* thread,
                 rtc::PacketSocketFactory* factory,
                 const rtc::Network* network,
                 rtc::AsyncPacketSocket* socket,
                 absl::string_view username,
                 absl::string_view password,
                 bool emit_local_for_anyaddress,
                 const webrtc::FieldTrialsView* field_trials)
    : Port(thread,
           LOCAL_PORT_TYPE,
           factory,
           network,
           username,
           password,
           field_trials),
      request_manager_(
          thread,
          [this](const void* data, size_t size, StunRequest* request) {
            OnSendPacket(data, size, request);
          }),
      socket_(socket),
      error_(0),
      ready_(false),
      stun_keepalive_delay_(STUN_KEEPALIVE_INTERVAL),
      dscp_(rtc::DSCP_NO_CHANGE),
      emit_local_for_anyaddress_(emit_local_for_anyaddress) {}

UDPPort::UDPPort(rtc::Thread* thread,
                 rtc::PacketSocketFactory* factory,
                 const rtc::Network* network,
                 uint16_t min_port,
                 uint16_t max_port,
                 absl::string_view username,
                 absl::string_view password,
                 bool emit_local_for_anyaddress,
                 const webrtc::FieldTrialsView* field_trials)
    : Port(thread,
           LOCAL_PORT_TYPE,
           factory,
           network,
           min_port,
           max_port,
           username,
           password,
           field_trials),
      request_manager_(
          thread,
          [this](const void* data, size_t size, StunRequest* request) {
            OnSendPacket(data, size, request);
          }),
      socket_(nullptr),
      error_(0),
      ready_(false),
      stun_keepalive_delay_(STUN_KEEPALIVE_INTERVAL),
      dscp_(rtc::DSCP_NO_CHANGE),
      emit_local_for_anyaddress_(emit_local_for_anyaddress) {}

bool UDPPort::Init() {
  stun_keepalive_lifetime_ = GetStunKeepaliveLifetime();
  if (!SharedSocket()) {
    RTC_DCHECK(socket_ == nullptr);
    socket_ = socket_factory()->CreateUdpSocket(
        rtc::SocketAddress(Network()->GetBestIP(), 0), min_port(), max_port());
    if (!socket_) {
      RTC_LOG(LS_WARNING) << ToString() << ": UDP socket creation failed";
      return false;
    }
    socket_->SignalReadPacket.connect(this, &UDPPort::OnReadPacket);
  }
  socket_->SignalSentPacket.connect(this, &UDPPort::OnSentPacket);
  socket_->SignalReadyToSend.connect(this, &UDPPort::OnReadyToSend);
  socket_->SignalAddressReady.connect(this, &UDPPort::OnLocalAddressReady);
  return true;
}

UDPPort::~UDPPort() {
  if (!SharedSocket())
    delete socket_;
}

void UDPPort::PrepareAddress() {
  RTC_DCHECK(request_manager_.empty());
  if (socket_->GetState() == rtc::AsyncPacketSocket::STATE_BOUND) {
    OnLocalAddressReady(socket_, socket_->GetLocalAddress());
  }
}

void UDPPort::MaybePrepareStunCandidate() {


  if (!server_addresses_.empty()) {
    SendStunBindingRequests();
  } else {

    MaybeSetPortCompleteOrError();
  }
}

Connection* UDPPort::CreateConnection(const Candidate& address,
                                      CandidateOrigin origin) {
  if (!SupportsProtocol(address.protocol())) {
    return nullptr;
  }

  if (!IsCompatibleAddress(address.address())) {
    return nullptr;
  }




  if (Candidates().empty()) {
    RTC_DCHECK_NOTREACHED();
    return nullptr;
  }











  RTC_DCHECK(!SharedSocket() || Candidates()[0].type() == LOCAL_PORT_TYPE ||
             mdns_name_registration_status() !=
                 MdnsNameRegistrationStatus::kNotStarted);

  Connection* conn = new ProxyConnection(NewWeakPtr(), 0, address);
  AddOrReplaceConnection(conn);
  return conn;
}

int UDPPort::SendTo(const void* data,
                    size_t size,
                    const rtc::SocketAddress& addr,
                    const rtc::PacketOptions& options,
                    bool payload) {
  rtc::PacketOptions modified_options(options);
  CopyPortInformationToPacketInfo(&modified_options.info_signaled_after_sent);
  int sent = socket_->SendTo(data, size, addr, modified_options);
  if (sent < 0) {
    error_ = socket_->GetError();


    if (send_error_count_ < kSendErrorLogLimit) {
      ++send_error_count_;
      RTC_LOG(LS_ERROR) << ToString() << ": UDP send of " << size
                        << " bytes to host " << addr.ToSensitiveString() << " ("
                        << addr.ToResolvedSensitiveString()
                        << ") failed with error " << error_;
    }
  } else {
    send_error_count_ = 0;
  }
  return sent;
}

void UDPPort::UpdateNetworkCost() {
  Port::UpdateNetworkCost();
  stun_keepalive_lifetime_ = GetStunKeepaliveLifetime();
}

rtc::DiffServCodePoint UDPPort::StunDscpValue() const {
  return dscp_;
}

int UDPPort::SetOption(rtc::Socket::Option opt, int value) {
  if (opt == rtc::Socket::OPT_DSCP) {

    dscp_ = static_cast<rtc::DiffServCodePoint>(value);
  }
  return socket_->SetOption(opt, value);
}

int UDPPort::GetOption(rtc::Socket::Option opt, int* value) {
  return socket_->GetOption(opt, value);
}

int UDPPort::GetError() {
  return error_;
}

bool UDPPort::HandleIncomingPacket(rtc::AsyncPacketSocket* socket,
                                   const char* data,
                                   size_t size,
                                   const rtc::SocketAddress& remote_addr,
                                   int64_t packet_time_us) {

  OnReadPacket(socket, data, size, remote_addr, packet_time_us);
  return true;
}

bool UDPPort::SupportsProtocol(absl::string_view protocol) const {
  return protocol == UDP_PROTOCOL_NAME;
}

ProtocolType UDPPort::GetProtocol() const {
  return PROTO_UDP;
}

void UDPPort::GetStunStats(absl::optional<StunStats>* stats) {
  *stats = stats_;
}

void UDPPort::set_stun_keepalive_delay(const absl::optional<int>& delay) {
  stun_keepalive_delay_ = delay.value_or(STUN_KEEPALIVE_INTERVAL);
}

void UDPPort::OnLocalAddressReady(rtc::AsyncPacketSocket* socket,
                                  const rtc::SocketAddress& address) {




  rtc::SocketAddress addr = address;


  MaybeSetDefaultLocalAddress(&addr);

  AddAddress(addr, addr, rtc::SocketAddress(), UDP_PROTOCOL_NAME, "", "",
             LOCAL_PORT_TYPE, ICE_TYPE_PREFERENCE_HOST, 0, "", false);
  MaybePrepareStunCandidate();
}

void UDPPort::PostAddAddress(bool is_final) {
  MaybeSetPortCompleteOrError();
}

void UDPPort::OnReadPacket(rtc::AsyncPacketSocket* socket,
                           const char* data,
                           size_t size,
                           const rtc::SocketAddress& remote_addr,
                           const int64_t& packet_time_us) {
  RTC_DCHECK(socket == socket_);
  RTC_DCHECK(!remote_addr.IsUnresolvedIP());




  if (server_addresses_.find(remote_addr) != server_addresses_.end()) {
    request_manager_.CheckResponse(data, size);
    return;
  }

  if (Connection* conn = GetConnection(remote_addr)) {
    conn->OnReadPacket(data, size, packet_time_us);
  } else {
    Port::OnReadPacket(data, size, remote_addr, PROTO_UDP);
  }
}

void UDPPort::OnSentPacket(rtc::AsyncPacketSocket* socket,
                           const rtc::SentPacket& sent_packet) {
  PortInterface::SignalSentPacket(sent_packet);
}

void UDPPort::OnReadyToSend(rtc::AsyncPacketSocket* socket) {
  Port::OnReadyToSend();
}

void UDPPort::SendStunBindingRequests() {


  RTC_DCHECK(request_manager_.empty());

  for (ServerAddresses::const_iterator it = server_addresses_.begin();
       it != server_addresses_.end();) {




    ServerAddresses::const_iterator addr = it++;
    SendStunBindingRequest(*addr);
  }
}

void UDPPort::ResolveStunAddress(const rtc::SocketAddress& stun_addr) {
  if (!resolver_) {
    resolver_.reset(new AddressResolver(
        socket_factory(), [&](const rtc::SocketAddress& input, int error) {
          OnResolveResult(input, error);
        }));
  }

  RTC_LOG(LS_INFO) << ToString() << ": Starting STUN host lookup for "
                   << stun_addr.ToSensitiveString();
  resolver_->Resolve(stun_addr, Network()->family(), field_trials());
}

void UDPPort::OnResolveResult(const rtc::SocketAddress& input, int error) {
  RTC_DCHECK(resolver_.get() != nullptr);

  rtc::SocketAddress resolved;
  if (error != 0 || !resolver_->GetResolvedAddress(
                        input, Network()->GetBestIP().family(), &resolved)) {
    RTC_LOG(LS_WARNING) << ToString()
                        << ": StunPort: stun host lookup received error "
                        << error;
    OnStunBindingOrResolveRequestFailed(input, SERVER_NOT_REACHABLE_ERROR,
                                        "STUN host lookup received error.");
    return;
  }

  server_addresses_.erase(input);

  if (server_addresses_.find(resolved) == server_addresses_.end()) {
    server_addresses_.insert(resolved);
    SendStunBindingRequest(resolved);
  }
}

void UDPPort::SendStunBindingRequest(const rtc::SocketAddress& stun_addr) {
  if (stun_addr.IsUnresolvedIP()) {
    ResolveStunAddress(stun_addr);

  } else if (socket_->GetState() == rtc::AsyncPacketSocket::STATE_BOUND) {

    if (IsCompatibleAddress(stun_addr)) {
      request_manager_.Send(
          new StunBindingRequest(this, stun_addr, rtc::TimeMillis()));
    } else {


      const char* reason = "STUN server address is incompatible.";
      RTC_LOG(LS_WARNING) << reason;
      OnStunBindingOrResolveRequestFailed(stun_addr, SERVER_NOT_REACHABLE_ERROR,
                                          reason);
    }
  }
}

bool UDPPort::MaybeSetDefaultLocalAddress(rtc::SocketAddress* addr) const {
  if (!addr->IsAnyIP() || !emit_local_for_anyaddress_ ||
      !Network()->default_local_address_provider()) {
    return true;
  }
  rtc::IPAddress default_address;
  bool result =
      Network()->default_local_address_provider()->GetDefaultLocalAddress(
          addr->family(), &default_address);
  if (!result || default_address.IsNil()) {
    return false;
  }

  addr->SetIP(default_address);
  return true;
}

void UDPPort::OnStunBindingRequestSucceeded(
    int rtt_ms,
    const rtc::SocketAddress& stun_server_addr,
    const rtc::SocketAddress& stun_reflected_addr) {
  RTC_DCHECK(stats_.stun_binding_responses_received <
             stats_.stun_binding_requests_sent);
  stats_.stun_binding_responses_received++;
  stats_.stun_binding_rtt_ms_total += rtt_ms;
  stats_.stun_binding_rtt_ms_squared_total += rtt_ms * rtt_ms;
  if (bind_request_succeeded_servers_.find(stun_server_addr) !=
      bind_request_succeeded_servers_.end()) {
    return;
  }
  bind_request_succeeded_servers_.insert(stun_server_addr);




  if ((!SharedSocket() || stun_reflected_addr != socket_->GetLocalAddress()) &&
      !HasCandidateWithAddress(stun_reflected_addr)) {
    rtc::SocketAddress related_address = socket_->GetLocalAddress();

    if (!MaybeSetDefaultLocalAddress(&related_address)) {
      related_address =
          rtc::EmptySocketAddressWithFamily(related_address.family());
    }

    rtc::StringBuilder url;
    url << "stun:" << stun_server_addr.hostname() << ":"
        << stun_server_addr.port();
    AddAddress(stun_reflected_addr, socket_->GetLocalAddress(), related_address,
               UDP_PROTOCOL_NAME, "", "", STUN_PORT_TYPE,
               ICE_TYPE_PREFERENCE_SRFLX, 0, url.str(), false);
  }
  MaybeSetPortCompleteOrError();
}

void UDPPort::OnStunBindingOrResolveRequestFailed(
    const rtc::SocketAddress& stun_server_addr,
    int error_code,
    absl::string_view reason) {
  rtc::StringBuilder url;
  url << "stun:" << stun_server_addr.ToString();
  SignalCandidateError(
      this, IceCandidateErrorEvent(GetLocalAddress().HostAsSensitiveURIString(),
                                   GetLocalAddress().port(), url.str(),
                                   error_code, reason));
  if (bind_request_failed_servers_.find(stun_server_addr) !=
      bind_request_failed_servers_.end()) {
    return;
  }
  bind_request_failed_servers_.insert(stun_server_addr);
  MaybeSetPortCompleteOrError();
}

void UDPPort::MaybeSetPortCompleteOrError() {
  if (mdns_name_registration_status() ==
      MdnsNameRegistrationStatus::kInProgress) {
    return;
  }

  if (ready_) {
    return;
  }

  const size_t servers_done_bind_request =
      bind_request_failed_servers_.size() +
      bind_request_succeeded_servers_.size();
  if (server_addresses_.size() != servers_done_bind_request) {
    return;
  }

  ready_ = true;


  if (server_addresses_.empty() || bind_request_succeeded_servers_.size() > 0 ||
      SharedSocket()) {
    SignalPortComplete(this);
  } else {
    SignalPortError(this);
  }
}

void UDPPort::OnSendPacket(const void* data, size_t size, StunRequest* req) {
  StunBindingRequest* sreq = static_cast<StunBindingRequest*>(req);
  rtc::PacketOptions options(StunDscpValue());
  options.info_signaled_after_sent.packet_type = rtc::PacketType::kStunMessage;
  CopyPortInformationToPacketInfo(&options.info_signaled_after_sent);
  if (socket_->SendTo(data, size, sreq->server_addr(), options) < 0) {
    RTC_LOG_ERR_EX(LS_ERROR, socket_->GetError())
        << "UDP send of " << size << " bytes to host "
        << sreq->server_addr().ToSensitiveString() << " ("
        << sreq->server_addr().ToResolvedSensitiveString()
        << ") failed with error " << error_;
  }
  stats_.stun_binding_requests_sent++;
}

bool UDPPort::HasCandidateWithAddress(const rtc::SocketAddress& addr) const {
  const std::vector<Candidate>& existing_candidates = Candidates();
  std::vector<Candidate>::const_iterator it = existing_candidates.begin();
  for (; it != existing_candidates.end(); ++it) {
    if (it->address() == addr)
      return true;
  }
  return false;
}

std::unique_ptr<StunPort> StunPort::Create(
    rtc::Thread* thread,
    rtc::PacketSocketFactory* factory,
    const rtc::Network* network,
    uint16_t min_port,
    uint16_t max_port,
    absl::string_view username,
    absl::string_view password,
    const ServerAddresses& servers,
    absl::optional<int> stun_keepalive_interval,
    const webrtc::FieldTrialsView* field_trials) {

  auto port = absl::WrapUnique(new StunPort(thread, factory, network, min_port,
                                            max_port, username, password,
                                            servers, field_trials));
  port->set_stun_keepalive_delay(stun_keepalive_interval);
  if (!port->Init()) {
    return nullptr;
  }
  return port;
}

StunPort::StunPort(rtc::Thread* thread,
                   rtc::PacketSocketFactory* factory,
                   const rtc::Network* network,
                   uint16_t min_port,
                   uint16_t max_port,
                   absl::string_view username,
                   absl::string_view password,
                   const ServerAddresses& servers,
                   const webrtc::FieldTrialsView* field_trials)
    : UDPPort(thread,
              factory,
              network,
              min_port,
              max_port,
              username,
              password,
              false,
              field_trials) {

  set_type(STUN_PORT_TYPE);
  set_server_addresses(servers);
}

void StunPort::PrepareAddress() {
  SendStunBindingRequests();
}

}  // namespace cricket
