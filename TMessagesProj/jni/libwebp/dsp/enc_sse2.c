// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// SSE2 version of speed-critical encoding functions.
//
// Author: Christian Duvivier (cduvivier@google.com)

#include "./dsp.h"

#if defined(WEBP_USE_SSE2)
#include <stdlib.h>  // for abs()
#include <emmintrin.h>

#include "../enc/cost.h"
#include "../enc/vp8enci.h"
#include "../utils/utils.h"

// Quite useful macro for debugging. Left here for convenience.

#if 0
#include <stdio.h>
static void PrintReg(const __m128i r, const char* const name, int size) {
  int n;
  union {
    __m128i r;
    uint8_t i8[16];
    uint16_t i16[8];
    uint32_t i32[4];
    uint64_t i64[2];
  } tmp;
  tmp.r = r;
  printf("%s\t: ", name);
  if (size == 8) {
    for (n = 0; n < 16; ++n) printf("%.2x ", tmp.i8[n]);
  } else if (size == 16) {
    for (n = 0; n < 8; ++n) printf("%.4x ", tmp.i16[n]);
  } else if (size == 32) {
    for (n = 0; n < 4; ++n) printf("%.8x ", tmp.i32[n]);
  } else {
    for (n = 0; n < 2; ++n) printf("%.16lx ", tmp.i64[n]);
  }
  printf("\n");
}
#endif

// Compute susceptibility based on DCT-coeff histograms:
// the higher, the "easier" the macroblock is to compress.

static void CollectHistogram(const uint8_t* ref, const uint8_t* pred,
                             int start_block, int end_block,
                             VP8Histogram* const histo) {
  const __m128i max_coeff_thresh = _mm_set1_epi16(MAX_COEFF_THRESH);
  int j;
  for (j = start_block; j < end_block; ++j) {
    int16_t out[16];
    int k;

    VP8FTransform(ref + VP8DspScan[j], pred + VP8DspScan[j], out);

    {

      const __m128i out0 = _mm_loadu_si128((__m128i*)&out[0]);
      const __m128i out1 = _mm_loadu_si128((__m128i*)&out[8]);

      const __m128i sign0 = _mm_srai_epi16(out0, 15);
      const __m128i sign1 = _mm_srai_epi16(out1, 15);

      const __m128i xor0 = _mm_xor_si128(out0, sign0);
      const __m128i xor1 = _mm_xor_si128(out1, sign1);
      const __m128i abs0 = _mm_sub_epi16(xor0, sign0);
      const __m128i abs1 = _mm_sub_epi16(xor1, sign1);

      const __m128i v0 = _mm_srai_epi16(abs0, 3);
      const __m128i v1 = _mm_srai_epi16(abs1, 3);

      const __m128i bin0 = _mm_min_epi16(v0, max_coeff_thresh);
      const __m128i bin1 = _mm_min_epi16(v1, max_coeff_thresh);

      _mm_storeu_si128((__m128i*)&out[0], bin0);
      _mm_storeu_si128((__m128i*)&out[8], bin1);
    }

    for (k = 0; k < 16; ++k) {
      histo->distribution[out[k]]++;
    }
  }
}

// Transforms (Paragraph 14.4)

