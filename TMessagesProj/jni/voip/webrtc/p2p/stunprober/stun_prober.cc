/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "p2p/stunprober/stun_prober.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "api/packet_socket_factory.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "api/transport/stun.h"
#include "api/units/time_delta.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/async_resolver_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/helpers.h"
#include "rtc_base/logging.h"
#include "rtc_base/thread.h"
#include "rtc_base/time_utils.h"

namespace stunprober {

namespace {
using ::webrtc::SafeTask;
using ::webrtc::TimeDelta;

const int THREAD_WAKE_UP_INTERVAL_MS = 5;

template <typename T>
void IncrementCounterByAddress(std::map<T, int>* counter_per_ip, const T& ip) {
  counter_per_ip->insert(std::make_pair(ip, 0)).first->second++;
}

}  // namespace

// STUN servers
class StunProber::Requester : public sigslot::has_slots<> {
 public:

  struct Request {

    int64_t sent_time_ms = 0;

    int64_t received_time_ms = 0;

    rtc::SocketAddress srflx_addr;

    rtc::IPAddress server_addr;

    int64_t rtt() { return received_time_ms - sent_time_ms; }
    void ProcessResponse(const char* buf, size_t buf_len);
  };



  Requester(StunProber* prober,
            rtc::AsyncPacketSocket* socket,
            const std::vector<rtc::SocketAddress>& server_ips);
  ~Requester() override;

  Requester(const Requester&) = delete;
  Requester& operator=(const Requester&) = delete;



  void SendStunRequest();

  void OnStunResponseReceived(rtc::AsyncPacketSocket* socket,
                              const char* buf,
                              size_t size,
                              const rtc::SocketAddress& addr,
                              const int64_t& packet_time_us);

  const std::vector<Request*>& requests() { return requests_; }

  bool Done() {
    return static_cast<size_t>(num_request_sent_) == server_ips_.size();
  }

 private:
  Request* GetRequestByAddress(const rtc::IPAddress& ip);

  StunProber* prober_;

  std::unique_ptr<rtc::AsyncPacketSocket> socket_;

  rtc::SocketAddress addr_;
  std::unique_ptr<rtc::ByteBufferWriter> response_packet_;

  std::vector<Request*> requests_;
  std::vector<rtc::SocketAddress> server_ips_;
  int16_t num_request_sent_ = 0;
  int16_t num_response_received_ = 0;

