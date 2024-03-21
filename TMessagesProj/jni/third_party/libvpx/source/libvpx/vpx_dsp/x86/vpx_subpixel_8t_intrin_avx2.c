/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <immintrin.h>
#include <stdio.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/x86/convolve.h"
#include "vpx_dsp/x86/convolve_avx2.h"
#include "vpx_dsp/x86/convolve_sse2.h"
#include "vpx_ports/mem.h"

DECLARE_ALIGNED(32, static const uint8_t,
                filt1_global_avx2[32]) = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
                                           6, 6, 7, 7, 8, 0, 1, 1, 2, 2, 3,
                                           3, 4, 4, 5, 5, 6, 6, 7, 7, 8 };

DECLARE_ALIGNED(32, static const uint8_t,
                filt2_global_avx2[32]) = { 2, 3, 3, 4, 4,  5, 5, 6, 6, 7, 7,
                                           8, 8, 9, 9, 10, 2, 3, 3, 4, 4, 5,
                                           5, 6, 6, 7, 7,  8, 8, 9, 9, 10 };

DECLARE_ALIGNED(32, static const uint8_t, filt3_global_avx2[32]) = {
  4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12,
  4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12
};

DECLARE_ALIGNED(32, static const uint8_t, filt4_global_avx2[32]) = {
  6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14,
  6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14
};

static INLINE void vpx_filter_block1d16_h8_x_avx2(
    const uint8_t *src_ptr, ptrdiff_t src_pixels_per_line, uint8_t *output_ptr,
    ptrdiff_t output_pitch, uint32_t output_height, const int16_t *filter,
    const int avg) {
  __m128i outReg1, outReg2;
  __m256i outReg32b1, outReg32b2;
  unsigned int i;
  ptrdiff_t src_stride, dst_stride;
  __m256i f[4], filt[4], s[4];

  shuffle_filter_avx2(filter, f);
  filt[0] = _mm256_load_si256((__m256i const *)filt1_global_avx2);
  filt[1] = _mm256_load_si256((__m256i const *)filt2_global_avx2);
  filt[2] = _mm256_load_si256((__m256i const *)filt3_global_avx2);
  filt[3] = _mm256_load_si256((__m256i const *)filt4_global_avx2);

  src_stride = src_pixels_per_line << 1;
  dst_stride = output_pitch << 1;
  for (i = output_height; i > 1; i -= 2) {
    __m256i srcReg;

    srcReg =
        _mm256_castsi128_si256(_mm_loadu_si128((const __m128i *)(src_ptr - 3)));
    srcReg = _mm256_inserti128_si256(
        srcReg,
        _mm_loadu_si128((const __m128i *)(src_ptr + src_pixels_per_line - 3)),
        1);

    s[0] = _mm256_shuffle_epi8(srcReg, filt[0]);
    s[1] = _mm256_shuffle_epi8(srcReg, filt[1]);
    s[2] = _mm256_shuffle_epi8(srcReg, filt[2]);
    s[3] = _mm256_shuffle_epi8(srcReg, filt[3]);
    outReg32b1 = convolve8_16_avx2(s, f);


    srcReg =
        _mm256_castsi128_si256(_mm_loadu_si128((const __m128i *)(src_ptr + 5)));
    srcReg = _mm256_inserti128_si256(
        srcReg,
        _mm_loadu_si128((const __m128i *)(src_ptr + src_pixels_per_line + 5)),
        1);

    s[0] = _mm256_shuffle_epi8(srcReg, filt[0]);
    s[1] = _mm256_shuffle_epi8(srcReg, filt[1]);
    s[2] = _mm256_shuffle_epi8(srcReg, filt[2]);
    s[3] = _mm256_shuffle_epi8(srcReg, filt[3]);
    outReg32b2 = convolve8_16_avx2(s, f);


    outReg32b1 = _mm256_packus_epi16(outReg32b1, outReg32b2);

    src_ptr += src_stride;

    outReg1 = _mm256_castsi256_si128(outReg32b1);
    outReg2 = _mm256_extractf128_si256(outReg32b1, 1);
    if (avg) {
      outReg1 = _mm_avg_epu8(outReg1, _mm_load_si128((__m128i *)output_ptr));
      outReg2 = _mm_avg_epu8(
          outReg2, _mm_load_si128((__m128i *)(output_ptr + output_pitch)));
    }

    _mm_store_si128((__m128i *)output_ptr, outReg1);

    _mm_store_si128((__m128i *)(output_ptr + output_pitch), outReg2);

    output_ptr += dst_stride;
  }


  if (i > 0) {
    __m128i srcReg;

    srcReg = _mm_loadu_si128((const __m128i *)(src_ptr - 3));

    s[0] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[0])));
    s[1] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[1])));
    s[2] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[2])));
    s[3] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[3])));
    outReg1 = convolve8_8_avx2(s, f);


    srcReg = _mm_loadu_si128((const __m128i *)(src_ptr + 5));

    s[0] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[0])));
    s[1] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[1])));
    s[2] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[2])));
    s[3] = _mm256_castsi128_si256(
        _mm_shuffle_epi8(srcReg, _mm256_castsi256_si128(filt[3])));
    outReg2 = convolve8_8_avx2(s, f);


    outReg1 = _mm_packus_epi16(outReg1, outReg2);

    if (avg) {
      outReg1 = _mm_avg_epu8(outReg1, _mm_load_si128((__m128i *)output_ptr));
    }

    _mm_store_si128((__m128i *)output_ptr, outReg1);
  }
}