static void ITransform(const uint8_t* ref, const int16_t* in, uint8_t* dst,
                       int do_two) {
















  const __m128i k1 = _mm_set1_epi16(20091);
  const __m128i k2 = _mm_set1_epi16(-30068);
  __m128i T0, T1, T2, T3;




  __m128i in0, in1, in2, in3;
  {
    in0 = _mm_loadl_epi64((__m128i*)&in[0]);
    in1 = _mm_loadl_epi64((__m128i*)&in[4]);
    in2 = _mm_loadl_epi64((__m128i*)&in[8]);
    in3 = _mm_loadl_epi64((__m128i*)&in[12]);




    if (do_two) {
      const __m128i inB0 = _mm_loadl_epi64((__m128i*)&in[16]);
      const __m128i inB1 = _mm_loadl_epi64((__m128i*)&in[20]);
      const __m128i inB2 = _mm_loadl_epi64((__m128i*)&in[24]);
      const __m128i inB3 = _mm_loadl_epi64((__m128i*)&in[28]);
      in0 = _mm_unpacklo_epi64(in0, inB0);
      in1 = _mm_unpacklo_epi64(in1, inB1);
      in2 = _mm_unpacklo_epi64(in2, inB2);
      in3 = _mm_unpacklo_epi64(in3, inB3);




    }
  }

  {


    const __m128i a = _mm_add_epi16(in0, in2);
    const __m128i b = _mm_sub_epi16(in0, in2);

    const __m128i c1 = _mm_mulhi_epi16(in1, k2);
    const __m128i c2 = _mm_mulhi_epi16(in3, k1);
    const __m128i c3 = _mm_sub_epi16(in1, in3);
    const __m128i c4 = _mm_sub_epi16(c1, c2);
    const __m128i c = _mm_add_epi16(c3, c4);

    const __m128i d1 = _mm_mulhi_epi16(in1, k1);
    const __m128i d2 = _mm_mulhi_epi16(in3, k2);
    const __m128i d3 = _mm_add_epi16(in1, in3);
    const __m128i d4 = _mm_add_epi16(d1, d2);
    const __m128i d = _mm_add_epi16(d3, d4);

    const __m128i tmp0 = _mm_add_epi16(a, d);
    const __m128i tmp1 = _mm_add_epi16(b, c);
    const __m128i tmp2 = _mm_sub_epi16(b, c);
    const __m128i tmp3 = _mm_sub_epi16(a, d);





    const __m128i transpose0_0 = _mm_unpacklo_epi16(tmp0, tmp1);
    const __m128i transpose0_1 = _mm_unpacklo_epi16(tmp2, tmp3);
    const __m128i transpose0_2 = _mm_unpackhi_epi16(tmp0, tmp1);
    const __m128i transpose0_3 = _mm_unpackhi_epi16(tmp2, tmp3);




    const __m128i transpose1_0 = _mm_unpacklo_epi32(transpose0_0, transpose0_1);
    const __m128i transpose1_1 = _mm_unpacklo_epi32(transpose0_2, transpose0_3);
    const __m128i transpose1_2 = _mm_unpackhi_epi32(transpose0_0, transpose0_1);
    const __m128i transpose1_3 = _mm_unpackhi_epi32(transpose0_2, transpose0_3);




    T0 = _mm_unpacklo_epi64(transpose1_0, transpose1_1);
    T1 = _mm_unpackhi_epi64(transpose1_0, transpose1_1);
    T2 = _mm_unpacklo_epi64(transpose1_2, transpose1_3);
    T3 = _mm_unpackhi_epi64(transpose1_2, transpose1_3);




  }

  {


    const __m128i four = _mm_set1_epi16(4);
    const __m128i dc = _mm_add_epi16(T0, four);
    const __m128i a =  _mm_add_epi16(dc, T2);
    const __m128i b =  _mm_sub_epi16(dc, T2);

    const __m128i c1 = _mm_mulhi_epi16(T1, k2);
    const __m128i c2 = _mm_mulhi_epi16(T3, k1);
    const __m128i c3 = _mm_sub_epi16(T1, T3);
    const __m128i c4 = _mm_sub_epi16(c1, c2);
    const __m128i c = _mm_add_epi16(c3, c4);

    const __m128i d1 = _mm_mulhi_epi16(T1, k1);
    const __m128i d2 = _mm_mulhi_epi16(T3, k2);
    const __m128i d3 = _mm_add_epi16(T1, T3);
    const __m128i d4 = _mm_add_epi16(d1, d2);
    const __m128i d = _mm_add_epi16(d3, d4);

    const __m128i tmp0 = _mm_add_epi16(a, d);
    const __m128i tmp1 = _mm_add_epi16(b, c);
    const __m128i tmp2 = _mm_sub_epi16(b, c);
    const __m128i tmp3 = _mm_sub_epi16(a, d);
    const __m128i shifted0 = _mm_srai_epi16(tmp0, 3);
    const __m128i shifted1 = _mm_srai_epi16(tmp1, 3);
    const __m128i shifted2 = _mm_srai_epi16(tmp2, 3);
    const __m128i shifted3 = _mm_srai_epi16(tmp3, 3);





    const __m128i transpose0_0 = _mm_unpacklo_epi16(shifted0, shifted1);
    const __m128i transpose0_1 = _mm_unpacklo_epi16(shifted2, shifted3);
    const __m128i transpose0_2 = _mm_unpackhi_epi16(shifted0, shifted1);
    const __m128i transpose0_3 = _mm_unpackhi_epi16(shifted2, shifted3);




    const __m128i transpose1_0 = _mm_unpacklo_epi32(transpose0_0, transpose0_1);
    const __m128i transpose1_1 = _mm_unpacklo_epi32(transpose0_2, transpose0_3);
    const __m128i transpose1_2 = _mm_unpackhi_epi32(transpose0_0, transpose0_1);
    const __m128i transpose1_3 = _mm_unpackhi_epi32(transpose0_2, transpose0_3);




    T0 = _mm_unpacklo_epi64(transpose1_0, transpose1_1);
    T1 = _mm_unpackhi_epi64(transpose1_0, transpose1_1);
    T2 = _mm_unpacklo_epi64(transpose1_2, transpose1_3);
    T3 = _mm_unpackhi_epi64(transpose1_2, transpose1_3);




  }

  {
    const __m128i zero = _mm_setzero_si128();

    __m128i ref0, ref1, ref2, ref3;
    if (do_two) {

      ref0 = _mm_loadl_epi64((__m128i*)&ref[0 * BPS]);
      ref1 = _mm_loadl_epi64((__m128i*)&ref[1 * BPS]);
      ref2 = _mm_loadl_epi64((__m128i*)&ref[2 * BPS]);
      ref3 = _mm_loadl_epi64((__m128i*)&ref[3 * BPS]);
    } else {

      ref0 = _mm_cvtsi32_si128(*(int*)&ref[0 * BPS]);
      ref1 = _mm_cvtsi32_si128(*(int*)&ref[1 * BPS]);
      ref2 = _mm_cvtsi32_si128(*(int*)&ref[2 * BPS]);
      ref3 = _mm_cvtsi32_si128(*(int*)&ref[3 * BPS]);
    }

    ref0 = _mm_unpacklo_epi8(ref0, zero);
    ref1 = _mm_unpacklo_epi8(ref1, zero);
    ref2 = _mm_unpacklo_epi8(ref2, zero);
    ref3 = _mm_unpacklo_epi8(ref3, zero);

    ref0 = _mm_add_epi16(ref0, T0);
    ref1 = _mm_add_epi16(ref1, T1);
    ref2 = _mm_add_epi16(ref2, T2);
    ref3 = _mm_add_epi16(ref3, T3);

    ref0 = _mm_packus_epi16(ref0, ref0);
    ref1 = _mm_packus_epi16(ref1, ref1);
    ref2 = _mm_packus_epi16(ref2, ref2);
    ref3 = _mm_packus_epi16(ref3, ref3);

    if (do_two) {

      _mm_storel_epi64((__m128i*)&dst[0 * BPS], ref0);
      _mm_storel_epi64((__m128i*)&dst[1 * BPS], ref1);
      _mm_storel_epi64((__m128i*)&dst[2 * BPS], ref2);
      _mm_storel_epi64((__m128i*)&dst[3 * BPS], ref3);
    } else {

      *((int32_t *)&dst[0 * BPS]) = _mm_cvtsi128_si32(ref0);
      *((int32_t *)&dst[1 * BPS]) = _mm_cvtsi128_si32(ref1);
      *((int32_t *)&dst[2 * BPS]) = _mm_cvtsi128_si32(ref2);
      *((int32_t *)&dst[3 * BPS]) = _mm_cvtsi128_si32(ref3);
    }
  }
}

