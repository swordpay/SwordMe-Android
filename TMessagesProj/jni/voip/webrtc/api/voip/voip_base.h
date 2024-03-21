/*
 *  Copyright (c) 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VOIP_VOIP_BASE_H_
#define API_VOIP_VOIP_BASE_H_

#include "absl/base/attributes.h"
#include "absl/types/optional.h"

namespace webrtc {

class Transport;

//
// VoipBase provides a management interface on a media session using a
// concept called 'channel'.  A channel represents an interface handle
// for application to request various media session operations.  This
// notion of channel is used throughout other interfaces as well.
//
// Underneath the interface, a channel id is mapped into an audio session
// object that is capable of sending and receiving a single RTP stream with
// another media endpoint.  It's possible to create and use multiple active
// channels simultaneously which would mean that particular application
// session has RTP streams with multiple remote endpoints.
//
// A typical example for the usage context is outlined in VoipEngine
// header file.

enum class ChannelId : int {};

enum class ABSL_MUST_USE_RESULT VoipResult {

  kOk,


  kInvalidArgument,


  kFailedPrecondition,



  kInternal,
};

class VoipBase {
 public:











  virtual ChannelId CreateChannel(Transport* transport,
                                  absl::optional<uint32_t> local_ssrc) = 0;





  virtual VoipResult ReleaseChannel(ChannelId channel_id) = 0;






  virtual VoipResult StartSend(ChannelId channel_id) = 0;






  virtual VoipResult StopSend(ChannelId channel_id) = 0;







  virtual VoipResult StartPlayout(ChannelId channel_id) = 0;




  virtual VoipResult StopPlayout(ChannelId channel_id) = 0;

 protected:
  virtual ~VoipBase() = default;
};

}  // namespace webrtc

#endif  // API_VOIP_VOIP_BASE_H_
