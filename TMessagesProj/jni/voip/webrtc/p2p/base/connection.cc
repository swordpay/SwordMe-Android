/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "p2p/base/connection.h"

#include <math.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "p2p/base/port_allocator.h"
#include "rtc_base/checks.h"
#include "rtc_base/crc32.h"
#include "rtc_base/helpers.h"
#include "rtc_base/logging.h"
#include "rtc_base/mdns_responder_interface.h"
#include "rtc_base/message_digest.h"
#include "rtc_base/network.h"
#include "rtc_base/numerics/safe_minmax.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/string_utils.h"
#include "rtc_base/strings/string_builder.h"
#include "rtc_base/third_party/base64/base64.h"

namespace cricket {
namespace {

// pings fail to have a response.
inline bool TooManyFailures(
    const std::vector<Connection::SentPing>& pings_since_last_response,
    uint32_t maximum_failures,
    int rtt_estimate,
    int64_t now) {

  if (pings_since_last_response.size() < maximum_failures)
    return false;


  int64_t expected_response_time =
      pings_since_last_response[maximum_failures - 1].sent_time + rtt_estimate;
  return now > expected_response_time;
}

inline bool TooLongWithoutResponse(
    const std::vector<Connection::SentPing>& pings_since_last_response,
    int64_t maximum_time,
    int64_t now) {
  if (pings_since_last_response.size() == 0)
    return false;

  auto first = pings_since_last_response[0];
  return now > (first.sent_time + maximum_time);
}

// enum.
webrtc::IceCandidateType GetCandidateTypeByString(absl::string_view type) {
  if (type == LOCAL_PORT_TYPE) {
    return webrtc::IceCandidateType::kLocal;
  } else if (type == STUN_PORT_TYPE) {
    return webrtc::IceCandidateType::kStun;
  } else if (type == PRFLX_PORT_TYPE) {
    return webrtc::IceCandidateType::kPrflx;
  } else if (type == RELAY_PORT_TYPE) {
    return webrtc::IceCandidateType::kRelay;
  }
  return webrtc::IceCandidateType::kUnknown;
}

webrtc::IceCandidatePairProtocol GetProtocolByString(
    absl::string_view protocol) {
  if (protocol == UDP_PROTOCOL_NAME) {
    return webrtc::IceCandidatePairProtocol::kUdp;
  } else if (protocol == TCP_PROTOCOL_NAME) {
    return webrtc::IceCandidatePairProtocol::kTcp;
  } else if (protocol == SSLTCP_PROTOCOL_NAME) {
    return webrtc::IceCandidatePairProtocol::kSsltcp;
  } else if (protocol == TLS_PROTOCOL_NAME) {
    return webrtc::IceCandidatePairProtocol::kTls;
  }
  return webrtc::IceCandidatePairProtocol::kUnknown;
}

webrtc::IceCandidatePairAddressFamily GetAddressFamilyByInt(
    int address_family) {
  if (address_family == AF_INET) {
    return webrtc::IceCandidatePairAddressFamily::kIpv4;
  } else if (address_family == AF_INET6) {
    return webrtc::IceCandidatePairAddressFamily::kIpv6;
  }
  return webrtc::IceCandidatePairAddressFamily::kUnknown;
}

webrtc::IceCandidateNetworkType ConvertNetworkType(rtc::AdapterType type) {
  switch (type) {
    case rtc::ADAPTER_TYPE_ETHERNET:
      return webrtc::IceCandidateNetworkType::kEthernet;
    case rtc::ADAPTER_TYPE_LOOPBACK:
      return webrtc::IceCandidateNetworkType::kLoopback;
    case rtc::ADAPTER_TYPE_WIFI:
      return webrtc::IceCandidateNetworkType::kWifi;
    case rtc::ADAPTER_TYPE_VPN:
      return webrtc::IceCandidateNetworkType::kVpn;
    case rtc::ADAPTER_TYPE_CELLULAR:
    case rtc::ADAPTER_TYPE_CELLULAR_2G:
    case rtc::ADAPTER_TYPE_CELLULAR_3G:
    case rtc::ADAPTER_TYPE_CELLULAR_4G:
    case rtc::ADAPTER_TYPE_CELLULAR_5G:
      return webrtc::IceCandidateNetworkType::kCellular;
    default:
      return webrtc::IceCandidateNetworkType::kUnknown;
  }
}

// use a large value just in case the connection is really slow.
const int DEFAULT_RTT = 3000;  // 3 seconds

// within a reasonable range.
const int MINIMUM_RTT = 100;    // 0.1 seconds
const int MAXIMUM_RTT = 60000;  // 60 seconds

const int DEFAULT_RTT_ESTIMATE_HALF_TIME_MS = 500;

inline int ConservativeRTTEstimate(int rtt) {
  return rtc::SafeClamp(2 * rtt, MINIMUM_RTT, MAXIMUM_RTT);
}

const int RTT_RATIO = 3;  // 3 : 1

constexpr int64_t kMinExtraPingDelayMs = 100;

const IceFieldTrials kDefaultFieldTrials;

constexpr int kSupportGoogPingVersionRequestIndex = static_cast<int>(
    IceGoogMiscInfoBindingRequestAttributeIndex::SUPPORT_GOOG_PING_VERSION);

constexpr int kSupportGoogPingVersionResponseIndex = static_cast<int>(
    IceGoogMiscInfoBindingResponseAttributeIndex::SUPPORT_GOOG_PING_VERSION);

}  // namespace

class Connection::ConnectionRequest : public StunRequest {
 public:
  ConnectionRequest(StunRequestManager& manager,
                    Connection* connection,
                    std::unique_ptr<IceMessage> message);
  void OnResponse(StunMessage* response) override;
  void OnErrorResponse(StunMessage* response) override;
  void OnTimeout() override;
  void OnSent() override;
  int resend_delay() override;

