/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_ICE_TRANSPORT_INTERNAL_H_
#define P2P_BASE_ICE_TRANSPORT_INTERNAL_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/candidate.h"
#include "api/rtc_error.h"
#include "api/transport/enums.h"
#include "p2p/base/connection.h"
#include "p2p/base/packet_transport_internal.h"
#include "p2p/base/port.h"
#include "p2p/base/transport_description.h"
#include "rtc_base/network_constants.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/time_utils.h"

namespace cricket {

struct IceTransportStats {
  CandidateStatsList candidate_stats_list;
  ConnectionInfos connection_infos;



  uint32_t selected_candidate_pair_changes = 0;




  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
  uint64_t packets_sent = 0;
  uint64_t packets_received = 0;

  IceRole ice_role = ICEROLE_UNKNOWN;
  std::string ice_local_username_fragment;
  webrtc::IceTransportState ice_state = webrtc::IceTransportState::kNew;
};

typedef std::vector<Candidate> Candidates;

enum IceConnectionState {
  kIceConnectionConnecting = 0,
  kIceConnectionFailed,
  kIceConnectionConnected,  // Writable, but still checking one or more

  kIceConnectionCompleted,
};

// once /talk/ and /webrtc/ are combined, and also switch to ENUM_NAME naming
// style.
enum IceGatheringState {
  kIceGatheringNew = 0,
  kIceGatheringGathering,
  kIceGatheringComplete,
};

enum ContinualGatheringPolicy {

  GATHER_ONCE = 0,

  GATHER_CONTINUALLY,
};

enum class NominationMode {
  REGULAR,         // Nominate once per ICE restart (Not implemented yet).
  AGGRESSIVE,      // Nominate every connection except that it will behave as if

  SEMI_AGGRESSIVE  // Our current implementation of the nomination algorithm.

};

// and contain valid values. If conditions are not met, an RTCError with the
// appropriated error number and description is returned. If the configuration
// is valid RTCError::OK() is returned.
webrtc::RTCError VerifyCandidate(const Candidate& cand);

// for each one, stopping on the first error encounted and returning that error
// value if so. On success returns RTCError::OK().
webrtc::RTCError VerifyCandidates(const Candidates& candidates);

// TODO(deadbeef): Use absl::optional to represent unset values, instead of
// -1.
struct IceConfig {

  absl::optional<int> receiving_timeout;


  absl::optional<int> backup_connection_ping_interval;

  ContinualGatheringPolicy continual_gathering_policy = GATHER_ONCE;

  bool gather_continually() const {
    return continual_gathering_policy == GATHER_CONTINUALLY;
  }


  bool prioritize_most_likely_candidate_pairs = false;

  absl::optional<int> stable_writable_connection_ping_interval;


  bool presume_writable_when_fully_relayed = false;





  bool surface_ice_candidates_on_ice_transport_type_changed = false;


  absl::optional<int> regather_on_failed_networks_interval;



  absl::optional<int> receiving_switching_delay;


  NominationMode default_nomination_mode = NominationMode::SEMI_AGGRESSIVE;




  absl::optional<int> ice_check_interval_strong_connectivity;




  absl::optional<int> ice_check_interval_weak_connectivity;







  absl::optional<int> ice_check_min_interval;




  absl::optional<int> ice_unwritable_timeout;




  absl::optional<int> ice_unwritable_min_checks;




  absl::optional<int> ice_inactive_timeout;


  absl::optional<int> stun_keepalive_interval;

  absl::optional<rtc::AdapterType> network_preference;

  webrtc::VpnPreference vpn_preference = webrtc::VpnPreference::kDefault;

  IceConfig();
  IceConfig(int receiving_timeout_ms,
            int backup_connection_ping_interval,
            ContinualGatheringPolicy gathering_policy,
            bool prioritize_most_likely_candidate_pairs,
            int stable_writable_connection_ping_interval_ms,
            bool presume_writable_when_fully_relayed,
            int regather_on_failed_networks_interval_ms,
            int receiving_switching_delay_ms);
  ~IceConfig();



