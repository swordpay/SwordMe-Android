/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_DSP_ARM_TRANSPOSE_NEON_H_
#define VPX_VPX_DSP_ARM_TRANSPOSE_NEON_H_

#include <arm_neon.h>

#include "./vpx_config.h"

// a0: 00 01 02 03 04 05 06 07
// a1: 16 17 18 19 20 21 22 23
//
// b0.val[0]: 00 01 02 03 16 17 18 19
// b0.val[1]: 04 05 06 07 20 21 22 23
static INLINE int16x8x2_t vpx_vtrnq_s64_to_s16(int32x4_t a0, int32x4_t a1) {
  int16x8x2_t b0;
  b0.val[0] = vcombine_s16(vreinterpret_s16_s32(vget_low_s32(a0)),
                           vreinterpret_s16_s32(vget_low_s32(a1)));
  b0.val[1] = vcombine_s16(vreinterpret_s16_s32(vget_high_s32(a0)),
                           vreinterpret_s16_s32(vget_high_s32(a1)));
  return b0;
}

static INLINE int32x4x2_t vpx_vtrnq_s64_to_s32(int32x4_t a0, int32x4_t a1) {
  int32x4x2_t b0;
  b0.val[0] = vcombine_s32(vget_low_s32(a0), vget_low_s32(a1));
  b0.val[1] = vcombine_s32(vget_high_s32(a0), vget_high_s32(a1));
  return b0;
}

static INLINE int64x2x2_t vpx_vtrnq_s64(int32x4_t a0, int32x4_t a1) {
  int64x2x2_t b0;
  b0.val[0] = vcombine_s64(vreinterpret_s64_s32(vget_low_s32(a0)),
                           vreinterpret_s64_s32(vget_low_s32(a1)));
  b0.val[1] = vcombine_s64(vreinterpret_s64_s32(vget_high_s32(a0)),
                           vreinterpret_s64_s32(vget_high_s32(a1)));
  return b0;
}

static INLINE uint8x16x2_t vpx_vtrnq_u64_to_u8(uint32x4_t a0, uint32x4_t a1) {
  uint8x16x2_t b0;
  b0.val[0] = vcombine_u8(vreinterpret_u8_u32(vget_low_u32(a0)),
                          vreinterpret_u8_u32(vget_low_u32(a1)));
  b0.val[1] = vcombine_u8(vreinterpret_u8_u32(vget_high_u32(a0)),
                          vreinterpret_u8_u32(vget_high_u32(a1)));
  return b0;
}

static INLINE uint16x8x2_t vpx_vtrnq_u64_to_u16(uint32x4_t a0, uint32x4_t a1) {
  uint16x8x2_t b0;
  b0.val[0] = vcombine_u16(vreinterpret_u16_u32(vget_low_u32(a0)),
                           vreinterpret_u16_u32(vget_low_u32(a1)));
  b0.val[1] = vcombine_u16(vreinterpret_u16_u32(vget_high_u32(a0)),
                           vreinterpret_u16_u32(vget_high_u32(a1)));
  return b0;
}

static INLINE void transpose_u8_4x4(uint8x8_t *a0, uint8x8_t *a1) {







  const uint16x4x2_t b0 =
      vtrn_u16(vreinterpret_u16_u8(*a0), vreinterpret_u16_u8(*a1));




  const uint32x2x2_t c0 = vtrn_u32(vreinterpret_u32_u16(b0.val[0]),
                                   vreinterpret_u32_u16(b0.val[1]));




  const uint8x8x2_t d0 =
      vtrn_u8(vreinterpret_u8_u32(c0.val[0]), vreinterpret_u8_u32(c0.val[1]));

  *a0 = d0.val[0];
  *a1 = d0.val[1];
}

static INLINE void transpose_s16_4x4d(int16x4_t *a0, int16x4_t *a1,
                                      int16x4_t *a2, int16x4_t *a3) {











  const int16x4x2_t b0 = vtrn_s16(*a0, *a1);
  const int16x4x2_t b1 = vtrn_s16(*a2, *a3);






  const int32x2x2_t c0 = vtrn_s32(vreinterpret_s32_s16(b0.val[0]),
                                  vreinterpret_s32_s16(b1.val[0]));
  const int32x2x2_t c1 = vtrn_s32(vreinterpret_s32_s16(b0.val[1]),
                                  vreinterpret_s32_s16(b1.val[1]));

  *a0 = vreinterpret_s16_s32(c0.val[0]);
  *a1 = vreinterpret_s16_s32(c1.val[0]);
  *a2 = vreinterpret_s16_s32(c0.val[1]);
  *a3 = vreinterpret_s16_s32(c1.val[1]);
}

static INLINE void transpose_s16_4x4q(int16x8_t *a0, int16x8_t *a1) {







  const int32x4x2_t b0 =
      vtrnq_s32(vreinterpretq_s32_s16(*a0), vreinterpretq_s32_s16(*a1));




  const int32x4_t c0 =
      vcombine_s32(vget_low_s32(b0.val[0]), vget_low_s32(b0.val[1]));
  const int32x4_t c1 =
      vcombine_s32(vget_high_s32(b0.val[0]), vget_high_s32(b0.val[1]));




  const int16x8x2_t d0 =
      vtrnq_s16(vreinterpretq_s16_s32(c0), vreinterpretq_s16_s32(c1));

  *a0 = d0.val[0];
  *a1 = d0.val[1];
}

