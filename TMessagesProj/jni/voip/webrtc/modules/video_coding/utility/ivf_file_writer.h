/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_VIDEO_CODING_UTILITY_IVF_FILE_WRITER_H_
#define MODULES_VIDEO_CODING_UTILITY_IVF_FILE_WRITER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "api/video/encoded_image.h"
#include "api/video/video_codec_type.h"
#include "rtc_base/system/file_wrapper.h"
#include "rtc_base/time_utils.h"

namespace webrtc {

class IvfFileWriter {
 public:




  static std::unique_ptr<IvfFileWriter> Wrap(FileWrapper file,
                                             size_t byte_limit);
  ~IvfFileWriter();

  IvfFileWriter(const IvfFileWriter&) = delete;
  IvfFileWriter& operator=(const IvfFileWriter&) = delete;

  bool WriteFrame(const EncodedImage& encoded_image, VideoCodecType codec_type);
  bool Close();

 private:
  explicit IvfFileWriter(FileWrapper file, size_t byte_limit);

  bool WriteHeader();
  bool InitFromFirstFrame(const EncodedImage& encoded_image,
                          VideoCodecType codec_type);
  bool WriteOneSpatialLayer(int64_t timestamp,
                            const uint8_t* data,
                            size_t size);

  VideoCodecType codec_type_;
  size_t bytes_written_;
  size_t byte_limit_;
  size_t num_frames_;
  uint16_t width_;
  uint16_t height_;
  int64_t last_timestamp_;
  bool using_capture_timestamps_;
  rtc::TimestampWrapAroundHandler wrap_handler_;
  FileWrapper file_;
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_UTILITY_IVF_FILE_WRITER_H_