  int receiving_timeout_or_default() const;
  int backup_connection_ping_interval_or_default() const;
  int stable_writable_connection_ping_interval_or_default() const;
  int regather_on_failed_networks_interval_or_default() const;
  int receiving_switching_delay_or_default() const;
  int ice_check_interval_strong_connectivity_or_default() const;
  int ice_check_interval_weak_connectivity_or_default() const;
  int ice_check_min_interval_or_default() const;
  int ice_unwritable_timeout_or_default() const;
  int ice_unwritable_min_checks_or_default() const;
  int ice_inactive_timeout_or_default() const;
  int stun_keepalive_interval_or_default() const;
};

// PeerConnectionInterface::IceConnectionState.
enum class IceTransportState {
  STATE_INIT,
  STATE_CONNECTING,  // Will enter this state once a connection is created
  STATE_COMPLETED,
  STATE_FAILED
};

// remoting/protocol/libjingle_transport_factory.cc
enum IceProtocolType {
  ICEPROTO_RFC5245  // Standard RFC 5245 version of ICE.
};

// Once the public interface is supported,
// (https://www.w3.org/TR/webrtc/#rtcicetransport)
// the IceTransportInterface will be split from this class.
class RTC_EXPORT IceTransportInternal : public rtc::PacketTransportInternal {
 public:
  IceTransportInternal();
  ~IceTransportInternal() override;


  virtual IceTransportState GetState() const = 0;
  virtual webrtc::IceTransportState GetIceTransportState() const = 0;

  virtual int component() const = 0;

  virtual IceRole GetIceRole() const = 0;

  virtual void SetIceRole(IceRole role) = 0;

  virtual void SetIceTiebreaker(uint64_t tiebreaker) = 0;


  virtual void SetIceProtocolType(IceProtocolType type) {}

  virtual void SetIceCredentials(absl::string_view ice_ufrag,
                                 absl::string_view ice_pwd);

  virtual void SetRemoteIceCredentials(absl::string_view ice_ufrag,
                                       absl::string_view ice_pwd);


  virtual void SetIceParameters(const IceParameters& ice_params) = 0;

  virtual void SetRemoteIceParameters(const IceParameters& ice_params) = 0;

  virtual void SetRemoteIceMode(IceMode mode) = 0;

  virtual void SetIceConfig(const IceConfig& config) = 0;


  virtual void MaybeStartGathering() = 0;

  virtual void AddRemoteCandidate(const Candidate& candidate) = 0;

  virtual void RemoveRemoteCandidate(const Candidate& candidate) = 0;

  virtual void RemoveAllRemoteCandidates() = 0;

  virtual IceGatheringState gathering_state() const = 0;

  virtual bool GetStats(IceTransportStats* ice_transport_stats) = 0;


  virtual absl::optional<int> GetRttEstimate() = 0;

  virtual const Connection* selected_connection() const = 0;


  virtual absl::optional<const CandidatePair> GetSelectedCandidatePair()
      const = 0;

  sigslot::signal1<IceTransportInternal*> SignalGatheringState;

  sigslot::signal2<IceTransportInternal*, const Candidate&>
      SignalCandidateGathered;

  sigslot::signal2<IceTransportInternal*, const IceCandidateErrorEvent&>
      SignalCandidateError;

  sigslot::signal2<IceTransportInternal*, const Candidates&>
      SignalCandidatesRemoved;






  sigslot::signal2<IceTransportInternal*, const Candidate&> SignalRouteChange;

  sigslot::signal1<const cricket::CandidatePairChangeEvent&>
      SignalCandidatePairChanged;


  sigslot::signal1<IceTransportInternal*> SignalRoleConflict;



  sigslot::signal1<IceTransportInternal*> SignalStateChanged;

  sigslot::signal1<IceTransportInternal*> SignalIceTransportStateChanged;

  sigslot::signal1<IceTransportInternal*> SignalDestroyed;
};

}  // namespace cricket

#endif  // P2P_BASE_ICE_TRANSPORT_INTERNAL_H_