static void vpx_filter_block1d16_h8_avx2(
    const uint8_t *src_ptr, ptrdiff_t src_stride, uint8_t *output_ptr,
    ptrdiff_t dst_stride, uint32_t output_height, const int16_t *filter) {
  vpx_filter_block1d16_h8_x_avx2(src_ptr, src_stride, output_ptr, dst_stride,
                                 output_height, filter, 0);
}

static void vpx_filter_block1d16_h8_avg_avx2(
    const uint8_t *src_ptr, ptrdiff_t src_stride, uint8_t *output_ptr,
    ptrdiff_t dst_stride, uint32_t output_height, const int16_t *filter) {
  vpx_filter_block1d16_h8_x_avx2(src_ptr, src_stride, output_ptr, dst_stride,
                                 output_height, filter, 1);
}

static INLINE void vpx_filter_block1d16_v8_x_avx2(
    const uint8_t *src_ptr, ptrdiff_t src_pitch, uint8_t *output_ptr,
    ptrdiff_t out_pitch, uint32_t output_height, const int16_t *filter,
    const int avg) {
  __m128i outReg1, outReg2;
  __m256i srcRegHead1;
  unsigned int i;
  ptrdiff_t src_stride, dst_stride;
  __m256i f[4], s1[4], s2[4];

  shuffle_filter_avx2(filter, f);

  src_stride = src_pitch << 1;
  dst_stride = out_pitch << 1;

  {
    __m128i s[6];
    __m256i s32b[6];

    s[0] = _mm_loadu_si128((const __m128i *)(src_ptr + 0 * src_pitch));
    s[1] = _mm_loadu_si128((const __m128i *)(src_ptr + 1 * src_pitch));
    s[2] = _mm_loadu_si128((const __m128i *)(src_ptr + 2 * src_pitch));
    s[3] = _mm_loadu_si128((const __m128i *)(src_ptr + 3 * src_pitch));
    s[4] = _mm_loadu_si128((const __m128i *)(src_ptr + 4 * src_pitch));
    s[5] = _mm_loadu_si128((const __m128i *)(src_ptr + 5 * src_pitch));
    srcRegHead1 = _mm256_castsi128_si256(
        _mm_loadu_si128((const __m128i *)(src_ptr + 6 * src_pitch)));

    s32b[0] = _mm256_inserti128_si256(_mm256_castsi128_si256(s[0]), s[1], 1);
    s32b[1] = _mm256_inserti128_si256(_mm256_castsi128_si256(s[1]), s[2], 1);
    s32b[2] = _mm256_inserti128_si256(_mm256_castsi128_si256(s[2]), s[3], 1);
    s32b[3] = _mm256_inserti128_si256(_mm256_castsi128_si256(s[3]), s[4], 1);
    s32b[4] = _mm256_inserti128_si256(_mm256_castsi128_si256(s[4]), s[5], 1);
    s32b[5] = _mm256_inserti128_si256(_mm256_castsi128_si256(s[5]),
                                      _mm256_castsi256_si128(srcRegHead1), 1);



    s1[0] = _mm256_unpacklo_epi8(s32b[0], s32b[1]);
    s2[0] = _mm256_unpackhi_epi8(s32b[0], s32b[1]);
    s1[1] = _mm256_unpacklo_epi8(s32b[2], s32b[3]);
    s2[1] = _mm256_unpackhi_epi8(s32b[2], s32b[3]);
    s1[2] = _mm256_unpacklo_epi8(s32b[4], s32b[5]);
    s2[2] = _mm256_unpackhi_epi8(s32b[4], s32b[5]);
  }

  for (i = output_height; i > 1; i -= 2) {
    __m256i srcRegHead2, srcRegHead3;


    srcRegHead2 = _mm256_castsi128_si256(
        _mm_loadu_si128((const __m128i *)(src_ptr + 7 * src_pitch)));
    srcRegHead1 = _mm256_inserti128_si256(
        srcRegHead1, _mm256_castsi256_si128(srcRegHead2), 1);
    srcRegHead3 = _mm256_castsi128_si256(
        _mm_loadu_si128((const __m128i *)(src_ptr + 8 * src_pitch)));
    srcRegHead2 = _mm256_inserti128_si256(
        srcRegHead2, _mm256_castsi256_si128(srcRegHead3), 1);



    s1[3] = _mm256_unpacklo_epi8(srcRegHead1, srcRegHead2);
    s2[3] = _mm256_unpackhi_epi8(srcRegHead1, srcRegHead2);

    s1[0] = convolve8_16_avx2(s1, f);
    s2[0] = convolve8_16_avx2(s2, f);


    s1[0] = _mm256_packus_epi16(s1[0], s2[0]);

    src_ptr += src_stride;

    outReg1 = _mm256_castsi256_si128(s1[0]);
    outReg2 = _mm256_extractf128_si256(s1[0], 1);
    if (avg) {
      outReg1 = _mm_avg_epu8(outReg1, _mm_load_si128((__m128i *)output_ptr));
      outReg2 = _mm_avg_epu8(
          outReg2, _mm_load_si128((__m128i *)(output_ptr + out_pitch)));
    }

    _mm_store_si128((__m128i *)output_ptr, outReg1);

    _mm_store_si128((__m128i *)(output_ptr + out_pitch), outReg2);

    output_ptr += dst_stride;

    s1[0] = s1[1];
    s2[0] = s2[1];
    s1[1] = s1[2];
    s2[1] = s2[2];
    s1[2] = s1[3];
    s2[2] = s2[3];
    srcRegHead1 = srcRegHead3;
  }


  if (i > 0) {

    const __m128i srcRegHead2 =
        _mm_loadu_si128((const __m128i *)(src_ptr + src_pitch * 7));

    s1[0] = _mm256_castsi128_si256(
        _mm_unpacklo_epi8(_mm256_castsi256_si128(srcRegHead1), srcRegHead2));
    s2[0] = _mm256_castsi128_si256(
        _mm_unpackhi_epi8(_mm256_castsi256_si128(srcRegHead1), srcRegHead2));

    outReg1 = convolve8_8_avx2(s1, f);
    outReg2 = convolve8_8_avx2(s2, f);


    outReg1 = _mm_packus_epi16(outReg1, outReg2);

    if (avg) {
      outReg1 = _mm_avg_epu8(outReg1, _mm_load_si128((__m128i *)output_ptr));
    }

    _mm_store_si128((__m128i *)output_ptr, outReg1);
  }
}

