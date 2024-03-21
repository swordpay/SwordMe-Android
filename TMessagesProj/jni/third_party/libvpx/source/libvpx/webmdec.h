/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef VPX_WEBMDEC_H_
#define VPX_WEBMDEC_H_

#include "./tools_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct VpxInputContext;

struct WebmInputContext {
  void *reader;
  void *segment;
  uint8_t *buffer;
  const void *cluster;
  const void *block_entry;
  const void *block;
  int block_frame_index;
  int video_track_index;
  uint64_t timestamp_ns;
  int is_key_frame;
  int reached_eos;
};

// that webm_read_frame can be called to retrieve a video frame.
// Returns 1 on success and 0 on failure or input is not WebM file.
// TODO(vigneshv): Refactor this function into two smaller functions specific
// to their task.
int file_is_webm(struct WebmInputContext *webm_ctx,
                 struct VpxInputContext *vpx_ctx);

// by this function. For the first call, |buffer| should be NULL and
// |*buffer_size| should be 0. Once all the frames are read and used,
// webm_free() should be called, otherwise there will be a leak.
// Parameters:
//      webm_ctx - WebmInputContext object
//      buffer - pointer where the frame data will be filled.
//      buffer_size - pointer to buffer size.
// Return values:
//      0 - Success
//      1 - End of Stream
//     -1 - Error
int webm_read_frame(struct WebmInputContext *webm_ctx, uint8_t **buffer,
                    size_t *buffer_size);

int webm_guess_framerate(struct WebmInputContext *webm_ctx,
                         struct VpxInputContext *vpx_ctx);

void webm_free(struct WebmInputContext *webm_ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_WEBMDEC_H_