 private:
  Connection* const connection_;
};

Connection::ConnectionRequest::ConnectionRequest(
    StunRequestManager& manager,
    Connection* connection,
    std::unique_ptr<IceMessage> message)
    : StunRequest(manager, std::move(message)), connection_(connection) {}

void Connection::ConnectionRequest::OnResponse(StunMessage* response) {
  RTC_DCHECK_RUN_ON(connection_->network_thread_);
  connection_->OnConnectionRequestResponse(this, response);
}

void Connection::ConnectionRequest::OnErrorResponse(StunMessage* response) {
  RTC_DCHECK_RUN_ON(connection_->network_thread_);
  connection_->OnConnectionRequestErrorResponse(this, response);
}

void Connection::ConnectionRequest::OnTimeout() {
  RTC_DCHECK_RUN_ON(connection_->network_thread_);
  connection_->OnConnectionRequestTimeout(this);
}

void Connection::ConnectionRequest::OnSent() {
  RTC_DCHECK_RUN_ON(connection_->network_thread_);
  connection_->OnConnectionRequestSent(this);


  set_timed_out();
}

int Connection::ConnectionRequest::resend_delay() {
  return CONNECTION_RESPONSE_TIMEOUT;
}

Connection::Connection(rtc::WeakPtr<Port> port,
                       size_t index,
                       const Candidate& remote_candidate)
    : network_thread_(port->thread()),
      id_(rtc::CreateRandomId()),
      port_(std::move(port)),
      local_candidate_(port_->Candidates()[index]),
      remote_candidate_(remote_candidate),
      recv_rate_tracker_(100, 10u),
      send_rate_tracker_(100, 10u),
      write_state_(STATE_WRITE_INIT),
      receiving_(false),
      connected_(true),
      pruned_(false),
      use_candidate_attr_(false),
      requests_(port_->thread(),
                [this](const void* data, size_t size, StunRequest* request) {
                  OnSendStunPacket(data, size, request);
                }),
      rtt_(DEFAULT_RTT),
      last_ping_sent_(0),
      last_ping_received_(0),
      last_data_received_(0),
      last_ping_response_received_(0),
      state_(IceCandidatePairState::WAITING),
      time_created_ms_(rtc::TimeMillis()),
      field_trials_(&kDefaultFieldTrials),
      rtt_estimate_(DEFAULT_RTT_ESTIMATE_HALF_TIME_MS) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK(port_);
  RTC_LOG(LS_INFO) << ToString() << ": Connection created";
}

Connection::~Connection() {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK(!port_);
}

webrtc::TaskQueueBase* Connection::network_thread() const {
  return network_thread_;
}

const Candidate& Connection::local_candidate() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return local_candidate_;
}

const Candidate& Connection::remote_candidate() const {
  return remote_candidate_;
}

const rtc::Network* Connection::network() const {
  return port()->Network();
}

int Connection::generation() const {
  return port()->generation();
}

uint64_t Connection::priority() const {
  if (!port_)
    return 0;

  uint64_t priority = 0;





  IceRole role = port_->GetIceRole();
  if (role != ICEROLE_UNKNOWN) {
    uint32_t g = 0;
    uint32_t d = 0;
    if (role == ICEROLE_CONTROLLING) {
      g = local_candidate().priority();
      d = remote_candidate_.priority();
    } else {
      g = remote_candidate_.priority();
      d = local_candidate().priority();
    }
    priority = std::min(g, d);
    priority = priority << 32;
    priority += 2 * std::max(g, d) + (g > d ? 1 : 0);
  }
  return priority;
}

void Connection::set_write_state(WriteState value) {
  RTC_DCHECK_RUN_ON(network_thread_);
  WriteState old_value = write_state_;
  write_state_ = value;
  if (value != old_value) {
    RTC_LOG(LS_VERBOSE) << ToString() << ": set_write_state from: " << old_value
                        << " to " << value;
    SignalStateChange(this);
  }
}

void Connection::UpdateReceiving(int64_t now) {
  RTC_DCHECK_RUN_ON(network_thread_);
  bool receiving;
  if (last_ping_sent() < last_ping_response_received()) {








    receiving = true;
  } else {
    receiving =
        last_received() > 0 && now <= last_received() + receiving_timeout();
  }
  if (receiving_ == receiving) {
    return;
  }
  RTC_LOG(LS_VERBOSE) << ToString() << ": set_receiving to " << receiving;
  receiving_ = receiving;
  receiving_unchanged_since_ = now;
  SignalStateChange(this);
}

void Connection::set_state(IceCandidatePairState state) {
  RTC_DCHECK_RUN_ON(network_thread_);
  IceCandidatePairState old_state = state_;
  state_ = state;
  if (state != old_state) {
    RTC_LOG(LS_VERBOSE) << ToString() << ": set_state";
  }
}

void Connection::set_connected(bool value) {
  RTC_DCHECK_RUN_ON(network_thread_);
  bool old_value = connected_;
  connected_ = value;
  if (value != old_value) {
    RTC_LOG(LS_VERBOSE) << ToString() << ": Change connected_ to " << value;
    SignalStateChange(this);
  }
}

bool Connection::use_candidate_attr() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return use_candidate_attr_;
}

void Connection::set_use_candidate_attr(bool enable) {
  RTC_DCHECK_RUN_ON(network_thread_);
  use_candidate_attr_ = enable;
}

void Connection::set_nomination(uint32_t value) {
  RTC_DCHECK_RUN_ON(network_thread_);
  nomination_ = value;
}

uint32_t Connection::remote_nomination() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return remote_nomination_;
}

bool Connection::nominated() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return acked_nomination_ || remote_nomination_;
}

int Connection::unwritable_timeout() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return unwritable_timeout_.value_or(CONNECTION_WRITE_CONNECT_TIMEOUT);
}

void Connection::set_unwritable_timeout(const absl::optional<int>& value_ms) {
  RTC_DCHECK_RUN_ON(network_thread_);
  unwritable_timeout_ = value_ms;
}

int Connection::unwritable_min_checks() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return unwritable_min_checks_.value_or(CONNECTION_WRITE_CONNECT_FAILURES);
}

void Connection::set_unwritable_min_checks(const absl::optional<int>& value) {
  RTC_DCHECK_RUN_ON(network_thread_);
  unwritable_min_checks_ = value;
}

int Connection::inactive_timeout() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return inactive_timeout_.value_or(CONNECTION_WRITE_TIMEOUT);
}

void Connection::set_inactive_timeout(const absl::optional<int>& value) {
  RTC_DCHECK_RUN_ON(network_thread_);
  inactive_timeout_ = value;
}

int Connection::receiving_timeout() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return receiving_timeout_.value_or(WEAK_CONNECTION_RECEIVE_TIMEOUT);
}

void Connection::set_receiving_timeout(
    absl::optional<int> receiving_timeout_ms) {
  RTC_DCHECK_RUN_ON(network_thread_);
  receiving_timeout_ = receiving_timeout_ms;
}

void Connection::SetIceFieldTrials(const IceFieldTrials* field_trials) {
  RTC_DCHECK_RUN_ON(network_thread_);
  field_trials_ = field_trials;
  rtt_estimate_.SetHalfTime(field_trials->rtt_estimate_halftime_ms);
}