static void vpx_filter_block1d16_v8_avx2(const uint8_t *src_ptr,
                                         ptrdiff_t src_stride, uint8_t *dst_ptr,
                                         ptrdiff_t dst_stride, uint32_t height,
                                         const int16_t *filter) {
  vpx_filter_block1d16_v8_x_avx2(src_ptr, src_stride, dst_ptr, dst_stride,
                                 height, filter, 0);
}

static void vpx_filter_block1d16_v8_avg_avx2(
    const uint8_t *src_ptr, ptrdiff_t src_stride, uint8_t *dst_ptr,
    ptrdiff_t dst_stride, uint32_t height, const int16_t *filter) {
  vpx_filter_block1d16_v8_x_avx2(src_ptr, src_stride, dst_ptr, dst_stride,
                                 height, filter, 1);
}

static void vpx_filter_block1d16_h4_avx2(const uint8_t *src_ptr,
                                         ptrdiff_t src_stride, uint8_t *dst_ptr,
                                         ptrdiff_t dst_stride, uint32_t height,
                                         const int16_t *kernel) {













  __m128i kernel_reg;  // Kernel
  __m256i kernel_reg_256, kernel_reg_23,
      kernel_reg_45;                             // Segments of the kernel used
  const __m256i reg_32 = _mm256_set1_epi16(32);  // Used for rounding
  const ptrdiff_t unrolled_src_stride = src_stride << 1;
  const ptrdiff_t unrolled_dst_stride = dst_stride << 1;
  int h;

  __m256i src_reg, src_reg_shift_0, src_reg_shift_2;
  __m256i dst_first, dst_second;
  __m256i tmp_0, tmp_1;
  __m256i idx_shift_0 =
      _mm256_setr_epi8(0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 0, 1, 1,
                       2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8);
  __m256i idx_shift_2 =
      _mm256_setr_epi8(2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 2, 3, 3,
                       4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10);

  src_ptr -= 1;

  kernel_reg = _mm_loadu_si128((const __m128i *)kernel);
  kernel_reg = _mm_srai_epi16(kernel_reg, 1);
  kernel_reg = _mm_packs_epi16(kernel_reg, kernel_reg);
  kernel_reg_256 = _mm256_broadcastsi128_si256(kernel_reg);
  kernel_reg_23 =
      _mm256_shuffle_epi8(kernel_reg_256, _mm256_set1_epi16(0x0302u));
  kernel_reg_45 =
      _mm256_shuffle_epi8(kernel_reg_256, _mm256_set1_epi16(0x0504u));

  for (h = height; h >= 2; h -= 2) {

    src_reg = mm256_loadu2_si128(src_ptr, src_ptr + src_stride);
    src_reg_shift_0 = _mm256_shuffle_epi8(src_reg, idx_shift_0);
    src_reg_shift_2 = _mm256_shuffle_epi8(src_reg, idx_shift_2);

    tmp_0 = _mm256_maddubs_epi16(src_reg_shift_0, kernel_reg_23);
    tmp_1 = _mm256_maddubs_epi16(src_reg_shift_2, kernel_reg_45);
    dst_first = _mm256_adds_epi16(tmp_0, tmp_1);


    src_reg = mm256_loadu2_si128(src_ptr + 8, src_ptr + src_stride + 8);
    src_reg_shift_0 = _mm256_shuffle_epi8(src_reg, idx_shift_0);
    src_reg_shift_2 = _mm256_shuffle_epi8(src_reg, idx_shift_2);

    tmp_0 = _mm256_maddubs_epi16(src_reg_shift_0, kernel_reg_23);
    tmp_1 = _mm256_maddubs_epi16(src_reg_shift_2, kernel_reg_45);
    dst_second = _mm256_adds_epi16(tmp_0, tmp_1);

    dst_first = mm256_round_epi16(&dst_first, &reg_32, 6);
    dst_second = mm256_round_epi16(&dst_second, &reg_32, 6);

    dst_first = _mm256_packus_epi16(dst_first, dst_second);
    mm256_store2_si128((__m128i *)dst_ptr, (__m128i *)(dst_ptr + dst_stride),
                       &dst_first);

    src_ptr += unrolled_src_stride;
    dst_ptr += unrolled_dst_stride;
  }

  if (h > 0) {
    src_reg = _mm256_loadu_si256((const __m256i *)src_ptr);

    src_reg = _mm256_permute4x64_epi64(src_reg, 0x94);

    src_reg_shift_0 = _mm256_shuffle_epi8(src_reg, idx_shift_0);
    src_reg_shift_2 = _mm256_shuffle_epi8(src_reg, idx_shift_2);

    tmp_0 = _mm256_maddubs_epi16(src_reg_shift_0, kernel_reg_23);
    tmp_1 = _mm256_maddubs_epi16(src_reg_shift_2, kernel_reg_45);
    dst_first = _mm256_adds_epi16(tmp_0, tmp_1);

    dst_first = mm256_round_epi16(&dst_first, &reg_32, 6);

    dst_first = _mm256_packus_epi16(dst_first, dst_first);
    dst_first = _mm256_permute4x64_epi64(dst_first, 0x8);

    _mm_store_si128((__m128i *)dst_ptr, _mm256_castsi256_si128(dst_first));
  }
}

