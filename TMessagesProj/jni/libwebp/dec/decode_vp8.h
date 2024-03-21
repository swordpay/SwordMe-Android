// Copyright 2010 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  Low-level API for VP8 decoder
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_WEBP_DECODE_VP8_H_
#define WEBP_WEBP_DECODE_VP8_H_

#include "../webp/decode.h"

#ifdef __cplusplus
extern "C" {
#endif

// Lower-level API
//
// These functions provide fine-grained control of the decoding process.
// The call flow should resemble:
//
//   VP8Io io;
//   VP8InitIo(&io);
//   io.data = data;
//   io.data_size = size;
//   /* customize io's functions (setup()/put()/teardown()) if needed. */
//
//   VP8Decoder* dec = VP8New();
//   bool ok = VP8Decode(dec);
//   if (!ok) printf("Error: %s\n", VP8StatusMessage(dec));
//   VP8Delete(dec);
//   return ok;

typedef struct VP8Io VP8Io;
typedef int (*VP8IoPutHook)(const VP8Io* io);
typedef int (*VP8IoSetupHook)(VP8Io* io);
typedef void (*VP8IoTeardownHook)(const VP8Io* io);

struct VP8Io {

  int width, height;         // picture dimensions, in pixels (invariable).




  int mb_y;                  // position of the current rows (in pixels)
  int mb_w;                  // number of columns in the sample
  int mb_h;                  // number of rows in the sample
  const uint8_t* y, *u, *v;  // rows to copy (in yuv420 format)
  int y_stride;              // row stride for luma
  int uv_stride;             // row stride for chroma

  void* opaque;              // user data





  VP8IoPutHook put;




  VP8IoSetupHook setup;


  VP8IoTeardownHook teardown;



  int fancy_upsampling;

  size_t data_size;
  const uint8_t* data;




  int bypass_filtering;

  int use_cropping;
  int crop_left, crop_right, crop_top, crop_bottom;

  int use_scaling;
  int scaled_width, scaled_height;



  const uint8_t* a;
};

int VP8InitIoInternal(VP8Io* const, int);

// should be called before initiating incremental decoding. Returns true if
// WebPIDecoder object is successfully modified, false otherwise.
int WebPISetIOHooks(WebPIDecoder* const idec,
                    VP8IoPutHook put,
                    VP8IoSetupHook setup,
                    VP8IoTeardownHook teardown,
                    void* user_data);

typedef struct VP8Decoder VP8Decoder;

VP8Decoder* VP8New(void);

// Returns false in case of version mismatch. Upon such failure, no other
// decoding function should be called (VP8Decode, VP8GetHeaders, ...)
static WEBP_INLINE int VP8InitIo(VP8Io* const io) {
  return VP8InitIoInternal(io, WEBP_DECODER_ABI_VERSION);
}

// Note: 'io->data' must be pointing to the start of the VP8 frame header.
int VP8GetHeaders(VP8Decoder* const dec, VP8Io* const io);

// Returns false in case of error.
int VP8Decode(VP8Decoder* const dec, VP8Io* const io);

VP8StatusCode VP8Status(VP8Decoder* const dec);

const char* VP8StatusMessage(VP8Decoder* const dec);

// Not a mandatory call between calls to VP8Decode().
void VP8Clear(VP8Decoder* const dec);

void VP8Delete(VP8Decoder* const dec);

// Miscellaneous VP8/VP8L bitstream probing functions.

WEBP_EXTERN(int) VP8CheckSignature(const uint8_t* const data, size_t data_size);

// width and height. Returns 0 in case of formatting error. *width/*height
// can be passed NULL.
WEBP_EXTERN(int) VP8GetInfo(
    const uint8_t* data,
    size_t data_size,    // data available so far
    size_t chunk_size,   // total data size expected in the chunk
    int* const width, int* const height);

WEBP_EXTERN(int) VP8LCheckSignature(const uint8_t* const data, size_t size);

// width, height and alpha. Returns 0 in case of formatting error.
// width/height/has_alpha can be passed NULL.
WEBP_EXTERN(int) VP8LGetInfo(
    const uint8_t* data, size_t data_size,  // data available so far
    int* const width, int* const height, int* const has_alpha);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_DECODE_VP8_H_ */
