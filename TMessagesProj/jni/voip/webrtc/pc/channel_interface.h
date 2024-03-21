/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_CHANNEL_INTERFACE_H_
#define PC_CHANNEL_INTERFACE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "api/jsep.h"
#include "api/media_types.h"
#include "media/base/media_channel.h"
#include "pc/rtp_transport_internal.h"

namespace webrtc {
class Call;
class VideoBitrateAllocatorFactory;
}  // namespace webrtc

namespace cricket {

class MediaContentDescription;
struct MediaConfig;

// (audio or video), both outgoing and incoming.
// When the PeerConnection API is used, a Channel corresponds one to one
// to an RtpTransceiver.
// When Unified Plan is used, there can only be at most one outgoing and
// one incoming stream. With Plan B, there can be more than one.

// As more methods are added to BaseChannel, they should be included in the
// interface as well.
// TODO(bugs.webrtc.org/13931): Merge this class into RtpTransceiver.
class ChannelInterface {
 public:
  virtual ~ChannelInterface() = default;
  virtual cricket::MediaType media_type() const = 0;

  virtual MediaChannel* media_channel() const = 0;


  virtual VideoMediaChannel* video_media_channel() const = 0;
  virtual VoiceMediaChannel* voice_media_channel() const = 0;




  virtual absl::string_view transport_name() const = 0;

  virtual const std::string& mid() const = 0;

  virtual void Enable(bool enable) = 0;

  virtual void SetFirstPacketReceivedCallback(
      std::function<void()> callback) = 0;

  virtual bool SetLocalContent(const MediaContentDescription* content,
                               webrtc::SdpType type,
                               std::string& error_desc) = 0;
  virtual bool SetRemoteContent(const MediaContentDescription* content,
                                webrtc::SdpType type,
                                std::string& error_desc) = 0;
  virtual bool SetPayloadTypeDemuxingEnabled(bool enabled) = 0;

  virtual const std::vector<StreamParams>& local_streams() const = 0;
  virtual const std::vector<StreamParams>& remote_streams() const = 0;





  virtual bool SetRtpTransport(webrtc::RtpTransportInternal* rtp_transport) = 0;
};

}  // namespace cricket

#endif  // PC_CHANNEL_INTERFACE_H_