void Connection::OnSendStunPacket(const void* data,
                                  size_t size,
                                  StunRequest* req) {
  RTC_DCHECK_RUN_ON(network_thread_);
  rtc::PacketOptions options(port_->StunDscpValue());
  options.info_signaled_after_sent.packet_type =
      rtc::PacketType::kIceConnectivityCheck;
  auto err =
      port_->SendTo(data, size, remote_candidate_.address(), options, false);
  if (err < 0) {
    RTC_LOG(LS_WARNING) << ToString()
                        << ": Failed to send STUN ping "
                           " err="
                        << err << " id=" << rtc::hex_encode(req->id());
  }
}

void Connection::OnReadPacket(const char* data,
                              size_t size,
                              int64_t packet_time_us) {
  RTC_DCHECK_RUN_ON(network_thread_);
  std::unique_ptr<IceMessage> msg;
  std::string remote_ufrag;
  const rtc::SocketAddress& addr(remote_candidate_.address());
  if (!port_->GetStunMessage(data, size, addr, &msg, &remote_ufrag)) {


    last_data_received_ = rtc::TimeMillis();
    UpdateReceiving(last_data_received_);
    recv_rate_tracker_.AddSamples(size);
    stats_.packets_received++;
    SignalReadPacket(this, data, size, packet_time_us);

    if (!pruned_ && (write_state_ == STATE_WRITE_TIMEOUT)) {
      RTC_LOG(LS_WARNING)
          << "Received a data packet on a timed-out Connection. "
             "Resetting state to STATE_WRITE_INIT.";
      set_write_state(STATE_WRITE_INIT);
    }
  } else if (!msg) {

  } else {





    rtc::LoggingSeverity sev = (!writable() ? rtc::LS_INFO : rtc::LS_VERBOSE);
    switch (msg->integrity()) {
      case StunMessage::IntegrityStatus::kNotSet:

        msg->ValidateMessageIntegrity(remote_candidate().password());
        break;
      case StunMessage::IntegrityStatus::kIntegrityOk:
        if (remote_candidate().password() != msg->password()) {


          msg->RevalidateMessageIntegrity(remote_candidate().password());
        }
        break;
      case StunMessage::IntegrityStatus::kIntegrityBad:


        msg->RevalidateMessageIntegrity(remote_candidate().password());
        break;
      default:

        RTC_DCHECK_NOTREACHED();
        break;
    }
    switch (msg->type()) {
      case STUN_BINDING_REQUEST:
        RTC_LOG_V(sev) << ToString() << ": Received "
                       << StunMethodToString(msg->type())
                       << ", id=" << rtc::hex_encode(msg->transaction_id());
        if (remote_ufrag == remote_candidate_.username()) {
          HandleStunBindingOrGoogPingRequest(msg.get());
        } else {


          RTC_LOG(LS_ERROR)
              << ToString()
              << ": Received STUN request with bad remote username "
              << remote_ufrag;
          port_->SendBindingErrorResponse(msg.get(), addr,
                                          STUN_ERROR_UNAUTHORIZED,
                                          STUN_ERROR_REASON_UNAUTHORIZED);
        }
        break;



      case STUN_BINDING_RESPONSE:
      case STUN_BINDING_ERROR_RESPONSE:
        if (msg->IntegrityOk()) {
          requests_.CheckResponse(msg.get());
        }

        break;



      case STUN_BINDING_INDICATION:
        ReceivedPing(msg->transaction_id());
        break;
      case GOOG_PING_REQUEST:
        HandleStunBindingOrGoogPingRequest(msg.get());
        break;
      case GOOG_PING_RESPONSE:
      case GOOG_PING_ERROR_RESPONSE:
        if (msg->IntegrityOk()) {
          requests_.CheckResponse(msg.get());
        }
        break;
      default:
        RTC_DCHECK_NOTREACHED();
        break;
    }
  }
}

void Connection::HandleStunBindingOrGoogPingRequest(IceMessage* msg) {
  RTC_DCHECK_RUN_ON(network_thread_);

  ReceivedPing(msg->transaction_id());
  if (field_trials_->extra_ice_ping && last_ping_response_received_ == 0) {
    if (local_candidate().type() == RELAY_PORT_TYPE ||
        local_candidate().type() == PRFLX_PORT_TYPE ||
        remote_candidate().type() == RELAY_PORT_TYPE ||
        remote_candidate().type() == PRFLX_PORT_TYPE) {
      const int64_t now = rtc::TimeMillis();
      if (last_ping_sent_ + kMinExtraPingDelayMs <= now) {
        RTC_LOG(LS_INFO) << ToString()
                         << "WebRTC-ExtraICEPing/Sending extra ping"
                            " last_ping_sent_: "
                         << last_ping_sent_ << " now: " << now
                         << " (diff: " << (now - last_ping_sent_) << ")";
        Ping(now);
      } else {
        RTC_LOG(LS_INFO) << ToString()
                         << "WebRTC-ExtraICEPing/Not sending extra ping"
                            " last_ping_sent_: "
                         << last_ping_sent_ << " now: " << now
                         << " (diff: " << (now - last_ping_sent_) << ")";
      }
    }
  }

  const rtc::SocketAddress& remote_addr = remote_candidate_.address();
  if (msg->type() == STUN_BINDING_REQUEST) {

    const std::string& remote_ufrag = remote_candidate_.username();
    if (!port_->MaybeIceRoleConflict(remote_addr, msg, remote_ufrag)) {

      RTC_LOG(LS_INFO) << "Received conflicting role from the peer.";
      return;
    }
  }

  stats_.recv_ping_requests++;
  LogCandidatePairEvent(webrtc::IceCandidatePairEventType::kCheckReceived,
                        msg->reduced_transaction_id());

  if (msg->type() == STUN_BINDING_REQUEST) {
    SendStunBindingResponse(msg);
  } else {
    RTC_DCHECK(msg->type() == GOOG_PING_REQUEST);
    SendGoogPingResponse(msg);
  }

  if (!pruned_ && write_state_ == STATE_WRITE_TIMEOUT) {
    set_write_state(STATE_WRITE_INIT);
  }

  if (port_->GetIceRole() == ICEROLE_CONTROLLED) {
    const StunUInt32Attribute* nomination_attr =
        msg->GetUInt32(STUN_ATTR_NOMINATION);
    uint32_t nomination = 0;
    if (nomination_attr) {
      nomination = nomination_attr->value();
      if (nomination == 0) {
        RTC_LOG(LS_ERROR) << "Invalid nomination: " << nomination;
      }
    } else {
      const StunByteStringAttribute* use_candidate_attr =
          msg->GetByteString(STUN_ATTR_USE_CANDIDATE);
      if (use_candidate_attr) {
        nomination = 1;
      }
    }

    if (nomination > remote_nomination_) {
      set_remote_nomination(nomination);
      SignalNominated(this);
    }
  }



  const StunUInt32Attribute* network_attr =
      msg->GetUInt32(STUN_ATTR_GOOG_NETWORK_INFO);
  if (network_attr) {
    uint32_t network_info = network_attr->value();
    uint16_t network_cost = static_cast<uint16_t>(network_info);
    if (network_cost != remote_candidate_.network_cost()) {
      remote_candidate_.set_network_cost(network_cost);


      SignalStateChange(this);
    }
  }

  if (field_trials_->piggyback_ice_check_acknowledgement) {
    HandlePiggybackCheckAcknowledgementIfAny(msg);
  }
}

