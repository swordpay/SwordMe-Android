/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_ICE_CONTROLLER_INTERFACE_H_
#define P2P_BASE_ICE_CONTROLLER_INTERFACE_H_

#include <string>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "p2p/base/connection.h"
#include "p2p/base/ice_switch_reason.h"
#include "p2p/base/ice_transport_internal.h"

namespace cricket {

struct IceFieldTrials;  // Forward declaration to avoid circular dependency.

struct IceRecheckEvent {
  IceRecheckEvent(IceSwitchReason _reason, int _recheck_delay_ms)
      : reason(_reason), recheck_delay_ms(_recheck_delay_ms) {}

  std::string ToString() const;

  IceSwitchReason reason;
  int recheck_delay_ms;
};

// - which connection to ping
// - which connection to use
// - which connection to prune
// - which connection to forget learned state on
//
// The P2PTransportChannel owns (creates and destroys) Connections,
// but P2PTransportChannel gives const pointers to the the IceController using
// `AddConnection`, i.e the IceController should not call any non-const methods
// on a Connection but signal back in the interface if any mutable function
// shall be called.
//
// Current these are limited to:
// Connection::Ping               - returned in PingResult
// Connection::Prune              - retuned in PruneConnections
// Connection::ForgetLearnedState - return in SwitchResult
//
// The IceController shall keep track of all connections added
// (and not destroyed) and give them back using the connections()-function-
//
// When a Connection gets destroyed
// - signals on Connection::SignalDestroyed
// - P2PTransportChannel calls IceController::OnConnectionDestroyed
class IceControllerInterface {
 public:

  struct SwitchResult {

    absl::optional<const Connection*> connection;

    absl::optional<IceRecheckEvent> recheck_event;

    std::vector<const Connection*> connections_to_forget_state_on;
  };

  struct PingResult {
    PingResult(const Connection* conn, int _recheck_delay_ms)
        : connection(conn ? absl::optional<const Connection*>(conn)
                          : absl::nullopt),
          recheck_delay_ms(_recheck_delay_ms) {}

    const absl::optional<const Connection*> connection;







    const int recheck_delay_ms = 0;
  };

  virtual ~IceControllerInterface() = default;

  virtual void SetIceConfig(const IceConfig& config) = 0;
  virtual void SetSelectedConnection(const Connection* selected_connection) = 0;
  virtual void AddConnection(const Connection* connection) = 0;
  virtual void OnConnectionDestroyed(const Connection* connection) = 0;

  virtual rtc::ArrayView<const Connection*> connections() const = 0;



  virtual bool HasPingableConnection() const = 0;

  virtual PingResult SelectConnectionToPing(int64_t last_ping_sent_ms) = 0;

  virtual bool GetUseCandidateAttr(const Connection* conn,
                                   NominationMode mode,
                                   IceMode remote_ice_mode) const = 0;


  virtual const Connection* FindNextPingableConnection() = 0;
  virtual void MarkConnectionPinged(const Connection* con) = 0;



  virtual SwitchResult ShouldSwitchConnection(IceSwitchReason reason,
                                              const Connection* connection) = 0;

  virtual SwitchResult SortAndSwitchConnection(IceSwitchReason reason) = 0;

  virtual std::vector<const Connection*> PruneConnections() = 0;
};

}  // namespace cricket

#endif  // P2P_BASE_ICE_CONTROLLER_INTERFACE_H_
