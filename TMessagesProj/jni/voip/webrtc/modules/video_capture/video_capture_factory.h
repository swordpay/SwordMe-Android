/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// and DeviceInfo.

#ifndef MODULES_VIDEO_CAPTURE_VIDEO_CAPTURE_FACTORY_H_
#define MODULES_VIDEO_CAPTURE_VIDEO_CAPTURE_FACTORY_H_

#include "api/scoped_refptr.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_defines.h"

namespace webrtc {

class VideoCaptureFactory {
 public:




  static rtc::scoped_refptr<VideoCaptureModule> Create(
      const char* deviceUniqueIdUTF8);

  static VideoCaptureModule::DeviceInfo* CreateDeviceInfo();

 private:
  ~VideoCaptureFactory();
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CAPTURE_VIDEO_CAPTURE_FACTORY_H_