void Connection::SendStunBindingResponse(const StunMessage* message) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK_EQ(message->type(), STUN_BINDING_REQUEST);

  const StunByteStringAttribute* username_attr =
      message->GetByteString(STUN_ATTR_USERNAME);
  RTC_DCHECK(username_attr != NULL);
  if (username_attr == NULL) {

    return;
  }

  StunMessage response(STUN_BINDING_RESPONSE, message->transaction_id());
  const StunUInt32Attribute* retransmit_attr =
      message->GetUInt32(STUN_ATTR_RETRANSMIT_COUNT);
  if (retransmit_attr) {


    response.AddAttribute(std::make_unique<StunUInt32Attribute>(
        STUN_ATTR_RETRANSMIT_COUNT, retransmit_attr->value()));

    if (retransmit_attr->value() > CONNECTION_WRITE_CONNECT_FAILURES) {
      RTC_LOG(LS_INFO)
          << ToString()
          << ": Received a remote ping with high retransmit count: "
          << retransmit_attr->value();
    }
  }

  response.AddAttribute(std::make_unique<StunXorAddressAttribute>(
      STUN_ATTR_XOR_MAPPED_ADDRESS, remote_candidate_.address()));

  if (field_trials_->announce_goog_ping) {

    auto goog_misc = message->GetUInt16List(STUN_ATTR_GOOG_MISC_INFO);
    if (goog_misc != nullptr &&
        goog_misc->Size() >= kSupportGoogPingVersionRequestIndex &&

        goog_misc->GetType(kSupportGoogPingVersionRequestIndex) >= 1) {
      auto list =
          StunAttribute::CreateUInt16ListAttribute(STUN_ATTR_GOOG_MISC_INFO);
      list->AddTypeAtIndex(kSupportGoogPingVersionResponseIndex,
                           kGoogPingVersion);
      response.AddAttribute(std::move(list));
    }
  }

  response.AddMessageIntegrity(local_candidate().password());
  response.AddFingerprint();

  SendResponseMessage(response);
}

void Connection::SendGoogPingResponse(const StunMessage* message) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK(message->type() == GOOG_PING_REQUEST);

  StunMessage response(GOOG_PING_RESPONSE, message->transaction_id());
  response.AddMessageIntegrity32(local_candidate().password());
  SendResponseMessage(response);
}

void Connection::SendResponseMessage(const StunMessage& response) {
  RTC_DCHECK_RUN_ON(network_thread_);

  const rtc::SocketAddress& addr = remote_candidate_.address();

  rtc::ByteBufferWriter buf;
  response.Write(&buf);
  rtc::PacketOptions options(port_->StunDscpValue());
  options.info_signaled_after_sent.packet_type =
      rtc::PacketType::kIceConnectivityCheckResponse;
  auto err = port_->SendTo(buf.Data(), buf.Length(), addr, options, false);
  if (err < 0) {
    RTC_LOG(LS_ERROR) << ToString() << ": Failed to send "
                      << StunMethodToString(response.type())
                      << ", to=" << addr.ToSensitiveString() << ", err=" << err
                      << ", id=" << rtc::hex_encode(response.transaction_id());
  } else {


    rtc::LoggingSeverity sev = (!writable()) ? rtc::LS_INFO : rtc::LS_VERBOSE;
    RTC_LOG_V(sev) << ToString() << ": Sent "
                   << StunMethodToString(response.type())
                   << ", to=" << addr.ToSensitiveString()
                   << ", id=" << rtc::hex_encode(response.transaction_id());

    stats_.sent_ping_responses++;
    LogCandidatePairEvent(webrtc::IceCandidatePairEventType::kCheckResponseSent,
                          response.reduced_transaction_id());
  }
}

uint32_t Connection::acked_nomination() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return acked_nomination_;
}

void Connection::set_remote_nomination(uint32_t remote_nomination) {
  RTC_DCHECK_RUN_ON(network_thread_);
  remote_nomination_ = remote_nomination;
}

void Connection::OnReadyToSend() {
  RTC_DCHECK_RUN_ON(network_thread_);
  SignalReadyToSend(this);
}

bool Connection::pruned() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return pruned_;
}

void Connection::Prune() {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (!pruned_ || active()) {
    RTC_LOG(LS_INFO) << ToString() << ": Connection pruned";
    pruned_ = true;
    requests_.Clear();
    set_write_state(STATE_WRITE_TIMEOUT);
  }
}

void Connection::Destroy() {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK(port_) << "Calling Destroy() twice?";
  if (port_)
    port_->DestroyConnection(this);
}

bool Connection::Shutdown() {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (!port_)
    return false;  // already shut down.

  RTC_DLOG(LS_VERBOSE) << ToString() << ": Connection destroyed";




  auto destroyed_signals = SignalDestroyed;
  SignalDestroyed.disconnect_all();
  destroyed_signals(this);

  LogCandidatePairConfig(webrtc::IceCandidatePairConfigType::kDestroyed);


  port_.reset();

  return true;
}

void Connection::FailAndPrune() {
  RTC_DCHECK_RUN_ON(network_thread_);








  if (!port_)
    return;

  set_state(IceCandidatePairState::FAILED);
  Prune();
}

void Connection::PrintPingsSinceLastResponse(std::string* s, size_t max) {
  RTC_DCHECK_RUN_ON(network_thread_);
  rtc::StringBuilder oss;
  if (pings_since_last_response_.size() > max) {
    for (size_t i = 0; i < max; i++) {
      const SentPing& ping = pings_since_last_response_[i];
      oss << rtc::hex_encode(ping.id) << " ";
    }
    oss << "... " << (pings_since_last_response_.size() - max) << " more";
  } else {
    for (const SentPing& ping : pings_since_last_response_) {
      oss << rtc::hex_encode(ping.id) << " ";
    }
  }
  *s = oss.str();
}

bool Connection::selected() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return selected_;
}

void Connection::set_selected(bool selected) {
  RTC_DCHECK_RUN_ON(network_thread_);
  selected_ = selected;
}

