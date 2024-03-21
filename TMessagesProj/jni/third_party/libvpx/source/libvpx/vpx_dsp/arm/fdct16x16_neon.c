/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/txfm_common.h"
#include "vpx_dsp/arm/mem_neon.h"
#include "vpx_dsp/arm/transpose_neon.h"

// functions.
#if !defined(__clang__) && !defined(__ANDROID__) && defined(__GNUC__) && \
    __GNUC__ == 4 && __GNUC_MINOR__ == 9 && __GNUC_PATCHLEVEL__ < 4

void vpx_fdct16x16_neon(const int16_t *input, tran_low_t *output, int stride) {
  vpx_fdct16x16_c(input, output, stride);
}

#else

static INLINE void load(const int16_t *a, int stride, int16x8_t *b /*[16]*/) {
  b[0] = vld1q_s16(a);
  a += stride;
  b[1] = vld1q_s16(a);
  a += stride;
  b[2] = vld1q_s16(a);
  a += stride;
  b[3] = vld1q_s16(a);
  a += stride;
  b[4] = vld1q_s16(a);
  a += stride;
  b[5] = vld1q_s16(a);
  a += stride;
  b[6] = vld1q_s16(a);
  a += stride;
  b[7] = vld1q_s16(a);
  a += stride;
  b[8] = vld1q_s16(a);
  a += stride;
  b[9] = vld1q_s16(a);
  a += stride;
  b[10] = vld1q_s16(a);
  a += stride;
  b[11] = vld1q_s16(a);
  a += stride;
  b[12] = vld1q_s16(a);
  a += stride;
  b[13] = vld1q_s16(a);
  a += stride;
  b[14] = vld1q_s16(a);
  a += stride;
  b[15] = vld1q_s16(a);
}

static INLINE void store(tran_low_t *a, const int16x8_t *b /*[8]*/) {
  store_s16q_to_tran_low(a, b[0]);
  a += 16;
  store_s16q_to_tran_low(a, b[1]);
  a += 16;
  store_s16q_to_tran_low(a, b[2]);
  a += 16;
  store_s16q_to_tran_low(a, b[3]);
  a += 16;
  store_s16q_to_tran_low(a, b[4]);
  a += 16;
  store_s16q_to_tran_low(a, b[5]);
  a += 16;
  store_s16q_to_tran_low(a, b[6]);
  a += 16;
  store_s16q_to_tran_low(a, b[7]);
}

// all 16 values to be loaded. For the first pass it also multiplies by 4.

// get the first 4 and last 4 values, cross those, then load the middle 8 values
// and cross them.
static INLINE void cross_input(const int16x8_t *a /*[16]*/,
                               int16x8_t *b /*[16]*/, const int pass) {
  if (pass == 0) {
    b[0] = vshlq_n_s16(vaddq_s16(a[0], a[15]), 2);
    b[1] = vshlq_n_s16(vaddq_s16(a[1], a[14]), 2);
    b[2] = vshlq_n_s16(vaddq_s16(a[2], a[13]), 2);
    b[3] = vshlq_n_s16(vaddq_s16(a[3], a[12]), 2);
    b[4] = vshlq_n_s16(vaddq_s16(a[4], a[11]), 2);
    b[5] = vshlq_n_s16(vaddq_s16(a[5], a[10]), 2);
    b[6] = vshlq_n_s16(vaddq_s16(a[6], a[9]), 2);
    b[7] = vshlq_n_s16(vaddq_s16(a[7], a[8]), 2);

    b[8] = vshlq_n_s16(vsubq_s16(a[7], a[8]), 2);
    b[9] = vshlq_n_s16(vsubq_s16(a[6], a[9]), 2);
    b[10] = vshlq_n_s16(vsubq_s16(a[5], a[10]), 2);
    b[11] = vshlq_n_s16(vsubq_s16(a[4], a[11]), 2);
    b[12] = vshlq_n_s16(vsubq_s16(a[3], a[12]), 2);
    b[13] = vshlq_n_s16(vsubq_s16(a[2], a[13]), 2);
    b[14] = vshlq_n_s16(vsubq_s16(a[1], a[14]), 2);
    b[15] = vshlq_n_s16(vsubq_s16(a[0], a[15]), 2);
  } else {
    b[0] = vaddq_s16(a[0], a[15]);
    b[1] = vaddq_s16(a[1], a[14]);
    b[2] = vaddq_s16(a[2], a[13]);
    b[3] = vaddq_s16(a[3], a[12]);
    b[4] = vaddq_s16(a[4], a[11]);
    b[5] = vaddq_s16(a[5], a[10]);
    b[6] = vaddq_s16(a[6], a[9]);
    b[7] = vaddq_s16(a[7], a[8]);

    b[8] = vsubq_s16(a[7], a[8]);
    b[9] = vsubq_s16(a[6], a[9]);
    b[10] = vsubq_s16(a[5], a[10]);
    b[11] = vsubq_s16(a[4], a[11]);
    b[12] = vsubq_s16(a[3], a[12]);
    b[13] = vsubq_s16(a[2], a[13]);
    b[14] = vsubq_s16(a[1], a[14]);
    b[15] = vsubq_s16(a[0], a[15]);
  }
}

