/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VIDEO_VIDEO_FRAME_BUFFER_H_
#define API_VIDEO_VIDEO_FRAME_BUFFER_H_

#include <stdint.h>

#include "api/array_view.h"
#include "api/scoped_refptr.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

class I420BufferInterface;
class I420ABufferInterface;
class I422BufferInterface;
class I444BufferInterface;
class I010BufferInterface;
class I210BufferInterface;
class NV12BufferInterface;

// The tag in type() indicates how the data is represented, and each type is
// implemented as a subclass. To access the pixel data, call the appropriate
// GetXXX() function, where XXX represents the type. There is also a function
// ToI420() that returns a frame buffer in I420 format, converting from the
// underlying representation if necessary. I420 is the most widely accepted
// format and serves as a fallback for video sinks that can only handle I420,
// e.g. the internal WebRTC software encoders. A special enum value 'kNative' is
// provided for external clients to implement their own frame buffer
// representations, e.g. as textures. The external client can produce such
// native frame buffers from custom video sources, and then cast it back to the
// correct subclass in custom video sinks. The purpose of this is to improve
// performance by providing an optimized path without intermediate conversions.
// Frame metadata such as rotation and timestamp are stored in
// webrtc::VideoFrame, and not here.
class RTC_EXPORT VideoFrameBuffer : public rtc::RefCountInterface {
 public:





  enum class Type {
    kNative,
    kI420,
    kI420A,
    kI422,
    kI444,
    kI010,
    kI210,
    kNV12,
  };

  virtual Type type() const = 0;


  virtual int width() const = 0;
  virtual int height() const = 0;






  virtual rtc::scoped_refptr<I420BufferInterface> ToI420() = 0;








  virtual const I420BufferInterface* GetI420() const;





  virtual rtc::scoped_refptr<VideoFrameBuffer> CropAndScale(int offset_x,
                                                            int offset_y,
                                                            int crop_width,
                                                            int crop_height,
                                                            int scaled_width,
                                                            int scaled_height);

  rtc::scoped_refptr<VideoFrameBuffer> Scale(int scaled_width,
                                             int scaled_height) {
    return CropAndScale(0, 0, width(), height(), scaled_width, scaled_height);
  }


  const I420ABufferInterface* GetI420A() const;
  const I422BufferInterface* GetI422() const;
  const I444BufferInterface* GetI444() const;
  const I010BufferInterface* GetI010() const;
  const I210BufferInterface* GetI210() const;
  const NV12BufferInterface* GetNV12() const;





  virtual rtc::scoped_refptr<VideoFrameBuffer> GetMappedFrameBuffer(
      rtc::ArrayView<Type> types);

 protected:
  ~VideoFrameBuffer() override {}
};

const char* VideoFrameBufferTypeToString(VideoFrameBuffer::Type type);

class PlanarYuvBuffer : public VideoFrameBuffer {
 public:
  virtual int ChromaWidth() const = 0;
  virtual int ChromaHeight() const = 0;


  virtual int StrideY() const = 0;
  virtual int StrideU() const = 0;
  virtual int StrideV() const = 0;

 protected:
  ~PlanarYuvBuffer() override {}
};

// Type::kI420A, Type::kI422 and Type::kI444.
class PlanarYuv8Buffer : public PlanarYuvBuffer {
 public:


  virtual const uint8_t* DataY() const = 0;
  virtual const uint8_t* DataU() const = 0;
  virtual const uint8_t* DataV() const = 0;

 protected:
  ~PlanarYuv8Buffer() override {}
};

class RTC_EXPORT I420BufferInterface : public PlanarYuv8Buffer {
 public:
  Type type() const override;

  int ChromaWidth() const final;
  int ChromaHeight() const final;

  rtc::scoped_refptr<I420BufferInterface> ToI420() final;
  const I420BufferInterface* GetI420() const final;

 protected:
  ~I420BufferInterface() override {}
};

class RTC_EXPORT I420ABufferInterface : public I420BufferInterface {
 public:
  Type type() const final;
  virtual const uint8_t* DataA() const = 0;
  virtual int StrideA() const = 0;

 protected:
  ~I420ABufferInterface() override {}
};

class I422BufferInterface : public PlanarYuv8Buffer {
 public:
  Type type() const final;

  int ChromaWidth() const final;
  int ChromaHeight() const final;

  rtc::scoped_refptr<VideoFrameBuffer> CropAndScale(int offset_x,
                                                    int offset_y,
                                                    int crop_width,
                                                    int crop_height,
                                                    int scaled_width,
                                                    int scaled_height) override;

 protected:
  ~I422BufferInterface() override {}
};

class I444BufferInterface : public PlanarYuv8Buffer {
 public:
  Type type() const final;

  int ChromaWidth() const final;
  int ChromaHeight() const final;

  rtc::scoped_refptr<VideoFrameBuffer> CropAndScale(int offset_x,
                                                    int offset_y,
                                                    int crop_width,
                                                    int crop_height,
                                                    int scaled_width,
                                                    int scaled_height) override;

 protected:
  ~I444BufferInterface() override {}
};

// Type::kI210 .
class PlanarYuv16BBuffer : public PlanarYuvBuffer {
 public:


  virtual const uint16_t* DataY() const = 0;
  virtual const uint16_t* DataU() const = 0;
  virtual const uint16_t* DataV() const = 0;

 protected:
  ~PlanarYuv16BBuffer() override {}
};

// significant bits with color information.
class I010BufferInterface : public PlanarYuv16BBuffer {
 public:
  Type type() const override;

  int ChromaWidth() const final;
  int ChromaHeight() const final;

 protected:
  ~I010BufferInterface() override {}
};

// significant bits with color information.
class I210BufferInterface : public PlanarYuv16BBuffer {
 public:
  Type type() const override;

  int ChromaWidth() const final;
  int ChromaHeight() const final;

 protected:
  ~I210BufferInterface() override {}
};

class BiplanarYuvBuffer : public VideoFrameBuffer {
 public:
  virtual int ChromaWidth() const = 0;
  virtual int ChromaHeight() const = 0;


  virtual int StrideY() const = 0;
  virtual int StrideUV() const = 0;

 protected:
  ~BiplanarYuvBuffer() override {}
};

class BiplanarYuv8Buffer : public BiplanarYuvBuffer {
 public:
  virtual const uint8_t* DataY() const = 0;
  virtual const uint8_t* DataUV() const = 0;

 protected:
  ~BiplanarYuv8Buffer() override {}
};

// interleved UV.
class RTC_EXPORT NV12BufferInterface : public BiplanarYuv8Buffer {
 public:
  Type type() const override;

  int ChromaWidth() const final;
  int ChromaHeight() const final;

  rtc::scoped_refptr<VideoFrameBuffer> CropAndScale(int offset_x,
                                                    int offset_y,
                                                    int crop_width,
                                                    int crop_height,
                                                    int scaled_width,
                                                    int scaled_height) override;

 protected:
  ~NV12BufferInterface() override {}
};

}  // namespace webrtc

#endif  // API_VIDEO_VIDEO_FRAME_BUFFER_H_