void Connection::UpdateState(int64_t now) {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (!port_)
    return;

  int rtt = ConservativeRTTEstimate(rtt_);

  if (RTC_LOG_CHECK_LEVEL(LS_VERBOSE)) {
    std::string pings;
    PrintPingsSinceLastResponse(&pings, 5);
    RTC_LOG(LS_VERBOSE) << ToString()
                        << ": UpdateState()"
                           ", ms since last received response="
                        << now - last_ping_response_received_
                        << ", ms since last received data="
                        << now - last_data_received_ << ", rtt=" << rtt
                        << ", pings_since_last_response=" << pings;
  }









  if ((write_state_ == STATE_WRITABLE) &&
      TooManyFailures(pings_since_last_response_, unwritable_min_checks(), rtt,
                      now) &&
      TooLongWithoutResponse(pings_since_last_response_, unwritable_timeout(),
                             now)) {
    uint32_t max_pings = unwritable_min_checks();
    RTC_LOG(LS_INFO) << ToString() << ": Unwritable after " << max_pings
                     << " ping failures and "
                     << now - pings_since_last_response_[0].sent_time
                     << " ms without a response,"
                        " ms since last received ping="
                     << now - last_ping_received_
                     << " ms since last received data="
                     << now - last_data_received_ << " rtt=" << rtt;
    set_write_state(STATE_WRITE_UNRELIABLE);
  }
  if ((write_state_ == STATE_WRITE_UNRELIABLE ||
       write_state_ == STATE_WRITE_INIT) &&
      TooLongWithoutResponse(pings_since_last_response_, inactive_timeout(),
                             now)) {
    RTC_LOG(LS_INFO) << ToString() << ": Timed out after "
                     << now - pings_since_last_response_[0].sent_time
                     << " ms without a response, rtt=" << rtt;
    set_write_state(STATE_WRITE_TIMEOUT);
  }

  UpdateReceiving(now);
  if (dead(now)) {
    port_->DestroyConnectionAsync(this);
  }
}

void Connection::UpdateLocalIceParameters(int component,
                                          absl::string_view username_fragment,
                                          absl::string_view password) {
  RTC_DCHECK_RUN_ON(network_thread_);
  local_candidate_.set_component(component);
  local_candidate_.set_username(username_fragment);
  local_candidate_.set_password(password);
}

int64_t Connection::last_ping_sent() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return last_ping_sent_;
}

void Connection::Ping(int64_t now) {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (!port_)
    return;

  last_ping_sent_ = now;



  int nomination = use_candidate_attr_ ? 1 : 0;
  if (nomination_ > 0) {
    nomination = nomination_;
  }

  auto req =
      std::make_unique<ConnectionRequest>(requests_, this, BuildPingRequest());

  if (ShouldSendGoogPing(req->msg())) {
    auto message = std::make_unique<IceMessage>(GOOG_PING_REQUEST, req->id());
    message->AddMessageIntegrity32(remote_candidate_.password());
    req.reset(new ConnectionRequest(requests_, this, std::move(message)));
  }

  pings_since_last_response_.push_back(SentPing(req->id(), now, nomination));
  RTC_LOG(LS_VERBOSE) << ToString() << ": Sending STUN ping, id="
                      << rtc::hex_encode(req->id())
                      << ", nomination=" << nomination_;
  requests_.Send(req.release());
  state_ = IceCandidatePairState::IN_PROGRESS;
  num_pings_sent_++;
}

std::unique_ptr<IceMessage> Connection::BuildPingRequest() {
  auto message = std::make_unique<IceMessage>(STUN_BINDING_REQUEST);



  message->AddAttribute(std::make_unique<StunByteStringAttribute>(
      STUN_ATTR_USERNAME,
      port()->CreateStunUsername(remote_candidate_.username())));
  message->AddAttribute(std::make_unique<StunUInt32Attribute>(
      STUN_ATTR_GOOG_NETWORK_INFO,
      (port_->Network()->id() << 16) | port_->network_cost()));

  if (field_trials_->piggyback_ice_check_acknowledgement &&
      last_ping_id_received_) {
    message->AddAttribute(std::make_unique<StunByteStringAttribute>(
        STUN_ATTR_GOOG_LAST_ICE_CHECK_RECEIVED, *last_ping_id_received_));
  }

  IceRole ice_role = port_->GetIceRole();
  RTC_DCHECK(ice_role == ICEROLE_CONTROLLING || ice_role == ICEROLE_CONTROLLED);
  message->AddAttribute(std::make_unique<StunUInt64Attribute>(
      ice_role == ICEROLE_CONTROLLING ? STUN_ATTR_ICE_CONTROLLING
                                      : STUN_ATTR_ICE_CONTROLLED,
      port_->IceTiebreaker()));

  if (ice_role == ICEROLE_CONTROLLING) {


    if (use_candidate_attr()) {
      message->AddAttribute(
          std::make_unique<StunByteStringAttribute>(STUN_ATTR_USE_CANDIDATE));
    }
    if (nomination_ && nomination_ != acked_nomination()) {
      message->AddAttribute(std::make_unique<StunUInt32Attribute>(
          STUN_ATTR_NOMINATION, nomination_));
    }
  }

  message->AddAttribute(std::make_unique<StunUInt32Attribute>(
      STUN_ATTR_PRIORITY, prflx_priority()));

  if (port()->send_retransmit_count_attribute()) {
    message->AddAttribute(std::make_unique<StunUInt32Attribute>(
        STUN_ATTR_RETRANSMIT_COUNT, pings_since_last_response_.size()));
  }
  if (field_trials_->enable_goog_ping &&
      !remote_support_goog_ping_.has_value()) {



    auto list =
        StunAttribute::CreateUInt16ListAttribute(STUN_ATTR_GOOG_MISC_INFO);
    list->AddTypeAtIndex(kSupportGoogPingVersionRequestIndex, kGoogPingVersion);
    message->AddAttribute(std::move(list));
  }
  message->AddMessageIntegrity(remote_candidate_.password());
  message->AddFingerprint();

  return message;
}

int64_t Connection::last_ping_response_received() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return last_ping_response_received_;
}

const absl::optional<std::string>& Connection::last_ping_id_received() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return last_ping_id_received_;
}

int Connection::rtt_samples() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return rtt_samples_;
}

// public because the connection intercepts the first ping for us.
int64_t Connection::last_ping_received() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return last_ping_received_;
}

void Connection::ReceivedPing(const absl::optional<std::string>& request_id) {
  RTC_DCHECK_RUN_ON(network_thread_);
  last_ping_received_ = rtc::TimeMillis();
  last_ping_id_received_ = request_id;
  UpdateReceiving(last_ping_received_);
}

