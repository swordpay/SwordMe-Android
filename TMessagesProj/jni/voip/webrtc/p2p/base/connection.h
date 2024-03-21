/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_CONNECTION_H_
#define P2P_BASE_CONNECTION_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/candidate.h"
#include "api/transport/stun.h"
#include "logging/rtc_event_log/ice_logger.h"
#include "p2p/base/candidate_pair_interface.h"
#include "p2p/base/connection_info.h"
#include "p2p/base/p2p_transport_channel_ice_field_trials.h"
#include "p2p/base/stun_request.h"
#include "p2p/base/transport_description.h"
#include "rtc_base/async_packet_socket.h"
#include "rtc_base/network.h"
#include "rtc_base/numerics/event_based_exponential_moving_average.h"
#include "rtc_base/rate_tracker.h"
#include "rtc_base/weak_ptr.h"

namespace cricket {

// adding other flavors in the future.
constexpr int kGoogPingVersion = 1;

// So we use forward declaration rather than include.
class Port;

class Connection;

struct CandidatePair final : public CandidatePairInterface {
  ~CandidatePair() override = default;

  const Candidate& local_candidate() const override { return local; }
  const Candidate& remote_candidate() const override { return remote; }

  Candidate local;
  Candidate remote;
};

// port on the remote client.
class Connection : public CandidatePairInterface {
 public:
  struct SentPing {
    SentPing(absl::string_view id, int64_t sent_time, uint32_t nomination)
        : id(id), sent_time(sent_time), nomination(nomination) {}

    std::string id;
    int64_t sent_time;
    uint32_t nomination;
  };

  ~Connection() override;

  uint32_t id() const { return id_; }

  webrtc::TaskQueueBase* network_thread() const;


  const Candidate& local_candidate() const override;

  const Candidate& remote_candidate() const override;

  virtual const rtc::Network* network() const;

  virtual int generation() const;

  virtual uint64_t priority() const;

  enum WriteState {
    STATE_WRITABLE = 0,          // we have received ping responses recently
    STATE_WRITE_UNRELIABLE = 1,  // we have had a few ping failures
    STATE_WRITE_INIT = 2,        // we have yet to receive a ping response
    STATE_WRITE_TIMEOUT = 3,     // we have had a large number of ping failures
  };

  WriteState write_state() const;
  bool writable() const;
  bool receiving() const;

  const Port* port() const {
    RTC_DCHECK_RUN_ON(network_thread_);
    return port_.get();
  }


  bool connected() const;
  bool weak() const;
  bool active() const;

  bool dead(int64_t now) const;

  int rtt() const;

  int unwritable_timeout() const;
  void set_unwritable_timeout(const absl::optional<int>& value_ms);
  int unwritable_min_checks() const;
  void set_unwritable_min_checks(const absl::optional<int>& value);
  int inactive_timeout() const;
  void set_inactive_timeout(const absl::optional<int>& value);


  ConnectionInfo stats();

  sigslot::signal1<Connection*> SignalStateChange;


  sigslot::signal1<Connection*> SignalDestroyed;



  virtual int Send(const void* data,
                   size_t size,
                   const rtc::PacketOptions& options) = 0;

  virtual int GetError() = 0;

  sigslot::signal4<Connection*, const char*, size_t, int64_t> SignalReadPacket;

  sigslot::signal1<Connection*> SignalReadyToSend;

  void OnReadPacket(const char* data, size_t size, int64_t packet_time_us);

  void OnReadyToSend();




  bool pruned() const;
  void Prune();

  bool use_candidate_attr() const;
  void set_use_candidate_attr(bool enable);

  void set_nomination(uint32_t value);

  uint32_t remote_nomination() const;







  bool nominated() const;

  int receiving_timeout() const;
  void set_receiving_timeout(absl::optional<int> receiving_timeout_ms);



  void Destroy();





  bool Shutdown();


  void FailAndPrune();


  void UpdateState(int64_t now);

  void UpdateLocalIceParameters(int component,
                                absl::string_view username_fragment,
                                absl::string_view password);

