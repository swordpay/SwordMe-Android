/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_AQ_CYCLICREFRESH_H_
#define VPX_VP9_ENCODER_VP9_AQ_CYCLICREFRESH_H_

#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/encoder/vp9_block.h"
#include "vp9/encoder/vp9_skin_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

// boost (higher delta-qp).
#define CR_SEGMENT_ID_BASE 0
#define CR_SEGMENT_ID_BOOST1 1
#define CR_SEGMENT_ID_BOOST2 2

#define CR_MAX_RATE_TARGET_RATIO 4.0

struct CYCLIC_REFRESH {


  int percent_refresh;

  int max_qdelta_perc;

  int sb_index;



  int time_for_refresh;

  int target_num_seg_blocks;

  int actual_num_seg1_blocks;
  int actual_num_seg2_blocks;

  int rdmult;

  signed char *map;

  uint8_t *last_coded_q_map;


  int64_t thresh_rate_sb;
  int64_t thresh_dist_sb;


  int16_t motion_thresh;

  double rate_ratio_qdelta;

  int rate_boost_fac;
  double low_content_avg;
  int qindex_delta[3];
  int reduce_refresh;
  double weight_segment;
  int apply_cyclic_refresh;
  int counter_encode_maxq_scene_change;
  int skip_flat_static_blocks;
};

struct VP9_COMP;

typedef struct CYCLIC_REFRESH CYCLIC_REFRESH;

CYCLIC_REFRESH *vp9_cyclic_refresh_alloc(int mi_rows, int mi_cols);

void vp9_cyclic_refresh_free(CYCLIC_REFRESH *cr);

// the frame.
int vp9_cyclic_refresh_estimate_bits_at_q(const struct VP9_COMP *cpi,
                                          double correction_factor);

// (for segment 1), prior to encoding the frame.
int vp9_cyclic_refresh_rc_bits_per_mb(const struct VP9_COMP *cpi, int i,
                                      double correction_factor);

// check if we should reset the segment_id, and update the cyclic_refresh map
// and segmentation map.
void vp9_cyclic_refresh_update_segment(struct VP9_COMP *const cpi,
                                       MODE_INFO *const mi, int mi_row,
                                       int mi_col, BLOCK_SIZE bsize,
                                       int64_t rate, int64_t dist, int skip,
                                       struct macroblock_plane *const p);

void vp9_cyclic_refresh_update_sb_postencode(struct VP9_COMP *const cpi,
                                             const MODE_INFO *const mi,
                                             int mi_row, int mi_col,
                                             BLOCK_SIZE bsize);

// applied the segment delta q, and the amount of low motion in the frame.
// Also check conditions for forcing golden update, or preventing golden
// update if the period is up.
void vp9_cyclic_refresh_postencode(struct VP9_COMP *const cpi);

void vp9_cyclic_refresh_set_golden_update(struct VP9_COMP *const cpi);

void vp9_cyclic_refresh_update_parameters(struct VP9_COMP *const cpi);

void vp9_cyclic_refresh_setup(struct VP9_COMP *const cpi);

int vp9_cyclic_refresh_get_rdmult(const CYCLIC_REFRESH *cr);

void vp9_cyclic_refresh_reset_resize(struct VP9_COMP *const cpi);

static INLINE int cyclic_refresh_segment_id_boosted(int segment_id) {
  return segment_id == CR_SEGMENT_ID_BOOST1 ||
         segment_id == CR_SEGMENT_ID_BOOST2;
}

static INLINE int cyclic_refresh_segment_id(int segment_id) {
  if (segment_id == CR_SEGMENT_ID_BOOST1)
    return CR_SEGMENT_ID_BOOST1;
  else if (segment_id == CR_SEGMENT_ID_BOOST2)
    return CR_SEGMENT_ID_BOOST2;
  else
    return CR_SEGMENT_ID_BASE;
}

void vp9_cyclic_refresh_limit_q(const struct VP9_COMP *cpi, int *q);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VP9_ENCODER_VP9_AQ_CYCLICREFRESH_H_
