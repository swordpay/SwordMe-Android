/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_SDP_SERIALIZER_H_
#define PC_SDP_SERIALIZER_H_

#include <string>

#include "absl/strings/string_view.h"
#include "api/rtc_error.h"
#include "media/base/rid_description.h"
#include "pc/session_description.h"
#include "pc/simulcast_description.h"

namespace webrtc {

// Example:
//     SimulcastDescription can be serialized and deserialized by this class.
//     The serializer will know how to translate the data to spec-compliant
//     format without knowing about the SDP attribute details (a=simulcast:)
// Usage:
//     Consider the SDP attribute for simulcast a=simulcast:<configuration>.
//     The SDP serializtion code (webrtcsdp.h) should use `SdpSerializer` to
//     serialize and deserialize the <configuration> section.
// This class will allow testing the serialization of components without
// having to serialize the entire SDP while hiding implementation details
// from callers of sdp serialization (webrtcsdp.h).
class SdpSerializer {
 public:


  std::string SerializeSimulcastDescription(
      const cricket::SimulcastDescription& simulcast) const;


  RTCErrorOr<cricket::SimulcastDescription> DeserializeSimulcastDescription(
      absl::string_view string) const;


  std::string SerializeRidDescription(
      const cricket::RidDescription& rid_description) const;


  RTCErrorOr<cricket::RidDescription> DeserializeRidDescription(
      absl::string_view string) const;
};

}  // namespace webrtc

#endif  // PC_SDP_SERIALIZER_H_