static void vpx_filter_block1d16_v4_avx2(const uint8_t *src_ptr,
                                         ptrdiff_t src_stride, uint8_t *dst_ptr,
                                         ptrdiff_t dst_stride, uint32_t height,
                                         const int16_t *kernel) {






  __m256i src_reg_1, src_reg_2, src_reg_3;

  __m256i src_reg_m10, src_reg_01, src_reg_12, src_reg_23;
  __m256i src_reg_m1001_lo, src_reg_m1001_hi, src_reg_1223_lo, src_reg_1223_hi;

  __m128i kernel_reg;  // Kernel
  __m256i kernel_reg_256, kernel_reg_23,
      kernel_reg_45;  // Segments of the kernel used

  __m256i res_reg_m1001_lo, res_reg_1223_lo, res_reg_m1001_hi, res_reg_1223_hi;
  __m256i res_reg, res_reg_lo, res_reg_hi;

  const __m256i reg_32 = _mm256_set1_epi16(32);  // Used for rounding

  const ptrdiff_t src_stride_unrolled = src_stride << 1;
  const ptrdiff_t dst_stride_unrolled = dst_stride << 1;
  int h;

  kernel_reg = _mm_loadu_si128((const __m128i *)kernel);
  kernel_reg = _mm_srai_epi16(kernel_reg, 1);
  kernel_reg = _mm_packs_epi16(kernel_reg, kernel_reg);
  kernel_reg_256 = _mm256_broadcastsi128_si256(kernel_reg);
  kernel_reg_23 =
      _mm256_shuffle_epi8(kernel_reg_256, _mm256_set1_epi16(0x0302u));
  kernel_reg_45 =
      _mm256_shuffle_epi8(kernel_reg_256, _mm256_set1_epi16(0x0504u));

  src_reg_m10 = mm256_loadu2_si128((const __m128i *)src_ptr,
                                   (const __m128i *)(src_ptr + src_stride));

  src_reg_1 = _mm256_castsi128_si256(
      _mm_loadu_si128((const __m128i *)(src_ptr + src_stride * 2)));
  src_reg_01 = _mm256_permute2x128_si256(src_reg_m10, src_reg_1, 0x21);

  src_reg_m1001_lo = _mm256_unpacklo_epi8(src_reg_m10, src_reg_01);
  src_reg_m1001_hi = _mm256_unpackhi_epi8(src_reg_m10, src_reg_01);

  for (h = height; h > 1; h -= 2) {
    src_reg_2 = _mm256_castsi128_si256(
        _mm_loadu_si128((const __m128i *)(src_ptr + src_stride * 3)));

    src_reg_12 = _mm256_inserti128_si256(src_reg_1,
                                         _mm256_castsi256_si128(src_reg_2), 1);

    src_reg_3 = _mm256_castsi128_si256(
        _mm_loadu_si128((const __m128i *)(src_ptr + src_stride * 4)));

    src_reg_23 = _mm256_inserti128_si256(src_reg_2,
                                         _mm256_castsi256_si128(src_reg_3), 1);

    src_reg_1223_lo = _mm256_unpacklo_epi8(src_reg_12, src_reg_23);
    src_reg_1223_hi = _mm256_unpackhi_epi8(src_reg_12, src_reg_23);

    res_reg_m1001_lo = _mm256_maddubs_epi16(src_reg_m1001_lo, kernel_reg_23);
    res_reg_1223_lo = _mm256_maddubs_epi16(src_reg_1223_lo, kernel_reg_45);
    res_reg_lo = _mm256_adds_epi16(res_reg_m1001_lo, res_reg_1223_lo);

    res_reg_m1001_hi = _mm256_maddubs_epi16(src_reg_m1001_hi, kernel_reg_23);
    res_reg_1223_hi = _mm256_maddubs_epi16(src_reg_1223_hi, kernel_reg_45);
    res_reg_hi = _mm256_adds_epi16(res_reg_m1001_hi, res_reg_1223_hi);

    res_reg_lo = mm256_round_epi16(&res_reg_lo, &reg_32, 6);
    res_reg_hi = mm256_round_epi16(&res_reg_hi, &reg_32, 6);

    res_reg = _mm256_packus_epi16(res_reg_lo, res_reg_hi);

    mm256_store2_si128((__m128i *)dst_ptr, (__m128i *)(dst_ptr + dst_stride),
                       &res_reg);

    src_ptr += src_stride_unrolled;
    dst_ptr += dst_stride_unrolled;

    src_reg_m1001_lo = src_reg_1223_lo;
    src_reg_m1001_hi = src_reg_1223_hi;
    src_reg_1 = src_reg_3;
  }
}