static INLINE void transpose_u16_4x4q(uint16x8_t *a0, uint16x8_t *a1) {







  const uint32x4x2_t b0 =
      vtrnq_u32(vreinterpretq_u32_u16(*a0), vreinterpretq_u32_u16(*a1));




  const uint32x4_t c0 =
      vcombine_u32(vget_low_u32(b0.val[0]), vget_low_u32(b0.val[1]));
  const uint32x4_t c1 =
      vcombine_u32(vget_high_u32(b0.val[0]), vget_high_u32(b0.val[1]));




  const uint16x8x2_t d0 =
      vtrnq_u16(vreinterpretq_u16_u32(c0), vreinterpretq_u16_u32(c1));

  *a0 = d0.val[0];
  *a1 = d0.val[1];
}

static INLINE void transpose_u8_4x8(uint8x8_t *a0, uint8x8_t *a1, uint8x8_t *a2,
                                    uint8x8_t *a3, const uint8x8_t a4,
                                    const uint8x8_t a5, const uint8x8_t a6,
                                    const uint8x8_t a7) {















  const uint32x2x2_t b0 =
      vtrn_u32(vreinterpret_u32_u8(*a0), vreinterpret_u32_u8(a4));
  const uint32x2x2_t b1 =
      vtrn_u32(vreinterpret_u32_u8(*a1), vreinterpret_u32_u8(a5));
  const uint32x2x2_t b2 =
      vtrn_u32(vreinterpret_u32_u8(*a2), vreinterpret_u32_u8(a6));
  const uint32x2x2_t b3 =
      vtrn_u32(vreinterpret_u32_u8(*a3), vreinterpret_u32_u8(a7));






  const uint16x4x2_t c0 = vtrn_u16(vreinterpret_u16_u32(b0.val[0]),
                                   vreinterpret_u16_u32(b2.val[0]));
  const uint16x4x2_t c1 = vtrn_u16(vreinterpret_u16_u32(b1.val[0]),
                                   vreinterpret_u16_u32(b3.val[0]));






  const uint8x8x2_t d0 =
      vtrn_u8(vreinterpret_u8_u16(c0.val[0]), vreinterpret_u8_u16(c1.val[0]));
  const uint8x8x2_t d1 =
      vtrn_u8(vreinterpret_u8_u16(c0.val[1]), vreinterpret_u8_u16(c1.val[1]));

  *a0 = d0.val[0];
  *a1 = d0.val[1];
  *a2 = d1.val[0];
  *a3 = d1.val[1];
}

static INLINE void transpose_s32_4x4(int32x4_t *a0, int32x4_t *a1,
                                     int32x4_t *a2, int32x4_t *a3) {











  const int32x4x2_t b0 = vtrnq_s32(*a0, *a1);
  const int32x4x2_t b1 = vtrnq_s32(*a2, *a3);






  const int32x4x2_t c0 = vpx_vtrnq_s64_to_s32(b0.val[0], b1.val[0]);
  const int32x4x2_t c1 = vpx_vtrnq_s64_to_s32(b0.val[1], b1.val[1]);

  *a0 = c0.val[0];
  *a1 = c1.val[0];
  *a2 = c0.val[1];
  *a3 = c1.val[1];
}

static INLINE void transpose_s16_4x8(const int16x4_t a0, const int16x4_t a1,
                                     const int16x4_t a2, const int16x4_t a3,
                                     const int16x4_t a4, const int16x4_t a5,
                                     const int16x4_t a6, const int16x4_t a7,
                                     int16x8_t *const o0, int16x8_t *const o1,
                                     int16x8_t *const o2, int16x8_t *const o3) {



















  const int16x4x2_t b0 = vtrn_s16(a0, a1);
  const int16x4x2_t b1 = vtrn_s16(a2, a3);
  const int16x4x2_t b2 = vtrn_s16(a4, a5);
  const int16x4x2_t b3 = vtrn_s16(a6, a7);










  const int32x2x2_t c0 = vtrn_s32(vreinterpret_s32_s16(b0.val[0]),
                                  vreinterpret_s32_s16(b1.val[0]));
  const int32x2x2_t c1 = vtrn_s32(vreinterpret_s32_s16(b0.val[1]),
                                  vreinterpret_s32_s16(b1.val[1]));
  const int32x2x2_t c2 = vtrn_s32(vreinterpret_s32_s16(b2.val[0]),
                                  vreinterpret_s32_s16(b3.val[0]));
  const int32x2x2_t c3 = vtrn_s32(vreinterpret_s32_s16(b2.val[1]),
                                  vreinterpret_s32_s16(b3.val[1]));






  *o0 = vcombine_s16(vreinterpret_s16_s32(c0.val[0]),
                     vreinterpret_s16_s32(c2.val[0]));
  *o1 = vcombine_s16(vreinterpret_s16_s32(c1.val[0]),
                     vreinterpret_s16_s32(c3.val[0]));
  *o2 = vcombine_s16(vreinterpret_s16_s32(c0.val[1]),
                     vreinterpret_s16_s32(c2.val[1]));
  *o3 = vcombine_s16(vreinterpret_s16_s32(c1.val[1]),
                     vreinterpret_s16_s32(c3.val[1]));
}