static void FTransform(const uint8_t* src, const uint8_t* ref, int16_t* out) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i seven = _mm_set1_epi16(7);
  const __m128i k937 = _mm_set1_epi32(937);
  const __m128i k1812 = _mm_set1_epi32(1812);
  const __m128i k51000 = _mm_set1_epi32(51000);
  const __m128i k12000_plus_one = _mm_set1_epi32(12000 + (1 << 16));
  const __m128i k5352_2217 = _mm_set_epi16(5352,  2217, 5352,  2217,
                                           5352,  2217, 5352,  2217);
  const __m128i k2217_5352 = _mm_set_epi16(2217, -5352, 2217, -5352,
                                           2217, -5352, 2217, -5352);
  const __m128i k88p = _mm_set_epi16(8, 8, 8, 8, 8, 8, 8, 8);
  const __m128i k88m = _mm_set_epi16(-8, 8, -8, 8, -8, 8, -8, 8);
  const __m128i k5352_2217p = _mm_set_epi16(2217, 5352, 2217, 5352,
                                            2217, 5352, 2217, 5352);
  const __m128i k5352_2217m = _mm_set_epi16(-5352, 2217, -5352, 2217,
                                            -5352, 2217, -5352, 2217);
  __m128i v01, v32;

  {

    const __m128i src0 = _mm_loadl_epi64((__m128i*)&src[0 * BPS]);
    const __m128i src1 = _mm_loadl_epi64((__m128i*)&src[1 * BPS]);
    const __m128i src2 = _mm_loadl_epi64((__m128i*)&src[2 * BPS]);
    const __m128i src3 = _mm_loadl_epi64((__m128i*)&src[3 * BPS]);
    const __m128i src_0 = _mm_unpacklo_epi8(src0, zero);
    const __m128i src_1 = _mm_unpacklo_epi8(src1, zero);
    const __m128i src_2 = _mm_unpacklo_epi8(src2, zero);
    const __m128i src_3 = _mm_unpacklo_epi8(src3, zero);

    const __m128i ref0 = _mm_loadl_epi64((__m128i*)&ref[0 * BPS]);
    const __m128i ref1 = _mm_loadl_epi64((__m128i*)&ref[1 * BPS]);
    const __m128i ref2 = _mm_loadl_epi64((__m128i*)&ref[2 * BPS]);
    const __m128i ref3 = _mm_loadl_epi64((__m128i*)&ref[3 * BPS]);
    const __m128i ref_0 = _mm_unpacklo_epi8(ref0, zero);
    const __m128i ref_1 = _mm_unpacklo_epi8(ref1, zero);
    const __m128i ref_2 = _mm_unpacklo_epi8(ref2, zero);
    const __m128i ref_3 = _mm_unpacklo_epi8(ref3, zero);

    const __m128i diff0 = _mm_sub_epi16(src_0, ref_0);
    const __m128i diff1 = _mm_sub_epi16(src_1, ref_1);
    const __m128i diff2 = _mm_sub_epi16(src_2, ref_2);
    const __m128i diff3 = _mm_sub_epi16(src_3, ref_3);





    const __m128i shuf01 = _mm_unpacklo_epi32(diff0, diff1);
    const __m128i shuf23 = _mm_unpacklo_epi32(diff2, diff3);


    const __m128i shuf01_p =
        _mm_shufflehi_epi16(shuf01, _MM_SHUFFLE(2, 3, 0, 1));
    const __m128i shuf23_p =
        _mm_shufflehi_epi16(shuf23, _MM_SHUFFLE(2, 3, 0, 1));


    const __m128i s01 = _mm_unpacklo_epi64(shuf01_p, shuf23_p);
    const __m128i s32 = _mm_unpackhi_epi64(shuf01_p, shuf23_p);


    const __m128i a01 = _mm_add_epi16(s01, s32);
    const __m128i a32 = _mm_sub_epi16(s01, s32);



    const __m128i tmp0 = _mm_madd_epi16(a01, k88p);  // [ (a0 + a1) << 3, ... ]
    const __m128i tmp2 = _mm_madd_epi16(a01, k88m);  // [ (a0 - a1) << 3, ... ]
    const __m128i tmp1_1 = _mm_madd_epi16(a32, k5352_2217p);
    const __m128i tmp3_1 = _mm_madd_epi16(a32, k5352_2217m);
    const __m128i tmp1_2 = _mm_add_epi32(tmp1_1, k1812);
    const __m128i tmp3_2 = _mm_add_epi32(tmp3_1, k937);
    const __m128i tmp1   = _mm_srai_epi32(tmp1_2, 9);
    const __m128i tmp3   = _mm_srai_epi32(tmp3_2, 9);
    const __m128i s03 = _mm_packs_epi32(tmp0, tmp2);
    const __m128i s12 = _mm_packs_epi32(tmp1, tmp3);
    const __m128i s_lo = _mm_unpacklo_epi16(s03, s12);   // 0 1 0 1 0 1...
    const __m128i s_hi = _mm_unpackhi_epi16(s03, s12);   // 2 3 2 3 2 3
    const __m128i v23 = _mm_unpackhi_epi32(s_lo, s_hi);
    v01 = _mm_unpacklo_epi32(s_lo, s_hi);
    v32 = _mm_shuffle_epi32(v23, _MM_SHUFFLE(1, 0, 3, 2));  // 3 2 3 2 3 2..
  }

  {





    const __m128i a01 = _mm_add_epi16(v01, v32);
    const __m128i a32 = _mm_sub_epi16(v01, v32);
    const __m128i a11 = _mm_unpackhi_epi64(a01, a01);
    const __m128i a22 = _mm_unpackhi_epi64(a32, a32);
    const __m128i a01_plus_7 = _mm_add_epi16(a01, seven);


    const __m128i c0 = _mm_add_epi16(a01_plus_7, a11);
    const __m128i c2 = _mm_sub_epi16(a01_plus_7, a11);
    const __m128i d0 = _mm_srai_epi16(c0, 4);
    const __m128i d2 = _mm_srai_epi16(c2, 4);


    const __m128i b23 = _mm_unpacklo_epi16(a22, a32);
    const __m128i c1 = _mm_madd_epi16(b23, k5352_2217);
    const __m128i c3 = _mm_madd_epi16(b23, k2217_5352);
    const __m128i d1 = _mm_add_epi32(c1, k12000_plus_one);
    const __m128i d3 = _mm_add_epi32(c3, k51000);
    const __m128i e1 = _mm_srai_epi32(d1, 16);
    const __m128i e3 = _mm_srai_epi32(d3, 16);
    const __m128i f1 = _mm_packs_epi32(e1, e1);
    const __m128i f3 = _mm_packs_epi32(e3, e3);




    const __m128i g1 = _mm_add_epi16(f1, _mm_cmpeq_epi16(a32, zero));

    const __m128i d0_g1 = _mm_unpacklo_epi64(d0, g1);
    const __m128i d2_f3 = _mm_unpacklo_epi64(d2, f3);
    _mm_storeu_si128((__m128i*)&out[0], d0_g1);
    _mm_storeu_si128((__m128i*)&out[8], d2_f3);
  }
}