  int64_t last_ping_sent() const;
  void Ping(int64_t now);
  void ReceivedPingResponse(
      int rtt,
      absl::string_view request_id,
      const absl::optional<uint32_t>& nomination = absl::nullopt);
  std::unique_ptr<IceMessage> BuildPingRequest() RTC_RUN_ON(network_thread_);

  int64_t last_ping_response_received() const;
  const absl::optional<std::string>& last_ping_id_received() const;

  int rtt_samples() const;


  int64_t last_ping_received() const;

  void ReceivedPing(
      const absl::optional<std::string>& request_id = absl::nullopt);

  void HandleStunBindingOrGoogPingRequest(IceMessage* msg);



  void HandlePiggybackCheckAcknowledgementIfAny(StunMessage* msg);

  int64_t last_send_data() const;
  int64_t last_data_received() const;

  std::string ToDebugId() const;
  std::string ToString() const;
  std::string ToSensitiveString() const;

  const webrtc::IceCandidatePairDescription& ToLogDescription();
  void set_ice_event_log(webrtc::IceEventLog* ice_event_log);

  void PrintPingsSinceLastResponse(std::string* pings, size_t max);



  bool selected() const;
  void set_selected(bool selected);


  sigslot::signal1<Connection*> SignalNominated;

  IceCandidatePairState state() const;

  int num_pings_sent() const;

  uint32_t ComputeNetworkCost() const;



  void MaybeSetRemoteIceParametersAndGeneration(const IceParameters& params,
                                                int generation);



  void MaybeUpdatePeerReflexiveCandidate(const Candidate& new_candidate);


  int64_t last_received() const;

  int64_t receiving_unchanged_since() const;


  uint32_t prflx_priority() const;

  bool stable(int64_t now) const;

  bool TooManyOutstandingPings(const absl::optional<int>& val) const;

  void SetLocalCandidateNetworkCost(uint16_t cost);

  void SetIceFieldTrials(const IceFieldTrials* field_trials);
  const rtc::EventBasedExponentialMovingAverage& GetRttEstimate() const {
    return rtt_estimate_;
  }












  void ForgetLearnedState();

  void SendStunBindingResponse(const StunMessage* message);
  void SendGoogPingResponse(const StunMessage* message);
  void SendResponseMessage(const StunMessage& response);

  Port* PortForTest() { return port_.get(); }
  const Port* PortForTest() const { return port_.get(); }

  uint32_t acked_nomination() const;
  void set_remote_nomination(uint32_t remote_nomination);

 protected:

  class ConnectionRequest;

  Connection(rtc::WeakPtr<Port> port, size_t index, const Candidate& candidate);

  void OnSendStunPacket(const void* data, size_t size, StunRequest* req);

  virtual void OnConnectionRequestResponse(StunRequest* req,
                                           StunMessage* response);
  void OnConnectionRequestErrorResponse(ConnectionRequest* req,
                                        StunMessage* response)
      RTC_RUN_ON(network_thread_);
  void OnConnectionRequestTimeout(ConnectionRequest* req)
      RTC_RUN_ON(network_thread_);
  void OnConnectionRequestSent(ConnectionRequest* req)
      RTC_RUN_ON(network_thread_);

  bool rtt_converged() const;


  bool missing_responses(int64_t now) const;

  void set_write_state(WriteState value);
  void UpdateReceiving(int64_t now);
  void set_state(IceCandidatePairState state);
  void set_connected(bool value);

  Port* port() { return port_.get(); }





  webrtc::TaskQueueBase* const network_thread_;
  const uint32_t id_;
  rtc::WeakPtr<Port> port_;
  Candidate local_candidate_ RTC_GUARDED_BY(network_thread_);
  Candidate remote_candidate_;

  ConnectionInfo stats_;
  rtc::RateTracker recv_rate_tracker_;
  rtc::RateTracker send_rate_tracker_;
  int64_t last_send_data_ = 0;

 private:


  void MaybeUpdateLocalCandidate(StunRequest* request, StunMessage* response)
      RTC_RUN_ON(network_thread_);

