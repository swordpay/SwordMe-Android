/*
 *  Copyright 2019 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_CONNECTION_INFO_H_
#define P2P_BASE_CONNECTION_INFO_H_

#include <vector>

#include "absl/types/optional.h"
#include "api/candidate.h"

namespace cricket {

enum class IceCandidatePairState {
  WAITING = 0,  // Check has not been performed, Waiting pair on CL.
  IN_PROGRESS,  // Check has been sent, transaction is in progress.
  SUCCEEDED,    // Check already done, produced a successful result.
  FAILED,       // Check for this connection failed.


};

// TODO(hta): Rename to ConnectionStats
struct ConnectionInfo {
  ConnectionInfo();
  ConnectionInfo(const ConnectionInfo&);
  ~ConnectionInfo();

  bool best_connection;      // Is this the best connection we have?
  bool writable;             // Has this connection received a STUN response?
  bool receiving;            // Has this connection received anything?
  bool timeout;              // Has this connection timed out?
  size_t rtt;                // The STUN RTT for this connection.
  size_t sent_discarded_bytes;  // Number of outgoing bytes discarded due to

  size_t sent_total_bytes;      // Total bytes sent on this connection. Does not

  size_t sent_bytes_second;  // Bps over the last measurement interval.
  size_t sent_discarded_packets;  // Number of outgoing packets discarded due to

  size_t sent_total_packets;  // Number of total outgoing packets attempted for

  size_t sent_ping_requests_total;  // Number of STUN ping request sent.
  size_t sent_ping_requests_before_first_response;  // Number of STUN ping

  size_t sent_ping_responses;  // Number of STUN ping response sent.

  size_t recv_total_bytes;     // Total bytes received on this connection.
  size_t recv_bytes_second;    // Bps over the last measurement interval.
  size_t packets_received;     // Number of packets that were received.
  size_t recv_ping_requests;   // Number of STUN ping request received.
  size_t recv_ping_responses;  // Number of STUN ping response received.
  Candidate local_candidate;   // The local candidate for this connection.
  Candidate remote_candidate;  // The remote candidate for this connection.
  void* key;                   // A static value that identifies this conn.

  IceCandidatePairState state;

  uint64_t priority;

  bool nominated;

  uint64_t total_round_trip_time_ms;

  absl::optional<uint32_t> current_round_trip_time_ms;
};

typedef std::vector<ConnectionInfo> ConnectionInfos;

}  // namespace cricket

#endif  // P2P_BASE_CONNECTION_INFO_H_
