/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_capture/linux/video_capture_v4l2.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include <new>
#include <string>

#include "api/scoped_refptr.h"
#include "media/base/video_common.h"
#include "modules/video_capture/video_capture.h"
#include "rtc_base/logging.h"

namespace webrtc {
namespace videocapturemodule {
VideoCaptureModuleV4L2::VideoCaptureModuleV4L2()
    : VideoCaptureImpl(),
      _deviceId(-1),
      _deviceFd(-1),
      _buffersAllocatedByDevice(-1),
      _currentWidth(-1),
      _currentHeight(-1),
      _currentFrameRate(-1),
      _captureStarted(false),
      _captureVideoType(VideoType::kI420),
      _pool(NULL) {}

int32_t VideoCaptureModuleV4L2::Init(const char* deviceUniqueIdUTF8) {
  int len = strlen((const char*)deviceUniqueIdUTF8);
  _deviceUniqueId = new (std::nothrow) char[len + 1];
  if (_deviceUniqueId) {
    memcpy(_deviceUniqueId, deviceUniqueIdUTF8, len + 1);
  }

  int fd;
  char device[32];
  bool found = false;

  /* detect /dev/video [0-63] entries */
  int n;
  for (n = 0; n < 64; n++) {
    snprintf(device, sizeof(device), "/dev/video%d", n);
    if ((fd = open(device, O_RDONLY)) != -1) {

      struct v4l2_capability cap;
      if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
        if (cap.bus_info[0] != 0) {
          if (strncmp((const char*)cap.bus_info,
                      (const char*)deviceUniqueIdUTF8,
                      strlen((const char*)deviceUniqueIdUTF8)) ==
              0) {  // match with device id
            close(fd);
            found = true;
            break;  // fd matches with device unique id supplied
          }
        }
      }
      close(fd);  // close since this is not the matching device
    }
  }
  if (!found) {
    RTC_LOG(LS_INFO) << "no matching device found";
    return -1;
  }
  _deviceId = n;  // store the device id
  return 0;
}

VideoCaptureModuleV4L2::~VideoCaptureModuleV4L2() {
  StopCapture();
  if (_deviceFd != -1)
    close(_deviceFd);
}

int32_t VideoCaptureModuleV4L2::StartCapture(
    const VideoCaptureCapability& capability) {
  if (_captureStarted) {
    if (capability.width == _currentWidth &&
        capability.height == _currentHeight &&
        _captureVideoType == capability.videoType) {
      return 0;
    } else {
      StopCapture();
    }
  }

  MutexLock lock(&capture_lock_);

  char device[20];
  snprintf(device, sizeof(device), "/dev/video%d", _deviceId);

  if ((_deviceFd = open(device, O_RDWR | O_NONBLOCK, 0)) < 0) {
    RTC_LOG(LS_INFO) << "error in opening " << device << " errono = " << errno;
    return -1;
  }



  const int nFormats = 5;
  unsigned int fmts[nFormats];
  if (capability.width > 640 || capability.height > 480) {
    fmts[0] = V4L2_PIX_FMT_MJPEG;
    fmts[1] = V4L2_PIX_FMT_YUV420;
    fmts[2] = V4L2_PIX_FMT_YUYV;
    fmts[3] = V4L2_PIX_FMT_UYVY;
    fmts[4] = V4L2_PIX_FMT_JPEG;
  } else {
    fmts[0] = V4L2_PIX_FMT_YUV420;
    fmts[1] = V4L2_PIX_FMT_YUYV;
    fmts[2] = V4L2_PIX_FMT_UYVY;
    fmts[3] = V4L2_PIX_FMT_MJPEG;
    fmts[4] = V4L2_PIX_FMT_JPEG;
  }

  struct v4l2_fmtdesc fmt;
  int fmtsIdx = nFormats;
  memset(&fmt, 0, sizeof(fmt));
  fmt.index = 0;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  RTC_LOG(LS_INFO) << "Video Capture enumerats supported image formats:";
  while (ioctl(_deviceFd, VIDIOC_ENUM_FMT, &fmt) == 0) {
    RTC_LOG(LS_INFO) << "  { pixelformat = "
                     << cricket::GetFourccName(fmt.pixelformat)
                     << ", description = '" << fmt.description << "' }";

    for (int i = 0; i < nFormats; i++) {
      if (fmt.pixelformat == fmts[i] && i < fmtsIdx)
        fmtsIdx = i;
    }

    fmt.index++;
  }

  if (fmtsIdx == nFormats) {
    RTC_LOG(LS_INFO) << "no supporting video formats found";
    return -1;
  } else {
    RTC_LOG(LS_INFO) << "We prefer format "
                     << cricket::GetFourccName(fmts[fmtsIdx]);
  }

  struct v4l2_format video_fmt;
  memset(&video_fmt, 0, sizeof(struct v4l2_format));
  video_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  video_fmt.fmt.pix.sizeimage = 0;
  video_fmt.fmt.pix.width = capability.width;
  video_fmt.fmt.pix.height = capability.height;
  video_fmt.fmt.pix.pixelformat = fmts[fmtsIdx];

  if (video_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)
    _captureVideoType = VideoType::kYUY2;
  else if (video_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420)
    _captureVideoType = VideoType::kI420;
  else if (video_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_UYVY)
    _captureVideoType = VideoType::kUYVY;
  else if (video_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG ||
           video_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG)
    _captureVideoType = VideoType::kMJPEG;

  if (ioctl(_deviceFd, VIDIOC_S_FMT, &video_fmt) < 0) {
    RTC_LOG(LS_INFO) << "error in VIDIOC_S_FMT, errno = " << errno;
    return -1;
  }

  _currentWidth = video_fmt.fmt.pix.width;
  _currentHeight = video_fmt.fmt.pix.height;

  bool driver_framerate_support = true;
  struct v4l2_streamparm streamparms;
  memset(&streamparms, 0, sizeof(streamparms));
  streamparms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(_deviceFd, VIDIOC_G_PARM, &streamparms) < 0) {
    RTC_LOG(LS_INFO) << "error in VIDIOC_G_PARM errno = " << errno;
    driver_framerate_support = false;

  } else {

    if (streamparms.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {

      memset(&streamparms, 0, sizeof(streamparms));
      streamparms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      streamparms.parm.capture.timeperframe.numerator = 1;
      streamparms.parm.capture.timeperframe.denominator = capability.maxFPS;
      if (ioctl(_deviceFd, VIDIOC_S_PARM, &streamparms) < 0) {
        RTC_LOG(LS_INFO) << "Failed to set the framerate. errno=" << errno;
        driver_framerate_support = false;
      } else {
        _currentFrameRate = capability.maxFPS;
      }
    }
  }


  if (!driver_framerate_support) {
    if (_currentWidth >= 800 && _captureVideoType != VideoType::kMJPEG) {
      _currentFrameRate = 15;
    } else {
      _currentFrameRate = 30;
    }
  }

  if (!AllocateVideoBuffers()) {
    RTC_LOG(LS_INFO) << "failed to allocate video capture buffers";
    return -1;
  }

  if (_captureThread.empty()) {
    quit_ = false;
    _captureThread = rtc::PlatformThread::SpawnJoinable(
        [this] {
          while (CaptureProcess()) {
          }
        },
        "CaptureThread",
        rtc::ThreadAttributes().SetPriority(rtc::ThreadPriority::kHigh));
  }

  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(_deviceFd, VIDIOC_STREAMON, &type) == -1) {
    RTC_LOG(LS_INFO) << "Failed to turn on stream";
    return -1;
  }

