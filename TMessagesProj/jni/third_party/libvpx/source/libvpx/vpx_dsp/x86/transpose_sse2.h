/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_DSP_X86_TRANSPOSE_SSE2_H_
#define VPX_VPX_DSP_X86_TRANSPOSE_SSE2_H_

#include <emmintrin.h>  // SSE2

#include "./vpx_config.h"

static INLINE __m128i transpose_8bit_4x4(const __m128i *const in) {








  const __m128i a0 = _mm_unpacklo_epi8(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi8(in[2], in[3]);


  return _mm_unpacklo_epi16(a0, a1);
}

static INLINE void transpose_8bit_8x8(const __m128i *const in,
                                      __m128i *const out) {














  const __m128i a0 = _mm_unpacklo_epi8(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi8(in[2], in[3]);
  const __m128i a2 = _mm_unpacklo_epi8(in[4], in[5]);
  const __m128i a3 = _mm_unpacklo_epi8(in[6], in[7]);





  const __m128i b0 = _mm_unpacklo_epi16(a0, a1);
  const __m128i b1 = _mm_unpackhi_epi16(a0, a1);
  const __m128i b2 = _mm_unpacklo_epi16(a2, a3);
  const __m128i b3 = _mm_unpackhi_epi16(a2, a3);





  const __m128i c0 = _mm_unpacklo_epi32(b0, b2);
  const __m128i c1 = _mm_unpackhi_epi32(b0, b2);
  const __m128i c2 = _mm_unpacklo_epi32(b1, b3);
  const __m128i c3 = _mm_unpackhi_epi32(b1, b3);









  out[0] = _mm_unpacklo_epi64(c0, c0);
  out[1] = _mm_unpackhi_epi64(c0, c0);
  out[2] = _mm_unpacklo_epi64(c1, c1);
  out[3] = _mm_unpackhi_epi64(c1, c1);
  out[4] = _mm_unpacklo_epi64(c2, c2);
  out[5] = _mm_unpackhi_epi64(c2, c2);
  out[6] = _mm_unpacklo_epi64(c3, c3);
  out[7] = _mm_unpackhi_epi64(c3, c3);
}

static INLINE void transpose_16bit_4x4(const __m128i *const in,
                                       __m128i *const out) {








  const __m128i a0 = _mm_unpacklo_epi16(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi16(in[2], in[3]);



  out[0] = _mm_unpacklo_epi32(a0, a1);
  out[1] = _mm_unpackhi_epi32(a0, a1);
}

static INLINE void transpose_16bit_4x8(const __m128i *const in,
                                       __m128i *const out) {














  const __m128i a0 = _mm_unpacklo_epi16(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi16(in[2], in[3]);
  const __m128i a2 = _mm_unpacklo_epi16(in[4], in[5]);
  const __m128i a3 = _mm_unpacklo_epi16(in[6], in[7]);





  const __m128i b0 = _mm_unpacklo_epi32(a0, a1);
  const __m128i b1 = _mm_unpacklo_epi32(a2, a3);
  const __m128i b2 = _mm_unpackhi_epi32(a0, a1);
  const __m128i b3 = _mm_unpackhi_epi32(a2, a3);





  out[0] = _mm_unpacklo_epi64(b0, b1);
  out[1] = _mm_unpackhi_epi64(b0, b1);
  out[2] = _mm_unpacklo_epi64(b2, b3);
  out[3] = _mm_unpackhi_epi64(b2, b3);
}

static INLINE void transpose_16bit_8x8(const __m128i *const in,
                                       __m128i *const out) {


















  const __m128i a0 = _mm_unpacklo_epi16(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi16(in[2], in[3]);
  const __m128i a2 = _mm_unpacklo_epi16(in[4], in[5]);
  const __m128i a3 = _mm_unpacklo_epi16(in[6], in[7]);
  const __m128i a4 = _mm_unpackhi_epi16(in[0], in[1]);
  const __m128i a5 = _mm_unpackhi_epi16(in[2], in[3]);
  const __m128i a6 = _mm_unpackhi_epi16(in[4], in[5]);
  const __m128i a7 = _mm_unpackhi_epi16(in[6], in[7]);









  const __m128i b0 = _mm_unpacklo_epi32(a0, a1);
  const __m128i b1 = _mm_unpacklo_epi32(a2, a3);
  const __m128i b2 = _mm_unpacklo_epi32(a4, a5);
  const __m128i b3 = _mm_unpacklo_epi32(a6, a7);
  const __m128i b4 = _mm_unpackhi_epi32(a0, a1);
  const __m128i b5 = _mm_unpackhi_epi32(a2, a3);
  const __m128i b6 = _mm_unpackhi_epi32(a4, a5);
  const __m128i b7 = _mm_unpackhi_epi32(a6, a7);









  out[0] = _mm_unpacklo_epi64(b0, b1);
  out[1] = _mm_unpackhi_epi64(b0, b1);
  out[2] = _mm_unpacklo_epi64(b4, b5);
  out[3] = _mm_unpackhi_epi64(b4, b5);
  out[4] = _mm_unpacklo_epi64(b2, b3);
  out[5] = _mm_unpackhi_epi64(b2, b3);
  out[6] = _mm_unpacklo_epi64(b6, b7);
  out[7] = _mm_unpackhi_epi64(b6, b7);
}

static INLINE void transpose_16bit_16x16(__m128i *const left,
                                         __m128i *const right) {
  __m128i tbuf[8];
  transpose_16bit_8x8(left, left);
  transpose_16bit_8x8(right, tbuf);
  transpose_16bit_8x8(left + 8, right);
  transpose_16bit_8x8(right + 8, right + 8);

  left[8] = tbuf[0];
  left[9] = tbuf[1];
  left[10] = tbuf[2];
  left[11] = tbuf[3];
  left[12] = tbuf[4];
  left[13] = tbuf[5];
  left[14] = tbuf[6];
  left[15] = tbuf[7];
}

static INLINE void transpose_32bit_4x4(const __m128i *const in,
                                       __m128i *const out) {











  const __m128i a0 = _mm_unpacklo_epi32(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi32(in[2], in[3]);
  const __m128i a2 = _mm_unpackhi_epi32(in[0], in[1]);
  const __m128i a3 = _mm_unpackhi_epi32(in[2], in[3]);





  out[0] = _mm_unpacklo_epi64(a0, a1);
  out[1] = _mm_unpackhi_epi64(a0, a1);
  out[2] = _mm_unpacklo_epi64(a2, a3);
  out[3] = _mm_unpackhi_epi64(a2, a3);
}

static INLINE void transpose_32bit_4x4x2(const __m128i *const in,
                                         __m128i *const out) {


















  const __m128i a0 = _mm_unpacklo_epi32(in[0], in[1]);
  const __m128i a1 = _mm_unpacklo_epi32(in[2], in[3]);
  const __m128i a2 = _mm_unpackhi_epi32(in[0], in[1]);
  const __m128i a3 = _mm_unpackhi_epi32(in[2], in[3]);
  const __m128i a4 = _mm_unpacklo_epi32(in[4], in[5]);
  const __m128i a5 = _mm_unpacklo_epi32(in[6], in[7]);
  const __m128i a6 = _mm_unpackhi_epi32(in[4], in[5]);
  const __m128i a7 = _mm_unpackhi_epi32(in[6], in[7]);









  out[0] = _mm_unpacklo_epi64(a0, a1);
  out[1] = _mm_unpackhi_epi64(a0, a1);
  out[2] = _mm_unpacklo_epi64(a2, a3);
  out[3] = _mm_unpackhi_epi64(a2, a3);
  out[4] = _mm_unpacklo_epi64(a4, a5);
  out[5] = _mm_unpackhi_epi64(a4, a5);
  out[6] = _mm_unpacklo_epi64(a6, a7);
  out[7] = _mm_unpackhi_epi64(a6, a7);
}

static INLINE void transpose_32bit_8x4(const __m128i *const in,
                                       __m128i *const out) {


















  const __m128i a0 = _mm_unpacklo_epi32(in[0], in[2]);
  const __m128i a1 = _mm_unpacklo_epi32(in[4], in[6]);
  const __m128i a2 = _mm_unpackhi_epi32(in[0], in[2]);
  const __m128i a3 = _mm_unpackhi_epi32(in[4], in[6]);
  const __m128i a4 = _mm_unpacklo_epi32(in[1], in[3]);
  const __m128i a5 = _mm_unpacklo_epi32(in[5], in[7]);
  const __m128i a6 = _mm_unpackhi_epi32(in[1], in[3]);
  const __m128i a7 = _mm_unpackhi_epi32(in[5], in[7]);









  out[0] = _mm_unpacklo_epi64(a0, a1);
  out[1] = _mm_unpackhi_epi64(a0, a1);
  out[2] = _mm_unpacklo_epi64(a2, a3);
  out[3] = _mm_unpackhi_epi64(a2, a3);
  out[4] = _mm_unpacklo_epi64(a4, a5);
  out[5] = _mm_unpackhi_epi64(a4, a5);
  out[6] = _mm_unpacklo_epi64(a6, a7);
  out[7] = _mm_unpackhi_epi64(a6, a7);
}

#endif  // VPX_VPX_DSP_X86_TRANSPOSE_SSE2_H_