static INLINE void transpose_s32_4x8(int32x4_t *const a0, int32x4_t *const a1,
                                     int32x4_t *const a2, int32x4_t *const a3,
                                     int32x4_t *const a4, int32x4_t *const a5,
                                     int32x4_t *const a6, int32x4_t *const a7) {



















  const int32x4x2_t b0 = vtrnq_s32(*a0, *a1);
  const int32x4x2_t b1 = vtrnq_s32(*a2, *a3);
  const int32x4x2_t b2 = vtrnq_s32(*a4, *a5);
  const int32x4x2_t b3 = vtrnq_s32(*a6, *a7);










  const int64x2x2_t c0 = vpx_vtrnq_s64(b0.val[0], b1.val[0]);
  const int64x2x2_t c1 = vpx_vtrnq_s64(b0.val[1], b1.val[1]);
  const int64x2x2_t c2 = vpx_vtrnq_s64(b2.val[0], b3.val[0]);
  const int64x2x2_t c3 = vpx_vtrnq_s64(b2.val[1], b3.val[1]);

  *a0 = vreinterpretq_s32_s64(c0.val[0]);
  *a1 = vreinterpretq_s32_s64(c2.val[0]);
  *a2 = vreinterpretq_s32_s64(c1.val[0]);
  *a3 = vreinterpretq_s32_s64(c3.val[0]);
  *a4 = vreinterpretq_s32_s64(c0.val[1]);
  *a5 = vreinterpretq_s32_s64(c2.val[1]);
  *a6 = vreinterpretq_s32_s64(c1.val[1]);
  *a7 = vreinterpretq_s32_s64(c3.val[1]);
}

static INLINE void transpose_u8_8x4(uint8x8_t *a0, uint8x8_t *a1, uint8x8_t *a2,
                                    uint8x8_t *a3) {











  const uint8x8x2_t b0 = vtrn_u8(*a0, *a1);
  const uint8x8x2_t b1 = vtrn_u8(*a2, *a3);






  const uint16x4x2_t c0 =
      vtrn_u16(vreinterpret_u16_u8(b0.val[0]), vreinterpret_u16_u8(b1.val[0]));
  const uint16x4x2_t c1 =
      vtrn_u16(vreinterpret_u16_u8(b0.val[1]), vreinterpret_u16_u8(b1.val[1]));

  *a0 = vreinterpret_u8_u16(c0.val[0]);
  *a1 = vreinterpret_u8_u16(c1.val[0]);
  *a2 = vreinterpret_u8_u16(c0.val[1]);
  *a3 = vreinterpret_u8_u16(c1.val[1]);
}

static INLINE void transpose_u16_8x4(uint16x8_t *a0, uint16x8_t *a1,
                                     uint16x8_t *a2, uint16x8_t *a3) {











  const uint16x8x2_t b0 = vtrnq_u16(*a0, *a1);
  const uint16x8x2_t b1 = vtrnq_u16(*a2, *a3);






  const uint32x4x2_t c0 = vtrnq_u32(vreinterpretq_u32_u16(b0.val[0]),
                                    vreinterpretq_u32_u16(b1.val[0]));
  const uint32x4x2_t c1 = vtrnq_u32(vreinterpretq_u32_u16(b0.val[1]),
                                    vreinterpretq_u32_u16(b1.val[1]));

  *a0 = vreinterpretq_u16_u32(c0.val[0]);
  *a1 = vreinterpretq_u16_u32(c1.val[0]);
  *a2 = vreinterpretq_u16_u32(c0.val[1]);
  *a3 = vreinterpretq_u16_u32(c1.val[1]);
}

static INLINE void transpose_s32_8x4(int32x4_t *const a0, int32x4_t *const a1,
                                     int32x4_t *const a2, int32x4_t *const a3,
                                     int32x4_t *const a4, int32x4_t *const a5,
                                     int32x4_t *const a6, int32x4_t *const a7) {



















  const int32x4x2_t b0 = vtrnq_s32(*a0, *a2);
  const int32x4x2_t b1 = vtrnq_s32(*a1, *a3);
  const int32x4x2_t b2 = vtrnq_s32(*a4, *a6);
  const int32x4x2_t b3 = vtrnq_s32(*a5, *a7);










  const int64x2x2_t c0 = vpx_vtrnq_s64(b0.val[0], b2.val[0]);
  const int64x2x2_t c1 = vpx_vtrnq_s64(b0.val[1], b2.val[1]);
  const int64x2x2_t c2 = vpx_vtrnq_s64(b1.val[0], b3.val[0]);
  const int64x2x2_t c3 = vpx_vtrnq_s64(b1.val[1], b3.val[1]);

  *a0 = vreinterpretq_s32_s64(c0.val[0]);
  *a1 = vreinterpretq_s32_s64(c1.val[0]);
  *a2 = vreinterpretq_s32_s64(c0.val[1]);
  *a3 = vreinterpretq_s32_s64(c1.val[1]);
  *a4 = vreinterpretq_s32_s64(c2.val[0]);
  *a5 = vreinterpretq_s32_s64(c3.val[0]);
  *a6 = vreinterpretq_s32_s64(c2.val[1]);
  *a7 = vreinterpretq_s32_s64(c3.val[1]);
}

