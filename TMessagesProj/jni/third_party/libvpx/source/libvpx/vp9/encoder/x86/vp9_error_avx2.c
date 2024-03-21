/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <immintrin.h>

#include "./vp9_rtcd.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_dsp/x86/bitdepth_conversion_avx2.h"

int64_t vp9_block_error_avx2(const tran_low_t *coeff, const tran_low_t *dqcoeff,
                             intptr_t block_size, int64_t *ssz) {
  __m256i sse_256, ssz_256;
  __m256i exp_dqcoeff_lo, exp_dqcoeff_hi, exp_coeff_lo, exp_coeff_hi;
  __m256i sse_hi, ssz_hi;
  __m128i sse_128, ssz_128;
  int64_t sse;
  const __m256i zero = _mm256_setzero_si256();

  if (block_size == 16) {
    __m256i coeff_256, dqcoeff_256, coeff_hi, dqcoeff_hi;

    coeff_256 = load_tran_low(coeff);
    dqcoeff_256 = load_tran_low(dqcoeff);

    dqcoeff_256 = _mm256_sub_epi16(dqcoeff_256, coeff_256);

    dqcoeff_256 = _mm256_madd_epi16(dqcoeff_256, dqcoeff_256);

    coeff_256 = _mm256_madd_epi16(coeff_256, coeff_256);

    dqcoeff_hi = _mm256_srli_si256(dqcoeff_256, 8);
    coeff_hi = _mm256_srli_si256(coeff_256, 8);

    dqcoeff_256 = _mm256_add_epi32(dqcoeff_256, dqcoeff_hi);
    coeff_256 = _mm256_add_epi32(coeff_256, coeff_hi);

    sse_256 = _mm256_unpacklo_epi32(dqcoeff_256, zero);
    ssz_256 = _mm256_unpacklo_epi32(coeff_256, zero);
  } else {
    int i;
    assert(block_size % 32 == 0);
    sse_256 = zero;
    ssz_256 = zero;

    for (i = 0; i < block_size; i += 32) {
      __m256i coeff_0, coeff_1, dqcoeff_0, dqcoeff_1;

      coeff_0 = load_tran_low(coeff + i);
      dqcoeff_0 = load_tran_low(dqcoeff + i);
      coeff_1 = load_tran_low(coeff + i + 16);
      dqcoeff_1 = load_tran_low(dqcoeff + i + 16);

      dqcoeff_0 = _mm256_sub_epi16(dqcoeff_0, coeff_0);
      dqcoeff_1 = _mm256_sub_epi16(dqcoeff_1, coeff_1);

      dqcoeff_0 = _mm256_madd_epi16(dqcoeff_0, dqcoeff_0);
      dqcoeff_1 = _mm256_madd_epi16(dqcoeff_1, dqcoeff_1);

      coeff_0 = _mm256_madd_epi16(coeff_0, coeff_0);
      coeff_1 = _mm256_madd_epi16(coeff_1, coeff_1);

      dqcoeff_0 = _mm256_add_epi32(dqcoeff_0, dqcoeff_1);

      coeff_0 = _mm256_add_epi32(coeff_0, coeff_1);

      exp_dqcoeff_lo = _mm256_unpacklo_epi32(dqcoeff_0, zero);
      exp_dqcoeff_hi = _mm256_unpackhi_epi32(dqcoeff_0, zero);

      exp_coeff_lo = _mm256_unpacklo_epi32(coeff_0, zero);
      exp_coeff_hi = _mm256_unpackhi_epi32(coeff_0, zero);

      sse_256 = _mm256_add_epi64(sse_256, exp_dqcoeff_lo);
      ssz_256 = _mm256_add_epi64(ssz_256, exp_coeff_lo);
      sse_256 = _mm256_add_epi64(sse_256, exp_dqcoeff_hi);
      ssz_256 = _mm256_add_epi64(ssz_256, exp_coeff_hi);
    }
  }

  sse_hi = _mm256_srli_si256(sse_256, 8);
  ssz_hi = _mm256_srli_si256(ssz_256, 8);

  sse_256 = _mm256_add_epi64(sse_256, sse_hi);
  ssz_256 = _mm256_add_epi64(ssz_256, ssz_hi);

  sse_128 = _mm_add_epi64(_mm256_castsi256_si128(sse_256),
                          _mm256_extractf128_si256(sse_256, 1));

  ssz_128 = _mm_add_epi64(_mm256_castsi256_si128(ssz_256),
                          _mm256_extractf128_si256(ssz_256, 1));

  _mm_storel_epi64((__m128i *)(&sse), sse_128);

  _mm_storel_epi64((__m128i *)(ssz), ssz_128);
  return sse;
}

int64_t vp9_block_error_fp_avx2(const tran_low_t *coeff,
                                const tran_low_t *dqcoeff, int block_size) {
  int i;
  const __m256i zero = _mm256_setzero_si256();
  __m256i sse_256 = zero;
  __m256i sse_hi;
  __m128i sse_128;
  int64_t sse;

  if (block_size == 16) {

    const __m256i _coeff = load_tran_low(coeff);
    const __m256i _dqcoeff = load_tran_low(dqcoeff);

    const __m256i diff = _mm256_sub_epi16(_dqcoeff, _coeff);

    const __m256i error_lo = _mm256_madd_epi16(diff, diff);

    const __m256i error_hi = _mm256_srli_si256(error_lo, 8);

    const __m256i error = _mm256_add_epi32(error_lo, error_hi);

    sse_256 = _mm256_unpacklo_epi32(error, zero);
  } else {
    for (i = 0; i < block_size; i += 16) {

      const __m256i _coeff = load_tran_low(coeff);
      const __m256i _dqcoeff = load_tran_low(dqcoeff);
      const __m256i diff = _mm256_sub_epi16(_dqcoeff, _coeff);
      const __m256i error = _mm256_madd_epi16(diff, diff);

      const __m256i exp_error_lo = _mm256_unpacklo_epi32(error, zero);
      const __m256i exp_error_hi = _mm256_unpackhi_epi32(error, zero);

      sse_256 = _mm256_add_epi64(sse_256, exp_error_lo);
      sse_256 = _mm256_add_epi64(sse_256, exp_error_hi);
      coeff += 16;
      dqcoeff += 16;
    }
  }

  sse_hi = _mm256_srli_si256(sse_256, 8);

  sse_256 = _mm256_add_epi64(sse_256, sse_hi);

  sse_128 = _mm_add_epi64(_mm256_castsi256_si128(sse_256),
                          _mm256_extractf128_si256(sse_256, 1));

  _mm_storel_epi64((__m128i *)&sse, sse_128);
  return sse;
}