  _captureStarted = true;
  return 0;
}

int32_t VideoCaptureModuleV4L2::StopCapture() {
  if (!_captureThread.empty()) {
    {
      MutexLock lock(&capture_lock_);
      quit_ = true;
    }

    _captureThread.Finalize();
  }

  MutexLock lock(&capture_lock_);
  if (_captureStarted) {
    _captureStarted = false;

    DeAllocateVideoBuffers();
    close(_deviceFd);
    _deviceFd = -1;
  }

  return 0;
}


bool VideoCaptureModuleV4L2::AllocateVideoBuffers() {
  struct v4l2_requestbuffers rbuffer;
  memset(&rbuffer, 0, sizeof(v4l2_requestbuffers));

  rbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  rbuffer.memory = V4L2_MEMORY_MMAP;
  rbuffer.count = kNoOfV4L2Bufffers;

  if (ioctl(_deviceFd, VIDIOC_REQBUFS, &rbuffer) < 0) {
    RTC_LOG(LS_INFO) << "Could not get buffers from device. errno = " << errno;
    return false;
  }

  if (rbuffer.count > kNoOfV4L2Bufffers)
    rbuffer.count = kNoOfV4L2Bufffers;

  _buffersAllocatedByDevice = rbuffer.count;

  _pool = new Buffer[rbuffer.count];

  for (unsigned int i = 0; i < rbuffer.count; i++) {
    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(v4l2_buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    if (ioctl(_deviceFd, VIDIOC_QUERYBUF, &buffer) < 0) {
      return false;
    }

    _pool[i].start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE,
                          MAP_SHARED, _deviceFd, buffer.m.offset);

    if (MAP_FAILED == _pool[i].start) {
      for (unsigned int j = 0; j < i; j++)
        munmap(_pool[j].start, _pool[j].length);
      return false;
    }

    _pool[i].length = buffer.length;

    if (ioctl(_deviceFd, VIDIOC_QBUF, &buffer) < 0) {
      return false;
    }
  }
  return true;
}

