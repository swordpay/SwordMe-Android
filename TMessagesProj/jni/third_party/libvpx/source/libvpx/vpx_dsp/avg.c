/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdlib.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_ports/mem.h"

unsigned int vpx_avg_8x8_c(const uint8_t *s, int p) {
  int i, j;
  int sum = 0;
  for (i = 0; i < 8; ++i, s += p)
    for (j = 0; j < 8; sum += s[j], ++j) {
    }

  return (sum + 32) >> 6;
}

unsigned int vpx_avg_4x4_c(const uint8_t *s, int p) {
  int i, j;
  int sum = 0;
  for (i = 0; i < 4; ++i, s += p)
    for (j = 0; j < 4; sum += s[j], ++j) {
    }

  return (sum + 8) >> 4;
}

#if CONFIG_VP9_HIGHBITDEPTH
// src_diff: 13 bit, dynamic range [-4095, 4095]
// coeff: 16 bit
static void hadamard_highbd_col8_first_pass(const int16_t *src_diff,
                                            ptrdiff_t src_stride,
                                            int16_t *coeff) {
  int16_t b0 = src_diff[0 * src_stride] + src_diff[1 * src_stride];
  int16_t b1 = src_diff[0 * src_stride] - src_diff[1 * src_stride];
  int16_t b2 = src_diff[2 * src_stride] + src_diff[3 * src_stride];
  int16_t b3 = src_diff[2 * src_stride] - src_diff[3 * src_stride];
  int16_t b4 = src_diff[4 * src_stride] + src_diff[5 * src_stride];
  int16_t b5 = src_diff[4 * src_stride] - src_diff[5 * src_stride];
  int16_t b6 = src_diff[6 * src_stride] + src_diff[7 * src_stride];
  int16_t b7 = src_diff[6 * src_stride] - src_diff[7 * src_stride];

  int16_t c0 = b0 + b2;
  int16_t c1 = b1 + b3;
  int16_t c2 = b0 - b2;
  int16_t c3 = b1 - b3;
  int16_t c4 = b4 + b6;
  int16_t c5 = b5 + b7;
  int16_t c6 = b4 - b6;
  int16_t c7 = b5 - b7;

  coeff[0] = c0 + c4;
  coeff[7] = c1 + c5;
  coeff[3] = c2 + c6;
  coeff[4] = c3 + c7;
  coeff[2] = c0 - c4;
  coeff[6] = c1 - c5;
  coeff[1] = c2 - c6;
  coeff[5] = c3 - c7;
}

// coeff: 19 bit
static void hadamard_highbd_col8_second_pass(const int16_t *src_diff,
                                             ptrdiff_t src_stride,
                                             int32_t *coeff) {
  int32_t b0 = src_diff[0 * src_stride] + src_diff[1 * src_stride];
  int32_t b1 = src_diff[0 * src_stride] - src_diff[1 * src_stride];
  int32_t b2 = src_diff[2 * src_stride] + src_diff[3 * src_stride];
  int32_t b3 = src_diff[2 * src_stride] - src_diff[3 * src_stride];
  int32_t b4 = src_diff[4 * src_stride] + src_diff[5 * src_stride];
  int32_t b5 = src_diff[4 * src_stride] - src_diff[5 * src_stride];
  int32_t b6 = src_diff[6 * src_stride] + src_diff[7 * src_stride];
  int32_t b7 = src_diff[6 * src_stride] - src_diff[7 * src_stride];

  int32_t c0 = b0 + b2;
  int32_t c1 = b1 + b3;
  int32_t c2 = b0 - b2;
  int32_t c3 = b1 - b3;
  int32_t c4 = b4 + b6;
  int32_t c5 = b5 + b7;
  int32_t c6 = b4 - b6;
  int32_t c7 = b5 - b7;

  coeff[0] = c0 + c4;
  coeff[7] = c1 + c5;
  coeff[3] = c2 + c6;
  coeff[4] = c3 + c7;
  coeff[2] = c0 - c4;
  coeff[6] = c1 - c5;
  coeff[1] = c2 - c6;
  coeff[5] = c3 - c7;
}