// 'q' registers here to save some instructions.
static INLINE void transpose_u8_8x8(uint8x8_t *a0, uint8x8_t *a1, uint8x8_t *a2,
                                    uint8x8_t *a3, uint8x8_t *a4, uint8x8_t *a5,
                                    uint8x8_t *a6, uint8x8_t *a7) {















  const uint8x16x2_t b0 =
      vtrnq_u8(vcombine_u8(*a0, *a4), vcombine_u8(*a1, *a5));
  const uint8x16x2_t b1 =
      vtrnq_u8(vcombine_u8(*a2, *a6), vcombine_u8(*a3, *a7));






  const uint16x8x2_t c0 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[0]),
                                    vreinterpretq_u16_u8(b1.val[0]));
  const uint16x8x2_t c1 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[1]),
                                    vreinterpretq_u16_u8(b1.val[1]));





  const uint32x4x2_t d0 = vuzpq_u32(vreinterpretq_u32_u16(c0.val[0]),
                                    vreinterpretq_u32_u16(c1.val[0]));
  const uint32x4x2_t d1 = vuzpq_u32(vreinterpretq_u32_u16(c0.val[1]),
                                    vreinterpretq_u32_u16(c1.val[1]));

  *a0 = vreinterpret_u8_u32(vget_low_u32(d0.val[0]));
  *a1 = vreinterpret_u8_u32(vget_high_u32(d0.val[0]));
  *a2 = vreinterpret_u8_u32(vget_low_u32(d1.val[0]));
  *a3 = vreinterpret_u8_u32(vget_high_u32(d1.val[0]));
  *a4 = vreinterpret_u8_u32(vget_low_u32(d0.val[1]));
  *a5 = vreinterpret_u8_u32(vget_high_u32(d0.val[1]));
  *a6 = vreinterpret_u8_u32(vget_low_u32(d1.val[1]));
  *a7 = vreinterpret_u8_u32(vget_high_u32(d1.val[1]));
}

static INLINE void transpose_s16_8x8(int16x8_t *a0, int16x8_t *a1,
                                     int16x8_t *a2, int16x8_t *a3,
                                     int16x8_t *a4, int16x8_t *a5,
                                     int16x8_t *a6, int16x8_t *a7) {



















  const int16x8x2_t b0 = vtrnq_s16(*a0, *a1);
  const int16x8x2_t b1 = vtrnq_s16(*a2, *a3);
  const int16x8x2_t b2 = vtrnq_s16(*a4, *a5);
  const int16x8x2_t b3 = vtrnq_s16(*a6, *a7);










  const int32x4x2_t c0 = vtrnq_s32(vreinterpretq_s32_s16(b0.val[0]),
                                   vreinterpretq_s32_s16(b1.val[0]));
  const int32x4x2_t c1 = vtrnq_s32(vreinterpretq_s32_s16(b0.val[1]),
                                   vreinterpretq_s32_s16(b1.val[1]));
  const int32x4x2_t c2 = vtrnq_s32(vreinterpretq_s32_s16(b2.val[0]),
                                   vreinterpretq_s32_s16(b3.val[0]));
  const int32x4x2_t c3 = vtrnq_s32(vreinterpretq_s32_s16(b2.val[1]),
                                   vreinterpretq_s32_s16(b3.val[1]));









  const int16x8x2_t d0 = vpx_vtrnq_s64_to_s16(c0.val[0], c2.val[0]);
  const int16x8x2_t d1 = vpx_vtrnq_s64_to_s16(c1.val[0], c3.val[0]);
  const int16x8x2_t d2 = vpx_vtrnq_s64_to_s16(c0.val[1], c2.val[1]);
  const int16x8x2_t d3 = vpx_vtrnq_s64_to_s16(c1.val[1], c3.val[1]);

  *a0 = d0.val[0];
  *a1 = d1.val[0];
  *a2 = d2.val[0];
  *a3 = d3.val[0];
  *a4 = d0.val[1];
  *a5 = d1.val[1];
  *a6 = d2.val[1];
  *a7 = d3.val[1];
}

static INLINE void transpose_u16_8x8(uint16x8_t *a0, uint16x8_t *a1,
                                     uint16x8_t *a2, uint16x8_t *a3,
                                     uint16x8_t *a4, uint16x8_t *a5,
                                     uint16x8_t *a6, uint16x8_t *a7) {



















  const uint16x8x2_t b0 = vtrnq_u16(*a0, *a1);
  const uint16x8x2_t b1 = vtrnq_u16(*a2, *a3);
  const uint16x8x2_t b2 = vtrnq_u16(*a4, *a5);
  const uint16x8x2_t b3 = vtrnq_u16(*a6, *a7);










  const uint32x4x2_t c0 = vtrnq_u32(vreinterpretq_u32_u16(b0.val[0]),
                                    vreinterpretq_u32_u16(b1.val[0]));
  const uint32x4x2_t c1 = vtrnq_u32(vreinterpretq_u32_u16(b0.val[1]),
                                    vreinterpretq_u32_u16(b1.val[1]));
  const uint32x4x2_t c2 = vtrnq_u32(vreinterpretq_u32_u16(b2.val[0]),
                                    vreinterpretq_u32_u16(b3.val[0]));
  const uint32x4x2_t c3 = vtrnq_u32(vreinterpretq_u32_u16(b2.val[1]),
                                    vreinterpretq_u32_u16(b3.val[1]));









  const uint16x8x2_t d0 = vpx_vtrnq_u64_to_u16(c0.val[0], c2.val[0]);
  const uint16x8x2_t d1 = vpx_vtrnq_u64_to_u16(c1.val[0], c3.val[0]);
  const uint16x8x2_t d2 = vpx_vtrnq_u64_to_u16(c0.val[1], c2.val[1]);
  const uint16x8x2_t d3 = vpx_vtrnq_u64_to_u16(c1.val[1], c3.val[1]);

  *a0 = d0.val[0];
  *a1 = d1.val[0];
  *a2 = d2.val[0];
  *a3 = d3.val[0];
  *a4 = d0.val[1];
  *a5 = d1.val[1];
  *a6 = d2.val[1];
  *a7 = d3.val[1];
}