static void vpx_filter_block1d8_h4_avx2(const uint8_t *src_ptr,
                                        ptrdiff_t src_stride, uint8_t *dst_ptr,
                                        ptrdiff_t dst_stride, uint32_t height,
                                        const int16_t *kernel) {













  __m128i kernel_reg_128;  // Kernel
  __m256i kernel_reg, kernel_reg_23,
      kernel_reg_45;                             // Segments of the kernel used
  const __m256i reg_32 = _mm256_set1_epi16(32);  // Used for rounding
  const ptrdiff_t unrolled_src_stride = src_stride << 1;
  const ptrdiff_t unrolled_dst_stride = dst_stride << 1;
  int h;

  __m256i src_reg, src_reg_shift_0, src_reg_shift_2;
  __m256i dst_reg;
  __m256i tmp_0, tmp_1;
  __m256i idx_shift_0 =
      _mm256_setr_epi8(0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 0, 1, 1,
                       2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8);
  __m256i idx_shift_2 =
      _mm256_setr_epi8(2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 2, 3, 3,
                       4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10);

  src_ptr -= 1;

  kernel_reg_128 = _mm_loadu_si128((const __m128i *)kernel);
  kernel_reg_128 = _mm_srai_epi16(kernel_reg_128, 1);
  kernel_reg_128 = _mm_packs_epi16(kernel_reg_128, kernel_reg_128);
  kernel_reg = _mm256_broadcastsi128_si256(kernel_reg_128);
  kernel_reg_23 = _mm256_shuffle_epi8(kernel_reg, _mm256_set1_epi16(0x0302u));
  kernel_reg_45 = _mm256_shuffle_epi8(kernel_reg, _mm256_set1_epi16(0x0504u));

  for (h = height; h >= 2; h -= 2) {

    src_reg = mm256_loadu2_si128(src_ptr, src_ptr + src_stride);
    src_reg_shift_0 = _mm256_shuffle_epi8(src_reg, idx_shift_0);
    src_reg_shift_2 = _mm256_shuffle_epi8(src_reg, idx_shift_2);

    tmp_0 = _mm256_maddubs_epi16(src_reg_shift_0, kernel_reg_23);
    tmp_1 = _mm256_maddubs_epi16(src_reg_shift_2, kernel_reg_45);
    dst_reg = _mm256_adds_epi16(tmp_0, tmp_1);

    dst_reg = mm256_round_epi16(&dst_reg, &reg_32, 6);

    dst_reg = _mm256_packus_epi16(dst_reg, dst_reg);
    mm256_storeu2_epi64((__m128i *)dst_ptr, (__m128i *)(dst_ptr + dst_stride),
                        &dst_reg);

    src_ptr += unrolled_src_stride;
    dst_ptr += unrolled_dst_stride;
  }

  if (h > 0) {
    __m128i src_reg = _mm_loadu_si128((const __m128i *)src_ptr);
    __m128i dst_reg;
    const __m128i reg_32 = _mm_set1_epi16(32);  // Used for rounding
    __m128i tmp_0, tmp_1;

    __m128i src_reg_shift_0 =
        _mm_shuffle_epi8(src_reg, _mm256_castsi256_si128(idx_shift_0));
    __m128i src_reg_shift_2 =
        _mm_shuffle_epi8(src_reg, _mm256_castsi256_si128(idx_shift_2));

    tmp_0 = _mm_maddubs_epi16(src_reg_shift_0,
                              _mm256_castsi256_si128(kernel_reg_23));
    tmp_1 = _mm_maddubs_epi16(src_reg_shift_2,
                              _mm256_castsi256_si128(kernel_reg_45));
    dst_reg = _mm_adds_epi16(tmp_0, tmp_1);

    dst_reg = mm_round_epi16_sse2(&dst_reg, &reg_32, 6);

    dst_reg = _mm_packus_epi16(dst_reg, _mm_setzero_si128());

    _mm_storel_epi64((__m128i *)dst_ptr, dst_reg);
  }
}

