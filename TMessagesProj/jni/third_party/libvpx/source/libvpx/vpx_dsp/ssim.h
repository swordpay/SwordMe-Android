/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_DSP_SSIM_H_
#define VPX_VPX_DSP_SSIM_H_

#define MAX_SSIM_DB 100.0;

#ifdef __cplusplus
extern "C" {
#endif

#include "./vpx_config.h"
#include "vpx_scale/yv12config.h"

typedef struct {

  uint32_t sum_s;

  uint32_t sum_r;

  uint32_t sum_sq_s;

  uint32_t sum_sq_r;

  uint32_t sum_sxr;

  double ssim;
} Ssimv;

typedef struct {

  double ssimc;

  double ssim;

  double ssim2;

  double dssim;

  double dssimd;

  double ssimcd;
} Metrics;

double vpx_get_ssim_metrics(uint8_t *img1, int img1_pitch, uint8_t *img2,
                            int img2_pitch, int width, int height, Ssimv *sv2,
                            Metrics *m, int do_inconsistency);

double vpx_calc_ssim(const YV12_BUFFER_CONFIG *source,
                     const YV12_BUFFER_CONFIG *dest, double *weight);

double vpx_calc_fastssim(const YV12_BUFFER_CONFIG *source,
                         const YV12_BUFFER_CONFIG *dest, double *ssim_y,
                         double *ssim_u, double *ssim_v, uint32_t bd,
                         uint32_t in_bd);

#if CONFIG_VP9_HIGHBITDEPTH
double vpx_highbd_calc_ssim(const YV12_BUFFER_CONFIG *source,
                            const YV12_BUFFER_CONFIG *dest, double *weight,
                            uint32_t bd, uint32_t in_bd);
#endif  // CONFIG_VP9_HIGHBITDEPTH

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VPX_DSP_SSIM_H_