// optimization purposes the final transpose may be skipped.
void vpx_highbd_hadamard_8x8_c(const int16_t *src_diff, ptrdiff_t src_stride,
                               tran_low_t *coeff) {
  int idx;
  int16_t buffer[64];
  int32_t buffer2[64];
  int16_t *tmp_buf = &buffer[0];
  for (idx = 0; idx < 8; ++idx) {


    hadamard_highbd_col8_first_pass(src_diff, src_stride, tmp_buf);
    tmp_buf += 8;
    ++src_diff;
  }

  tmp_buf = &buffer[0];
  for (idx = 0; idx < 8; ++idx) {


    hadamard_highbd_col8_second_pass(tmp_buf, 8, buffer2 + 8 * idx);
    ++tmp_buf;
  }

  for (idx = 0; idx < 64; ++idx) coeff[idx] = (tran_low_t)buffer2[idx];
}

void vpx_highbd_hadamard_16x16_c(const int16_t *src_diff, ptrdiff_t src_stride,
                                 tran_low_t *coeff) {
  int idx;
  for (idx = 0; idx < 4; ++idx) {

    const int16_t *src_ptr =
        src_diff + (idx >> 1) * 8 * src_stride + (idx & 0x01) * 8;
    vpx_highbd_hadamard_8x8_c(src_ptr, src_stride, coeff + idx * 64);
  }

  for (idx = 0; idx < 64; ++idx) {
    tran_low_t a0 = coeff[0];
    tran_low_t a1 = coeff[64];
    tran_low_t a2 = coeff[128];
    tran_low_t a3 = coeff[192];

    tran_low_t b0 = (a0 + a1) >> 1;
    tran_low_t b1 = (a0 - a1) >> 1;
    tran_low_t b2 = (a2 + a3) >> 1;
    tran_low_t b3 = (a2 - a3) >> 1;

    coeff[0] = b0 + b2;
    coeff[64] = b1 + b3;
    coeff[128] = b0 - b2;
    coeff[192] = b1 - b3;

    ++coeff;
  }
}