bool VideoCaptureModuleV4L2::DeAllocateVideoBuffers() {

  for (int i = 0; i < _buffersAllocatedByDevice; i++)
    munmap(_pool[i].start, _pool[i].length);

  delete[] _pool;

  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(_deviceFd, VIDIOC_STREAMOFF, &type) < 0) {
    RTC_LOG(LS_INFO) << "VIDIOC_STREAMOFF error. errno: " << errno;
  }

  return true;
}

bool VideoCaptureModuleV4L2::CaptureStarted() {
  return _captureStarted;
}

bool VideoCaptureModuleV4L2::CaptureProcess() {
  int retVal = 0;
  fd_set rSet;
  struct timeval timeout;

  FD_ZERO(&rSet);
  FD_SET(_deviceFd, &rSet);
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  retVal = select(_deviceFd + 1, &rSet, NULL, NULL, &timeout);

  {
    MutexLock lock(&capture_lock_);

    if (quit_) {
      return false;
    }

    if (retVal < 0 && errno != EINTR) {  // continue if interrupted

      return false;
    } else if (retVal == 0) {

      return true;
    } else if (!FD_ISSET(_deviceFd, &rSet)) {

      return true;
    }

    if (_captureStarted) {
      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(struct v4l2_buffer));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;

      while (ioctl(_deviceFd, VIDIOC_DQBUF, &buf) < 0) {
        if (errno != EINTR) {
          RTC_LOG(LS_INFO) << "could not sync on a buffer on device "
                           << strerror(errno);
          return true;
        }
      }
      VideoCaptureCapability frameInfo;
      frameInfo.width = _currentWidth;
      frameInfo.height = _currentHeight;
      frameInfo.videoType = _captureVideoType;

      IncomingFrame(reinterpret_cast<uint8_t*>(_pool[buf.index].start),
                    buf.bytesused, frameInfo);

      if (ioctl(_deviceFd, VIDIOC_QBUF, &buf) == -1) {
        RTC_LOG(LS_INFO) << "Failed to enqueue capture buffer";
      }
    }
  }
  usleep(0);
  return true;
}

int32_t VideoCaptureModuleV4L2::CaptureSettings(
    VideoCaptureCapability& settings) {
  settings.width = _currentWidth;
  settings.height = _currentHeight;
  settings.maxFPS = _currentFrameRate;
  settings.videoType = _captureVideoType;

  return 0;
}
}  // namespace videocapturemodule
}  // namespace webrtc
