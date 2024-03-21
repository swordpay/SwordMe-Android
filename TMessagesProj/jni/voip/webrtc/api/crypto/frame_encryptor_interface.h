/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_CRYPTO_FRAME_ENCRYPTOR_INTERFACE_H_
#define API_CRYPTO_FRAME_ENCRYPTOR_INTERFACE_H_

#include "api/array_view.h"
#include "api/media_types.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

// implementation to encrypt all outgoing audio and video frames. The user must
// also provide a FrameDecryptorInterface to be able to decrypt the frames on
// the receiving device. Note this is an additional layer of encryption in
// addition to the standard SRTP mechanism and is not intended to be used
// without it. Implementations of this interface will have the same lifetime as
// the RTPSenders it is attached to. Additional data may be null.
class FrameEncryptorInterface : public rtc::RefCountInterface {
 public:
  ~FrameEncryptorInterface() override {}







  virtual int Encrypt(cricket::MediaType media_type,
                      uint32_t ssrc,
                      rtc::ArrayView<const uint8_t> additional_data,
                      rtc::ArrayView<const uint8_t> frame,
                      rtc::ArrayView<uint8_t> encrypted_frame,
                      size_t* bytes_written) = 0;



  virtual size_t GetMaxCiphertextByteSize(cricket::MediaType media_type,
                                          size_t frame_size) = 0;
};

}  // namespace webrtc

#endif  // API_CRYPTO_FRAME_ENCRYPTOR_INTERFACE_H_