static INLINE void transpose_s32_8x8(int32x4x2_t *a0, int32x4x2_t *a1,
                                     int32x4x2_t *a2, int32x4x2_t *a3,
                                     int32x4x2_t *a4, int32x4x2_t *a5,
                                     int32x4x2_t *a6, int32x4x2_t *a7) {



















  const int32x4x2_t b0 = vtrnq_s32(a0->val[0], a1->val[0]);
  const int32x4x2_t b1 = vtrnq_s32(a2->val[0], a3->val[0]);
  const int32x4x2_t b2 = vtrnq_s32(a4->val[0], a5->val[0]);
  const int32x4x2_t b3 = vtrnq_s32(a6->val[0], a7->val[0]);
  const int32x4x2_t b4 = vtrnq_s32(a0->val[1], a1->val[1]);
  const int32x4x2_t b5 = vtrnq_s32(a2->val[1], a3->val[1]);
  const int32x4x2_t b6 = vtrnq_s32(a4->val[1], a5->val[1]);
  const int32x4x2_t b7 = vtrnq_s32(a6->val[1], a7->val[1]);









  const int32x4x2_t c0 = vpx_vtrnq_s64_to_s32(b0.val[0], b1.val[0]);
  const int32x4x2_t c1 = vpx_vtrnq_s64_to_s32(b0.val[1], b1.val[1]);
  const int32x4x2_t c2 = vpx_vtrnq_s64_to_s32(b2.val[0], b3.val[0]);
  const int32x4x2_t c3 = vpx_vtrnq_s64_to_s32(b2.val[1], b3.val[1]);
  const int32x4x2_t c4 = vpx_vtrnq_s64_to_s32(b4.val[0], b5.val[0]);
  const int32x4x2_t c5 = vpx_vtrnq_s64_to_s32(b4.val[1], b5.val[1]);
  const int32x4x2_t c6 = vpx_vtrnq_s64_to_s32(b6.val[0], b7.val[0]);
  const int32x4x2_t c7 = vpx_vtrnq_s64_to_s32(b6.val[1], b7.val[1]);









  a0->val[0] = c0.val[0];
  a0->val[1] = c2.val[0];
  a1->val[0] = c1.val[0];
  a1->val[1] = c3.val[0];
  a2->val[0] = c0.val[1];
  a2->val[1] = c2.val[1];
  a3->val[0] = c1.val[1];
  a3->val[1] = c3.val[1];
  a4->val[0] = c4.val[0];
  a4->val[1] = c6.val[0];
  a5->val[0] = c5.val[0];
  a5->val[1] = c7.val[0];
  a6->val[0] = c4.val[1];
  a6->val[1] = c6.val[1];
  a7->val[0] = c5.val[1];
  a7->val[1] = c7.val[1];
}

static INLINE void transpose_u8_16x8(
    const uint8x16_t i0, const uint8x16_t i1, const uint8x16_t i2,
    const uint8x16_t i3, const uint8x16_t i4, const uint8x16_t i5,
    const uint8x16_t i6, const uint8x16_t i7, uint8x8_t *o0, uint8x8_t *o1,
    uint8x8_t *o2, uint8x8_t *o3, uint8x8_t *o4, uint8x8_t *o5, uint8x8_t *o6,
    uint8x8_t *o7, uint8x8_t *o8, uint8x8_t *o9, uint8x8_t *o10, uint8x8_t *o11,
    uint8x8_t *o12, uint8x8_t *o13, uint8x8_t *o14, uint8x8_t *o15) {


















  const uint8x16x2_t b0 = vtrnq_u8(i0, i1);
  const uint8x16x2_t b1 = vtrnq_u8(i2, i3);
  const uint8x16x2_t b2 = vtrnq_u8(i4, i5);
  const uint8x16x2_t b3 = vtrnq_u8(i6, i7);









  const uint16x8x2_t c0 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[0]),
                                    vreinterpretq_u16_u8(b1.val[0]));
  const uint16x8x2_t c1 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[1]),
                                    vreinterpretq_u16_u8(b1.val[1]));
  const uint16x8x2_t c2 = vtrnq_u16(vreinterpretq_u16_u8(b2.val[0]),
                                    vreinterpretq_u16_u8(b3.val[0]));
  const uint16x8x2_t c3 = vtrnq_u16(vreinterpretq_u16_u8(b2.val[1]),
                                    vreinterpretq_u16_u8(b3.val[1]));









  const uint32x4x2_t d0 = vtrnq_u32(vreinterpretq_u32_u16(c0.val[0]),
                                    vreinterpretq_u32_u16(c2.val[0]));
  const uint32x4x2_t d1 = vtrnq_u32(vreinterpretq_u32_u16(c0.val[1]),
                                    vreinterpretq_u32_u16(c2.val[1]));
  const uint32x4x2_t d2 = vtrnq_u32(vreinterpretq_u32_u16(c1.val[0]),
                                    vreinterpretq_u32_u16(c3.val[0]));
  const uint32x4x2_t d3 = vtrnq_u32(vreinterpretq_u32_u16(c1.val[1]),
                                    vreinterpretq_u32_u16(c3.val[1]));

















  *o0 = vget_low_u8(vreinterpretq_u8_u32(d0.val[0]));
  *o1 = vget_low_u8(vreinterpretq_u8_u32(d2.val[0]));
  *o2 = vget_low_u8(vreinterpretq_u8_u32(d1.val[0]));
  *o3 = vget_low_u8(vreinterpretq_u8_u32(d3.val[0]));
  *o4 = vget_low_u8(vreinterpretq_u8_u32(d0.val[1]));
  *o5 = vget_low_u8(vreinterpretq_u8_u32(d2.val[1]));
  *o6 = vget_low_u8(vreinterpretq_u8_u32(d1.val[1]));
  *o7 = vget_low_u8(vreinterpretq_u8_u32(d3.val[1]));
  *o8 = vget_high_u8(vreinterpretq_u8_u32(d0.val[0]));
  *o9 = vget_high_u8(vreinterpretq_u8_u32(d2.val[0]));
  *o10 = vget_high_u8(vreinterpretq_u8_u32(d1.val[0]));
  *o11 = vget_high_u8(vreinterpretq_u8_u32(d3.val[0]));
  *o12 = vget_high_u8(vreinterpretq_u8_u32(d0.val[1]));
  *o13 = vget_high_u8(vreinterpretq_u8_u32(d2.val[1]));
  *o14 = vget_high_u8(vreinterpretq_u8_u32(d1.val[1]));
  *o15 = vget_high_u8(vreinterpretq_u8_u32(d3.val[1]));
}