static void FTransformWHT(const int16_t* in, int16_t* out) {
  int32_t tmp[16];
  int i;
  for (i = 0; i < 4; ++i, in += 64) {
    const int a0 = (in[0 * 16] + in[2 * 16]);
    const int a1 = (in[1 * 16] + in[3 * 16]);
    const int a2 = (in[1 * 16] - in[3 * 16]);
    const int a3 = (in[0 * 16] - in[2 * 16]);
    tmp[0 + i * 4] = a0 + a1;
    tmp[1 + i * 4] = a3 + a2;
    tmp[2 + i * 4] = a3 - a2;
    tmp[3 + i * 4] = a0 - a1;
  }
  {
    const __m128i src0 = _mm_loadu_si128((__m128i*)&tmp[0]);
    const __m128i src1 = _mm_loadu_si128((__m128i*)&tmp[4]);
    const __m128i src2 = _mm_loadu_si128((__m128i*)&tmp[8]);
    const __m128i src3 = _mm_loadu_si128((__m128i*)&tmp[12]);
    const __m128i a0 = _mm_add_epi32(src0, src2);
    const __m128i a1 = _mm_add_epi32(src1, src3);
    const __m128i a2 = _mm_sub_epi32(src1, src3);
    const __m128i a3 = _mm_sub_epi32(src0, src2);
    const __m128i b0 = _mm_srai_epi32(_mm_add_epi32(a0, a1), 1);
    const __m128i b1 = _mm_srai_epi32(_mm_add_epi32(a3, a2), 1);
    const __m128i b2 = _mm_srai_epi32(_mm_sub_epi32(a3, a2), 1);
    const __m128i b3 = _mm_srai_epi32(_mm_sub_epi32(a0, a1), 1);
    const __m128i out0 = _mm_packs_epi32(b0, b1);
    const __m128i out1 = _mm_packs_epi32(b2, b3);
    _mm_storeu_si128((__m128i*)&out[0], out0);
    _mm_storeu_si128((__m128i*)&out[8], out1);
  }
}

// Metric

