/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_BLOCK_H_
#define VPX_VP9_ENCODER_VP9_BLOCK_H_

#include "vpx_util/vpx_thread.h"

#include "vp9/common/vp9_entropymv.h"
#include "vp9/common/vp9_entropy.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned int sse;
  int sum;
  unsigned int var;
} diff;

struct macroblock_plane {
  DECLARE_ALIGNED(16, int16_t, src_diff[64 * 64]);
  tran_low_t *qcoeff;
  tran_low_t *coeff;
  uint16_t *eobs;
  struct buf_2d src;

  DECLARE_ALIGNED(16, int16_t, round_fp[8]);
  int16_t *quant_fp;
  int16_t *quant;
  int16_t *quant_shift;
  int16_t *zbin;
  int16_t *round;

  int64_t quant_thred[2];
};

/* The [2] dimension is for whether we skip the EOB node (i.e. if previous
 * coefficient in this block was zero) or not. */
typedef unsigned int vp9_coeff_cost[PLANE_TYPES][REF_TYPES][COEF_BANDS][2]
                                   [COEFF_CONTEXTS][ENTROPY_TOKENS];

typedef struct {
  int_mv ref_mvs[MAX_REF_FRAMES][MAX_MV_REF_CANDIDATES];
  uint8_t mode_context[MAX_REF_FRAMES];
} MB_MODE_INFO_EXT;

typedef struct {
  int col_min;
  int col_max;
  int row_min;
  int row_max;
} MvLimits;

typedef struct macroblock MACROBLOCK;
struct macroblock {
// cf. https://bugs.chromium.org/p/webm/issues/detail?id=1054
#if defined(_MSC_VER) && _MSC_VER < 1900
  int64_t bsse[MAX_MB_PLANE << 2];
#endif

  struct macroblock_plane plane[MAX_MB_PLANE];

  MACROBLOCKD e_mbd;
  MB_MODE_INFO_EXT *mbmi_ext;
  MB_MODE_INFO_EXT *mbmi_ext_base;
  int skip_block;
  int select_tx_size;
  int skip_recode;
  int skip_optimize;
  int q_index;
  int block_qcoeff_opt;
  int block_tx_domain;


  int errorperbit;


  int sadperbit16;


  int sadperbit4;
  int rddiv;
  int rdmult;
  int cb_rdmult;
  int segment_id;
  int mb_energy;


  BLOCK_SIZE min_partition_size;
  BLOCK_SIZE max_partition_size;

  int mv_best_ref_index[MAX_REF_FRAMES];
  unsigned int max_mv_context[MAX_REF_FRAMES];
  unsigned int source_variance;
  unsigned int pred_sse[MAX_REF_FRAMES];
  int pred_mv_sad[MAX_REF_FRAMES];

  int nmvjointcost[MV_JOINTS];
  int *nmvcost[2];
  int *nmvcost_hp[2];
  int **mvcost;

  int nmvjointsadcost[MV_JOINTS];
  int *nmvsadcost[2];
  int *nmvsadcost_hp[2];
  int **mvsadcost;

  int sharpness;

  int adjust_rdmult_by_segment;


  MvLimits mv_limits;


  uint8_t zcoeff_blk[TX_SIZES][256];

  int32_t sum_y_eobs[TX_SIZES];

  int skip;

  int encode_breakout;

  vp9_coeff_cost token_costs[TX_SIZES];

  int optimize;

  int use_lp32x32fdct;
  int skip_encode;


  int fp_src_pred;

  int quant_fp;

  uint8_t skip_txfm[MAX_MB_PLANE << 2];
#define SKIP_TXFM_NONE 0
#define SKIP_TXFM_AC_DC 1
#define SKIP_TXFM_AC_ONLY 2

#if !defined(_MSC_VER) || _MSC_VER >= 1900
  int64_t bsse[MAX_MB_PLANE << 2];
#endif

  MV pred_mv[MAX_REF_FRAMES];


  uint8_t color_sensitivity[2];

  uint8_t sb_is_skin;

  uint8_t skip_low_source_sad;

  uint8_t lowvar_highsumdiff;

  uint8_t last_sb_high_content;

  int sb_use_mv_part;

  int sb_mvcol_part;

  int sb_mvrow_part;

  int sb_pickmode_part;

  int zero_temp_sad_source;


  uint8_t content_state_sb;



  uint8_t variance_low[25];

  uint8_t arf_frame_usage;
  uint8_t lastgolden_frame_usage;

  void (*fwd_txfm4x4)(const int16_t *input, tran_low_t *output, int stride);
  void (*inv_txfm_add)(const tran_low_t *input, uint8_t *dest, int stride,
                       int eob);
#if CONFIG_VP9_HIGHBITDEPTH
  void (*highbd_inv_txfm_add)(const tran_low_t *input, uint16_t *dest,
                              int stride, int eob, int bd);
#endif
  DECLARE_ALIGNED(16, uint8_t, est_pred[64 * 64]);

  struct scale_factors *me_sf;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VP9_ENCODER_VP9_BLOCK_H_