static INLINE void transpose_u8_8x16(
    const uint8x8_t i0, const uint8x8_t i1, const uint8x8_t i2,
    const uint8x8_t i3, const uint8x8_t i4, const uint8x8_t i5,
    const uint8x8_t i6, const uint8x8_t i7, const uint8x8_t i8,
    const uint8x8_t i9, const uint8x8_t i10, const uint8x8_t i11,
    const uint8x8_t i12, const uint8x8_t i13, const uint8x8_t i14,
    const uint8x8_t i15, uint8x16_t *o0, uint8x16_t *o1, uint8x16_t *o2,
    uint8x16_t *o3, uint8x16_t *o4, uint8x16_t *o5, uint8x16_t *o6,
    uint8x16_t *o7) {


























  const uint8x16_t a0 = vcombine_u8(i0, i8);
  const uint8x16_t a1 = vcombine_u8(i1, i9);
  const uint8x16_t a2 = vcombine_u8(i2, i10);
  const uint8x16_t a3 = vcombine_u8(i3, i11);
  const uint8x16_t a4 = vcombine_u8(i4, i12);
  const uint8x16_t a5 = vcombine_u8(i5, i13);
  const uint8x16_t a6 = vcombine_u8(i6, i14);
  const uint8x16_t a7 = vcombine_u8(i7, i15);









  const uint8x16x2_t b0 = vtrnq_u8(a0, a1);
  const uint8x16x2_t b1 = vtrnq_u8(a2, a3);
  const uint8x16x2_t b2 = vtrnq_u8(a4, a5);
  const uint8x16x2_t b3 = vtrnq_u8(a6, a7);









  const uint16x8x2_t c0 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[0]),
                                    vreinterpretq_u16_u8(b1.val[0]));
  const uint16x8x2_t c1 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[1]),
                                    vreinterpretq_u16_u8(b1.val[1]));
  const uint16x8x2_t c2 = vtrnq_u16(vreinterpretq_u16_u8(b2.val[0]),
                                    vreinterpretq_u16_u8(b3.val[0]));
  const uint16x8x2_t c3 = vtrnq_u16(vreinterpretq_u16_u8(b2.val[1]),
                                    vreinterpretq_u16_u8(b3.val[1]));









  const uint32x4x2_t d0 = vtrnq_u32(vreinterpretq_u32_u16(c0.val[0]),
                                    vreinterpretq_u32_u16(c2.val[0]));
  const uint32x4x2_t d1 = vtrnq_u32(vreinterpretq_u32_u16(c0.val[1]),
                                    vreinterpretq_u32_u16(c2.val[1]));
  const uint32x4x2_t d2 = vtrnq_u32(vreinterpretq_u32_u16(c1.val[0]),
                                    vreinterpretq_u32_u16(c3.val[0]));
  const uint32x4x2_t d3 = vtrnq_u32(vreinterpretq_u32_u16(c1.val[1]),
                                    vreinterpretq_u32_u16(c3.val[1]));









  *o0 = vreinterpretq_u8_u32(d0.val[0]);
  *o1 = vreinterpretq_u8_u32(d2.val[0]);
  *o2 = vreinterpretq_u8_u32(d1.val[0]);
  *o3 = vreinterpretq_u8_u32(d3.val[0]);
  *o4 = vreinterpretq_u8_u32(d0.val[1]);
  *o5 = vreinterpretq_u8_u32(d2.val[1]);
  *o6 = vreinterpretq_u8_u32(d1.val[1]);
  *o7 = vreinterpretq_u8_u32(d3.val[1]);
}