// because this only adds 1, not 1 << 2.
static INLINE void partial_round_shift(int16x8_t *a /*[16]*/) {
  const int16x8_t one = vdupq_n_s16(1);
  a[0] = vshrq_n_s16(vaddq_s16(a[0], one), 2);
  a[1] = vshrq_n_s16(vaddq_s16(a[1], one), 2);
  a[2] = vshrq_n_s16(vaddq_s16(a[2], one), 2);
  a[3] = vshrq_n_s16(vaddq_s16(a[3], one), 2);
  a[4] = vshrq_n_s16(vaddq_s16(a[4], one), 2);
  a[5] = vshrq_n_s16(vaddq_s16(a[5], one), 2);
  a[6] = vshrq_n_s16(vaddq_s16(a[6], one), 2);
  a[7] = vshrq_n_s16(vaddq_s16(a[7], one), 2);
  a[8] = vshrq_n_s16(vaddq_s16(a[8], one), 2);
  a[9] = vshrq_n_s16(vaddq_s16(a[9], one), 2);
  a[10] = vshrq_n_s16(vaddq_s16(a[10], one), 2);
  a[11] = vshrq_n_s16(vaddq_s16(a[11], one), 2);
  a[12] = vshrq_n_s16(vaddq_s16(a[12], one), 2);
  a[13] = vshrq_n_s16(vaddq_s16(a[13], one), 2);
  a[14] = vshrq_n_s16(vaddq_s16(a[14], one), 2);
  a[15] = vshrq_n_s16(vaddq_s16(a[15], one), 2);
}

static INLINE void butterfly_one_coeff(const int16x8_t a, const int16x8_t b,
                                       const tran_high_t c, int16x8_t *add,
                                       int16x8_t *sub) {
  const int32x4_t a0 = vmull_n_s16(vget_low_s16(a), c);
  const int32x4_t a1 = vmull_n_s16(vget_high_s16(a), c);
  const int32x4_t sum0 = vmlal_n_s16(a0, vget_low_s16(b), c);
  const int32x4_t sum1 = vmlal_n_s16(a1, vget_high_s16(b), c);
  const int32x4_t diff0 = vmlsl_n_s16(a0, vget_low_s16(b), c);
  const int32x4_t diff1 = vmlsl_n_s16(a1, vget_high_s16(b), c);
  const int16x4_t rounded0 = vqrshrn_n_s32(sum0, 14);
  const int16x4_t rounded1 = vqrshrn_n_s32(sum1, 14);
  const int16x4_t rounded2 = vqrshrn_n_s32(diff0, 14);
  const int16x4_t rounded3 = vqrshrn_n_s32(diff1, 14);
  *add = vcombine_s16(rounded0, rounded1);
  *sub = vcombine_s16(rounded2, rounded3);
}

