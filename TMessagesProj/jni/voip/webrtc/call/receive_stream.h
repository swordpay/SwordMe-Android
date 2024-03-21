/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef CALL_RECEIVE_STREAM_H_
#define CALL_RECEIVE_STREAM_H_

#include <vector>

#include "api/crypto/frame_decryptor_interface.h"
#include "api/frame_transformer_interface.h"
#include "api/media_types.h"
#include "api/scoped_refptr.h"
#include "api/transport/rtp/rtp_source.h"
#include "modules/rtp_rtcp/include/rtp_header_extension_map.h"

namespace webrtc {

// FlexfecReceiveStream.
class ReceiveStreamInterface {
 public:



  struct ReceiveStreamRtpConfig {



    uint32_t remote_ssrc = 0;



    uint32_t local_ssrc = 0;






    bool transport_cc = false;



    std::vector<RtpExtension> extensions;
  };


  virtual void SetRtpExtensions(std::vector<RtpExtension> extensions) = 0;
  virtual RtpHeaderExtensionMap GetRtpExtensionMap() const = 0;






  virtual bool transport_cc() const = 0;

  virtual void SetTransportCc(bool transport_cc) = 0;

 protected:
  virtual ~ReceiveStreamInterface() {}
};

class MediaReceiveStreamInterface : public ReceiveStreamInterface {
 public:


  virtual void Start() = 0;




  virtual void Stop() = 0;

  virtual void SetDepacketizerToDecoderFrameTransformer(
      rtc::scoped_refptr<webrtc::FrameTransformerInterface>
          frame_transformer) = 0;

  virtual void SetFrameDecryptor(
      rtc::scoped_refptr<webrtc::FrameDecryptorInterface> frame_decryptor) = 0;

  virtual std::vector<RtpSource> GetSources() const = 0;
};

}  // namespace webrtc

#endif  // CALL_RECEIVE_STREAM_H_
