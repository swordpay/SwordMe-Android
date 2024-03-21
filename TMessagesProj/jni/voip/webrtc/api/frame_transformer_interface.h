/*
 *  Copyright 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_FRAME_TRANSFORMER_INTERFACE_H_
#define API_FRAME_TRANSFORMER_INTERFACE_H_

#include <memory>
#include <vector>

#include "api/scoped_refptr.h"
#include "api/video/encoded_frame.h"
#include "api/video/video_frame_metadata.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

class TransformableFrameInterface {
 public:
  virtual ~TransformableFrameInterface() = default;


  virtual rtc::ArrayView<const uint8_t> GetData() const = 0;

  virtual void SetData(rtc::ArrayView<const uint8_t> data) = 0;

  virtual uint8_t GetPayloadType() const = 0;
  virtual uint32_t GetSsrc() const = 0;
  virtual uint32_t GetTimestamp() const = 0;

  enum class Direction {
    kUnknown,
    kReceiver,
    kSender,
  };



  virtual Direction GetDirection() const { return Direction::kUnknown; }
};

class TransformableVideoFrameInterface : public TransformableFrameInterface {
 public:
  virtual ~TransformableVideoFrameInterface() = default;
  virtual bool IsKeyFrame() const = 0;






  virtual std::vector<uint8_t> GetAdditionalData() const = 0;

  virtual const VideoFrameMetadata& GetMetadata() const = 0;
};

class TransformableAudioFrameInterface : public TransformableFrameInterface {
 public:
  virtual ~TransformableAudioFrameInterface() = default;



  virtual const RTPHeader& GetHeader() const = 0;

  virtual rtc::ArrayView<const uint32_t> GetContributingSources() const = 0;
};

class TransformedFrameCallback : public rtc::RefCountInterface {
 public:
  virtual void OnTransformedFrame(
      std::unique_ptr<TransformableFrameInterface> frame) = 0;

 protected:
  ~TransformedFrameCallback() override = default;
};

// the TransformedFrameCallback interface (see above).
class FrameTransformerInterface : public rtc::RefCountInterface {
 public:

  virtual void Transform(
      std::unique_ptr<TransformableFrameInterface> transformable_frame) = 0;

  virtual void RegisterTransformedFrameCallback(
      rtc::scoped_refptr<TransformedFrameCallback>) {}
  virtual void RegisterTransformedFrameSinkCallback(
      rtc::scoped_refptr<TransformedFrameCallback>,
      uint32_t ssrc) {}
  virtual void UnregisterTransformedFrameCallback() {}
  virtual void UnregisterTransformedFrameSinkCallback(uint32_t ssrc) {}

 protected:
  ~FrameTransformerInterface() override = default;
};

}  // namespace webrtc

#endif  // API_FRAME_TRANSFORMER_INTERFACE_H_