static INLINE void butterfly_two_coeff(const int16x8_t a, const int16x8_t b,
                                       const tran_coef_t c0,
                                       const tran_coef_t c1, int16x8_t *add,
                                       int16x8_t *sub) {
  const int32x4_t a0 = vmull_n_s16(vget_low_s16(a), c0);
  const int32x4_t a1 = vmull_n_s16(vget_high_s16(a), c0);
  const int32x4_t a2 = vmull_n_s16(vget_low_s16(a), c1);
  const int32x4_t a3 = vmull_n_s16(vget_high_s16(a), c1);
  const int32x4_t sum0 = vmlal_n_s16(a2, vget_low_s16(b), c0);
  const int32x4_t sum1 = vmlal_n_s16(a3, vget_high_s16(b), c0);
  const int32x4_t diff0 = vmlsl_n_s16(a0, vget_low_s16(b), c1);
  const int32x4_t diff1 = vmlsl_n_s16(a1, vget_high_s16(b), c1);
  const int16x4_t rounded0 = vqrshrn_n_s32(sum0, 14);
  const int16x4_t rounded1 = vqrshrn_n_s32(sum1, 14);
  const int16x4_t rounded2 = vqrshrn_n_s32(diff0, 14);
  const int16x4_t rounded3 = vqrshrn_n_s32(diff1, 14);
  *add = vcombine_s16(rounded0, rounded1);
  *sub = vcombine_s16(rounded2, rounded3);
}

// are all in-place.
static INLINE void transpose_8x8(const int16x8_t *a /*[8]*/,
                                 int16x8_t *b /*[8]*/) {

  const int16x8x2_t c0 = vtrnq_s16(a[0], a[1]);
  const int16x8x2_t c1 = vtrnq_s16(a[2], a[3]);
  const int16x8x2_t c2 = vtrnq_s16(a[4], a[5]);
  const int16x8x2_t c3 = vtrnq_s16(a[6], a[7]);

  const int32x4x2_t d0 = vtrnq_s32(vreinterpretq_s32_s16(c0.val[0]),
                                   vreinterpretq_s32_s16(c1.val[0]));
  const int32x4x2_t d1 = vtrnq_s32(vreinterpretq_s32_s16(c0.val[1]),
                                   vreinterpretq_s32_s16(c1.val[1]));
  const int32x4x2_t d2 = vtrnq_s32(vreinterpretq_s32_s16(c2.val[0]),
                                   vreinterpretq_s32_s16(c3.val[0]));
  const int32x4x2_t d3 = vtrnq_s32(vreinterpretq_s32_s16(c2.val[1]),
                                   vreinterpretq_s32_s16(c3.val[1]));

  const int16x8x2_t e0 = vpx_vtrnq_s64_to_s16(d0.val[0], d2.val[0]);
  const int16x8x2_t e1 = vpx_vtrnq_s64_to_s16(d1.val[0], d3.val[0]);
  const int16x8x2_t e2 = vpx_vtrnq_s64_to_s16(d0.val[1], d2.val[1]);
  const int16x8x2_t e3 = vpx_vtrnq_s64_to_s16(d1.val[1], d3.val[1]);

  b[0] = e0.val[0];
  b[1] = e1.val[0];
  b[2] = e2.val[0];
  b[3] = e3.val[0];
  b[4] = e0.val[1];
  b[5] = e1.val[1];
  b[6] = e2.val[1];
  b[7] = e3.val[1];
}