void Connection::HandlePiggybackCheckAcknowledgementIfAny(StunMessage* msg) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK(msg->type() == STUN_BINDING_REQUEST ||
             msg->type() == GOOG_PING_REQUEST);
  const StunByteStringAttribute* last_ice_check_received_attr =
      msg->GetByteString(STUN_ATTR_GOOG_LAST_ICE_CHECK_RECEIVED);
  if (last_ice_check_received_attr) {
    const absl::string_view request_id =
        last_ice_check_received_attr->string_view();
    auto iter = absl::c_find_if(
        pings_since_last_response_,
        [&request_id](const SentPing& ping) { return ping.id == request_id; });
    if (iter != pings_since_last_response_.end()) {
      rtc::LoggingSeverity sev = !writable() ? rtc::LS_INFO : rtc::LS_VERBOSE;
      RTC_LOG_V(sev) << ToString()
                     << ": Received piggyback STUN ping response, id="
                     << rtc::hex_encode(request_id);
      const int64_t rtt = rtc::TimeMillis() - iter->sent_time;
      ReceivedPingResponse(rtt, request_id, iter->nomination);
    }
  }
}

int64_t Connection::last_send_data() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return last_send_data_;
}

int64_t Connection::last_data_received() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return last_data_received_;
}

void Connection::ReceivedPingResponse(
    int rtt,
    absl::string_view request_id,
    const absl::optional<uint32_t>& nomination) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_DCHECK_GE(rtt, 0);





  if (nomination && nomination.value() > acked_nomination_) {
    acked_nomination_ = nomination.value();
  }

  int64_t now = rtc::TimeMillis();
  total_round_trip_time_ms_ += rtt;
  current_round_trip_time_ms_ = static_cast<uint32_t>(rtt);
  rtt_estimate_.AddSample(now, rtt);

  pings_since_last_response_.clear();
  last_ping_response_received_ = now;
  UpdateReceiving(last_ping_response_received_);
  set_write_state(STATE_WRITABLE);
  set_state(IceCandidatePairState::SUCCEEDED);
  if (rtt_samples_ > 0) {
    rtt_ = rtc::GetNextMovingAverage(rtt_, rtt, RTT_RATIO);
  } else {
    rtt_ = rtt;
  }
  rtt_samples_++;
}

Connection::WriteState Connection::write_state() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return write_state_;
}

bool Connection::writable() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return write_state_ == STATE_WRITABLE;
}

bool Connection::receiving() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return receiving_;
}

// be false for TCP connections.
bool Connection::connected() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return connected_;
}

bool Connection::weak() const {
  return !(writable() && receiving() && connected());
}

bool Connection::active() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return write_state_ != STATE_WRITE_TIMEOUT;
}

bool Connection::dead(int64_t now) const {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (last_received() > 0) {











    if (now <= (last_received() + DEAD_CONNECTION_RECEIVE_TIMEOUT)) {

      return false;
    }
    if (!pings_since_last_response_.empty()) {


      return now > (pings_since_last_response_[0].sent_time +
                    DEAD_CONNECTION_RECEIVE_TIMEOUT);
    }


    return now > (last_received() + field_trials_->dead_connection_timeout_ms);
  }

  if (active()) {




    return false;
  }




  return now > (time_created_ms_ + MIN_CONNECTION_LIFETIME);
}

int Connection::rtt() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return rtt_;
}

bool Connection::stable(int64_t now) const {




  return rtt_converged() && !missing_responses(now);
}

std::string Connection::ToDebugId() const {
  return rtc::ToHex(reinterpret_cast<uintptr_t>(this));
}

uint32_t Connection::ComputeNetworkCost() const {

  return port()->network_cost() + remote_candidate_.network_cost();
}

std::string Connection::ToString() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  constexpr absl::string_view CONNECT_STATE_ABBREV[2] = {
      "-",  // not connected (false)
      "C",  // connected (true)
  };
  constexpr absl::string_view RECEIVE_STATE_ABBREV[2] = {
      "-",  // not receiving (false)
      "R",  // receiving (true)
  };
  constexpr absl::string_view WRITE_STATE_ABBREV[4] = {
      "W",  // STATE_WRITABLE
      "w",  // STATE_WRITE_UNRELIABLE
      "-",  // STATE_WRITE_INIT
      "x",  // STATE_WRITE_TIMEOUT
  };
  constexpr absl::string_view ICESTATE[4] = {
      "W",  // STATE_WAITING
      "I",  // STATE_INPROGRESS
      "S",  // STATE_SUCCEEDED
      "F"   // STATE_FAILED
  };
  constexpr absl::string_view SELECTED_STATE_ABBREV[2] = {
      "-",  // candidate pair not selected (false)
      "S",  // selected (true)
  };
  rtc::StringBuilder ss;
  ss << "Conn[" << ToDebugId();

  if (!port_) {


    ss << ":#:#:";
  } else {
    ss << ":" << port_->content_name() << ":" << port_->Network()->ToString()
       << ":";
  }

  const Candidate& local = local_candidate();
  const Candidate& remote = remote_candidate();
  ss << local.id() << ":" << local.component() << ":" << local.generation()
     << ":" << local.type() << ":" << local.protocol() << ":"
     << local.address().ToSensitiveString() << "->" << remote.id() << ":"
     << remote.component() << ":" << remote.priority() << ":" << remote.type()
     << ":" << remote.protocol() << ":" << remote.address().ToSensitiveString()
     << "|";

  ss << CONNECT_STATE_ABBREV[connected_] << RECEIVE_STATE_ABBREV[receiving_]
     << WRITE_STATE_ABBREV[write_state_] << ICESTATE[static_cast<int>(state_)]
     << "|" << SELECTED_STATE_ABBREV[selected_] << "|" << remote_nomination_
     << "|" << nomination_ << "|";

  if (port_)
    ss << priority() << "|";

  if (rtt_ < DEFAULT_RTT) {
    ss << rtt_ << "]";
  } else {
    ss << "-]";
  }

  return ss.Release();
}

std::string Connection::ToSensitiveString() const {
  return ToString();
}

const webrtc::IceCandidatePairDescription& Connection::ToLogDescription() {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (log_description_.has_value()) {
    return log_description_.value();
  }
  const Candidate& local = local_candidate();
  const Candidate& remote = remote_candidate();
  const rtc::Network* network = port()->Network();
  log_description_ = webrtc::IceCandidatePairDescription();
  log_description_->local_candidate_type =
      GetCandidateTypeByString(local.type());
  log_description_->local_relay_protocol =
      GetProtocolByString(local.relay_protocol());
  log_description_->local_network_type = ConvertNetworkType(network->type());
  log_description_->local_address_family =
      GetAddressFamilyByInt(local.address().family());
  log_description_->remote_candidate_type =
      GetCandidateTypeByString(remote.type());
  log_description_->remote_address_family =
      GetAddressFamilyByInt(remote.address().family());
  log_description_->candidate_pair_protocol =
      GetProtocolByString(local.protocol());
  return log_description_.value();
}