static INLINE void transpose_u8_16x16(
    const uint8x16_t i0, const uint8x16_t i1, const uint8x16_t i2,
    const uint8x16_t i3, const uint8x16_t i4, const uint8x16_t i5,
    const uint8x16_t i6, const uint8x16_t i7, const uint8x16_t i8,
    const uint8x16_t i9, const uint8x16_t i10, const uint8x16_t i11,
    const uint8x16_t i12, const uint8x16_t i13, const uint8x16_t i14,
    const uint8x16_t i15, uint8x16_t *o0, uint8x16_t *o1, uint8x16_t *o2,
    uint8x16_t *o3, uint8x16_t *o4, uint8x16_t *o5, uint8x16_t *o6,
    uint8x16_t *o7, uint8x16_t *o8, uint8x16_t *o9, uint8x16_t *o10,
    uint8x16_t *o11, uint8x16_t *o12, uint8x16_t *o13, uint8x16_t *o14,
    uint8x16_t *o15) {


































  const uint8x16x2_t b0 = vtrnq_u8(i0, i1);
  const uint8x16x2_t b1 = vtrnq_u8(i2, i3);
  const uint8x16x2_t b2 = vtrnq_u8(i4, i5);
  const uint8x16x2_t b3 = vtrnq_u8(i6, i7);
  const uint8x16x2_t b4 = vtrnq_u8(i8, i9);
  const uint8x16x2_t b5 = vtrnq_u8(i10, i11);
  const uint8x16x2_t b6 = vtrnq_u8(i12, i13);
  const uint8x16x2_t b7 = vtrnq_u8(i14, i15);

















  const uint16x8x2_t c0 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[0]),
                                    vreinterpretq_u16_u8(b1.val[0]));
  const uint16x8x2_t c1 = vtrnq_u16(vreinterpretq_u16_u8(b0.val[1]),
                                    vreinterpretq_u16_u8(b1.val[1]));
  const uint16x8x2_t c2 = vtrnq_u16(vreinterpretq_u16_u8(b2.val[0]),
                                    vreinterpretq_u16_u8(b3.val[0]));
  const uint16x8x2_t c3 = vtrnq_u16(vreinterpretq_u16_u8(b2.val[1]),
                                    vreinterpretq_u16_u8(b3.val[1]));
  const uint16x8x2_t c4 = vtrnq_u16(vreinterpretq_u16_u8(b4.val[0]),
                                    vreinterpretq_u16_u8(b5.val[0]));
  const uint16x8x2_t c5 = vtrnq_u16(vreinterpretq_u16_u8(b4.val[1]),
                                    vreinterpretq_u16_u8(b5.val[1]));
  const uint16x8x2_t c6 = vtrnq_u16(vreinterpretq_u16_u8(b6.val[0]),
                                    vreinterpretq_u16_u8(b7.val[0]));
  const uint16x8x2_t c7 = vtrnq_u16(vreinterpretq_u16_u8(b6.val[1]),
                                    vreinterpretq_u16_u8(b7.val[1]));

















  const uint32x4x2_t d0 = vtrnq_u32(vreinterpretq_u32_u16(c0.val[0]),
                                    vreinterpretq_u32_u16(c2.val[0]));
  const uint32x4x2_t d1 = vtrnq_u32(vreinterpretq_u32_u16(c0.val[1]),
                                    vreinterpretq_u32_u16(c2.val[1]));
  const uint32x4x2_t d2 = vtrnq_u32(vreinterpretq_u32_u16(c1.val[0]),
                                    vreinterpretq_u32_u16(c3.val[0]));
  const uint32x4x2_t d3 = vtrnq_u32(vreinterpretq_u32_u16(c1.val[1]),
                                    vreinterpretq_u32_u16(c3.val[1]));
  const uint32x4x2_t d4 = vtrnq_u32(vreinterpretq_u32_u16(c4.val[0]),
                                    vreinterpretq_u32_u16(c6.val[0]));
  const uint32x4x2_t d5 = vtrnq_u32(vreinterpretq_u32_u16(c4.val[1]),
                                    vreinterpretq_u32_u16(c6.val[1]));
  const uint32x4x2_t d6 = vtrnq_u32(vreinterpretq_u32_u16(c5.val[0]),
                                    vreinterpretq_u32_u16(c7.val[0]));
  const uint32x4x2_t d7 = vtrnq_u32(vreinterpretq_u32_u16(c5.val[1]),
                                    vreinterpretq_u32_u16(c7.val[1]));

















  const uint8x16x2_t e0 = vpx_vtrnq_u64_to_u8(d0.val[0], d4.val[0]);
  const uint8x16x2_t e1 = vpx_vtrnq_u64_to_u8(d2.val[0], d6.val[0]);
  const uint8x16x2_t e2 = vpx_vtrnq_u64_to_u8(d1.val[0], d5.val[0]);
  const uint8x16x2_t e3 = vpx_vtrnq_u64_to_u8(d3.val[0], d7.val[0]);
  const uint8x16x2_t e4 = vpx_vtrnq_u64_to_u8(d0.val[1], d4.val[1]);
  const uint8x16x2_t e5 = vpx_vtrnq_u64_to_u8(d2.val[1], d6.val[1]);
  const uint8x16x2_t e6 = vpx_vtrnq_u64_to_u8(d1.val[1], d5.val[1]);
  const uint8x16x2_t e7 = vpx_vtrnq_u64_to_u8(d3.val[1], d7.val[1]);

















  *o0 = e0.val[0];
  *o1 = e1.val[0];
  *o2 = e2.val[0];
  *o3 = e3.val[0];
  *o4 = e4.val[0];
  *o5 = e5.val[0];
  *o6 = e6.val[0];
  *o7 = e7.val[0];
  *o8 = e0.val[1];
  *o9 = e1.val[1];
  *o10 = e2.val[1];
  *o11 = e3.val[1];
  *o12 = e4.val[1];
  *o13 = e5.val[1];
  *o14 = e6.val[1];
  *o15 = e7.val[1];
}