static void vpx_filter_block1d8_v4_avx2(const uint8_t *src_ptr,
                                        ptrdiff_t src_stride, uint8_t *dst_ptr,
                                        ptrdiff_t dst_stride, uint32_t height,
                                        const int16_t *kernel) {






  __m256i src_reg_1, src_reg_2, src_reg_3;

  __m256i src_reg_m10, src_reg_01, src_reg_12, src_reg_23;
  __m256i src_reg_m1001, src_reg_1223;

  __m128i kernel_reg_128;  // Kernel
  __m256i kernel_reg, kernel_reg_23,
      kernel_reg_45;  // Segments of the kernel used

  __m256i res_reg_m1001, res_reg_1223;
  __m256i res_reg;

  const __m256i reg_32 = _mm256_set1_epi16(32);  // Used for rounding

  const ptrdiff_t src_stride_unrolled = src_stride << 1;
  const ptrdiff_t dst_stride_unrolled = dst_stride << 1;
  int h;

  kernel_reg_128 = _mm_loadu_si128((const __m128i *)kernel);
  kernel_reg_128 = _mm_srai_epi16(kernel_reg_128, 1);
  kernel_reg_128 = _mm_packs_epi16(kernel_reg_128, kernel_reg_128);
  kernel_reg = _mm256_broadcastsi128_si256(kernel_reg_128);
  kernel_reg_23 = _mm256_shuffle_epi8(kernel_reg, _mm256_set1_epi16(0x0302u));
  kernel_reg_45 = _mm256_shuffle_epi8(kernel_reg, _mm256_set1_epi16(0x0504u));

  src_reg_m10 = mm256_loadu2_epi64((const __m128i *)src_ptr,
                                   (const __m128i *)(src_ptr + src_stride));

  src_reg_1 = _mm256_castsi128_si256(
      _mm_loadu_si128((const __m128i *)(src_ptr + src_stride * 2)));
  src_reg_01 = _mm256_permute2x128_si256(src_reg_m10, src_reg_1, 0x21);

  src_reg_m1001 = _mm256_unpacklo_epi8(src_reg_m10, src_reg_01);

  for (h = height; h > 1; h -= 2) {
    src_reg_2 = _mm256_castsi128_si256(
        _mm_loadl_epi64((const __m128i *)(src_ptr + src_stride * 3)));

    src_reg_12 = _mm256_inserti128_si256(src_reg_1,
                                         _mm256_castsi256_si128(src_reg_2), 1);

    src_reg_3 = _mm256_castsi128_si256(
        _mm_loadl_epi64((const __m128i *)(src_ptr + src_stride * 4)));

    src_reg_23 = _mm256_inserti128_si256(src_reg_2,
                                         _mm256_castsi256_si128(src_reg_3), 1);

    src_reg_1223 = _mm256_unpacklo_epi8(src_reg_12, src_reg_23);

    res_reg_m1001 = _mm256_maddubs_epi16(src_reg_m1001, kernel_reg_23);
    res_reg_1223 = _mm256_maddubs_epi16(src_reg_1223, kernel_reg_45);
    res_reg = _mm256_adds_epi16(res_reg_m1001, res_reg_1223);

    res_reg = mm256_round_epi16(&res_reg, &reg_32, 6);

    res_reg = _mm256_packus_epi16(res_reg, res_reg);

    mm256_storeu2_epi64((__m128i *)dst_ptr, (__m128i *)(dst_ptr + dst_stride),
                        &res_reg);

    src_ptr += src_stride_unrolled;
    dst_ptr += dst_stride_unrolled;

    src_reg_m1001 = src_reg_1223;
    src_reg_1 = src_reg_3;
  }
}

static void vpx_filter_block1d4_h4_avx2(const uint8_t *src_ptr,
                                        ptrdiff_t src_stride, uint8_t *dst_ptr,
                                        ptrdiff_t dst_stride, uint32_t height,
                                        const int16_t *kernel) {









  __m128i kernel_reg_128;  // Kernel
  __m256i kernel_reg;
  const __m256i reg_32 = _mm256_set1_epi16(32);  // Used for rounding
  int h;
  const ptrdiff_t unrolled_src_stride = src_stride << 1;
  const ptrdiff_t unrolled_dst_stride = dst_stride << 1;

  __m256i src_reg, src_reg_shuf;
  __m256i dst;
  __m256i shuf_idx =
      _mm256_setr_epi8(0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6, 0, 1, 2,
                       3, 1, 2, 3, 4, 2, 3, 4, 5, 3, 4, 5, 6);

  src_ptr -= 1;

  kernel_reg_128 = _mm_loadu_si128((const __m128i *)kernel);
  kernel_reg_128 = _mm_srai_epi16(kernel_reg_128, 1);
  kernel_reg_128 = _mm_packs_epi16(kernel_reg_128, kernel_reg_128);
  kernel_reg = _mm256_broadcastsi128_si256(kernel_reg_128);
  kernel_reg = _mm256_shuffle_epi8(kernel_reg, _mm256_set1_epi32(0x05040302u));

  for (h = height; h > 1; h -= 2) {

    src_reg = mm256_loadu2_epi64((const __m128i *)src_ptr,
                                 (const __m128i *)(src_ptr + src_stride));
    src_reg_shuf = _mm256_shuffle_epi8(src_reg, shuf_idx);

    dst = _mm256_maddubs_epi16(src_reg_shuf, kernel_reg);
    dst = _mm256_hadds_epi16(dst, _mm256_setzero_si256());

    dst = mm256_round_epi16(&dst, &reg_32, 6);

    dst = _mm256_packus_epi16(dst, _mm256_setzero_si256());

    mm256_storeu2_epi32((__m128i *const)dst_ptr,
                        (__m128i *const)(dst_ptr + dst_stride), &dst);

    src_ptr += unrolled_src_stride;
    dst_ptr += unrolled_dst_stride;
  }

  if (h > 0) {

    const __m128i reg_32 = _mm_set1_epi16(32);  // Used for rounding
    __m128i src_reg = _mm_loadl_epi64((const __m128i *)src_ptr);
    __m128i src_reg_shuf =
        _mm_shuffle_epi8(src_reg, _mm256_castsi256_si128(shuf_idx));

    __m128i dst =
        _mm_maddubs_epi16(src_reg_shuf, _mm256_castsi256_si128(kernel_reg));
    dst = _mm_hadds_epi16(dst, _mm_setzero_si128());

    dst = mm_round_epi16_sse2(&dst, &reg_32, 6);

    dst = _mm_packus_epi16(dst, _mm_setzero_si128());
    *((uint32_t *)(dst_ptr)) = _mm_cvtsi128_si32(dst);
  }
}

