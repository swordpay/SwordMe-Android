/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef NET_DCSCTP_PUBLIC_TIMEOUT_H_
#define NET_DCSCTP_PUBLIC_TIMEOUT_H_

#include <cstdint>

#include "net/dcsctp/public/types.h"

namespace dcsctp {

// it will be given a unique `timeout_id` which should be provided to
// `DcSctpSocket::HandleTimeout` when it expires.
class Timeout {
 public:
  virtual ~Timeout() = default;






  virtual void Start(DurationMs duration, TimeoutID timeout_id) = 0;






  virtual void Stop() = 0;



  virtual void Restart(DurationMs duration, TimeoutID timeout_id) {
    Stop();
    Start(duration, timeout_id);
  }
};

}  // namespace dcsctp

#endif  // NET_DCSCTP_PUBLIC_TIMEOUT_H_