static INLINE void load_and_transpose_u8_4x8(const uint8_t *a,
                                             const int a_stride, uint8x8_t *a0,
                                             uint8x8_t *a1, uint8x8_t *a2,
                                             uint8x8_t *a3) {
  uint8x8_t a4, a5, a6, a7;
  *a0 = vld1_u8(a);
  a += a_stride;
  *a1 = vld1_u8(a);
  a += a_stride;
  *a2 = vld1_u8(a);
  a += a_stride;
  *a3 = vld1_u8(a);
  a += a_stride;
  a4 = vld1_u8(a);
  a += a_stride;
  a5 = vld1_u8(a);
  a += a_stride;
  a6 = vld1_u8(a);
  a += a_stride;
  a7 = vld1_u8(a);

  transpose_u8_4x8(a0, a1, a2, a3, a4, a5, a6, a7);
}

static INLINE void load_and_transpose_u8_8x8(const uint8_t *a,
                                             const int a_stride, uint8x8_t *a0,
                                             uint8x8_t *a1, uint8x8_t *a2,
                                             uint8x8_t *a3, uint8x8_t *a4,
                                             uint8x8_t *a5, uint8x8_t *a6,
                                             uint8x8_t *a7) {
  *a0 = vld1_u8(a);
  a += a_stride;
  *a1 = vld1_u8(a);
  a += a_stride;
  *a2 = vld1_u8(a);
  a += a_stride;
  *a3 = vld1_u8(a);
  a += a_stride;
  *a4 = vld1_u8(a);
  a += a_stride;
  *a5 = vld1_u8(a);
  a += a_stride;
  *a6 = vld1_u8(a);
  a += a_stride;
  *a7 = vld1_u8(a);

  transpose_u8_8x8(a0, a1, a2, a3, a4, a5, a6, a7);
}

static INLINE void transpose_and_store_u8_8x8(uint8_t *a, const int a_stride,
                                              uint8x8_t a0, uint8x8_t a1,
                                              uint8x8_t a2, uint8x8_t a3,
                                              uint8x8_t a4, uint8x8_t a5,
                                              uint8x8_t a6, uint8x8_t a7) {
  transpose_u8_8x8(&a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7);

  vst1_u8(a, a0);
  a += a_stride;
  vst1_u8(a, a1);
  a += a_stride;
  vst1_u8(a, a2);
  a += a_stride;
  vst1_u8(a, a3);
  a += a_stride;
  vst1_u8(a, a4);
  a += a_stride;
  vst1_u8(a, a5);
  a += a_stride;
  vst1_u8(a, a6);
  a += a_stride;
  vst1_u8(a, a7);
}

static INLINE void load_and_transpose_s16_8x8(const int16_t *a,
                                              const int a_stride, int16x8_t *a0,
                                              int16x8_t *a1, int16x8_t *a2,
                                              int16x8_t *a3, int16x8_t *a4,
                                              int16x8_t *a5, int16x8_t *a6,
                                              int16x8_t *a7) {
  *a0 = vld1q_s16(a);
  a += a_stride;
  *a1 = vld1q_s16(a);
  a += a_stride;
  *a2 = vld1q_s16(a);
  a += a_stride;
  *a3 = vld1q_s16(a);
  a += a_stride;
  *a4 = vld1q_s16(a);
  a += a_stride;
  *a5 = vld1q_s16(a);
  a += a_stride;
  *a6 = vld1q_s16(a);
  a += a_stride;
  *a7 = vld1q_s16(a);

  transpose_s16_8x8(a0, a1, a2, a3, a4, a5, a6, a7);
}

static INLINE void load_and_transpose_s32_8x8(
    const int32_t *a, const int a_stride, int32x4x2_t *const a0,
    int32x4x2_t *const a1, int32x4x2_t *const a2, int32x4x2_t *const a3,
    int32x4x2_t *const a4, int32x4x2_t *const a5, int32x4x2_t *const a6,
    int32x4x2_t *const a7) {
  a0->val[0] = vld1q_s32(a);
  a0->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a1->val[0] = vld1q_s32(a);
  a1->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a2->val[0] = vld1q_s32(a);
  a2->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a3->val[0] = vld1q_s32(a);
  a3->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a4->val[0] = vld1q_s32(a);
  a4->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a5->val[0] = vld1q_s32(a);
  a5->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a6->val[0] = vld1q_s32(a);
  a6->val[1] = vld1q_s32(a + 4);
  a += a_stride;
  a7->val[0] = vld1q_s32(a);
  a7->val[1] = vld1q_s32(a + 4);

  transpose_s32_8x8(a0, a1, a2, a3, a4, a5, a6, a7);
}
#endif  // VPX_VPX_DSP_ARM_TRANSPOSE_NEON_H_