static int SSE_Nx4(const uint8_t* a, const uint8_t* b,
                   int num_quads, int do_16) {
  const __m128i zero = _mm_setzero_si128();
  __m128i sum1 = zero;
  __m128i sum2 = zero;

  while (num_quads-- > 0) {


    const __m128i a0 = _mm_loadu_si128((__m128i*)&a[BPS * 0]);
    const __m128i a1 = _mm_loadu_si128((__m128i*)&a[BPS * 1]);
    const __m128i a2 = _mm_loadu_si128((__m128i*)&a[BPS * 2]);
    const __m128i a3 = _mm_loadu_si128((__m128i*)&a[BPS * 3]);
    const __m128i b0 = _mm_loadu_si128((__m128i*)&b[BPS * 0]);
    const __m128i b1 = _mm_loadu_si128((__m128i*)&b[BPS * 1]);
    const __m128i b2 = _mm_loadu_si128((__m128i*)&b[BPS * 2]);
    const __m128i b3 = _mm_loadu_si128((__m128i*)&b[BPS * 3]);

    const __m128i a0p = _mm_subs_epu8(a0, b0);
    const __m128i a0m = _mm_subs_epu8(b0, a0);
    const __m128i a1p = _mm_subs_epu8(a1, b1);
    const __m128i a1m = _mm_subs_epu8(b1, a1);
    const __m128i a2p = _mm_subs_epu8(a2, b2);
    const __m128i a2m = _mm_subs_epu8(b2, a2);
    const __m128i a3p = _mm_subs_epu8(a3, b3);
    const __m128i a3m = _mm_subs_epu8(b3, a3);

    const __m128i diff0 = _mm_or_si128(a0p, a0m);
    const __m128i diff1 = _mm_or_si128(a1p, a1m);
    const __m128i diff2 = _mm_or_si128(a2p, a2m);
    const __m128i diff3 = _mm_or_si128(a3p, a3m);

    const __m128i low0 = _mm_unpacklo_epi8(diff0, zero);
    const __m128i low1 = _mm_unpacklo_epi8(diff1, zero);
    const __m128i low2 = _mm_unpacklo_epi8(diff2, zero);
    const __m128i low3 = _mm_unpacklo_epi8(diff3, zero);

    const __m128i low_madd0 = _mm_madd_epi16(low0, low0);
    const __m128i low_madd1 = _mm_madd_epi16(low1, low1);
    const __m128i low_madd2 = _mm_madd_epi16(low2, low2);
    const __m128i low_madd3 = _mm_madd_epi16(low3, low3);

    const __m128i low_sum0 = _mm_add_epi32(low_madd0, low_madd1);
    const __m128i low_sum1 = _mm_add_epi32(low_madd2, low_madd3);
    sum1 = _mm_add_epi32(sum1, low_sum0);
    sum2 = _mm_add_epi32(sum2, low_sum1);

    if (do_16) {  // if necessary, process the higher 8 bytes similarly
      const __m128i hi0 = _mm_unpackhi_epi8(diff0, zero);
      const __m128i hi1 = _mm_unpackhi_epi8(diff1, zero);
      const __m128i hi2 = _mm_unpackhi_epi8(diff2, zero);
      const __m128i hi3 = _mm_unpackhi_epi8(diff3, zero);

      const __m128i hi_madd0 = _mm_madd_epi16(hi0, hi0);
      const __m128i hi_madd1 = _mm_madd_epi16(hi1, hi1);
      const __m128i hi_madd2 = _mm_madd_epi16(hi2, hi2);
      const __m128i hi_madd3 = _mm_madd_epi16(hi3, hi3);
      const __m128i hi_sum0 = _mm_add_epi32(hi_madd0, hi_madd1);
      const __m128i hi_sum1 = _mm_add_epi32(hi_madd2, hi_madd3);
      sum1 = _mm_add_epi32(sum1, hi_sum0);
      sum2 = _mm_add_epi32(sum2, hi_sum1);
    }
    a += 4 * BPS;
    b += 4 * BPS;
  }
  {
    int32_t tmp[4];
    const __m128i sum = _mm_add_epi32(sum1, sum2);
    _mm_storeu_si128((__m128i*)tmp, sum);
    return (tmp[3] + tmp[2] + tmp[1] + tmp[0]);
  }
}

static int SSE16x16(const uint8_t* a, const uint8_t* b) {
  return SSE_Nx4(a, b, 4, 1);
}

static int SSE16x8(const uint8_t* a, const uint8_t* b) {
  return SSE_Nx4(a, b, 2, 1);
}

static int SSE8x8(const uint8_t* a, const uint8_t* b) {
  return SSE_Nx4(a, b, 2, 0);
}

static int SSE4x4(const uint8_t* a, const uint8_t* b) {
  const __m128i zero = _mm_setzero_si128();


  const __m128i a0 = _mm_loadl_epi64((__m128i*)&a[BPS * 0]);
  const __m128i a1 = _mm_loadl_epi64((__m128i*)&a[BPS * 1]);
  const __m128i a2 = _mm_loadl_epi64((__m128i*)&a[BPS * 2]);
  const __m128i a3 = _mm_loadl_epi64((__m128i*)&a[BPS * 3]);
  const __m128i b0 = _mm_loadl_epi64((__m128i*)&b[BPS * 0]);
  const __m128i b1 = _mm_loadl_epi64((__m128i*)&b[BPS * 1]);
  const __m128i b2 = _mm_loadl_epi64((__m128i*)&b[BPS * 2]);
  const __m128i b3 = _mm_loadl_epi64((__m128i*)&b[BPS * 3]);

  const __m128i a01 = _mm_unpacklo_epi32(a0, a1);
  const __m128i a23 = _mm_unpacklo_epi32(a2, a3);
  const __m128i b01 = _mm_unpacklo_epi32(b0, b1);
  const __m128i b23 = _mm_unpacklo_epi32(b2, b3);
  const __m128i a01s = _mm_unpacklo_epi8(a01, zero);
  const __m128i a23s = _mm_unpacklo_epi8(a23, zero);
  const __m128i b01s = _mm_unpacklo_epi8(b01, zero);
  const __m128i b23s = _mm_unpacklo_epi8(b23, zero);





  const __m128i d0 = _mm_subs_epu8(a01s, b01s);
  const __m128i d1 = _mm_subs_epu8(b01s, a01s);
  const __m128i d2 = _mm_subs_epu8(a23s, b23s);
  const __m128i d3 = _mm_subs_epu8(b23s, a23s);

  const __m128i madd0 = _mm_madd_epi16(d0, d0);
  const __m128i madd1 = _mm_madd_epi16(d1, d1);
  const __m128i madd2 = _mm_madd_epi16(d2, d2);
  const __m128i madd3 = _mm_madd_epi16(d3, d3);
  const __m128i sum0 = _mm_add_epi32(madd0, madd1);
  const __m128i sum1 = _mm_add_epi32(madd2, madd3);
  const __m128i sum2 = _mm_add_epi32(sum0, sum1);

  int32_t tmp[4];
  _mm_storeu_si128((__m128i*)tmp, sum2);
  return (tmp[3] + tmp[2] + tmp[1] + tmp[0]);
}