static void dct_body(const int16x8_t *in /*[16]*/, int16x8_t *out /*[16]*/) {
  int16x8_t s[8];
  int16x8_t x[4];
  int16x8_t step[8];



  s[0] = vaddq_s16(in[0], in[7]);
  s[1] = vaddq_s16(in[1], in[6]);
  s[2] = vaddq_s16(in[2], in[5]);
  s[3] = vaddq_s16(in[3], in[4]);
  s[4] = vsubq_s16(in[3], in[4]);
  s[5] = vsubq_s16(in[2], in[5]);
  s[6] = vsubq_s16(in[1], in[6]);
  s[7] = vsubq_s16(in[0], in[7]);

  x[0] = vaddq_s16(s[0], s[3]);
  x[1] = vaddq_s16(s[1], s[2]);
  x[2] = vsubq_s16(s[1], s[2]);
  x[3] = vsubq_s16(s[0], s[3]);


  butterfly_one_coeff(x[0], x[1], cospi_16_64, &out[0], &out[8]);


  butterfly_two_coeff(x[3], x[2], cospi_24_64, cospi_8_64, &out[4], &out[12]);




  butterfly_one_coeff(s[6], s[5], cospi_16_64, &s[6], &s[5]);

  x[0] = vaddq_s16(s[4], s[5]);
  x[1] = vsubq_s16(s[4], s[5]);
  x[2] = vsubq_s16(s[7], s[6]);
  x[3] = vaddq_s16(s[7], s[6]);



  butterfly_two_coeff(x[3], x[0], cospi_28_64, cospi_4_64, &out[2], &out[14]);


  butterfly_two_coeff(x[2], x[1], cospi_12_64, cospi_20_64, &out[10], &out[6]);









  butterfly_one_coeff(in[13], in[10], cospi_16_64, &s[5], &s[2]);
  butterfly_one_coeff(in[12], in[11], cospi_16_64, &s[4], &s[3]);

  s[0] = vaddq_s16(in[8], s[3]);
  s[1] = vaddq_s16(in[9], s[2]);
  x[0] = vsubq_s16(in[9], s[2]);
  x[1] = vsubq_s16(in[8], s[3]);
  x[2] = vsubq_s16(in[15], s[4]);
  x[3] = vsubq_s16(in[14], s[5]);
  s[6] = vaddq_s16(in[14], s[5]);
  s[7] = vaddq_s16(in[15], s[4]);



  butterfly_two_coeff(s[6], s[1], cospi_24_64, cospi_8_64, &s[6], &s[1]);


  butterfly_two_coeff(x[0], x[3], cospi_8_64, cospi_24_64, &s[2], &s[5]);

  step[0] = vaddq_s16(s[0], s[1]);
  step[1] = vsubq_s16(s[0], s[1]);
  step[2] = vaddq_s16(x[1], s[2]);
  step[3] = vsubq_s16(x[1], s[2]);
  step[4] = vsubq_s16(x[2], s[5]);
  step[5] = vaddq_s16(x[2], s[5]);
  step[6] = vsubq_s16(s[7], s[6]);
  step[7] = vaddq_s16(s[7], s[6]);










  butterfly_two_coeff(step[6], step[1], cospi_14_64, cospi_18_64, &out[9],
                      &out[7]);
  butterfly_two_coeff(step[7], step[0], cospi_30_64, cospi_2_64, &out[1],
                      &out[15]);
  butterfly_two_coeff(step[4], step[3], cospi_6_64, cospi_26_64, &out[13],
                      &out[3]);
  butterfly_two_coeff(step[5], step[2], cospi_22_64, cospi_10_64, &out[5],
                      &out[11]);
}

void vpx_fdct16x16_neon(const int16_t *input, tran_low_t *output, int stride) {
  int16x8_t temp0[16];
  int16x8_t temp1[16];
  int16x8_t temp2[16];
  int16x8_t temp3[16];

  load(input, stride, temp0);
  cross_input(temp0, temp1, 0);
  dct_body(temp1, temp0);

  load(input + 8, stride, temp1);
  cross_input(temp1, temp2, 0);
  dct_body(temp2, temp1);


  transpose_8x8(&temp0[0], &temp2[0]);
  transpose_8x8(&temp1[0], &temp2[8]);
  partial_round_shift(temp2);
  cross_input(temp2, temp3, 1);
  dct_body(temp3, temp2);
  transpose_s16_8x8(&temp2[0], &temp2[1], &temp2[2], &temp2[3], &temp2[4],
                    &temp2[5], &temp2[6], &temp2[7]);
  transpose_s16_8x8(&temp2[8], &temp2[9], &temp2[10], &temp2[11], &temp2[12],
                    &temp2[13], &temp2[14], &temp2[15]);
  store(output, temp2);
  store(output + 8, temp2 + 8);
  output += 8 * 16;


  transpose_8x8(&temp0[8], &temp1[0]);
  transpose_s16_8x8(&temp1[8], &temp1[9], &temp1[10], &temp1[11], &temp1[12],
                    &temp1[13], &temp1[14], &temp1[15]);
  partial_round_shift(temp1);
  cross_input(temp1, temp0, 1);
  dct_body(temp0, temp1);
  transpose_s16_8x8(&temp1[0], &temp1[1], &temp1[2], &temp1[3], &temp1[4],
                    &temp1[5], &temp1[6], &temp1[7]);
  transpose_s16_8x8(&temp1[8], &temp1[9], &temp1[10], &temp1[11], &temp1[12],
                    &temp1[13], &temp1[14], &temp1[15]);
  store(output, temp1);
  store(output + 8, temp1 + 8);
}
#endif  // !defined(__clang__) && !defined(__ANDROID__) && defined(__GNUC__) &&

