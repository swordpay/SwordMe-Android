/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 */

#ifndef MODULES_VIDEO_CODING_CODECS_VP9_VP9_FRAME_BUFFER_POOL_H_
#define MODULES_VIDEO_CODING_CODECS_VP9_VP9_FRAME_BUFFER_POOL_H_

#ifdef RTC_ENABLE_VP9

#include <vector>

#include "api/ref_counted_base.h"
#include "api/scoped_refptr.h"
#include "rtc_base/buffer.h"
#include "rtc_base/synchronization/mutex.h"

struct vpx_codec_ctx;
struct vpx_codec_frame_buffer;

namespace webrtc {

// debug mode. VP9 is defined to have 8 reference buffers, of which 3 can be
// referenced by any frame, see
// https://tools.ietf.org/html/draft-grange-vp9-bitstream-00#section-2.2.2.
// Assuming VP9 holds on to at most 8 buffers, any more buffers than that
// would have to be by application code. Decoded frames should not be
// referenced for longer than necessary. If we allow ~60 additional buffers
// then the application has ~1 second to e.g. render each frame of a 60 fps
// video.
constexpr size_t kDefaultMaxNumBuffers = 68;

// VP9, which is set up in InitializeVPXUsePool. After the initialization any
// time libvpx wants to decode a frame it will use buffers provided and released
// through VpxGetFrameBuffer and VpxReleaseFrameBuffer.
// The benefit of owning the pool that libvpx relies on for decoding is that the
// decoded frames returned by libvpx (from vpx_codec_get_frame) use parts of our
// buffers for the decoded image data. By retaining ownership of this buffer
// using scoped_refptr, the image buffer can be reused by VideoFrames and no
// frame copy has to occur during decoding and frame delivery.
//
// Pseudo example usage case:
//    Vp9FrameBufferPool pool;
//    pool.InitializeVpxUsePool(decoder_ctx);
//    ...
//
//    // During decoding, libvpx will get and release buffers from the pool.
//    vpx_codec_decode(decoder_ctx, ...);
//
//    vpx_image_t* img = vpx_codec_get_frame(decoder_ctx, &iter);
//    // Important to use scoped_refptr to protect it against being recycled by
//    // the pool.
//    scoped_refptr<Vp9FrameBuffer> img_buffer = (Vp9FrameBuffer*)img->fb_priv;
//    ...
//
//    // Destroying the codec will make libvpx release any buffers it was using.
//    vpx_codec_destroy(decoder_ctx);
class Vp9FrameBufferPool {
 public:
  class Vp9FrameBuffer final
      : public rtc::RefCountedNonVirtual<Vp9FrameBuffer> {
   public:
    uint8_t* GetData();
    size_t GetDataSize() const;
    void SetSize(size_t size);

    using rtc::RefCountedNonVirtual<Vp9FrameBuffer>::HasOneRef;

   private:

    rtc::Buffer data_;
  };


  bool InitializeVpxUsePool(vpx_codec_ctx* vpx_codec_context);



  rtc::scoped_refptr<Vp9FrameBuffer> GetFrameBuffer(size_t min_size);

  int GetNumBuffersInUse() const;



  bool Resize(size_t max_number_of_buffers);


  void ClearPool();








  static int32_t VpxGetFrameBuffer(void* user_priv,
                                   size_t min_size,
                                   vpx_codec_frame_buffer* fb);






  static int32_t VpxReleaseFrameBuffer(void* user_priv,
                                       vpx_codec_frame_buffer* fb);

 private:

  mutable Mutex buffers_lock_;

  std::vector<rtc::scoped_refptr<Vp9FrameBuffer>> allocated_buffers_
      RTC_GUARDED_BY(buffers_lock_);
  size_t max_num_buffers_ = kDefaultMaxNumBuffers;
};

}  // namespace webrtc

#endif  // RTC_ENABLE_VP9

#endif  // MODULES_VIDEO_CODING_CODECS_VP9_VP9_FRAME_BUFFER_POOL_H_