  void LogCandidatePairConfig(webrtc::IceCandidatePairConfigType type)
      RTC_RUN_ON(network_thread_);
  void LogCandidatePairEvent(webrtc::IceCandidatePairEventType type,
                             uint32_t transaction_id)
      RTC_RUN_ON(network_thread_);


  bool ShouldSendGoogPing(const StunMessage* message)
      RTC_RUN_ON(network_thread_);

  WriteState write_state_ RTC_GUARDED_BY(network_thread_);
  bool receiving_ RTC_GUARDED_BY(network_thread_);
  bool connected_ RTC_GUARDED_BY(network_thread_);
  bool pruned_ RTC_GUARDED_BY(network_thread_);
  bool selected_ RTC_GUARDED_BY(network_thread_) = false;




  bool use_candidate_attr_ RTC_GUARDED_BY(network_thread_);





  uint32_t nomination_ RTC_GUARDED_BY(network_thread_) = 0;

  uint32_t acked_nomination_ RTC_GUARDED_BY(network_thread_) = 0;



  uint32_t remote_nomination_ RTC_GUARDED_BY(network_thread_) = 0;

  StunRequestManager requests_ RTC_GUARDED_BY(network_thread_);
  int rtt_ RTC_GUARDED_BY(network_thread_);
  int rtt_samples_ RTC_GUARDED_BY(network_thread_) = 0;

  uint64_t total_round_trip_time_ms_ RTC_GUARDED_BY(network_thread_) = 0;

  absl::optional<uint32_t> current_round_trip_time_ms_
      RTC_GUARDED_BY(network_thread_);
  int64_t last_ping_sent_ RTC_GUARDED_BY(
      network_thread_);  // last time we sent a ping to the other side
  int64_t last_ping_received_
      RTC_GUARDED_BY(network_thread_);  // last time we received a ping from the

  int64_t last_data_received_ RTC_GUARDED_BY(network_thread_);
  int64_t last_ping_response_received_ RTC_GUARDED_BY(network_thread_);
  int64_t receiving_unchanged_since_ RTC_GUARDED_BY(network_thread_) = 0;
  std::vector<SentPing> pings_since_last_response_
      RTC_GUARDED_BY(network_thread_);


  absl::optional<std::string> last_ping_id_received_
      RTC_GUARDED_BY(network_thread_);

  absl::optional<int> unwritable_timeout_ RTC_GUARDED_BY(network_thread_);
  absl::optional<int> unwritable_min_checks_ RTC_GUARDED_BY(network_thread_);
  absl::optional<int> inactive_timeout_ RTC_GUARDED_BY(network_thread_);

  IceCandidatePairState state_ RTC_GUARDED_BY(network_thread_);

  absl::optional<int> receiving_timeout_ RTC_GUARDED_BY(network_thread_);
  int64_t time_created_ms_ RTC_GUARDED_BY(network_thread_);
  int num_pings_sent_ RTC_GUARDED_BY(network_thread_) = 0;

  absl::optional<webrtc::IceCandidatePairDescription> log_description_
      RTC_GUARDED_BY(network_thread_);
  webrtc::IceEventLog* ice_event_log_ RTC_GUARDED_BY(network_thread_) = nullptr;




  absl::optional<bool> remote_support_goog_ping_
      RTC_GUARDED_BY(network_thread_);
  std::unique_ptr<StunMessage> cached_stun_binding_
      RTC_GUARDED_BY(network_thread_);

  const IceFieldTrials* field_trials_;
  rtc::EventBasedExponentialMovingAverage rtt_estimate_
      RTC_GUARDED_BY(network_thread_);
};

class ProxyConnection : public Connection {
 public:
  ProxyConnection(rtc::WeakPtr<Port> port,
                  size_t index,
                  const Candidate& remote_candidate);

  int Send(const void* data,
           size_t size,
           const rtc::PacketOptions& options) override;
  int GetError() override;

 private:
  int error_ = 0;
};

}  // namespace cricket

#endif  // P2P_BASE_CONNECTION_H_
