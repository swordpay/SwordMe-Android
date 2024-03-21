/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_STUNPROBER_STUN_PROBER_H_
#define P2P_STUNPROBER_STUN_PROBER_H_

#include <set>
#include <string>
#include <vector>

#include "api/sequence_checker.h"
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/byte_buffer.h"
#include "rtc_base/ip_address.h"
#include "rtc_base/network.h"
#include "rtc_base/socket_address.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/thread.h"

namespace rtc {
class AsyncPacketSocket;
class PacketSocketFactory;
class Thread;
class NetworkManager;
class AsyncResolverInterface;
}  // namespace rtc

namespace stunprober {

class StunProber;

static const int kMaxUdpBufferSize = 1200;

typedef std::function<void(StunProber*, int)> AsyncCallback;

enum NatType {
  NATTYPE_INVALID,
  NATTYPE_NONE,          // Not behind a NAT.
  NATTYPE_UNKNOWN,       // Behind a NAT but type can't be determine.
  NATTYPE_SYMMETRIC,     // Behind a symmetric NAT.
  NATTYPE_NON_SYMMETRIC  // Behind a non-symmetric NAT.
};

class RTC_EXPORT StunProber : public sigslot::has_slots<> {
 public:
  enum Status {       // Used in UMA_HISTOGRAM_ENUMERATION.
    SUCCESS,          // Successfully received bytes from the server.
    GENERIC_FAILURE,  // Generic failure.
    RESOLVE_FAILED,   // Host resolution failed.
    WRITE_FAILED,     // Sending a message to the server failed.
    READ_FAILED,      // Reading the reply from the server failed.
  };

  class Observer {
   public:
    virtual ~Observer() = default;
    virtual void OnPrepared(StunProber* prober, StunProber::Status status) = 0;
    virtual void OnFinished(StunProber* prober, StunProber::Status status) = 0;
  };

  struct RTC_EXPORT Stats {
    Stats();
    ~Stats();





    int raw_num_request_sent = 0;
    int num_request_sent = 0;

    int num_response_received = 0;
    NatType nat_type = NATTYPE_INVALID;
    int average_rtt_ms = -1;
    int success_percent = 0;
    int target_request_interval_ns = 0;
    int actual_request_interval_ns = 0;



    bool shared_socket_mode = false;

    std::string host_ip;

    std::set<std::string> srflx_addrs;
  };

  StunProber(rtc::PacketSocketFactory* socket_factory,
             rtc::Thread* thread,
             std::vector<const rtc::Network*> networks);
  ~StunProber() override;

  StunProber(const StunProber&) = delete;
  StunProber& operator=(const StunProber&) = delete;













  bool Start(const std::vector<rtc::SocketAddress>& servers,
             bool shared_socket_mode,
             int stun_ta_interval_ms,
             int requests_per_ip,
             int timeout_ms,
             AsyncCallback finish_callback);


  bool Prepare(const std::vector<rtc::SocketAddress>& servers,
               bool shared_socket_mode,
               int stun_ta_interval_ms,
               int requests_per_ip,
               int timeout_ms,
               StunProber::Observer* observer);

  bool Start(StunProber::Observer* observer);



  bool GetStats(Stats* stats) const;

  int estimated_execution_time() {
    return static_cast<int>(requests_per_ip_ * all_servers_addrs_.size() *
                            interval_ms_);
  }

 private:


  class Requester;


  class ObserverAdapter : public Observer {
   public:
    ObserverAdapter();
    ~ObserverAdapter() override;

    void set_callback(AsyncCallback callback) { callback_ = callback; }
    void OnPrepared(StunProber* stunprober, Status status) override;
    void OnFinished(StunProber* stunprober, Status status) override;

   private:
    AsyncCallback callback_;
  };

  bool ResolveServerName(const rtc::SocketAddress& addr);
  void OnServerResolved(rtc::AsyncResolverInterface* resolver);

  void OnSocketReady(rtc::AsyncPacketSocket* socket,
                     const rtc::SocketAddress& addr);

  void CreateSockets();

  bool Done() {
    return num_request_sent_ >= requests_per_ip_ * all_servers_addrs_.size();
  }

  size_t total_socket_required() {
    return (shared_socket_mode_ ? 1 : all_servers_addrs_.size()) *
           requests_per_ip_;
  }

  bool should_send_next_request(int64_t now);
  int get_wake_up_interval_ms();

  bool SendNextRequest();


  void MaybeScheduleStunRequests();

  void ReportOnPrepared(StunProber::Status status);
  void ReportOnFinished(StunProber::Status status);

  Requester* CreateRequester();

  Requester* current_requester_ = nullptr;

  int64_t next_request_time_ms_ = 0;

  uint32_t num_request_sent_ = 0;

  bool shared_socket_mode_ = false;

  uint32_t requests_per_ip_ = 0;

  int interval_ms_;

  int timeout_ms_;

  std::vector<rtc::SocketAddress> servers_;

  rtc::PacketSocketFactory* socket_factory_;
  rtc::Thread* thread_;

  std::vector<rtc::SocketAddress> all_servers_addrs_;

  std::vector<Requester*> requesters_;

  webrtc::SequenceChecker thread_checker_;

  std::vector<rtc::AsyncPacketSocket*> sockets_;

  size_t total_ready_sockets_ = 0;

  Observer* observer_ = nullptr;


  ObserverAdapter observer_adapter_;

  const std::vector<const rtc::Network*> networks_;

  webrtc::ScopedTaskSafety task_safety_;
};

}  // namespace stunprober

#endif  // P2P_STUNPROBER_STUN_PROBER_H_
