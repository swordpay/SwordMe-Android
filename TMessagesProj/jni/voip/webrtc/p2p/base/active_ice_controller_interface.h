/*
 *  Copyright 2022 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_ACTIVE_ICE_CONTROLLER_INTERFACE_H_
#define P2P_BASE_ACTIVE_ICE_CONTROLLER_INTERFACE_H_

#include "absl/types/optional.h"
#include "api/array_view.h"
#include "p2p/base/connection.h"
#include "p2p/base/ice_switch_reason.h"
#include "p2p/base/ice_transport_internal.h"
#include "p2p/base/transport_description.h"

namespace cricket {

// manages the connection used by an ICE transport.
//
// An active ICE controller receives updates from the ICE transport when
//   - the connections state is mutated
//   - a new connection should be selected as a result of an external event (eg.
//     a different connection nominated by the remote peer)
//
// The active ICE controller takes the appropriate decisions and requests the
// ICE agent to perform the necessary actions through the IceAgentInterface.
class ActiveIceControllerInterface {
 public:
  virtual ~ActiveIceControllerInterface() = default;

  virtual void SetIceConfig(const IceConfig& config) = 0;

  virtual void OnConnectionAdded(const Connection* connection) = 0;

  virtual void OnConnectionSwitched(const Connection* connection) = 0;

  virtual void OnConnectionDestroyed(const Connection* connection) = 0;


  virtual void OnConnectionPinged(const Connection* connection) = 0;






  virtual void OnConnectionUpdated(const Connection* connection) = 0;

  virtual bool GetUseCandidateAttribute(const Connection* connection,
                                        NominationMode mode,
                                        IceMode remote_ice_mode) const = 0;


  virtual void OnSortAndSwitchRequest(IceSwitchReason reason) = 0;

  virtual void OnImmediateSortAndSwitchRequest(IceSwitchReason reason) = 0;


  virtual bool OnImmediateSwitchRequest(IceSwitchReason reason,
                                        const Connection* selected) = 0;

  virtual const Connection* FindNextPingableConnection() = 0;
};

}  // namespace cricket

#endif  // P2P_BASE_ACTIVE_ICE_CONTROLLER_INTERFACE_H_