  webrtc::SequenceChecker& thread_checker_;
};

StunProber::Requester::Requester(
    StunProber* prober,
    rtc::AsyncPacketSocket* socket,
    const std::vector<rtc::SocketAddress>& server_ips)
    : prober_(prober),
      socket_(socket),
      response_packet_(new rtc::ByteBufferWriter(nullptr, kMaxUdpBufferSize)),
      server_ips_(server_ips),
      thread_checker_(prober->thread_checker_) {
  socket_->SignalReadPacket.connect(
      this, &StunProber::Requester::OnStunResponseReceived);
}

StunProber::Requester::~Requester() {
  if (socket_) {
    socket_->Close();
  }
  for (auto* req : requests_) {
    if (req) {
      delete req;
    }
  }
}

void StunProber::Requester::SendStunRequest() {
  RTC_DCHECK(thread_checker_.IsCurrent());
  requests_.push_back(new Request());
  Request& request = *(requests_.back());

  cricket::StunMessage message(cricket::STUN_BINDING_REQUEST);

  std::unique_ptr<rtc::ByteBufferWriter> request_packet(
      new rtc::ByteBufferWriter(nullptr, kMaxUdpBufferSize));
  if (!message.Write(request_packet.get())) {
    prober_->ReportOnFinished(WRITE_FAILED);
    return;
  }

  auto addr = server_ips_[num_request_sent_];
  request.server_addr = addr.ipaddr();



  rtc::PacketOptions options;
  int rv = socket_->SendTo(const_cast<char*>(request_packet->Data()),
                           request_packet->Length(), addr, options);
  if (rv < 0) {
    prober_->ReportOnFinished(WRITE_FAILED);
    return;
  }

  request.sent_time_ms = rtc::TimeMillis();

  num_request_sent_++;
  RTC_DCHECK(static_cast<size_t>(num_request_sent_) <= server_ips_.size());
}

void StunProber::Requester::Request::ProcessResponse(const char* buf,
                                                     size_t buf_len) {
  int64_t now = rtc::TimeMillis();
  rtc::ByteBufferReader message(buf, buf_len);
  cricket::StunMessage stun_response;
  if (!stun_response.Read(&message)) {

    received_time_ms = 0;
    return;
  }

  const cricket::StunAddressAttribute* addr_attr =
      stun_response.GetAddress(cricket::STUN_ATTR_MAPPED_ADDRESS);
  if (addr_attr == nullptr) {

    return;
  }

  if (addr_attr->family() != cricket::STUN_ADDRESS_IPV4 &&
      addr_attr->family() != cricket::STUN_ADDRESS_IPV6) {
    return;
  }

  received_time_ms = now;

  srflx_addr = addr_attr->GetAddress();
}

void StunProber::Requester::OnStunResponseReceived(
    rtc::AsyncPacketSocket* socket,
    const char* buf,
    size_t size,
    const rtc::SocketAddress& addr,
    const int64_t& /* packet_time_us */) {
  RTC_DCHECK(thread_checker_.IsCurrent());
  RTC_DCHECK(socket_);
  Request* request = GetRequestByAddress(addr.ipaddr());
  if (!request) {

    prober_->ReportOnFinished(GENERIC_FAILURE);
    return;
  }

  num_response_received_++;
  request->ProcessResponse(buf, size);
}

StunProber::Requester::Request* StunProber::Requester::GetRequestByAddress(
    const rtc::IPAddress& ipaddr) {
  RTC_DCHECK(thread_checker_.IsCurrent());
  for (auto* request : requests_) {
    if (request->server_addr == ipaddr) {
      return request;
    }
  }

  return nullptr;
}

StunProber::Stats::Stats() = default;

StunProber::Stats::~Stats() = default;

StunProber::ObserverAdapter::ObserverAdapter() = default;

StunProber::ObserverAdapter::~ObserverAdapter() = default;

void StunProber::ObserverAdapter::OnPrepared(StunProber* stunprober,
                                             Status status) {
  if (status == SUCCESS) {
    stunprober->Start(this);
  } else {
    callback_(stunprober, status);
  }
}

void StunProber::ObserverAdapter::OnFinished(StunProber* stunprober,
                                             Status status) {
  callback_(stunprober, status);
}

StunProber::StunProber(rtc::PacketSocketFactory* socket_factory,
                       rtc::Thread* thread,
                       std::vector<const rtc::Network*> networks)
    : interval_ms_(0),
      socket_factory_(socket_factory),
      thread_(thread),
      networks_(std::move(networks)) {}

StunProber::~StunProber() {
  RTC_DCHECK(thread_checker_.IsCurrent());
  for (auto* req : requesters_) {
    if (req) {
      delete req;
    }
  }
  for (auto* s : sockets_) {
    if (s) {
      delete s;
    }
  }
}

bool StunProber::Start(const std::vector<rtc::SocketAddress>& servers,
                       bool shared_socket_mode,
                       int interval_ms,
                       int num_request_per_ip,
                       int timeout_ms,
                       const AsyncCallback callback) {
  observer_adapter_.set_callback(callback);
  return Prepare(servers, shared_socket_mode, interval_ms, num_request_per_ip,
                 timeout_ms, &observer_adapter_);
}

bool StunProber::Prepare(const std::vector<rtc::SocketAddress>& servers,
                         bool shared_socket_mode,
                         int interval_ms,
                         int num_request_per_ip,
                         int timeout_ms,
                         StunProber::Observer* observer) {
  RTC_DCHECK(thread_checker_.IsCurrent());
  interval_ms_ = interval_ms;
  shared_socket_mode_ = shared_socket_mode;

  requests_per_ip_ = num_request_per_ip;
  if (requests_per_ip_ == 0 || servers.size() == 0) {
    return false;
  }

  timeout_ms_ = timeout_ms;
  servers_ = servers;
  observer_ = observer;

  for (auto it = servers_.begin(); it != servers_.end();) {
    if (it->ipaddr().family() != AF_UNSPEC) {
      all_servers_addrs_.push_back(*it);
      it = servers_.erase(it);
    } else {
      ++it;
    }
  }
  if (servers_.empty()) {
    CreateSockets();
    return true;
  }
  return ResolveServerName(servers_.back());
}

bool StunProber::Start(StunProber::Observer* observer) {
  observer_ = observer;
  if (total_ready_sockets_ != total_socket_required()) {
    return false;
  }
  MaybeScheduleStunRequests();
  return true;
}

bool StunProber::ResolveServerName(const rtc::SocketAddress& addr) {
  rtc::AsyncResolverInterface* resolver =
      socket_factory_->CreateAsyncResolver();
  if (!resolver) {
    return false;
  }
  resolver->SignalDone.connect(this, &StunProber::OnServerResolved);
  resolver->Start(addr);
  return true;
}

void StunProber::OnSocketReady(rtc::AsyncPacketSocket* socket,
                               const rtc::SocketAddress& addr) {
  total_ready_sockets_++;
  if (total_ready_sockets_ == total_socket_required()) {
    ReportOnPrepared(SUCCESS);
  }
}

void StunProber::OnServerResolved(rtc::AsyncResolverInterface* resolver) {
  RTC_DCHECK(thread_checker_.IsCurrent());

  if (resolver->GetError() == 0) {
    rtc::SocketAddress addr(resolver->address().ipaddr(),
                            resolver->address().port());
    all_servers_addrs_.push_back(addr);
  }


  thread_->PostTask([resolver] { resolver->Destroy(false); });
  servers_.pop_back();

  if (servers_.size()) {
    if (!ResolveServerName(servers_.back())) {
      ReportOnPrepared(RESOLVE_FAILED);
    }
    return;
  }

  if (all_servers_addrs_.size() == 0) {
    ReportOnPrepared(RESOLVE_FAILED);
    return;
  }

  CreateSockets();
}

void StunProber::CreateSockets() {

  std::set<rtc::SocketAddress> addrs(all_servers_addrs_.begin(),
                                     all_servers_addrs_.end());
  all_servers_addrs_.assign(addrs.begin(), addrs.end());

  while (sockets_.size() < total_socket_required()) {
    std::unique_ptr<rtc::AsyncPacketSocket> socket(
        socket_factory_->CreateUdpSocket(rtc::SocketAddress(INADDR_ANY, 0), 0,
                                         0));
    if (!socket) {
      ReportOnPrepared(GENERIC_FAILURE);
      return;
    }


    if (socket->GetState() == rtc::AsyncPacketSocket::STATE_BINDING) {
      socket->SignalAddressReady.connect(this, &StunProber::OnSocketReady);
    } else {
      OnSocketReady(socket.get(), rtc::SocketAddress(INADDR_ANY, 0));
    }
    sockets_.push_back(socket.release());
  }
}

StunProber::Requester* StunProber::CreateRequester() {
  RTC_DCHECK(thread_checker_.IsCurrent());
  if (!sockets_.size()) {
    return nullptr;
  }
  StunProber::Requester* requester;
  if (shared_socket_mode_) {
    requester = new Requester(this, sockets_.back(), all_servers_addrs_);
  } else {
    std::vector<rtc::SocketAddress> server_ip;
    server_ip.push_back(
        all_servers_addrs_[(num_request_sent_ % all_servers_addrs_.size())]);
    requester = new Requester(this, sockets_.back(), server_ip);
  }

  sockets_.pop_back();
  return requester;
}

bool StunProber::SendNextRequest() {
  if (!current_requester_ || current_requester_->Done()) {
    current_requester_ = CreateRequester();
    requesters_.push_back(current_requester_);
  }
  if (!current_requester_) {
    return false;
  }
  current_requester_->SendStunRequest();
  num_request_sent_++;
  return true;
}

bool StunProber::should_send_next_request(int64_t now) {
  if (interval_ms_ < THREAD_WAKE_UP_INTERVAL_MS) {
    return now >= next_request_time_ms_;
  } else {
    return (now + (THREAD_WAKE_UP_INTERVAL_MS / 2)) >= next_request_time_ms_;
  }
}

int StunProber::get_wake_up_interval_ms() {
  if (interval_ms_ < THREAD_WAKE_UP_INTERVAL_MS) {
    return 1;
  } else {
    return THREAD_WAKE_UP_INTERVAL_MS;
  }
}

void StunProber::MaybeScheduleStunRequests() {
  RTC_DCHECK_RUN_ON(thread_);
  int64_t now = rtc::TimeMillis();

  if (Done()) {
    thread_->PostDelayedTask(
        SafeTask(task_safety_.flag(), [this] { ReportOnFinished(SUCCESS); }),
        TimeDelta::Millis(timeout_ms_));
    return;
  }
  if (should_send_next_request(now)) {
    if (!SendNextRequest()) {
      ReportOnFinished(GENERIC_FAILURE);
      return;
    }
    next_request_time_ms_ = now + interval_ms_;
  }
  thread_->PostDelayedTask(
      SafeTask(task_safety_.flag(), [this] { MaybeScheduleStunRequests(); }),
      TimeDelta::Millis(get_wake_up_interval_ms()));
}

bool StunProber::GetStats(StunProber::Stats* prob_stats) const {

  if (!prob_stats) {
    return false;
  }

  StunProber::Stats stats;

  int rtt_sum = 0;
  int64_t first_sent_time = 0;
  int64_t last_sent_time = 0;
  NatType nat_type = NATTYPE_INVALID;

  std::set<rtc::IPAddress> srflx_ips;


  std::map<rtc::IPAddress, int> num_response_per_server;
  std::map<rtc::IPAddress, int> num_request_per_server;

  for (auto* requester : requesters_) {
    std::map<rtc::SocketAddress, int> num_response_per_srflx_addr;
    for (auto* request : requester->requests()) {
      if (request->sent_time_ms <= 0) {
        continue;
      }

      ++stats.raw_num_request_sent;
      IncrementCounterByAddress(&num_request_per_server, request->server_addr);

      if (!first_sent_time) {
        first_sent_time = request->sent_time_ms;
      }
      last_sent_time = request->sent_time_ms;

      if (request->received_time_ms < request->sent_time_ms) {
        continue;
      }

      IncrementCounterByAddress(&num_response_per_server, request->server_addr);
      IncrementCounterByAddress(&num_response_per_srflx_addr,
                                request->srflx_addr);
      rtt_sum += request->rtt();
      stats.srflx_addrs.insert(request->srflx_addr.ToString());
      srflx_ips.insert(request->srflx_addr.ipaddr());
    }


    if (shared_socket_mode_ && num_response_per_srflx_addr.size() > 1) {
      nat_type = NATTYPE_SYMMETRIC;
    }
  }


  if (srflx_ips.size() > 1) {
    return false;
  }

  int num_sent = 0;
  int num_received = 0;
  int num_server_ip_with_response = 0;

  for (const auto& kv : num_response_per_server) {
    RTC_DCHECK_GT(kv.second, 0);
    num_server_ip_with_response++;
    num_received += kv.second;
    num_sent += num_request_per_server[kv.first];
  }


  stats.shared_socket_mode =
      shared_socket_mode_ && (num_server_ip_with_response > 1);

  if (stats.shared_socket_mode && nat_type == NATTYPE_INVALID) {
    nat_type = NATTYPE_NON_SYMMETRIC;
  }

  rtc::SocketAddress srflx_addr;
  if (stats.srflx_addrs.size() &&
      !srflx_addr.FromString(*(stats.srflx_addrs.begin()))) {
    return false;
  }
  for (const auto* net : networks_) {
    if (srflx_addr.ipaddr() == net->GetBestIP()) {
      nat_type = stunprober::NATTYPE_NONE;
      stats.host_ip = net->GetBestIP().ToString();
      break;
    }
  }

  if (nat_type == NATTYPE_INVALID) {
    nat_type = NATTYPE_UNKNOWN;
  }

  stats.nat_type = nat_type;
  stats.num_request_sent = num_sent;
  stats.num_response_received = num_received;
  stats.target_request_interval_ns = interval_ms_ * 1000;

  if (num_sent) {
    stats.success_percent = static_cast<int>(100 * num_received / num_sent);
  }

  if (stats.raw_num_request_sent > 1) {
    stats.actual_request_interval_ns =
        (1000 * (last_sent_time - first_sent_time)) /
        (stats.raw_num_request_sent - 1);
  }

  if (num_received) {
    stats.average_rtt_ms = static_cast<int>((rtt_sum / num_received));
  }

  *prob_stats = stats;
  return true;
}

void StunProber::ReportOnPrepared(StunProber::Status status) {
  if (observer_) {
    observer_->OnPrepared(this, status);
  }
}

void StunProber::ReportOnFinished(StunProber::Status status) {
  if (observer_) {
    observer_->OnFinished(this, status);
  }
}

}  // namespace stunprober
