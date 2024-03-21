/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef NET_DCSCTP_PACKET_DATA_H_
#define NET_DCSCTP_PACKET_DATA_H_

#include <cstdint>
#include <utility>
#include <vector>

#include "net/dcsctp/common/internal_types.h"
#include "net/dcsctp/public/types.h"

namespace dcsctp {

// chunk, or data that is supposed to be sent, and wrapped in a DATA/I-DATA
// chunk (depending on peer capabilities).
//
// The data wrapped in this structure is actually the same as the DATA/I-DATA
// chunk (actually the union of them), but to avoid having all components be
// aware of the implementation details of the different chunks, this abstraction
// is used instead. A notable difference is also that it doesn't carry a
// Transmission Sequence Number (TSN), as that is not known when a chunk is
// created (assigned late, just when sending), and that the TSNs in DATA/I-DATA
// are wrapped numbers, and within the library, unwrapped sequence numbers are
// preferably used.
struct Data {


  using IsBeginning = webrtc::StrongAlias<class IsBeginningTag, bool>;


  using IsEnd = webrtc::StrongAlias<class IsEndTag, bool>;

  Data(StreamID stream_id,
       SSN ssn,
       MID message_id,
       FSN fsn,
       PPID ppid,
       std::vector<uint8_t> payload,
       IsBeginning is_beginning,
       IsEnd is_end,
       IsUnordered is_unordered)
      : stream_id(stream_id),
        ssn(ssn),
        message_id(message_id),
        fsn(fsn),
        ppid(ppid),
        payload(std::move(payload)),
        is_beginning(is_beginning),
        is_end(is_end),
        is_unordered(is_unordered) {}

  Data(Data&& other) = default;
  Data& operator=(Data&& other) = default;

  Data Clone() const {
    return Data(stream_id, ssn, message_id, fsn, ppid, payload, is_beginning,
                is_end, is_unordered);
  }

  size_t size() const { return payload.size(); }

  StreamID stream_id;


  SSN ssn;



  MID message_id;

  FSN fsn;

  PPID ppid;

  std::vector<uint8_t> payload;

  IsBeginning is_beginning;
  IsEnd is_end;

  IsUnordered is_unordered;
};
}  // namespace dcsctp

#endif  // NET_DCSCTP_PACKET_DATA_H_