// Texture distortion
//
// We try to match the spectral content (weighted) between source and
// reconstructed samples.

// Returns the difference between the weighted sum of the absolute value of
// transformed coefficients.
static int TTransform(const uint8_t* inA, const uint8_t* inB,
                      const uint16_t* const w) {
  int32_t sum[4];
  __m128i tmp_0, tmp_1, tmp_2, tmp_3;
  const __m128i zero = _mm_setzero_si128();

  {
    const __m128i inA_0 = _mm_loadl_epi64((__m128i*)&inA[BPS * 0]);
    const __m128i inA_1 = _mm_loadl_epi64((__m128i*)&inA[BPS * 1]);
    const __m128i inA_2 = _mm_loadl_epi64((__m128i*)&inA[BPS * 2]);
    const __m128i inA_3 = _mm_loadl_epi64((__m128i*)&inA[BPS * 3]);
    const __m128i inB_0 = _mm_loadl_epi64((__m128i*)&inB[BPS * 0]);
    const __m128i inB_1 = _mm_loadl_epi64((__m128i*)&inB[BPS * 1]);
    const __m128i inB_2 = _mm_loadl_epi64((__m128i*)&inB[BPS * 2]);
    const __m128i inB_3 = _mm_loadl_epi64((__m128i*)&inB[BPS * 3]);

    const __m128i inAB_0 = _mm_unpacklo_epi8(inA_0, inB_0);
    const __m128i inAB_1 = _mm_unpacklo_epi8(inA_1, inB_1);
    const __m128i inAB_2 = _mm_unpacklo_epi8(inA_2, inB_2);
    const __m128i inAB_3 = _mm_unpacklo_epi8(inA_3, inB_3);





    const __m128i transpose0_0 = _mm_unpacklo_epi8(inAB_0, inAB_2);
    const __m128i transpose0_1 = _mm_unpacklo_epi8(inAB_1, inAB_3);


    const __m128i transpose1_0 = _mm_unpacklo_epi8(transpose0_0, transpose0_1);
    const __m128i transpose1_1 = _mm_unpackhi_epi8(transpose0_0, transpose0_1);



    tmp_0 = _mm_unpacklo_epi8(transpose1_0, zero);
    tmp_1 = _mm_unpackhi_epi8(transpose1_0, zero);
    tmp_2 = _mm_unpacklo_epi8(transpose1_1, zero);
    tmp_3 = _mm_unpackhi_epi8(transpose1_1, zero);




  }

  {

    const __m128i a0 = _mm_add_epi16(tmp_0, tmp_2);
    const __m128i a1 = _mm_add_epi16(tmp_1, tmp_3);
    const __m128i a2 = _mm_sub_epi16(tmp_1, tmp_3);
    const __m128i a3 = _mm_sub_epi16(tmp_0, tmp_2);
    const __m128i b0 = _mm_add_epi16(a0, a1);
    const __m128i b1 = _mm_add_epi16(a3, a2);
    const __m128i b2 = _mm_sub_epi16(a3, a2);
    const __m128i b3 = _mm_sub_epi16(a0, a1);





    const __m128i transpose0_0 = _mm_unpacklo_epi16(b0, b1);
    const __m128i transpose0_1 = _mm_unpacklo_epi16(b2, b3);
    const __m128i transpose0_2 = _mm_unpackhi_epi16(b0, b1);
    const __m128i transpose0_3 = _mm_unpackhi_epi16(b2, b3);




    const __m128i transpose1_0 = _mm_unpacklo_epi32(transpose0_0, transpose0_1);
    const __m128i transpose1_1 = _mm_unpacklo_epi32(transpose0_2, transpose0_3);
    const __m128i transpose1_2 = _mm_unpackhi_epi32(transpose0_0, transpose0_1);
    const __m128i transpose1_3 = _mm_unpackhi_epi32(transpose0_2, transpose0_3);




    tmp_0 = _mm_unpacklo_epi64(transpose1_0, transpose1_1);
    tmp_1 = _mm_unpackhi_epi64(transpose1_0, transpose1_1);
    tmp_2 = _mm_unpacklo_epi64(transpose1_2, transpose1_3);
    tmp_3 = _mm_unpackhi_epi64(transpose1_2, transpose1_3);




  }

  {



    const __m128i w_0 = _mm_loadu_si128((__m128i*)&w[0]);
    const __m128i w_8 = _mm_loadu_si128((__m128i*)&w[8]);

    const __m128i a0 = _mm_add_epi16(tmp_0, tmp_2);
    const __m128i a1 = _mm_add_epi16(tmp_1, tmp_3);
    const __m128i a2 = _mm_sub_epi16(tmp_1, tmp_3);
    const __m128i a3 = _mm_sub_epi16(tmp_0, tmp_2);
    const __m128i b0 = _mm_add_epi16(a0, a1);
    const __m128i b1 = _mm_add_epi16(a3, a2);
    const __m128i b2 = _mm_sub_epi16(a3, a2);
    const __m128i b3 = _mm_sub_epi16(a0, a1);

    __m128i A_b0 = _mm_unpacklo_epi64(b0, b1);
    __m128i A_b2 = _mm_unpacklo_epi64(b2, b3);
    __m128i B_b0 = _mm_unpackhi_epi64(b0, b1);
    __m128i B_b2 = _mm_unpackhi_epi64(b2, b3);

    {

      const __m128i sign_A_b0 = _mm_srai_epi16(A_b0, 15);
      const __m128i sign_A_b2 = _mm_srai_epi16(A_b2, 15);
      const __m128i sign_B_b0 = _mm_srai_epi16(B_b0, 15);
      const __m128i sign_B_b2 = _mm_srai_epi16(B_b2, 15);

      A_b0 = _mm_xor_si128(A_b0, sign_A_b0);
      A_b2 = _mm_xor_si128(A_b2, sign_A_b2);
      B_b0 = _mm_xor_si128(B_b0, sign_B_b0);
      B_b2 = _mm_xor_si128(B_b2, sign_B_b2);
      A_b0 = _mm_sub_epi16(A_b0, sign_A_b0);
      A_b2 = _mm_sub_epi16(A_b2, sign_A_b2);
      B_b0 = _mm_sub_epi16(B_b0, sign_B_b0);
      B_b2 = _mm_sub_epi16(B_b2, sign_B_b2);
    }

    A_b0 = _mm_madd_epi16(A_b0, w_0);
    A_b2 = _mm_madd_epi16(A_b2, w_8);
    B_b0 = _mm_madd_epi16(B_b0, w_0);
    B_b2 = _mm_madd_epi16(B_b2, w_8);
    A_b0 = _mm_add_epi32(A_b0, A_b2);
    B_b0 = _mm_add_epi32(B_b0, B_b2);

    A_b0 = _mm_sub_epi32(A_b0, B_b0);
    _mm_storeu_si128((__m128i*)&sum[0], A_b0);
  }
  return sum[0] + sum[1] + sum[2] + sum[3];
}

