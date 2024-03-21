/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VIDEO_BUFFERED_FRAME_DECRYPTOR_H_
#define VIDEO_BUFFERED_FRAME_DECRYPTOR_H_

#include <deque>
#include <memory>

#include "api/crypto/crypto_options.h"
#include "api/crypto/frame_decryptor_interface.h"
#include "api/field_trials_view.h"
#include "modules/video_coding/frame_object.h"

namespace webrtc {

// BufferedFrameDecryptor and is called each time a frame is sucessfully
// decrypted by the buffer.
class OnDecryptedFrameCallback {
 public:
  virtual ~OnDecryptedFrameCallback() = default;

  virtual void OnDecryptedFrame(std::unique_ptr<RtpFrameObject> frame) = 0;
};

// stream. For example going from a none state to a first decryption or going
// frome a decryptable state to a non decryptable state.
class OnDecryptionStatusChangeCallback {
 public:
  virtual ~OnDecryptionStatusChangeCallback() = default;




  virtual void OnDecryptionStatusChange(
      FrameDecryptorInterface::Status status) = 0;
};

// decrypted received frames onto the OnDecryptedFrameCallback. Frames can be
// delayed when frame encryption is enabled but the key hasn't arrived yet. In
// this case we stash about 1 second of encrypted frames instead of dropping
// them to prevent re-requesting the key frame. This optimization is
// particularly important on low bandwidth networks. Note stashing is only ever
// done if we have never sucessfully decrypted a frame before. After the first
// successful decryption payloads will never be stashed.
class BufferedFrameDecryptor final {
 public:

  explicit BufferedFrameDecryptor(
      OnDecryptedFrameCallback* decrypted_frame_callback,
      OnDecryptionStatusChangeCallback* decryption_status_change_callback,
      const FieldTrialsView& field_trials);

  ~BufferedFrameDecryptor();

  BufferedFrameDecryptor(const BufferedFrameDecryptor&) = delete;
  BufferedFrameDecryptor& operator=(const BufferedFrameDecryptor&) = delete;



  void SetFrameDecryptor(
      rtc::scoped_refptr<FrameDecryptorInterface> frame_decryptor);


  void ManageEncryptedFrame(std::unique_ptr<RtpFrameObject> encrypted_frame);

 private:

  enum class FrameDecision { kStash, kDecrypted, kDrop };



  FrameDecision DecryptFrame(RtpFrameObject* frame);


  void RetryStashedFrames();

  static const size_t kMaxStashedFrames = 24;

  const bool generic_descriptor_auth_experiment_;
  bool first_frame_decrypted_ = false;
  FrameDecryptorInterface::Status last_status_ =
      FrameDecryptorInterface::Status::kUnknown;
  rtc::scoped_refptr<FrameDecryptorInterface> frame_decryptor_;
  OnDecryptedFrameCallback* const decrypted_frame_callback_;
  OnDecryptionStatusChangeCallback* const decryption_status_change_callback_;
  std::deque<std::unique_ptr<RtpFrameObject>> stashed_frames_;
};

}  // namespace webrtc

#endif  // VIDEO_BUFFERED_FRAME_DECRYPTOR_H_