void vpx_highbd_hadamard_32x32_c(const int16_t *src_diff, ptrdiff_t src_stride,
                                 tran_low_t *coeff) {
  int idx;
  for (idx = 0; idx < 4; ++idx) {

    const int16_t *src_ptr =
        src_diff + (idx >> 1) * 16 * src_stride + (idx & 0x01) * 16;
    vpx_highbd_hadamard_16x16_c(src_ptr, src_stride, coeff + idx * 256);
  }

  for (idx = 0; idx < 256; ++idx) {
    tran_low_t a0 = coeff[0];
    tran_low_t a1 = coeff[256];
    tran_low_t a2 = coeff[512];
    tran_low_t a3 = coeff[768];

    tran_low_t b0 = (a0 + a1) >> 2;
    tran_low_t b1 = (a0 - a1) >> 2;
    tran_low_t b2 = (a2 + a3) >> 2;
    tran_low_t b3 = (a2 - a3) >> 2;

    coeff[0] = b0 + b2;
    coeff[256] = b1 + b3;
    coeff[512] = b0 - b2;
    coeff[768] = b1 - b3;

    ++coeff;
  }
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

//           second pass, 12 bit, dynamic range [-2040, 2040]
static void hadamard_col8(const int16_t *src_diff, ptrdiff_t src_stride,
                          int16_t *coeff) {
  int16_t b0 = src_diff[0 * src_stride] + src_diff[1 * src_stride];
  int16_t b1 = src_diff[0 * src_stride] - src_diff[1 * src_stride];
  int16_t b2 = src_diff[2 * src_stride] + src_diff[3 * src_stride];
  int16_t b3 = src_diff[2 * src_stride] - src_diff[3 * src_stride];
  int16_t b4 = src_diff[4 * src_stride] + src_diff[5 * src_stride];
  int16_t b5 = src_diff[4 * src_stride] - src_diff[5 * src_stride];
  int16_t b6 = src_diff[6 * src_stride] + src_diff[7 * src_stride];
  int16_t b7 = src_diff[6 * src_stride] - src_diff[7 * src_stride];

  int16_t c0 = b0 + b2;
  int16_t c1 = b1 + b3;
  int16_t c2 = b0 - b2;
  int16_t c3 = b1 - b3;
  int16_t c4 = b4 + b6;
  int16_t c5 = b5 + b7;
  int16_t c6 = b4 - b6;
  int16_t c7 = b5 - b7;

  coeff[0] = c0 + c4;
  coeff[7] = c1 + c5;
  coeff[3] = c2 + c6;
  coeff[4] = c3 + c7;
  coeff[2] = c0 - c4;
  coeff[6] = c1 - c5;
  coeff[1] = c2 - c6;
  coeff[5] = c3 - c7;
}

// optimization purposes the final transpose may be skipped.
void vpx_hadamard_8x8_c(const int16_t *src_diff, ptrdiff_t src_stride,
                        tran_low_t *coeff) {
  int idx;
  int16_t buffer[64];
  int16_t buffer2[64];
  int16_t *tmp_buf = &buffer[0];
  for (idx = 0; idx < 8; ++idx) {
    hadamard_col8(src_diff, src_stride, tmp_buf);  // src_diff: 9 bit

    tmp_buf += 8;
    ++src_diff;
  }

  tmp_buf = &buffer[0];
  for (idx = 0; idx < 8; ++idx) {
    hadamard_col8(tmp_buf, 8, buffer2 + 8 * idx);  // tmp_buf: 12 bit



    ++tmp_buf;
  }

  for (idx = 0; idx < 64; ++idx) coeff[idx] = (tran_low_t)buffer2[idx];
}

void vpx_hadamard_16x16_c(const int16_t *src_diff, ptrdiff_t src_stride,
                          tran_low_t *coeff) {
  int idx;
  for (idx = 0; idx < 4; ++idx) {

    const int16_t *src_ptr =
        src_diff + (idx >> 1) * 8 * src_stride + (idx & 0x01) * 8;
    vpx_hadamard_8x8_c(src_ptr, src_stride, coeff + idx * 64);
  }

  for (idx = 0; idx < 64; ++idx) {
    tran_low_t a0 = coeff[0];
    tran_low_t a1 = coeff[64];
    tran_low_t a2 = coeff[128];
    tran_low_t a3 = coeff[192];

    tran_low_t b0 = (a0 + a1) >> 1;  // (a0 + a1): 16 bit, [-32640, 32640]
    tran_low_t b1 = (a0 - a1) >> 1;  // b0-b3: 15 bit, dynamic range
    tran_low_t b2 = (a2 + a3) >> 1;  // [-16320, 16320]
    tran_low_t b3 = (a2 - a3) >> 1;

    coeff[0] = b0 + b2;  // 16 bit, [-32640, 32640]
    coeff[64] = b1 + b3;
    coeff[128] = b0 - b2;
    coeff[192] = b1 - b3;

    ++coeff;
  }
}

void vpx_hadamard_32x32_c(const int16_t *src_diff, ptrdiff_t src_stride,
                          tran_low_t *coeff) {
  int idx;
  for (idx = 0; idx < 4; ++idx) {

    const int16_t *src_ptr =
        src_diff + (idx >> 1) * 16 * src_stride + (idx & 0x01) * 16;
    vpx_hadamard_16x16_c(src_ptr, src_stride, coeff + idx * 256);
  }

  for (idx = 0; idx < 256; ++idx) {
    tran_low_t a0 = coeff[0];
    tran_low_t a1 = coeff[256];
    tran_low_t a2 = coeff[512];
    tran_low_t a3 = coeff[768];

    tran_low_t b0 = (a0 + a1) >> 2;  // (a0 + a1): 16 bit, [-32640, 32640]
    tran_low_t b1 = (a0 - a1) >> 2;  // b0-b3: 15 bit, dynamic range
    tran_low_t b2 = (a2 + a3) >> 2;  // [-16320, 16320]
    tran_low_t b3 = (a2 - a3) >> 2;

    coeff[0] = b0 + b2;  // 16 bit, [-32640, 32640]
    coeff[256] = b1 + b3;
    coeff[512] = b0 - b2;
    coeff[768] = b1 - b3;

    ++coeff;
  }
}

#if CONFIG_VP9_HIGHBITDEPTH
// coeff: dynamic range 20 bit.
// length: value range {16, 64, 256, 1024}.
int vpx_highbd_satd_c(const tran_low_t *coeff, int length) {
  int i;
  int satd = 0;
  for (i = 0; i < length; ++i) satd += abs(coeff[i]);

  return satd;
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

// length: value range {16, 64, 256, 1024}.
int vpx_satd_c(const tran_low_t *coeff, int length) {
  int i;
  int satd = 0;
  for (i = 0; i < length; ++i) satd += abs(coeff[i]);

  return satd;
}

// height: value range {16, 32, 64}.
void vpx_int_pro_row_c(int16_t hbuf[16], const uint8_t *ref,
                       const int ref_stride, const int height) {
  int idx;
  const int norm_factor = height >> 1;
  for (idx = 0; idx < 16; ++idx) {
    int i;
    hbuf[idx] = 0;

    for (i = 0; i < height; ++i) hbuf[idx] += ref[i * ref_stride];

    hbuf[idx] /= norm_factor;
    ++ref;
  }
}

int16_t vpx_int_pro_col_c(const uint8_t *ref, const int width) {
  int idx;
  int16_t sum = 0;

  for (idx = 0; idx < width; ++idx) sum += ref[idx];
  return sum;
}

// src: [0 - 510]
// bwl: {2, 3, 4}
int vpx_vector_var_c(const int16_t *ref, const int16_t *src, const int bwl) {
  int i;
  int width = 4 << bwl;
  int sse = 0, mean = 0, var;

  for (i = 0; i < width; ++i) {
    int diff = ref[i] - src[i];  // diff: dynamic range [-510, 510], 10 bits.
    mean += diff;                // mean: dynamic range 16 bits.
    sse += diff * diff;          // sse:  dynamic range 26 bits.
  }

  var = sse - ((mean * mean) >> (bwl + 2));
  return var;
}

void vpx_minmax_8x8_c(const uint8_t *s, int p, const uint8_t *d, int dp,
                      int *min, int *max) {
  int i, j;
  *min = 255;
  *max = 0;
  for (i = 0; i < 8; ++i, s += p, d += dp) {
    for (j = 0; j < 8; ++j) {
      int diff = abs(s[j] - d[j]);
      *min = diff < *min ? diff : *min;
      *max = diff > *max ? diff : *max;
    }
  }
}

#if CONFIG_VP9_HIGHBITDEPTH
unsigned int vpx_highbd_avg_8x8_c(const uint8_t *s8, int p) {
  int i, j;
  int sum = 0;
  const uint16_t *s = CONVERT_TO_SHORTPTR(s8);
  for (i = 0; i < 8; ++i, s += p)
    for (j = 0; j < 8; sum += s[j], ++j) {
    }

  return (sum + 32) >> 6;
}

unsigned int vpx_highbd_avg_4x4_c(const uint8_t *s8, int p) {
  int i, j;
  int sum = 0;
  const uint16_t *s = CONVERT_TO_SHORTPTR(s8);
  for (i = 0; i < 4; ++i, s += p)
    for (j = 0; j < 4; sum += s[j], ++j) {
    }

  return (sum + 8) >> 4;
}

void vpx_highbd_minmax_8x8_c(const uint8_t *s8, int p, const uint8_t *d8,
                             int dp, int *min, int *max) {
  int i, j;
  const uint16_t *s = CONVERT_TO_SHORTPTR(s8);
  const uint16_t *d = CONVERT_TO_SHORTPTR(d8);
  *min = 255;
  *max = 0;
  for (i = 0; i < 8; ++i, s += p, d += dp) {
    for (j = 0; j < 8; ++j) {
      int diff = abs(s[j] - d[j]);
      *min = diff < *min ? diff : *min;
      *max = diff > *max ? diff : *max;
    }
  }
}
#endif  // CONFIG_VP9_HIGHBITDEPTH