static int Disto4x4(const uint8_t* const a, const uint8_t* const b,
                    const uint16_t* const w) {
  const int diff_sum = TTransform(a, b, w);
  return abs(diff_sum) >> 5;
}

static int Disto16x16(const uint8_t* const a, const uint8_t* const b,
                      const uint16_t* const w) {
  int D = 0;
  int x, y;
  for (y = 0; y < 16 * BPS; y += 4 * BPS) {
    for (x = 0; x < 16; x += 4) {
      D += Disto4x4(a + x + y, b + x + y, w);
    }
  }
  return D;
}

// Quantization
//

static WEBP_INLINE int DoQuantizeBlock(int16_t in[16], int16_t out[16],
                                       const uint16_t* const sharpen,
                                       const VP8Matrix* const mtx) {
  const __m128i max_coeff_2047 = _mm_set1_epi16(MAX_LEVEL);
  const __m128i zero = _mm_setzero_si128();
  __m128i coeff0, coeff8;
  __m128i out0, out8;
  __m128i packed_out;



  __m128i in0 = _mm_loadu_si128((__m128i*)&in[0]);
  __m128i in8 = _mm_loadu_si128((__m128i*)&in[8]);
  const __m128i iq0 = _mm_loadu_si128((__m128i*)&mtx->iq_[0]);
  const __m128i iq8 = _mm_loadu_si128((__m128i*)&mtx->iq_[8]);
  const __m128i q0 = _mm_loadu_si128((__m128i*)&mtx->q_[0]);
  const __m128i q8 = _mm_loadu_si128((__m128i*)&mtx->q_[8]);

  const __m128i sign0 = _mm_cmpgt_epi16(zero, in0);
  const __m128i sign8 = _mm_cmpgt_epi16(zero, in8);

  coeff0 = _mm_xor_si128(in0, sign0);
  coeff8 = _mm_xor_si128(in8, sign8);
  coeff0 = _mm_sub_epi16(coeff0, sign0);
  coeff8 = _mm_sub_epi16(coeff8, sign8);

  if (sharpen != NULL) {
    const __m128i sharpen0 = _mm_loadu_si128((__m128i*)&sharpen[0]);
    const __m128i sharpen8 = _mm_loadu_si128((__m128i*)&sharpen[8]);
    coeff0 = _mm_add_epi16(coeff0, sharpen0);
    coeff8 = _mm_add_epi16(coeff8, sharpen8);
  }

  {


    const __m128i coeff_iQ0H = _mm_mulhi_epu16(coeff0, iq0);
    const __m128i coeff_iQ0L = _mm_mullo_epi16(coeff0, iq0);
    const __m128i coeff_iQ8H = _mm_mulhi_epu16(coeff8, iq8);
    const __m128i coeff_iQ8L = _mm_mullo_epi16(coeff8, iq8);
    __m128i out_00 = _mm_unpacklo_epi16(coeff_iQ0L, coeff_iQ0H);
    __m128i out_04 = _mm_unpackhi_epi16(coeff_iQ0L, coeff_iQ0H);
    __m128i out_08 = _mm_unpacklo_epi16(coeff_iQ8L, coeff_iQ8H);
    __m128i out_12 = _mm_unpackhi_epi16(coeff_iQ8L, coeff_iQ8H);

    const __m128i bias_00 = _mm_loadu_si128((__m128i*)&mtx->bias_[0]);
    const __m128i bias_04 = _mm_loadu_si128((__m128i*)&mtx->bias_[4]);
    const __m128i bias_08 = _mm_loadu_si128((__m128i*)&mtx->bias_[8]);
    const __m128i bias_12 = _mm_loadu_si128((__m128i*)&mtx->bias_[12]);
    out_00 = _mm_add_epi32(out_00, bias_00);
    out_04 = _mm_add_epi32(out_04, bias_04);
    out_08 = _mm_add_epi32(out_08, bias_08);
    out_12 = _mm_add_epi32(out_12, bias_12);

    out_00 = _mm_srai_epi32(out_00, QFIX);
    out_04 = _mm_srai_epi32(out_04, QFIX);
    out_08 = _mm_srai_epi32(out_08, QFIX);
    out_12 = _mm_srai_epi32(out_12, QFIX);

    out0 = _mm_packs_epi32(out_00, out_04);
    out8 = _mm_packs_epi32(out_08, out_12);

    out0 = _mm_min_epi16(out0, max_coeff_2047);
    out8 = _mm_min_epi16(out8, max_coeff_2047);
  }

  out0 = _mm_xor_si128(out0, sign0);
  out8 = _mm_xor_si128(out8, sign8);
  out0 = _mm_sub_epi16(out0, sign0);
  out8 = _mm_sub_epi16(out8, sign8);

  in0 = _mm_mullo_epi16(out0, q0);
  in8 = _mm_mullo_epi16(out8, q8);

  _mm_storeu_si128((__m128i*)&in[0], in0);
  _mm_storeu_si128((__m128i*)&in[8], in8);





  {
    __m128i outZ0, outZ8;
    outZ0 = _mm_shufflehi_epi16(out0,  _MM_SHUFFLE(2, 1, 3, 0));
    outZ0 = _mm_shuffle_epi32  (outZ0, _MM_SHUFFLE(3, 1, 2, 0));
    outZ0 = _mm_shufflehi_epi16(outZ0, _MM_SHUFFLE(3, 1, 0, 2));
    outZ8 = _mm_shufflelo_epi16(out8,  _MM_SHUFFLE(3, 0, 2, 1));
    outZ8 = _mm_shuffle_epi32  (outZ8, _MM_SHUFFLE(3, 1, 2, 0));
    outZ8 = _mm_shufflelo_epi16(outZ8, _MM_SHUFFLE(1, 3, 2, 0));
    _mm_storeu_si128((__m128i*)&out[0], outZ0);
    _mm_storeu_si128((__m128i*)&out[8], outZ8);
    packed_out = _mm_packs_epi16(outZ0, outZ8);
  }
  {
    const int16_t outZ_12 = out[12];
    const int16_t outZ_3 = out[3];
    out[3] = outZ_12;
    out[12] = outZ_3;
  }

  return (_mm_movemask_epi8(_mm_cmpeq_epi8(packed_out, zero)) != 0xffff);
}