void Connection::set_ice_event_log(webrtc::IceEventLog* ice_event_log) {
  RTC_DCHECK_RUN_ON(network_thread_);
  ice_event_log_ = ice_event_log;
}

void Connection::LogCandidatePairConfig(
    webrtc::IceCandidatePairConfigType type) {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (ice_event_log_ == nullptr) {
    return;
  }
  ice_event_log_->LogCandidatePairConfig(type, id(), ToLogDescription());
}

void Connection::LogCandidatePairEvent(webrtc::IceCandidatePairEventType type,
                                       uint32_t transaction_id) {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (ice_event_log_ == nullptr) {
    return;
  }
  ice_event_log_->LogCandidatePairEvent(type, id(), transaction_id);
}

void Connection::OnConnectionRequestResponse(StunRequest* request,
                                             StunMessage* response) {
  RTC_DCHECK_RUN_ON(network_thread_);


  rtc::LoggingSeverity sev = !writable() ? rtc::LS_INFO : rtc::LS_VERBOSE;

  int rtt = request->Elapsed();

  if (RTC_LOG_CHECK_LEVEL_V(sev)) {
    std::string pings;
    PrintPingsSinceLastResponse(&pings, 5);
    RTC_LOG_V(sev) << ToString() << ": Received "
                   << StunMethodToString(response->type())
                   << ", id=" << rtc::hex_encode(request->id())
                   << ", code=0"  // Makes logging easier to parse.
                      ", rtt="
                   << rtt << ", pings_since_last_response=" << pings;
  }
  absl::optional<uint32_t> nomination;
  const std::string request_id = request->id();
  auto iter = absl::c_find_if(
      pings_since_last_response_,
      [&request_id](const SentPing& ping) { return ping.id == request_id; });
  if (iter != pings_since_last_response_.end()) {
    nomination.emplace(iter->nomination);
  }
  ReceivedPingResponse(rtt, request_id, nomination);

  stats_.recv_ping_responses++;
  LogCandidatePairEvent(
      webrtc::IceCandidatePairEventType::kCheckResponseReceived,
      response->reduced_transaction_id());

  if (request->msg()->type() == STUN_BINDING_REQUEST) {
    if (!remote_support_goog_ping_.has_value()) {
      auto goog_misc = response->GetUInt16List(STUN_ATTR_GOOG_MISC_INFO);
      if (goog_misc != nullptr &&
          goog_misc->Size() >= kSupportGoogPingVersionResponseIndex) {


        remote_support_goog_ping_ =
            goog_misc->GetType(kSupportGoogPingVersionResponseIndex) >=
            kGoogPingVersion;
      } else {
        remote_support_goog_ping_ = false;
      }
    }

    MaybeUpdateLocalCandidate(request, response);

    if (field_trials_->enable_goog_ping && remote_support_goog_ping_) {
      cached_stun_binding_ = request->msg()->Clone();
    }
  }
}

void Connection::OnConnectionRequestErrorResponse(ConnectionRequest* request,
                                                  StunMessage* response) {
  if (!port_)
    return;

  int error_code = response->GetErrorCodeValue();
  RTC_LOG(LS_WARNING) << ToString() << ": Received "
                      << StunMethodToString(response->type())
                      << " id=" << rtc::hex_encode(request->id())
                      << " code=" << error_code
                      << " rtt=" << request->Elapsed();

  cached_stun_binding_.reset();
  if (error_code == STUN_ERROR_UNKNOWN_ATTRIBUTE ||
      error_code == STUN_ERROR_SERVER_ERROR ||
      error_code == STUN_ERROR_UNAUTHORIZED) {

  } else if (error_code == STUN_ERROR_ROLE_CONFLICT) {
    port_->SignalRoleConflict(port_.get());
  } else if (request->msg()->type() == GOOG_PING_REQUEST) {

  } else {

    RTC_LOG(LS_ERROR) << ToString()
                      << ": Received STUN error response, code=" << error_code
                      << "; killing connection";
    set_state(IceCandidatePairState::FAILED);
    port_->DestroyConnectionAsync(this);
  }
}

void Connection::OnConnectionRequestTimeout(ConnectionRequest* request) {

  rtc::LoggingSeverity sev = writable() ? rtc::LS_INFO : rtc::LS_VERBOSE;
  RTC_LOG_V(sev) << ToString() << ": Timing-out STUN ping "
                 << rtc::hex_encode(request->id()) << " after "
                 << request->Elapsed() << " ms";
}

void Connection::OnConnectionRequestSent(ConnectionRequest* request) {
  RTC_DCHECK_RUN_ON(network_thread_);

  rtc::LoggingSeverity sev = !writable() ? rtc::LS_INFO : rtc::LS_VERBOSE;
  RTC_LOG_V(sev) << ToString() << ": Sent "
                 << StunMethodToString(request->msg()->type())
                 << ", id=" << rtc::hex_encode(request->id())
                 << ", use_candidate=" << use_candidate_attr()
                 << ", nomination=" << nomination_;
  stats_.sent_ping_requests_total++;
  LogCandidatePairEvent(webrtc::IceCandidatePairEventType::kCheckSent,
                        request->reduced_transaction_id());
  if (stats_.recv_ping_responses == 0) {
    stats_.sent_ping_requests_before_first_response++;
  }
}

IceCandidatePairState Connection::state() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return state_;
}

int Connection::num_pings_sent() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return num_pings_sent_;
}

void Connection::MaybeSetRemoteIceParametersAndGeneration(
    const IceParameters& ice_params,
    int generation) {
  if (remote_candidate_.username() == ice_params.ufrag &&
      remote_candidate_.password().empty()) {
    remote_candidate_.set_password(ice_params.pwd);
  }



  if (remote_candidate_.username() == ice_params.ufrag &&
      remote_candidate_.password() == ice_params.pwd &&
      remote_candidate_.generation() == 0) {
    remote_candidate_.set_generation(generation);
  }
}

void Connection::MaybeUpdatePeerReflexiveCandidate(
    const Candidate& new_candidate) {
  if (remote_candidate_.type() == PRFLX_PORT_TYPE &&
      new_candidate.type() != PRFLX_PORT_TYPE &&
      remote_candidate_.protocol() == new_candidate.protocol() &&
      remote_candidate_.address() == new_candidate.address() &&
      remote_candidate_.username() == new_candidate.username() &&
      remote_candidate_.password() == new_candidate.password() &&
      remote_candidate_.generation() == new_candidate.generation()) {
    remote_candidate_ = new_candidate;
  }
}

int64_t Connection::last_received() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return std::max(last_data_received_,
                  std::max(last_ping_received_, last_ping_response_received_));
}

