// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//   WebP encoder: main interface
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_WEBP_ENCODE_H_
#define WEBP_WEBP_ENCODE_H_

#include "./types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WEBP_ENCODER_ABI_VERSION 0x0202    // MAJOR(8b) + MINOR(8b)

// the types are left here for reference.
// typedef enum WebPImageHint WebPImageHint;
// typedef enum WebPEncCSP WebPEncCSP;
// typedef enum WebPPreset WebPPreset;
// typedef enum WebPEncodingError WebPEncodingError;
typedef struct WebPConfig WebPConfig;
typedef struct WebPPicture WebPPicture;   // main structure for I/O
typedef struct WebPAuxStats WebPAuxStats;
typedef struct WebPMemoryWriter WebPMemoryWriter;

// each of major/minor/revision. E.g: v2.5.7 is 0x020507.
WEBP_EXTERN(int) WebPGetEncoderVersion(void);

// One-stop-shop call! No questions asked:

// an error occurred. The compressed data must be released by the caller
// using the call 'free(*output)'.
// These functions compress using the lossy format, and the quality_factor
// can go from 0 (smaller output, lower quality) to 100 (best quality,
// larger output).
WEBP_EXTERN(size_t) WebPEncodeRGB(const uint8_t* rgb,
                                  int width, int height, int stride,
                                  float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeBGR(const uint8_t* bgr,
                                  int width, int height, int stride,
                                  float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeRGBA(const uint8_t* rgba,
                                   int width, int height, int stride,
                                   float quality_factor, uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeBGRA(const uint8_t* bgra,
                                   int width, int height, int stride,
                                   float quality_factor, uint8_t** output);

// lossless manner. Files are usually larger than lossy format, but will
// not suffer any compression loss.
WEBP_EXTERN(size_t) WebPEncodeLosslessRGB(const uint8_t* rgb,
                                          int width, int height, int stride,
                                          uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeLosslessBGR(const uint8_t* bgr,
                                          int width, int height, int stride,
                                          uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeLosslessRGBA(const uint8_t* rgba,
                                           int width, int height, int stride,
                                           uint8_t** output);
WEBP_EXTERN(size_t) WebPEncodeLosslessBGRA(const uint8_t* bgra,
                                           int width, int height, int stride,
                                           uint8_t** output);

// Coding parameters

typedef enum WebPImageHint {
  WEBP_HINT_DEFAULT = 0,  // default preset.
  WEBP_HINT_PICTURE,      // digital picture, like portrait, inner shot
  WEBP_HINT_PHOTO,        // outdoor photograph, with natural lighting
  WEBP_HINT_GRAPH,        // Discrete tone image (graph, map-tile etc).
  WEBP_HINT_LAST
} WebPImageHint;

struct WebPConfig {
  int lossless;           // Lossless encoding (0=lossy(default), 1=lossless).
  float quality;          // between 0 (smallest file) and 100 (biggest)
  int method;             // quality/speed trade-off (0=fast, 6=slower-better)

  WebPImageHint image_hint;  // Hint for image type (lossless only for now).

  int target_size;        // if non-zero, set the desired target size in bytes.

  float target_PSNR;      // if non-zero, specifies the minimal distortion to

  int segments;           // maximum number of segments to use, in [1..4]
  int sns_strength;       // Spatial Noise Shaping. 0=off, 100=maximum.
  int filter_strength;    // range: [0 = off .. 100 = strongest]
  int filter_sharpness;   // range: [0 = off .. 7 = least sharp]
  int filter_type;        // filtering type: 0 = simple, 1 = strong (only used

  int autofilter;         // Auto adjust filter's strength [0 = off, 1 = on]
  int alpha_compression;  // Algorithm for encoding the alpha plane (0 = none,

  int alpha_filtering;    // Predictive filtering method for alpha plane.

  int alpha_quality;      // Between 0 (smallest size) and 100 (lossless).

  int pass;               // number of entropy-analysis passes (in [1..10]).

  int show_compressed;    // if true, export the compressed picture back.

  int preprocessing;      // preprocessing filter:

  int partitions;         // log2(number of token partitions) in [0..3]. Default

  int partition_limit;    // quality degradation allowed to fit the 512k limit


  int emulate_jpeg_size;  // If true, compression parameters will be remapped



  int thread_level;       // If non-zero, try and use multi-threaded encoding.
  int low_memory;         // If set, reduce memory usage (but increase CPU use).

  uint32_t pad[5];        // padding for later use
};

// of source picture. These presets are used when calling WebPConfigPreset().
typedef enum WebPPreset {
  WEBP_PRESET_DEFAULT = 0,  // default preset.
  WEBP_PRESET_PICTURE,      // digital picture, like portrait, inner shot
  WEBP_PRESET_PHOTO,        // outdoor photograph, with natural lighting
  WEBP_PRESET_DRAWING,      // hand or line drawing, with high-contrast details
  WEBP_PRESET_ICON,         // small-sized colorful images
  WEBP_PRESET_TEXT          // text-like
} WebPPreset;

WEBP_EXTERN(int) WebPConfigInitInternal(WebPConfig*, WebPPreset, float, int);

// modification. Returns false in case of version mismatch. WebPConfigInit()
// must have succeeded before using the 'config' object.
// Note that the default values are lossless=0 and quality=75.
static WEBP_INLINE int WebPConfigInit(WebPConfig* config) {
  return WebPConfigInitInternal(config, WEBP_PRESET_DEFAULT, 75.f,
                                WEBP_ENCODER_ABI_VERSION);
}

// set of parameters (referred to by 'preset') and a given quality factor.
// This function can be called as a replacement to WebPConfigInit(). Will
// return false in case of error.
static WEBP_INLINE int WebPConfigPreset(WebPConfig* config,
                                        WebPPreset preset, float quality) {
  return WebPConfigInitInternal(config, preset, quality,
                                WEBP_ENCODER_ABI_VERSION);
}

#if WEBP_ENCODER_ABI_VERSION > 0x0202
// Activate the lossless compression mode with the desired efficiency level
// between 0 (fastest, lowest compression) and 9 (slower, best compression).
// A good default level is '6', providing a fair tradeoff between compression
// speed and final compressed size.
// This function will overwrite several fields from config: 'method', 'quality'
// and 'lossless'. Returns false in case of parameter error.
WEBP_EXTERN(int) WebPConfigLosslessPreset(WebPConfig* config, int level);
#endif

// within their valid ranges.
WEBP_EXTERN(int) WebPValidateConfig(const WebPConfig* config);

// Input / Output
// Structure for storing auxiliary statistics (mostly for lossy encoding).

struct WebPAuxStats {
  int coded_size;         // final size

  float PSNR[5];          // peak-signal-to-noise ratio for Y/U/V/All/Alpha
  int block_count[3];     // number of intra4/intra16/skipped macroblocks
  int header_bytes[2];    // approximate number of bytes spent for header

  int residual_bytes[3][4];  // approximate number of bytes spent for

  int segment_size[4];    // number of macroblocks in each segments
  int segment_quant[4];   // quantizer values for each segments
  int segment_level[4];   // filtering strength for each segments [0..63]

  int alpha_data_size;    // size of the transparency data
  int layer_data_size;    // size of the enhancement layer data

  uint32_t lossless_features;  // bit0:predictor bit1:cross-color transform

  int histogram_bits;          // number of precision bits of histogram
  int transform_bits;          // precision bits for transform
  int cache_bits;              // number of bits for color cache lookup
  int palette_size;            // number of color in palette, if used
  int lossless_size;           // final lossless size

  uint32_t pad[4];        // padding for later use
};

// data/data_size is the segment of data to write, and 'picture' is for
// reference (and so one can make use of picture->custom_ptr).
typedef int (*WebPWriterFunction)(const uint8_t* data, size_t data_size,
                                  const WebPPicture* picture);

// the following WebPMemoryWriter object (to be set as a custom_ptr).
struct WebPMemoryWriter {
  uint8_t* mem;       // final buffer (of size 'max_size', larger than 'size').
  size_t   size;      // final size
  size_t   max_size;  // total capacity
  uint32_t pad[1];    // padding for later use
};

WEBP_EXTERN(void) WebPMemoryWriterInit(WebPMemoryWriter* writer);

#if WEBP_ENCODER_ABI_VERSION > 0x0203
// The following must be called to deallocate writer->mem memory. The 'writer'
// object itself is not deallocated.
WEBP_EXTERN(void) WebPMemoryWriterClear(WebPMemoryWriter* writer);
#endif
// The custom writer to be used with WebPMemoryWriter as custom_ptr. Upon
// completion, writer.mem and writer.size will hold the coded data.
#if WEBP_ENCODER_ABI_VERSION > 0x0203
// writer.mem must be freed by calling WebPMemoryWriterClear.
#else
// writer.mem must be freed by calling 'free(writer.mem)'.
#endif
WEBP_EXTERN(int) WebPMemoryWrite(const uint8_t* data, size_t data_size,
                                 const WebPPicture* picture);

// false to request an abort of the encoding process, or true otherwise if
// everything is OK.
typedef int (*WebPProgressHook)(int percent, const WebPPicture* picture);

typedef enum WebPEncCSP {

  WEBP_YUV420  = 0,        // 4:2:0
  WEBP_YUV420A = 4,        // alpha channel variant
  WEBP_CSP_UV_MASK = 3,    // bit-mask to get the UV sampling factors
  WEBP_CSP_ALPHA_BIT = 4   // bit that is set if alpha is present
} WebPEncCSP;

typedef enum WebPEncodingError {
  VP8_ENC_OK = 0,
  VP8_ENC_ERROR_OUT_OF_MEMORY,            // memory error allocating objects
  VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY,  // memory error while flushing bits
  VP8_ENC_ERROR_NULL_PARAMETER,           // a pointer parameter is NULL
  VP8_ENC_ERROR_INVALID_CONFIGURATION,    // configuration is invalid
  VP8_ENC_ERROR_BAD_DIMENSION,            // picture has invalid width/height
  VP8_ENC_ERROR_PARTITION0_OVERFLOW,      // partition is bigger than 512k
  VP8_ENC_ERROR_PARTITION_OVERFLOW,       // partition is bigger than 16M
  VP8_ENC_ERROR_BAD_WRITE,                // error while flushing bytes
  VP8_ENC_ERROR_FILE_TOO_BIG,             // file is bigger than 4G
  VP8_ENC_ERROR_USER_ABORT,               // abort request by user
  VP8_ENC_ERROR_LAST                      // list terminator. always last.
} WebPEncodingError;

#define WEBP_MAX_DIMENSION 16383

struct WebPPicture {






  int use_argb;

  WebPEncCSP colorspace;     // colorspace: should be YUV420 for now (=Y'CbCr).
  int width, height;         // dimensions (less or equal to WEBP_MAX_DIMENSION)
  uint8_t *y, *u, *v;        // pointers to luma/chroma planes.
  int y_stride, uv_stride;   // luma/chroma strides.
  uint8_t* a;                // pointer to the alpha plane
  int a_stride;              // stride of the alpha plane
  uint32_t pad1[2];          // padding for later use

  uint32_t* argb;            // Pointer to argb (32 bit) plane.
  int argb_stride;           // This is stride in pixels units, not bytes.
  uint32_t pad2[3];          // padding for later use



  WebPWriterFunction writer;  // can be NULL
  void* custom_ptr;           // can be used by the writer.

  int extra_info_type;    // 1: intra type, 2: segment, 3: quant



  uint8_t* extra_info;    // if not NULL, points to an array of size






  WebPAuxStats* stats;

  WebPEncodingError error_code;

  WebPProgressHook progress_hook;

  void* user_data;        // this field is free to be set to any value and


  uint32_t pad3[3];       // padding for later use

  uint8_t *pad4, *pad5;
  uint32_t pad6[8];       // padding for later use


  void* memory_;          // row chunk of memory for yuva planes
  void* memory_argb_;     // and for argb too.
  void* pad7[2];          // padding for later use
};

WEBP_EXTERN(int) WebPPictureInitInternal(WebPPicture*, int);

// of version mismatch. WebPPictureInit() must have succeeded before using the
// 'picture' object.
// Note that, by default, use_argb is false and colorspace is WEBP_YUV420.
static WEBP_INLINE int WebPPictureInit(WebPPicture* picture) {
  return WebPPictureInitInternal(picture, WEBP_ENCODER_ABI_VERSION);
}

// WebPPicture utils

// Allocate y/u/v buffers as per colorspace/width/height specification.
// Note! This function will free the previous buffer if needed.
// Returns false in case of memory error.
WEBP_EXTERN(int) WebPPictureAlloc(WebPPicture* picture);

// Note that this function does _not_ free the memory used by the 'picture'
// object itself.
// Besides memory (which is reclaimed) all other fields of 'picture' are
// preserved.
WEBP_EXTERN(void) WebPPictureFree(WebPPicture* picture);

// will fully own the copied pixels (this is not a view). The 'dst' picture need
// not be initialized as its content is overwritten.
// Returns false in case of memory allocation error.
WEBP_EXTERN(int) WebPPictureCopy(const WebPPicture* src, WebPPicture* dst);

// Result is in dB, stores in result[] in the Y/U/V/Alpha/All order.
// Returns false in case of error (src and ref don't have same dimension, ...)
// Warning: this function is rather CPU-intensive.
WEBP_EXTERN(int) WebPPictureDistortion(
    const WebPPicture* src, const WebPPicture* ref,
    int metric_type,           // 0 = PSNR, 1 = SSIM, 2 = LSIM
    float result[5]);

// Returns false in case of memory allocation error, or if the rectangle is
// outside of the source picture.
// The rectangle for the view is defined by the top-left corner pixel
// coordinates (left, top) as well as its width and height. This rectangle
// must be fully be comprised inside the 'src' source picture. If the source
// picture uses the YUV420 colorspace, the top and left coordinates will be
// snapped to even values.
WEBP_EXTERN(int) WebPPictureCrop(WebPPicture* picture,
                                 int left, int top, int width, int height);

// is defined by the top-left corner pixel coordinates (left, top) as well
// as its width and height. This rectangle must be fully be comprised inside
// the 'src' source picture. If the source picture uses the YUV420 colorspace,
// the top and left coordinates will be snapped to even values.
// Picture 'src' must out-live 'dst' picture. Self-extraction of view is allowed
// ('src' equal to 'dst') as a mean of fast-cropping (but note that doing so,
// the original dimension will be lost). Picture 'dst' need not be initialized
// with WebPPictureInit() if it is different from 'src', since its content will
// be overwritten.
// Returns false in case of memory allocation error or invalid parameters.
WEBP_EXTERN(int) WebPPictureView(const WebPPicture* src,
                                 int left, int top, int width, int height,
                                 WebPPicture* dst);

// not own the memory for pixels.
WEBP_EXTERN(int) WebPPictureIsView(const WebPPicture* picture);

// Now gamma correction is applied.
// Returns false in case of error (invalid parameter or insufficient memory).
WEBP_EXTERN(int) WebPPictureRescale(WebPPicture* pic, int width, int height);

// Previous buffer will be free'd, if any.
// *rgb buffer should have a size of at least height * rgb_stride.
// Returns false in case of memory error.
WEBP_EXTERN(int) WebPPictureImportRGB(
    WebPPicture* picture, const uint8_t* rgb, int rgb_stride);
// Same, but for RGBA buffer.
WEBP_EXTERN(int) WebPPictureImportRGBA(
    WebPPicture* picture, const uint8_t* rgba, int rgba_stride);
// Same, but for RGBA buffer. Imports the RGB direct from the 32-bit format
// input buffer ignoring the alpha channel. Avoids needing to copy the data
// to a temporary 24-bit RGB buffer to import the RGB only.
WEBP_EXTERN(int) WebPPictureImportRGBX(
    WebPPicture* picture, const uint8_t* rgbx, int rgbx_stride);

WEBP_EXTERN(int) WebPPictureImportBGR(
    WebPPicture* picture, const uint8_t* bgr, int bgr_stride);
WEBP_EXTERN(int) WebPPictureImportBGRA(
    WebPPicture* picture, const uint8_t* bgra, int bgra_stride);
WEBP_EXTERN(int) WebPPictureImportBGRX(
    WebPPicture* picture, const uint8_t* bgrx, int bgrx_stride);

// parameter is deprecated and should be equal to WEBP_YUV420.
// Upon return, picture->use_argb is set to false. The presence of real
// non-opaque transparent values is detected, and 'colorspace' will be
// adjusted accordingly. Note that this method is lossy.
// Returns false in case of error.
WEBP_EXTERN(int) WebPPictureARGBToYUVA(WebPPicture* picture,
                                       WebPEncCSP /*colorspace = WEBP_YUV420*/);

// pseudo-random dithering with a strength 'dithering' between
// 0.0 (no dithering) and 1.0 (maximum dithering). This is useful
// for photographic picture.
WEBP_EXTERN(int) WebPPictureARGBToYUVADithered(
    WebPPicture* picture, WebPEncCSP colorspace, float dithering);

#if WEBP_ENCODER_ABI_VERSION > 0x0204
// Performs 'smart' RGBA->YUVA420 downsampling and colorspace conversion.
// Downsampling is handled with extra care in case of color clipping. This
// method is roughly 2x slower than WebPPictureARGBToYUVA() but produces better
// YUV representation.
// Returns false in case of error.
WEBP_EXTERN(int) WebPPictureSmartARGBToYUVA(WebPPicture* picture);
#endif

// The input format must be YUV_420 or YUV_420A.
// Note that the use of this method is discouraged if one has access to the
// raw ARGB samples, since using YUV420 is comparatively lossy. Also, the
// conversion from YUV420 to ARGB incurs a small loss too.
// Returns false in case of error.
WEBP_EXTERN(int) WebPPictureYUVAToARGB(WebPPicture* picture);

// clean-up the YUV or RGB samples under fully transparent area, to help
// compressibility (no guarantee, though).
WEBP_EXTERN(void) WebPCleanupTransparentArea(WebPPicture* picture);

// Returns true in such case. Otherwise returns false (indicating that the
// alpha plane can be ignored altogether e.g.).
WEBP_EXTERN(int) WebPPictureHasTransparency(const WebPPicture* picture);

// the background color 'background_rgb' (specified as 24bit RGB triplet).
// After this call, all alpha values are reset to 0xff.
WEBP_EXTERN(void) WebPBlendAlpha(WebPPicture* pic, uint32_t background_rgb);

// Main call

// 'picture' must be less than 16384x16384 in dimension (cf WEBP_MAX_DIMENSION),
// and the 'config' object must be a valid one.
// Returns false in case of error, true otherwise.
// In case of error, picture->error_code is updated accordingly.
// 'picture' can hold the source samples in both YUV(A) or ARGB input, depending
// on the value of 'picture->use_argb'. It is highly recommended to use
// the former for lossy encoding, and the latter for lossless encoding
// (when config.lossless is true). Automatic conversion from one format to
// another is provided but they both incur some loss.
WEBP_EXTERN(int) WebPEncode(const WebPConfig* config, WebPPicture* picture);


#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_WEBP_ENCODE_H_ */