static int QuantizeBlock(int16_t in[16], int16_t out[16],
                         const VP8Matrix* const mtx) {
  return DoQuantizeBlock(in, out, &mtx->sharpen_[0], mtx);
}

static int QuantizeBlockWHT(int16_t in[16], int16_t out[16],
                            const VP8Matrix* const mtx) {
  return DoQuantizeBlock(in, out, NULL, mtx);
}

void VP8SetResidualCoeffsSSE2(const int16_t* const coeffs,
                              VP8Residual* const res);

void VP8SetResidualCoeffsSSE2(const int16_t* const coeffs,
                              VP8Residual* const res) {
  const __m128i c0 = _mm_loadu_si128((const __m128i*)coeffs);
  const __m128i c1 = _mm_loadu_si128((const __m128i*)(coeffs + 8));

  const __m128i zero = _mm_setzero_si128();
  const __m128i m0 = _mm_cmpeq_epi16(c0, zero);
  const __m128i m1 = _mm_cmpeq_epi16(c1, zero);





  const uint32_t mask =
      ~(((uint32_t)_mm_movemask_epi8(m1) << 16) | _mm_movemask_epi8(m0));



  assert(res->first == 0 || coeffs[0] == 0);
  res->last = mask ? (BitsLog2Floor(mask) >> 1) : -1;
  res->coeffs = coeffs;
}

#endif   // WEBP_USE_SSE2

// Entry point

extern void VP8EncDspInitSSE2(void);

void VP8EncDspInitSSE2(void) {
#if defined(WEBP_USE_SSE2)
  VP8CollectHistogram = CollectHistogram;
  VP8EncQuantizeBlock = QuantizeBlock;
  VP8EncQuantizeBlockWHT = QuantizeBlockWHT;
  VP8ITransform = ITransform;
  VP8FTransform = FTransform;
  VP8FTransformWHT = FTransformWHT;
  VP8SSE16x16 = SSE16x16;
  VP8SSE16x8 = SSE16x8;
  VP8SSE8x8 = SSE8x8;
  VP8SSE4x4 = SSE4x4;
  VP8TDisto4x4 = Disto4x4;
  VP8TDisto16x16 = Disto16x16;
#endif   // WEBP_USE_SSE2
}

