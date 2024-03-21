/*
 *  Copyright (c) 2019 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <smmintrin.h>

#include "./vp9_rtcd.h"
#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_temporal_filter.h"
#include "vp9/encoder/x86/temporal_filter_constants.h"

static INLINE void highbd_store_dist_8(const uint16_t *a, const uint16_t *b,
                                       uint32_t *dst) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_reg = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_reg = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_first = _mm_cvtepu16_epi32(a_reg);
  const __m128i a_second = _mm_unpackhi_epi16(a_reg, zero);
  const __m128i b_first = _mm_cvtepu16_epi32(b_reg);
  const __m128i b_second = _mm_unpackhi_epi16(b_reg, zero);

  __m128i dist_first, dist_second;

  dist_first = _mm_sub_epi32(a_first, b_first);
  dist_second = _mm_sub_epi32(a_second, b_second);
  dist_first = _mm_mullo_epi32(dist_first, dist_first);
  dist_second = _mm_mullo_epi32(dist_second, dist_second);

  _mm_storeu_si128((__m128i *)dst, dist_first);
  _mm_storeu_si128((__m128i *)(dst + 4), dist_second);
}

static INLINE void highbd_get_sum_4(const uint32_t *dist, __m128i *sum) {
  __m128i dist_reg, dist_left, dist_right;

  dist_reg = _mm_loadu_si128((const __m128i *)dist);
  dist_left = _mm_loadu_si128((const __m128i *)(dist - 1));
  dist_right = _mm_loadu_si128((const __m128i *)(dist + 1));

  *sum = _mm_add_epi32(dist_reg, dist_left);
  *sum = _mm_add_epi32(*sum, dist_right);
}

static INLINE void highbd_get_sum_8(const uint32_t *dist, __m128i *sum_first,
                                    __m128i *sum_second) {
  highbd_get_sum_4(dist, sum_first);
  highbd_get_sum_4(dist + 4, sum_second);
}

// from the border, 4 for pixels in corners, and 6 for other edge values, plus
// however many values from y/uv plane are).
//
// Add in the rounding factor and shift, clamp to 16, invert and shift. Multiply
// by weight.
static INLINE void highbd_average_4(__m128i *output, const __m128i *sum,
                                    const __m128i *mul_constants,
                                    const int strength, const int rounding,
                                    const int weight) {

  const __m128i strength_u128 = _mm_set_epi32(0, 0, 0, strength);
  const __m128i rounding_u32 = _mm_set1_epi32(rounding);
  const __m128i weight_u32 = _mm_set1_epi32(weight);
  const __m128i sixteen = _mm_set1_epi32(16);
  const __m128i zero = _mm_setzero_si128();

  const __m128i sum_lo = _mm_unpacklo_epi32(*sum, zero);
  const __m128i sum_hi = _mm_unpackhi_epi32(*sum, zero);
  const __m128i const_lo = _mm_unpacklo_epi32(*mul_constants, zero);
  const __m128i const_hi = _mm_unpackhi_epi32(*mul_constants, zero);

  const __m128i mul_lo = _mm_mul_epu32(sum_lo, const_lo);
  const __m128i mul_lo_div = _mm_srli_epi64(mul_lo, 32);
  const __m128i mul_hi = _mm_mul_epu32(sum_hi, const_hi);
  const __m128i mul_hi_div = _mm_srli_epi64(mul_hi, 32);







  const __m128i mul_even = _mm_unpacklo_epi64(mul_lo_div, mul_hi_div);
  const __m128i mul_odd = _mm_unpackhi_epi64(mul_lo_div, mul_hi_div);
  const __m128i mul_odd_shift = _mm_slli_si128(mul_odd, 4);
  const __m128i mul = _mm_or_si128(mul_even, mul_odd_shift);

  *output = _mm_add_epi32(mul, rounding_u32);
  *output = _mm_srl_epi32(*output, strength_u128);

  *output = _mm_min_epu32(*output, sixteen);
  *output = _mm_sub_epi32(sixteen, *output);
  *output = _mm_mullo_epi32(*output, weight_u32);
}

static INLINE void highbd_average_8(__m128i *output_0, __m128i *output_1,
                                    const __m128i *sum_0_u32,
                                    const __m128i *sum_1_u32,
                                    const __m128i *mul_constants_0,
                                    const __m128i *mul_constants_1,
                                    const int strength, const int rounding,
                                    const int weight) {
  highbd_average_4(output_0, sum_0_u32, mul_constants_0, strength, rounding,
                   weight);
  highbd_average_4(output_1, sum_1_u32, mul_constants_1, strength, rounding,
                   weight);
}

static INLINE void highbd_accumulate_and_store_8(const __m128i sum_first_u32,
                                                 const __m128i sum_second_u32,
                                                 const uint16_t *pred,
                                                 uint16_t *count,
                                                 uint32_t *accumulator) {

  const __m128i sum_u16 = _mm_packus_epi32(sum_first_u32, sum_second_u32);
  const __m128i zero = _mm_setzero_si128();

  __m128i pred_u16 = _mm_loadu_si128((const __m128i *)pred);
  __m128i count_u16 = _mm_loadu_si128((const __m128i *)count);

  __m128i pred_0_u32, pred_1_u32;
  __m128i accum_0_u32, accum_1_u32;

  count_u16 = _mm_adds_epu16(count_u16, sum_u16);
  _mm_storeu_si128((__m128i *)count, count_u16);

  pred_u16 = _mm_mullo_epi16(sum_u16, pred_u16);

  pred_0_u32 = _mm_cvtepu16_epi32(pred_u16);
  pred_1_u32 = _mm_unpackhi_epi16(pred_u16, zero);

  accum_0_u32 = _mm_loadu_si128((const __m128i *)accumulator);
  accum_1_u32 = _mm_loadu_si128((const __m128i *)(accumulator + 4));

  accum_0_u32 = _mm_add_epi32(pred_0_u32, accum_0_u32);
  accum_1_u32 = _mm_add_epi32(pred_1_u32, accum_1_u32);

  _mm_storeu_si128((__m128i *)accumulator, accum_0_u32);
  _mm_storeu_si128((__m128i *)(accumulator + 4), accum_1_u32);
}

static INLINE void highbd_read_dist_4(const uint32_t *dist, __m128i *dist_reg) {
  *dist_reg = _mm_loadu_si128((const __m128i *)dist);
}

static INLINE void highbd_read_dist_8(const uint32_t *dist, __m128i *reg_first,
                                      __m128i *reg_second) {
  highbd_read_dist_4(dist, reg_first);
  highbd_read_dist_4(dist + 4, reg_second);
}

static INLINE void highbd_read_chroma_dist_row_8(
    int ss_x, const uint32_t *u_dist, const uint32_t *v_dist, __m128i *u_first,
    __m128i *u_second, __m128i *v_first, __m128i *v_second) {
  if (!ss_x) {


    highbd_read_dist_8(u_dist, u_first, u_second);
    highbd_read_dist_8(v_dist, v_first, v_second);
  } else {  // ss_x == 1

    __m128i u_reg, v_reg;

    highbd_read_dist_4(u_dist, &u_reg);

    *u_first = _mm_unpacklo_epi32(u_reg, u_reg);
    *u_second = _mm_unpackhi_epi32(u_reg, u_reg);

    highbd_read_dist_4(v_dist, &v_reg);

    *v_first = _mm_unpacklo_epi32(v_reg, v_reg);
    *v_second = _mm_unpackhi_epi32(v_reg, v_reg);
  }
}

static void vp9_highbd_apply_temporal_filter_luma_8(
    const uint16_t *y_src, int y_src_stride, const uint16_t *y_pre,
    int y_pre_stride, const uint16_t *u_src, const uint16_t *v_src,
    int uv_src_stride, const uint16_t *u_pre, const uint16_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, int use_whole_blk, uint32_t *y_accum,
    uint16_t *y_count, const uint32_t *y_dist, const uint32_t *u_dist,
    const uint32_t *v_dist, const uint32_t *const *neighbors_first,
    const uint32_t *const *neighbors_second, int top_weight,
    int bottom_weight) {
  const int rounding = (1 << strength) >> 1;
  int weight = top_weight;

  __m128i mul_first, mul_second;

  __m128i sum_row_1_first, sum_row_1_second;
  __m128i sum_row_2_first, sum_row_2_second;
  __m128i sum_row_3_first, sum_row_3_second;

  __m128i u_first, u_second;
  __m128i v_first, v_second;

  __m128i sum_row_first;
  __m128i sum_row_second;

  unsigned int h;

  assert(strength >= 4 && strength <= 14 &&
         "invalid adjusted temporal filter strength");
  assert(block_width == 8);

  (void)block_width;

  mul_first = _mm_load_si128((const __m128i *)neighbors_first[0]);
  mul_second = _mm_load_si128((const __m128i *)neighbors_second[0]);

  highbd_get_sum_8(y_dist, &sum_row_2_first, &sum_row_2_second);
  highbd_get_sum_8(y_dist + DIST_STRIDE, &sum_row_3_first, &sum_row_3_second);


  sum_row_first = _mm_add_epi32(sum_row_2_first, sum_row_3_first);
  sum_row_second = _mm_add_epi32(sum_row_2_second, sum_row_3_second);

  highbd_read_chroma_dist_row_8(ss_x, u_dist, v_dist, &u_first, &u_second,
                                &v_first, &v_second);

  sum_row_first = _mm_add_epi32(sum_row_first, u_first);
  sum_row_second = _mm_add_epi32(sum_row_second, u_second);

  sum_row_first = _mm_add_epi32(sum_row_first, v_first);
  sum_row_second = _mm_add_epi32(sum_row_second, v_second);

  highbd_average_8(&sum_row_first, &sum_row_second, &sum_row_first,
                   &sum_row_second, &mul_first, &mul_second, strength, rounding,
                   weight);

  highbd_accumulate_and_store_8(sum_row_first, sum_row_second, y_pre, y_count,
                                y_accum);

  y_src += y_src_stride;
  y_pre += y_pre_stride;
  y_count += y_pre_stride;
  y_accum += y_pre_stride;
  y_dist += DIST_STRIDE;

  u_src += uv_src_stride;
  u_pre += uv_pre_stride;
  u_dist += DIST_STRIDE;
  v_src += uv_src_stride;
  v_pre += uv_pre_stride;
  v_dist += DIST_STRIDE;

  mul_first = _mm_load_si128((const __m128i *)neighbors_first[1]);
  mul_second = _mm_load_si128((const __m128i *)neighbors_second[1]);

  for (h = 1; h < block_height - 1; ++h) {

    if (!use_whole_blk && h == block_height / 2) {
      weight = bottom_weight;
    }

    sum_row_1_first = sum_row_2_first;
    sum_row_1_second = sum_row_2_second;
    sum_row_2_first = sum_row_3_first;
    sum_row_2_second = sum_row_3_second;

    sum_row_first = _mm_add_epi32(sum_row_1_first, sum_row_2_first);
    sum_row_second = _mm_add_epi32(sum_row_1_second, sum_row_2_second);

    highbd_get_sum_8(y_dist + DIST_STRIDE, &sum_row_3_first, &sum_row_3_second);

    sum_row_first = _mm_add_epi32(sum_row_first, sum_row_3_first);
    sum_row_second = _mm_add_epi32(sum_row_second, sum_row_3_second);

    if (ss_y == 0 || h % 2 == 0) {


      highbd_read_chroma_dist_row_8(ss_x, u_dist, v_dist, &u_first, &u_second,
                                    &v_first, &v_second);

      u_src += uv_src_stride;
      u_pre += uv_pre_stride;
      u_dist += DIST_STRIDE;
      v_src += uv_src_stride;
      v_pre += uv_pre_stride;
      v_dist += DIST_STRIDE;
    }

    sum_row_first = _mm_add_epi32(sum_row_first, u_first);
    sum_row_second = _mm_add_epi32(sum_row_second, u_second);
    sum_row_first = _mm_add_epi32(sum_row_first, v_first);
    sum_row_second = _mm_add_epi32(sum_row_second, v_second);

    highbd_average_8(&sum_row_first, &sum_row_second, &sum_row_first,
                     &sum_row_second, &mul_first, &mul_second, strength,
                     rounding, weight);
    highbd_accumulate_and_store_8(sum_row_first, sum_row_second, y_pre, y_count,
                                  y_accum);

    y_src += y_src_stride;
    y_pre += y_pre_stride;
    y_count += y_pre_stride;
    y_accum += y_pre_stride;
    y_dist += DIST_STRIDE;
  }

  mul_first = _mm_load_si128((const __m128i *)neighbors_first[0]);
  mul_second = _mm_load_si128((const __m128i *)neighbors_second[0]);

  sum_row_1_first = sum_row_2_first;
  sum_row_1_second = sum_row_2_second;
  sum_row_2_first = sum_row_3_first;
  sum_row_2_second = sum_row_3_second;

  sum_row_first = _mm_add_epi32(sum_row_1_first, sum_row_2_first);
  sum_row_second = _mm_add_epi32(sum_row_1_second, sum_row_2_second);

  if (ss_y == 0) {


    highbd_read_chroma_dist_row_8(ss_x, u_dist, v_dist, &u_first, &u_second,
                                  &v_first, &v_second);
  }

  sum_row_first = _mm_add_epi32(sum_row_first, u_first);
  sum_row_second = _mm_add_epi32(sum_row_second, u_second);
  sum_row_first = _mm_add_epi32(sum_row_first, v_first);
  sum_row_second = _mm_add_epi32(sum_row_second, v_second);

  highbd_average_8(&sum_row_first, &sum_row_second, &sum_row_first,
                   &sum_row_second, &mul_first, &mul_second, strength, rounding,
                   weight);
  highbd_accumulate_and_store_8(sum_row_first, sum_row_second, y_pre, y_count,
                                y_accum);
}

static void vp9_highbd_apply_temporal_filter_luma(
    const uint16_t *y_src, int y_src_stride, const uint16_t *y_pre,
    int y_pre_stride, const uint16_t *u_src, const uint16_t *v_src,
    int uv_src_stride, const uint16_t *u_pre, const uint16_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *blk_fw, int use_whole_blk,
    uint32_t *y_accum, uint16_t *y_count, const uint32_t *y_dist,
    const uint32_t *u_dist, const uint32_t *v_dist) {
  unsigned int blk_col = 0, uv_blk_col = 0;
  const unsigned int blk_col_step = 8, uv_blk_col_step = 8 >> ss_x;
  const unsigned int mid_width = block_width >> 1,
                     last_width = block_width - blk_col_step;
  int top_weight = blk_fw[0],
      bottom_weight = use_whole_blk ? blk_fw[0] : blk_fw[2];
  const uint32_t *const *neighbors_first;
  const uint32_t *const *neighbors_second;

  neighbors_first = HIGHBD_LUMA_LEFT_COLUMN_NEIGHBORS;
  neighbors_second = HIGHBD_LUMA_MIDDLE_COLUMN_NEIGHBORS;
  vp9_highbd_apply_temporal_filter_luma_8(
      y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
      u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride, u_pre + uv_blk_col,
      v_pre + uv_blk_col, uv_pre_stride, blk_col_step, block_height, ss_x, ss_y,
      strength, use_whole_blk, y_accum + blk_col, y_count + blk_col,
      y_dist + blk_col, u_dist + uv_blk_col, v_dist + uv_blk_col,
      neighbors_first, neighbors_second, top_weight, bottom_weight);

  blk_col += blk_col_step;
  uv_blk_col += uv_blk_col_step;

  neighbors_first = HIGHBD_LUMA_MIDDLE_COLUMN_NEIGHBORS;
  for (; blk_col < mid_width;
       blk_col += blk_col_step, uv_blk_col += uv_blk_col_step) {
    vp9_highbd_apply_temporal_filter_luma_8(
        y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
        u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride,
        u_pre + uv_blk_col, v_pre + uv_blk_col, uv_pre_stride, blk_col_step,
        block_height, ss_x, ss_y, strength, use_whole_blk, y_accum + blk_col,
        y_count + blk_col, y_dist + blk_col, u_dist + uv_blk_col,
        v_dist + uv_blk_col, neighbors_first, neighbors_second, top_weight,
        bottom_weight);
  }

  if (!use_whole_blk) {
    top_weight = blk_fw[1];
    bottom_weight = blk_fw[3];
  }

  for (; blk_col < last_width;
       blk_col += blk_col_step, uv_blk_col += uv_blk_col_step) {
    vp9_highbd_apply_temporal_filter_luma_8(
        y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
        u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride,
        u_pre + uv_blk_col, v_pre + uv_blk_col, uv_pre_stride, blk_col_step,
        block_height, ss_x, ss_y, strength, use_whole_blk, y_accum + blk_col,
        y_count + blk_col, y_dist + blk_col, u_dist + uv_blk_col,
        v_dist + uv_blk_col, neighbors_first, neighbors_second, top_weight,
        bottom_weight);
  }

  neighbors_second = HIGHBD_LUMA_RIGHT_COLUMN_NEIGHBORS;
  vp9_highbd_apply_temporal_filter_luma_8(
      y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
      u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride, u_pre + uv_blk_col,
      v_pre + uv_blk_col, uv_pre_stride, blk_col_step, block_height, ss_x, ss_y,
      strength, use_whole_blk, y_accum + blk_col, y_count + blk_col,
      y_dist + blk_col, u_dist + uv_blk_col, v_dist + uv_blk_col,
      neighbors_first, neighbors_second, top_weight, bottom_weight);
}

// subsampling in x direction, then we have 16 lumas, else we have 8.
static INLINE void highbd_add_luma_dist_to_8_chroma_mod(
    const uint32_t *y_dist, int ss_x, int ss_y, __m128i *u_mod_fst,
    __m128i *u_mod_snd, __m128i *v_mod_fst, __m128i *v_mod_snd) {
  __m128i y_reg_fst, y_reg_snd;
  if (!ss_x) {
    highbd_read_dist_8(y_dist, &y_reg_fst, &y_reg_snd);
    if (ss_y == 1) {
      __m128i y_tmp_fst, y_tmp_snd;
      highbd_read_dist_8(y_dist + DIST_STRIDE, &y_tmp_fst, &y_tmp_snd);
      y_reg_fst = _mm_add_epi32(y_reg_fst, y_tmp_fst);
      y_reg_snd = _mm_add_epi32(y_reg_snd, y_tmp_snd);
    }
  } else {

    __m128i y_fst, y_snd;

    highbd_read_dist_8(y_dist, &y_fst, &y_snd);
    if (ss_y == 1) {
      __m128i y_tmp_fst, y_tmp_snd;
      highbd_read_dist_8(y_dist + DIST_STRIDE, &y_tmp_fst, &y_tmp_snd);

      y_fst = _mm_add_epi32(y_fst, y_tmp_fst);
      y_snd = _mm_add_epi32(y_snd, y_tmp_snd);
    }

    y_reg_fst = _mm_hadd_epi32(y_fst, y_snd);

    highbd_read_dist_8(y_dist + 8, &y_fst, &y_snd);
    if (ss_y == 1) {
      __m128i y_tmp_fst, y_tmp_snd;
      highbd_read_dist_8(y_dist + 8 + DIST_STRIDE, &y_tmp_fst, &y_tmp_snd);

      y_fst = _mm_add_epi32(y_fst, y_tmp_fst);
      y_snd = _mm_add_epi32(y_snd, y_tmp_snd);
    }

    y_reg_snd = _mm_hadd_epi32(y_fst, y_snd);
  }

  *u_mod_fst = _mm_add_epi32(*u_mod_fst, y_reg_fst);
  *u_mod_snd = _mm_add_epi32(*u_mod_snd, y_reg_snd);
  *v_mod_fst = _mm_add_epi32(*v_mod_fst, y_reg_fst);
  *v_mod_snd = _mm_add_epi32(*v_mod_snd, y_reg_snd);
}

// filtering on a chroma block of 8 X uv_height. If blk_fw is not NULL, use
// blk_fw as an array of size 4 for the weights for each of the 4 subblocks,
// else use top_weight for top half, and bottom weight for bottom half.
static void vp9_highbd_apply_temporal_filter_chroma_8(
    const uint16_t *y_src, int y_src_stride, const uint16_t *y_pre,
    int y_pre_stride, const uint16_t *u_src, const uint16_t *v_src,
    int uv_src_stride, const uint16_t *u_pre, const uint16_t *v_pre,
    int uv_pre_stride, unsigned int uv_block_width,
    unsigned int uv_block_height, int ss_x, int ss_y, int strength,
    uint32_t *u_accum, uint16_t *u_count, uint32_t *v_accum, uint16_t *v_count,
    const uint32_t *y_dist, const uint32_t *u_dist, const uint32_t *v_dist,
    const uint32_t *const *neighbors_fst, const uint32_t *const *neighbors_snd,
    int top_weight, int bottom_weight, const int *blk_fw) {
  const int rounding = (1 << strength) >> 1;
  int weight = top_weight;

  __m128i mul_fst, mul_snd;

  __m128i u_sum_row_1_fst, u_sum_row_2_fst, u_sum_row_3_fst;
  __m128i v_sum_row_1_fst, v_sum_row_2_fst, v_sum_row_3_fst;
  __m128i u_sum_row_1_snd, u_sum_row_2_snd, u_sum_row_3_snd;
  __m128i v_sum_row_1_snd, v_sum_row_2_snd, v_sum_row_3_snd;

  __m128i u_sum_row_fst, v_sum_row_fst;
  __m128i u_sum_row_snd, v_sum_row_snd;

  unsigned int h;

  (void)uv_block_width;

  mul_fst = _mm_load_si128((const __m128i *)neighbors_fst[0]);
  mul_snd = _mm_load_si128((const __m128i *)neighbors_snd[0]);

  highbd_get_sum_8(u_dist, &u_sum_row_2_fst, &u_sum_row_2_snd);
  highbd_get_sum_8(u_dist + DIST_STRIDE, &u_sum_row_3_fst, &u_sum_row_3_snd);

  u_sum_row_fst = _mm_add_epi32(u_sum_row_2_fst, u_sum_row_3_fst);
  u_sum_row_snd = _mm_add_epi32(u_sum_row_2_snd, u_sum_row_3_snd);

  highbd_get_sum_8(v_dist, &v_sum_row_2_fst, &v_sum_row_2_snd);
  highbd_get_sum_8(v_dist + DIST_STRIDE, &v_sum_row_3_fst, &v_sum_row_3_snd);

  v_sum_row_fst = _mm_add_epi32(v_sum_row_2_fst, v_sum_row_3_fst);
  v_sum_row_snd = _mm_add_epi32(v_sum_row_2_snd, v_sum_row_3_snd);

  highbd_add_luma_dist_to_8_chroma_mod(y_dist, ss_x, ss_y, &u_sum_row_fst,
                                       &u_sum_row_snd, &v_sum_row_fst,
                                       &v_sum_row_snd);

  if (blk_fw) {
    highbd_average_4(&u_sum_row_fst, &u_sum_row_fst, &mul_fst, strength,
                     rounding, blk_fw[0]);
    highbd_average_4(&u_sum_row_snd, &u_sum_row_snd, &mul_snd, strength,
                     rounding, blk_fw[1]);

    highbd_average_4(&v_sum_row_fst, &v_sum_row_fst, &mul_fst, strength,
                     rounding, blk_fw[0]);
    highbd_average_4(&v_sum_row_snd, &v_sum_row_snd, &mul_snd, strength,
                     rounding, blk_fw[1]);

  } else {
    highbd_average_8(&u_sum_row_fst, &u_sum_row_snd, &u_sum_row_fst,
                     &u_sum_row_snd, &mul_fst, &mul_snd, strength, rounding,
                     weight);
    highbd_average_8(&v_sum_row_fst, &v_sum_row_snd, &v_sum_row_fst,
                     &v_sum_row_snd, &mul_fst, &mul_snd, strength, rounding,
                     weight);
  }
  highbd_accumulate_and_store_8(u_sum_row_fst, u_sum_row_snd, u_pre, u_count,
                                u_accum);
  highbd_accumulate_and_store_8(v_sum_row_fst, v_sum_row_snd, v_pre, v_count,
                                v_accum);

  u_src += uv_src_stride;
  u_pre += uv_pre_stride;
  u_dist += DIST_STRIDE;
  v_src += uv_src_stride;
  v_pre += uv_pre_stride;
  v_dist += DIST_STRIDE;
  u_count += uv_pre_stride;
  u_accum += uv_pre_stride;
  v_count += uv_pre_stride;
  v_accum += uv_pre_stride;

  y_src += y_src_stride * (1 + ss_y);
  y_pre += y_pre_stride * (1 + ss_y);
  y_dist += DIST_STRIDE * (1 + ss_y);

  mul_fst = _mm_load_si128((const __m128i *)neighbors_fst[1]);
  mul_snd = _mm_load_si128((const __m128i *)neighbors_snd[1]);

  for (h = 1; h < uv_block_height - 1; ++h) {

    if (h == uv_block_height / 2) {
      if (blk_fw) {
        blk_fw += 2;
      } else {
        weight = bottom_weight;
      }
    }

    u_sum_row_1_fst = u_sum_row_2_fst;
    u_sum_row_2_fst = u_sum_row_3_fst;
    u_sum_row_1_snd = u_sum_row_2_snd;
    u_sum_row_2_snd = u_sum_row_3_snd;

    v_sum_row_1_fst = v_sum_row_2_fst;
    v_sum_row_2_fst = v_sum_row_3_fst;
    v_sum_row_1_snd = v_sum_row_2_snd;
    v_sum_row_2_snd = v_sum_row_3_snd;

    u_sum_row_fst = _mm_add_epi32(u_sum_row_1_fst, u_sum_row_2_fst);
    u_sum_row_snd = _mm_add_epi32(u_sum_row_1_snd, u_sum_row_2_snd);
    highbd_get_sum_8(u_dist + DIST_STRIDE, &u_sum_row_3_fst, &u_sum_row_3_snd);
    u_sum_row_fst = _mm_add_epi32(u_sum_row_fst, u_sum_row_3_fst);
    u_sum_row_snd = _mm_add_epi32(u_sum_row_snd, u_sum_row_3_snd);

    v_sum_row_fst = _mm_add_epi32(v_sum_row_1_fst, v_sum_row_2_fst);
    v_sum_row_snd = _mm_add_epi32(v_sum_row_1_snd, v_sum_row_2_snd);
    highbd_get_sum_8(v_dist + DIST_STRIDE, &v_sum_row_3_fst, &v_sum_row_3_snd);
    v_sum_row_fst = _mm_add_epi32(v_sum_row_fst, v_sum_row_3_fst);
    v_sum_row_snd = _mm_add_epi32(v_sum_row_snd, v_sum_row_3_snd);

    highbd_add_luma_dist_to_8_chroma_mod(y_dist, ss_x, ss_y, &u_sum_row_fst,
                                         &u_sum_row_snd, &v_sum_row_fst,
                                         &v_sum_row_snd);

    if (blk_fw) {
      highbd_average_4(&u_sum_row_fst, &u_sum_row_fst, &mul_fst, strength,
                       rounding, blk_fw[0]);
      highbd_average_4(&u_sum_row_snd, &u_sum_row_snd, &mul_snd, strength,
                       rounding, blk_fw[1]);

      highbd_average_4(&v_sum_row_fst, &v_sum_row_fst, &mul_fst, strength,
                       rounding, blk_fw[0]);
      highbd_average_4(&v_sum_row_snd, &v_sum_row_snd, &mul_snd, strength,
                       rounding, blk_fw[1]);

    } else {
      highbd_average_8(&u_sum_row_fst, &u_sum_row_snd, &u_sum_row_fst,
                       &u_sum_row_snd, &mul_fst, &mul_snd, strength, rounding,
                       weight);
      highbd_average_8(&v_sum_row_fst, &v_sum_row_snd, &v_sum_row_fst,
                       &v_sum_row_snd, &mul_fst, &mul_snd, strength, rounding,
                       weight);
    }

    highbd_accumulate_and_store_8(u_sum_row_fst, u_sum_row_snd, u_pre, u_count,
                                  u_accum);
    highbd_accumulate_and_store_8(v_sum_row_fst, v_sum_row_snd, v_pre, v_count,
                                  v_accum);

    u_src += uv_src_stride;
    u_pre += uv_pre_stride;
    u_dist += DIST_STRIDE;
    v_src += uv_src_stride;
    v_pre += uv_pre_stride;
    v_dist += DIST_STRIDE;
    u_count += uv_pre_stride;
    u_accum += uv_pre_stride;
    v_count += uv_pre_stride;
    v_accum += uv_pre_stride;

    y_src += y_src_stride * (1 + ss_y);
    y_pre += y_pre_stride * (1 + ss_y);
    y_dist += DIST_STRIDE * (1 + ss_y);
  }

  mul_fst = _mm_load_si128((const __m128i *)neighbors_fst[0]);
  mul_snd = _mm_load_si128((const __m128i *)neighbors_snd[0]);

  u_sum_row_1_fst = u_sum_row_2_fst;
  u_sum_row_2_fst = u_sum_row_3_fst;
  u_sum_row_1_snd = u_sum_row_2_snd;
  u_sum_row_2_snd = u_sum_row_3_snd;

  v_sum_row_1_fst = v_sum_row_2_fst;
  v_sum_row_2_fst = v_sum_row_3_fst;
  v_sum_row_1_snd = v_sum_row_2_snd;
  v_sum_row_2_snd = v_sum_row_3_snd;

  u_sum_row_fst = _mm_add_epi32(u_sum_row_1_fst, u_sum_row_2_fst);
  v_sum_row_fst = _mm_add_epi32(v_sum_row_1_fst, v_sum_row_2_fst);
  u_sum_row_snd = _mm_add_epi32(u_sum_row_1_snd, u_sum_row_2_snd);
  v_sum_row_snd = _mm_add_epi32(v_sum_row_1_snd, v_sum_row_2_snd);

  highbd_add_luma_dist_to_8_chroma_mod(y_dist, ss_x, ss_y, &u_sum_row_fst,
                                       &u_sum_row_snd, &v_sum_row_fst,
                                       &v_sum_row_snd);

  if (blk_fw) {
    highbd_average_4(&u_sum_row_fst, &u_sum_row_fst, &mul_fst, strength,
                     rounding, blk_fw[0]);
    highbd_average_4(&u_sum_row_snd, &u_sum_row_snd, &mul_snd, strength,
                     rounding, blk_fw[1]);

    highbd_average_4(&v_sum_row_fst, &v_sum_row_fst, &mul_fst, strength,
                     rounding, blk_fw[0]);
    highbd_average_4(&v_sum_row_snd, &v_sum_row_snd, &mul_snd, strength,
                     rounding, blk_fw[1]);

  } else {
    highbd_average_8(&u_sum_row_fst, &u_sum_row_snd, &u_sum_row_fst,
                     &u_sum_row_snd, &mul_fst, &mul_snd, strength, rounding,
                     weight);
    highbd_average_8(&v_sum_row_fst, &v_sum_row_snd, &v_sum_row_fst,
                     &v_sum_row_snd, &mul_fst, &mul_snd, strength, rounding,
                     weight);
  }

  highbd_accumulate_and_store_8(u_sum_row_fst, u_sum_row_snd, u_pre, u_count,
                                u_accum);
  highbd_accumulate_and_store_8(v_sum_row_fst, v_sum_row_snd, v_pre, v_count,
                                v_accum);
}

static void vp9_highbd_apply_temporal_filter_chroma(
    const uint16_t *y_src, int y_src_stride, const uint16_t *y_pre,
    int y_pre_stride, const uint16_t *u_src, const uint16_t *v_src,
    int uv_src_stride, const uint16_t *u_pre, const uint16_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *blk_fw, int use_whole_blk,
    uint32_t *u_accum, uint16_t *u_count, uint32_t *v_accum, uint16_t *v_count,
    const uint32_t *y_dist, const uint32_t *u_dist, const uint32_t *v_dist) {
  const unsigned int uv_width = block_width >> ss_x,
                     uv_height = block_height >> ss_y;

  unsigned int blk_col = 0, uv_blk_col = 0;
  const unsigned int uv_blk_col_step = 8, blk_col_step = 8 << ss_x;
  const unsigned int uv_mid_width = uv_width >> 1,
                     uv_last_width = uv_width - uv_blk_col_step;
  int top_weight = blk_fw[0],
      bottom_weight = use_whole_blk ? blk_fw[0] : blk_fw[2];
  const uint32_t *const *neighbors_fst;
  const uint32_t *const *neighbors_snd;

  if (uv_width == 8) {



    assert(ss_x);

    if (ss_y) {
      neighbors_fst = HIGHBD_CHROMA_DOUBLE_SS_LEFT_COLUMN_NEIGHBORS;
      neighbors_snd = HIGHBD_CHROMA_DOUBLE_SS_RIGHT_COLUMN_NEIGHBORS;
    } else {
      neighbors_fst = HIGHBD_CHROMA_SINGLE_SS_LEFT_COLUMN_NEIGHBORS;
      neighbors_snd = HIGHBD_CHROMA_SINGLE_SS_RIGHT_COLUMN_NEIGHBORS;
    }

    if (use_whole_blk) {
      vp9_highbd_apply_temporal_filter_chroma_8(
          y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
          u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride,
          u_pre + uv_blk_col, v_pre + uv_blk_col, uv_pre_stride, uv_width,
          uv_height, ss_x, ss_y, strength, u_accum + uv_blk_col,
          u_count + uv_blk_col, v_accum + uv_blk_col, v_count + uv_blk_col,
          y_dist + blk_col, u_dist + uv_blk_col, v_dist + uv_blk_col,
          neighbors_fst, neighbors_snd, top_weight, bottom_weight, NULL);
    } else {
      vp9_highbd_apply_temporal_filter_chroma_8(
          y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
          u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride,
          u_pre + uv_blk_col, v_pre + uv_blk_col, uv_pre_stride, uv_width,
          uv_height, ss_x, ss_y, strength, u_accum + uv_blk_col,
          u_count + uv_blk_col, v_accum + uv_blk_col, v_count + uv_blk_col,
          y_dist + blk_col, u_dist + uv_blk_col, v_dist + uv_blk_col,
          neighbors_fst, neighbors_snd, 0, 0, blk_fw);
    }

    return;
  }

  if (ss_x && ss_y) {
    neighbors_fst = HIGHBD_CHROMA_DOUBLE_SS_LEFT_COLUMN_NEIGHBORS;
    neighbors_snd = HIGHBD_CHROMA_DOUBLE_SS_MIDDLE_COLUMN_NEIGHBORS;
  } else if (ss_x || ss_y) {
    neighbors_fst = HIGHBD_CHROMA_SINGLE_SS_LEFT_COLUMN_NEIGHBORS;
    neighbors_snd = HIGHBD_CHROMA_SINGLE_SS_MIDDLE_COLUMN_NEIGHBORS;
  } else {
    neighbors_fst = HIGHBD_CHROMA_NO_SS_LEFT_COLUMN_NEIGHBORS;
    neighbors_snd = HIGHBD_CHROMA_NO_SS_MIDDLE_COLUMN_NEIGHBORS;
  }

  vp9_highbd_apply_temporal_filter_chroma_8(
      y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
      u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride, u_pre + uv_blk_col,
      v_pre + uv_blk_col, uv_pre_stride, uv_width, uv_height, ss_x, ss_y,
      strength, u_accum + uv_blk_col, u_count + uv_blk_col,
      v_accum + uv_blk_col, v_count + uv_blk_col, y_dist + blk_col,
      u_dist + uv_blk_col, v_dist + uv_blk_col, neighbors_fst, neighbors_snd,
      top_weight, bottom_weight, NULL);

  blk_col += blk_col_step;
  uv_blk_col += uv_blk_col_step;

  if (ss_x && ss_y) {
    neighbors_fst = HIGHBD_CHROMA_DOUBLE_SS_MIDDLE_COLUMN_NEIGHBORS;
  } else if (ss_x || ss_y) {
    neighbors_fst = HIGHBD_CHROMA_SINGLE_SS_MIDDLE_COLUMN_NEIGHBORS;
  } else {
    neighbors_fst = HIGHBD_CHROMA_NO_SS_MIDDLE_COLUMN_NEIGHBORS;
  }

  for (; uv_blk_col < uv_mid_width;
       blk_col += blk_col_step, uv_blk_col += uv_blk_col_step) {
    vp9_highbd_apply_temporal_filter_chroma_8(
        y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
        u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride,
        u_pre + uv_blk_col, v_pre + uv_blk_col, uv_pre_stride, uv_width,
        uv_height, ss_x, ss_y, strength, u_accum + uv_blk_col,
        u_count + uv_blk_col, v_accum + uv_blk_col, v_count + uv_blk_col,
        y_dist + blk_col, u_dist + uv_blk_col, v_dist + uv_blk_col,
        neighbors_fst, neighbors_snd, top_weight, bottom_weight, NULL);
  }

  if (!use_whole_blk) {
    top_weight = blk_fw[1];
    bottom_weight = blk_fw[3];
  }

  for (; uv_blk_col < uv_last_width;
       blk_col += blk_col_step, uv_blk_col += uv_blk_col_step) {
    vp9_highbd_apply_temporal_filter_chroma_8(
        y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
        u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride,
        u_pre + uv_blk_col, v_pre + uv_blk_col, uv_pre_stride, uv_width,
        uv_height, ss_x, ss_y, strength, u_accum + uv_blk_col,
        u_count + uv_blk_col, v_accum + uv_blk_col, v_count + uv_blk_col,
        y_dist + blk_col, u_dist + uv_blk_col, v_dist + uv_blk_col,
        neighbors_fst, neighbors_snd, top_weight, bottom_weight, NULL);
  }

  if (ss_x && ss_y) {
    neighbors_snd = HIGHBD_CHROMA_DOUBLE_SS_RIGHT_COLUMN_NEIGHBORS;
  } else if (ss_x || ss_y) {
    neighbors_snd = HIGHBD_CHROMA_SINGLE_SS_RIGHT_COLUMN_NEIGHBORS;
  } else {
    neighbors_snd = HIGHBD_CHROMA_NO_SS_RIGHT_COLUMN_NEIGHBORS;
  }

  vp9_highbd_apply_temporal_filter_chroma_8(
      y_src + blk_col, y_src_stride, y_pre + blk_col, y_pre_stride,
      u_src + uv_blk_col, v_src + uv_blk_col, uv_src_stride, u_pre + uv_blk_col,
      v_pre + uv_blk_col, uv_pre_stride, uv_width, uv_height, ss_x, ss_y,
      strength, u_accum + uv_blk_col, u_count + uv_blk_col,
      v_accum + uv_blk_col, v_count + uv_blk_col, y_dist + blk_col,
      u_dist + uv_blk_col, v_dist + uv_blk_col, neighbors_fst, neighbors_snd,
      top_weight, bottom_weight, NULL);
}

void vp9_highbd_apply_temporal_filter_sse4_1(
    const uint16_t *y_src, int y_src_stride, const uint16_t *y_pre,
    int y_pre_stride, const uint16_t *u_src, const uint16_t *v_src,
    int uv_src_stride, const uint16_t *u_pre, const uint16_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *const blk_fw,
    int use_whole_blk, uint32_t *y_accum, uint16_t *y_count, uint32_t *u_accum,
    uint16_t *u_count, uint32_t *v_accum, uint16_t *v_count) {
  const unsigned int chroma_height = block_height >> ss_y,
                     chroma_width = block_width >> ss_x;

  DECLARE_ALIGNED(16, uint32_t, y_dist[BH * DIST_STRIDE]) = { 0 };
  DECLARE_ALIGNED(16, uint32_t, u_dist[BH * DIST_STRIDE]) = { 0 };
  DECLARE_ALIGNED(16, uint32_t, v_dist[BH * DIST_STRIDE]) = { 0 };

  uint32_t *y_dist_ptr = y_dist + 1, *u_dist_ptr = u_dist + 1,
           *v_dist_ptr = v_dist + 1;
  const uint16_t *y_src_ptr = y_src, *u_src_ptr = u_src, *v_src_ptr = v_src;
  const uint16_t *y_pre_ptr = y_pre, *u_pre_ptr = u_pre, *v_pre_ptr = v_pre;

  unsigned int row, blk_col;

  assert(block_width <= BW && "block width too large");
  assert(block_height <= BH && "block height too large");
  assert(block_width % 16 == 0 && "block width must be multiple of 16");
  assert(block_height % 2 == 0 && "block height must be even");
  assert((ss_x == 0 || ss_x == 1) && (ss_y == 0 || ss_y == 1) &&
         "invalid chroma subsampling");
  assert(strength >= 4 && strength <= 14 &&
         "invalid adjusted temporal filter strength");
  assert(blk_fw[0] >= 0 && "filter weight must be positive");
  assert(
      (use_whole_blk || (blk_fw[1] >= 0 && blk_fw[2] >= 0 && blk_fw[3] >= 0)) &&
      "subblock filter weight must be positive");
  assert(blk_fw[0] <= 2 && "sublock filter weight must be less than 2");
  assert(
      (use_whole_blk || (blk_fw[1] <= 2 && blk_fw[2] <= 2 && blk_fw[3] <= 2)) &&
      "subblock filter weight must be less than 2");

  for (row = 0; row < block_height; row++) {
    for (blk_col = 0; blk_col < block_width; blk_col += 8) {
      highbd_store_dist_8(y_src_ptr + blk_col, y_pre_ptr + blk_col,
                          y_dist_ptr + blk_col);
    }
    y_src_ptr += y_src_stride;
    y_pre_ptr += y_pre_stride;
    y_dist_ptr += DIST_STRIDE;
  }

  for (row = 0; row < chroma_height; row++) {
    for (blk_col = 0; blk_col < chroma_width; blk_col += 8) {
      highbd_store_dist_8(u_src_ptr + blk_col, u_pre_ptr + blk_col,
                          u_dist_ptr + blk_col);
      highbd_store_dist_8(v_src_ptr + blk_col, v_pre_ptr + blk_col,
                          v_dist_ptr + blk_col);
    }

    u_src_ptr += uv_src_stride;
    u_pre_ptr += uv_pre_stride;
    u_dist_ptr += DIST_STRIDE;
    v_src_ptr += uv_src_stride;
    v_pre_ptr += uv_pre_stride;
    v_dist_ptr += DIST_STRIDE;
  }

  y_dist_ptr = y_dist + 1;
  u_dist_ptr = u_dist + 1;
  v_dist_ptr = v_dist + 1;

  vp9_highbd_apply_temporal_filter_luma(
      y_src, y_src_stride, y_pre, y_pre_stride, u_src, v_src, uv_src_stride,
      u_pre, v_pre, uv_pre_stride, block_width, block_height, ss_x, ss_y,
      strength, blk_fw, use_whole_blk, y_accum, y_count, y_dist_ptr, u_dist_ptr,
      v_dist_ptr);

  vp9_highbd_apply_temporal_filter_chroma(
      y_src, y_src_stride, y_pre, y_pre_stride, u_src, v_src, uv_src_stride,
      u_pre, v_pre, uv_pre_stride, block_width, block_height, ss_x, ss_y,
      strength, blk_fw, use_whole_blk, u_accum, u_count, v_accum, v_count,
      y_dist_ptr, u_dist_ptr, v_dist_ptr);
}