static void vpx_filter_block1d4_v4_avx2(const uint8_t *src_ptr,
                                        ptrdiff_t src_stride, uint8_t *dst_ptr,
                                        ptrdiff_t dst_stride, uint32_t height,
                                        const int16_t *kernel) {






  __m256i src_reg_1, src_reg_2, src_reg_3;

  __m256i src_reg_m10, src_reg_01, src_reg_12, src_reg_23;
  __m256i src_reg_m1001, src_reg_1223, src_reg_m1012_1023;

  __m128i kernel_reg_128;  // Kernel
  __m256i kernel_reg;

  __m256i res_reg;

  const __m256i reg_32 = _mm256_set1_epi16(32);  // Used for rounding

  const ptrdiff_t src_stride_unrolled = src_stride << 1;
  const ptrdiff_t dst_stride_unrolled = dst_stride << 1;
  int h;

  kernel_reg_128 = _mm_loadu_si128((const __m128i *)kernel);
  kernel_reg_128 = _mm_srai_epi16(kernel_reg_128, 1);
  kernel_reg_128 = _mm_packs_epi16(kernel_reg_128, kernel_reg_128);
  kernel_reg = _mm256_broadcastsi128_si256(kernel_reg_128);
  kernel_reg = _mm256_shuffle_epi8(kernel_reg, _mm256_set1_epi32(0x05040302u));

  src_reg_m10 = mm256_loadu2_si128((const __m128i *)src_ptr,
                                   (const __m128i *)(src_ptr + src_stride));

  src_reg_1 = _mm256_castsi128_si256(
      _mm_loadu_si128((const __m128i *)(src_ptr + src_stride * 2)));
  src_reg_01 = _mm256_permute2x128_si256(src_reg_m10, src_reg_1, 0x21);

  src_reg_m1001 = _mm256_unpacklo_epi8(src_reg_m10, src_reg_01);

  for (h = height; h > 1; h -= 2) {
    src_reg_2 = _mm256_castsi128_si256(
        _mm_loadl_epi64((const __m128i *)(src_ptr + src_stride * 3)));

    src_reg_12 = _mm256_inserti128_si256(src_reg_1,
                                         _mm256_castsi256_si128(src_reg_2), 1);

    src_reg_3 = _mm256_castsi128_si256(
        _mm_loadl_epi64((const __m128i *)(src_ptr + src_stride * 4)));

    src_reg_23 = _mm256_inserti128_si256(src_reg_2,
                                         _mm256_castsi256_si128(src_reg_3), 1);

    src_reg_1223 = _mm256_unpacklo_epi8(src_reg_12, src_reg_23);

    src_reg_m1012_1023 = _mm256_unpacklo_epi16(src_reg_m1001, src_reg_1223);

    res_reg = _mm256_maddubs_epi16(src_reg_m1012_1023, kernel_reg);
    res_reg = _mm256_hadds_epi16(res_reg, _mm256_setzero_si256());

    res_reg = mm256_round_epi16(&res_reg, &reg_32, 6);

    res_reg = _mm256_packus_epi16(res_reg, res_reg);

    mm256_storeu2_epi32((__m128i *)dst_ptr, (__m128i *)(dst_ptr + dst_stride),
                        &res_reg);

    src_ptr += src_stride_unrolled;
    dst_ptr += dst_stride_unrolled;

    src_reg_m1001 = src_reg_1223;
    src_reg_1 = src_reg_3;
  }
}

#if HAVE_AVX2 && HAVE_SSSE3
filter8_1dfunction vpx_filter_block1d4_v8_ssse3;
#if VPX_ARCH_X86_64
filter8_1dfunction vpx_filter_block1d8_v8_intrin_ssse3;
filter8_1dfunction vpx_filter_block1d8_h8_intrin_ssse3;
filter8_1dfunction vpx_filter_block1d4_h8_intrin_ssse3;
#define vpx_filter_block1d8_v8_avx2 vpx_filter_block1d8_v8_intrin_ssse3
#define vpx_filter_block1d8_h8_avx2 vpx_filter_block1d8_h8_intrin_ssse3
#define vpx_filter_block1d4_h8_avx2 vpx_filter_block1d4_h8_intrin_ssse3
#else  // VPX_ARCH_X86
filter8_1dfunction vpx_filter_block1d8_v8_ssse3;
filter8_1dfunction vpx_filter_block1d8_h8_ssse3;
filter8_1dfunction vpx_filter_block1d4_h8_ssse3;
#define vpx_filter_block1d8_v8_avx2 vpx_filter_block1d8_v8_ssse3
#define vpx_filter_block1d8_h8_avx2 vpx_filter_block1d8_h8_ssse3
#define vpx_filter_block1d4_h8_avx2 vpx_filter_block1d4_h8_ssse3
#endif  // VPX_ARCH_X86_64
filter8_1dfunction vpx_filter_block1d8_v8_avg_ssse3;
filter8_1dfunction vpx_filter_block1d8_h8_avg_ssse3;
filter8_1dfunction vpx_filter_block1d4_v8_avg_ssse3;
filter8_1dfunction vpx_filter_block1d4_h8_avg_ssse3;
#define vpx_filter_block1d8_v8_avg_avx2 vpx_filter_block1d8_v8_avg_ssse3
#define vpx_filter_block1d8_h8_avg_avx2 vpx_filter_block1d8_h8_avg_ssse3
#define vpx_filter_block1d4_v8_avg_avx2 vpx_filter_block1d4_v8_avg_ssse3
#define vpx_filter_block1d4_h8_avg_avx2 vpx_filter_block1d4_h8_avg_ssse3
filter8_1dfunction vpx_filter_block1d16_v2_ssse3;
filter8_1dfunction vpx_filter_block1d16_h2_ssse3;
filter8_1dfunction vpx_filter_block1d8_v2_ssse3;
filter8_1dfunction vpx_filter_block1d8_h2_ssse3;
filter8_1dfunction vpx_filter_block1d4_v2_ssse3;
filter8_1dfunction vpx_filter_block1d4_h2_ssse3;
#define vpx_filter_block1d4_v8_avx2 vpx_filter_block1d4_v8_ssse3
#define vpx_filter_block1d16_v2_avx2 vpx_filter_block1d16_v2_ssse3
#define vpx_filter_block1d16_h2_avx2 vpx_filter_block1d16_h2_ssse3
#define vpx_filter_block1d8_v2_avx2 vpx_filter_block1d8_v2_ssse3
#define vpx_filter_block1d8_h2_avx2 vpx_filter_block1d8_h2_ssse3
#define vpx_filter_block1d4_v2_avx2 vpx_filter_block1d4_v2_ssse3
#define vpx_filter_block1d4_h2_avx2 vpx_filter_block1d4_h2_ssse3
filter8_1dfunction vpx_filter_block1d16_v2_avg_ssse3;
filter8_1dfunction vpx_filter_block1d16_h2_avg_ssse3;
filter8_1dfunction vpx_filter_block1d8_v2_avg_ssse3;
filter8_1dfunction vpx_filter_block1d8_h2_avg_ssse3;
filter8_1dfunction vpx_filter_block1d4_v2_avg_ssse3;
filter8_1dfunction vpx_filter_block1d4_h2_avg_ssse3;
#define vpx_filter_block1d16_v2_avg_avx2 vpx_filter_block1d16_v2_avg_ssse3
#define vpx_filter_block1d16_h2_avg_avx2 vpx_filter_block1d16_h2_avg_ssse3
#define vpx_filter_block1d8_v2_avg_avx2 vpx_filter_block1d8_v2_avg_ssse3
#define vpx_filter_block1d8_h2_avg_avx2 vpx_filter_block1d8_h2_avg_ssse3
#define vpx_filter_block1d4_v2_avg_avx2 vpx_filter_block1d4_v2_avg_ssse3
#define vpx_filter_block1d4_h2_avg_avx2 vpx_filter_block1d4_h2_avg_ssse3