int64_t Connection::receiving_unchanged_since() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return receiving_unchanged_since_;
}

uint32_t Connection::prflx_priority() const {
  RTC_DCHECK_RUN_ON(network_thread_);






  IcePriorityValue type_preference =
      (local_candidate_.protocol() == TCP_PROTOCOL_NAME)
          ? ICE_TYPE_PREFERENCE_PRFLX_TCP
          : ICE_TYPE_PREFERENCE_PRFLX;
  return type_preference << 24 | (local_candidate_.priority() & 0x00FFFFFF);
}

ConnectionInfo Connection::stats() {
  RTC_DCHECK_RUN_ON(network_thread_);
  stats_.recv_bytes_second = round(recv_rate_tracker_.ComputeRate());
  stats_.recv_total_bytes = recv_rate_tracker_.TotalSampleCount();
  stats_.sent_bytes_second = round(send_rate_tracker_.ComputeRate());
  stats_.sent_total_bytes = send_rate_tracker_.TotalSampleCount();
  stats_.receiving = receiving_;
  stats_.writable = write_state_ == STATE_WRITABLE;
  stats_.timeout = write_state_ == STATE_WRITE_TIMEOUT;
  stats_.rtt = rtt_;
  stats_.key = this;
  stats_.state = state_;
  if (port_) {
    stats_.priority = priority();
    stats_.local_candidate = local_candidate();
  }
  stats_.nominated = nominated();
  stats_.total_round_trip_time_ms = total_round_trip_time_ms_;
  stats_.current_round_trip_time_ms = current_round_trip_time_ms_;
  stats_.remote_candidate = remote_candidate();
  return stats_;
}

void Connection::MaybeUpdateLocalCandidate(StunRequest* request,
                                           StunMessage* response) {
  if (!port_)
    return;





  const StunAddressAttribute* addr =
      response->GetAddress(STUN_ATTR_XOR_MAPPED_ADDRESS);
  if (!addr) {
    RTC_LOG(LS_WARNING)
        << "Connection::OnConnectionRequestResponse - "
           "No MAPPED-ADDRESS or XOR-MAPPED-ADDRESS found in the "
           "stun response message";
    return;
  }

  for (const Candidate& candidate : port_->Candidates()) {
    if (absl::EndsWith(candidate.address().hostname(), ".reflector")) {
      Candidate testCandidate = candidate;
      testCandidate.set_address(local_candidate_.address());
      if (testCandidate == local_candidate_) {
        return;
      }
    }
      
    if (candidate.address() == addr->GetAddress()) {
      if (local_candidate_ != candidate) {
        RTC_LOG(LS_INFO) << ToString()
                         << ": Updating local candidate type to srflx.";
        local_candidate_ = candidate;


        SignalStateChange(this);
      }
      return;
    }
  }



  const StunUInt32Attribute* priority_attr =
      request->msg()->GetUInt32(STUN_ATTR_PRIORITY);
  if (!priority_attr) {
    RTC_LOG(LS_WARNING) << "Connection::OnConnectionRequestResponse - "
                           "No STUN_ATTR_PRIORITY found in the "
                           "stun response message";
    return;
  }
  const uint32_t priority = priority_attr->value();
  std::string id = rtc::CreateRandomString(8);

  local_candidate_.set_id(id);
  local_candidate_.set_type(PRFLX_PORT_TYPE);


  local_candidate_.set_related_address(local_candidate_.address());
  local_candidate_.set_foundation(port()->ComputeFoundation(
      PRFLX_PORT_TYPE, local_candidate_.protocol(),
      local_candidate_.relay_protocol(), local_candidate_.address()));
  local_candidate_.set_priority(priority);
  local_candidate_.set_address(addr->GetAddress());

  RTC_LOG(LS_INFO) << ToString() << ": Updating local candidate type to prflx.";
  port_->AddPrflxCandidate(local_candidate_);


  SignalStateChange(this);
}

bool Connection::rtt_converged() const {
  RTC_DCHECK_RUN_ON(network_thread_);
  return rtt_samples_ > (RTT_RATIO + 1);
}

bool Connection::missing_responses(int64_t now) const {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (pings_since_last_response_.empty()) {
    return false;
  }

  int64_t waiting = now - pings_since_last_response_[0].sent_time;
  return waiting > 2 * rtt();
}

bool Connection::TooManyOutstandingPings(
    const absl::optional<int>& max_outstanding_pings) const {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (!max_outstanding_pings.has_value()) {
    return false;
  }
  if (static_cast<int>(pings_since_last_response_.size()) <
      *max_outstanding_pings) {
    return false;
  }
  return true;
}

void Connection::SetLocalCandidateNetworkCost(uint16_t cost) {
  RTC_DCHECK_RUN_ON(network_thread_);

  if (cost == local_candidate_.network_cost())
    return;

  local_candidate_.set_network_cost(cost);



  SignalStateChange(this);
}

bool Connection::ShouldSendGoogPing(const StunMessage* message) {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (remote_support_goog_ping_ == true && cached_stun_binding_ &&
      cached_stun_binding_->EqualAttributes(message, [](int type) {



        return type != STUN_ATTR_FINGERPRINT &&
               type != STUN_ATTR_MESSAGE_INTEGRITY &&
               type != STUN_ATTR_RETRANSMIT_COUNT &&
               type != STUN_ATTR_GOOG_MISC_INFO;
      })) {
    return true;
  }
  return false;
}

void Connection::ForgetLearnedState() {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_LOG(LS_INFO) << ToString() << ": Connection forget learned state";
  requests_.Clear();
  receiving_ = false;
  write_state_ = STATE_WRITE_INIT;
  rtt_estimate_.Reset();
  pings_since_last_response_.clear();
}

ProxyConnection::ProxyConnection(rtc::WeakPtr<Port> port,
                                 size_t index,
                                 const Candidate& remote_candidate)
    : Connection(std::move(port), index, remote_candidate) {}

int ProxyConnection::Send(const void* data,
                          size_t size,
                          const rtc::PacketOptions& options) {
  if (!port_)
    return SOCKET_ERROR;

  stats_.sent_total_packets++;
  int sent =
      port_->SendTo(data, size, remote_candidate_.address(), options, true);
  int64_t now = rtc::TimeMillis();
  if (sent <= 0) {
    RTC_DCHECK(sent < 0);
    error_ = port_->GetError();
    stats_.sent_discarded_packets++;
    stats_.sent_discarded_bytes += size;
  } else {
    send_rate_tracker_.AddSamplesAtTime(now, sent);
  }
  last_send_data_ = now;
  return sent;
}

int ProxyConnection::GetError() {
  return error_;
}

}  // namespace cricket