#define vpx_filter_block1d16_v4_avg_avx2 vpx_filter_block1d16_v8_avg_avx2
#define vpx_filter_block1d16_h4_avg_avx2 vpx_filter_block1d16_h8_avg_avx2
#define vpx_filter_block1d8_v4_avg_avx2 vpx_filter_block1d8_v8_avg_avx2
#define vpx_filter_block1d8_h4_avg_avx2 vpx_filter_block1d8_h8_avg_avx2
#define vpx_filter_block1d4_v4_avg_avx2 vpx_filter_block1d4_v8_avg_avx2
#define vpx_filter_block1d4_h4_avg_avx2 vpx_filter_block1d4_h8_avg_avx2
// void vpx_convolve8_horiz_avx2(const uint8_t *src, ptrdiff_t src_stride,
//                                uint8_t *dst, ptrdiff_t dst_stride,
//                                const InterpKernel *filter, int x0_q4,
//                                int32_t x_step_q4, int y0_q4, int y_step_q4,
//                                int w, int h);
// void vpx_convolve8_vert_avx2(const uint8_t *src, ptrdiff_t src_stride,
//                               uint8_t *dst, ptrdiff_t dst_stride,
//                               const InterpKernel *filter, int x0_q4,
//                               int32_t x_step_q4, int y0_q4, int y_step_q4,
//                               int w, int h);
// void vpx_convolve8_avg_horiz_avx2(const uint8_t *src, ptrdiff_t src_stride,
//                                    uint8_t *dst, ptrdiff_t dst_stride,
//                                    const InterpKernel *filter, int x0_q4,
//                                    int32_t x_step_q4, int y0_q4,
//                                    int y_step_q4, int w, int h);
// void vpx_convolve8_avg_vert_avx2(const uint8_t *src, ptrdiff_t src_stride,
//                                   uint8_t *dst, ptrdiff_t dst_stride,
//                                   const InterpKernel *filter, int x0_q4,
//                                   int32_t x_step_q4, int y0_q4,
//                                   int y_step_q4, int w, int h);
FUN_CONV_1D(horiz, x0_q4, x_step_q4, h, src, , avx2, 0);
FUN_CONV_1D(vert, y0_q4, y_step_q4, v, src - src_stride * (num_taps / 2 - 1), ,
            avx2, 0);
FUN_CONV_1D(avg_horiz, x0_q4, x_step_q4, h, src, avg_, avx2, 1);
FUN_CONV_1D(avg_vert, y0_q4, y_step_q4, v,
            src - src_stride * (num_taps / 2 - 1), avg_, avx2, 1);

//                          uint8_t *dst, ptrdiff_t dst_stride,
//                          const InterpKernel *filter, int x0_q4,
//                          int32_t x_step_q4, int y0_q4, int y_step_q4,
//                          int w, int h);
// void vpx_convolve8_avg_avx2(const uint8_t *src, ptrdiff_t src_stride,
//                              uint8_t *dst, ptrdiff_t dst_stride,
//                              const InterpKernel *filter, int x0_q4,
//                              int32_t x_step_q4, int y0_q4, int y_step_q4,
//                              int w, int h);
FUN_CONV_2D(, avx2, 0);
FUN_CONV_2D(avg_, avx2, 1);
#endif  // HAVE_AX2 && HAVE_SSSE3
