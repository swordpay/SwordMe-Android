/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <limits.h>
#include <math.h>

#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_ports/system_state.h"

#include "vp9/encoder/vp9_aq_cyclicrefresh.h"

#include "vp9/common/vp9_seg_common.h"

#include "vp9/encoder/vp9_ratectrl.h"
#include "vp9/encoder/vp9_segmentation.h"

static const uint8_t VP9_VAR_OFFS[64] = {
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};

CYCLIC_REFRESH *vp9_cyclic_refresh_alloc(int mi_rows, int mi_cols) {
  size_t last_coded_q_map_size;
  CYCLIC_REFRESH *const cr = vpx_calloc(1, sizeof(*cr));
  if (cr == NULL) return NULL;

  cr->map = vpx_calloc(mi_rows * mi_cols, sizeof(*cr->map));
  if (cr->map == NULL) {
    vp9_cyclic_refresh_free(cr);
    return NULL;
  }
  last_coded_q_map_size = mi_rows * mi_cols * sizeof(*cr->last_coded_q_map);
  cr->last_coded_q_map = vpx_malloc(last_coded_q_map_size);
  if (cr->last_coded_q_map == NULL) {
    vp9_cyclic_refresh_free(cr);
    return NULL;
  }
  assert(MAXQ <= 255);
  memset(cr->last_coded_q_map, MAXQ, last_coded_q_map_size);
  cr->counter_encode_maxq_scene_change = 0;
  return cr;
}

void vp9_cyclic_refresh_free(CYCLIC_REFRESH *cr) {
  if (cr != NULL) {
    vpx_free(cr->map);
    vpx_free(cr->last_coded_q_map);
    vpx_free(cr);
  }
}

// (lower-qp coding). Decision can be based on various factors, such as
// size of the coding block (i.e., below min_block size rejected), coding
// mode, and rate/distortion.
static int candidate_refresh_aq(const CYCLIC_REFRESH *cr, const MODE_INFO *mi,
                                int64_t rate, int64_t dist, int bsize) {
  MV mv = mi->mv[0].as_mv;





  if (dist > cr->thresh_dist_sb &&
      (mv.row > cr->motion_thresh || mv.row < -cr->motion_thresh ||
       mv.col > cr->motion_thresh || mv.col < -cr->motion_thresh ||
       !is_inter_block(mi)))
    return CR_SEGMENT_ID_BASE;
  else if (bsize >= BLOCK_16X16 && rate < cr->thresh_rate_sb &&
           is_inter_block(mi) && mi->mv[0].as_int == 0 &&
           cr->rate_boost_fac > 10)

    return CR_SEGMENT_ID_BOOST2;
  else
    return CR_SEGMENT_ID_BOOST1;
}

static int compute_deltaq(const VP9_COMP *cpi, int q, double rate_factor) {
  const CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  const RATE_CONTROL *const rc = &cpi->rc;
  int deltaq = vp9_compute_qdelta_by_rate(rc, cpi->common.frame_type, q,
                                          rate_factor, cpi->common.bit_depth);
  if ((-deltaq) > cr->max_qdelta_perc * q / 100) {
    deltaq = -cr->max_qdelta_perc * q / 100;
  }
  return deltaq;
}

// from non-base segment. For now ignore effect of multiple segments
// (with different delta-q). Note this function is called in the postencode
// (called from rc_update_rate_correction_factors()).
int vp9_cyclic_refresh_estimate_bits_at_q(const VP9_COMP *cpi,
                                          double correction_factor) {
  const VP9_COMMON *const cm = &cpi->common;
  const CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  int estimated_bits;
  int mbs = cm->MBs;
  int num8x8bl = mbs << 2;


  double weight_segment1 = (double)cr->actual_num_seg1_blocks / num8x8bl;
  double weight_segment2 = (double)cr->actual_num_seg2_blocks / num8x8bl;

  estimated_bits =
      (int)((1.0 - weight_segment1 - weight_segment2) *
                vp9_estimate_bits_at_q(cm->frame_type, cm->base_qindex, mbs,
                                       correction_factor, cm->bit_depth) +
            weight_segment1 *
                vp9_estimate_bits_at_q(cm->frame_type,
                                       cm->base_qindex + cr->qindex_delta[1],
                                       mbs, correction_factor, cm->bit_depth) +
            weight_segment2 *
                vp9_estimate_bits_at_q(cm->frame_type,
                                       cm->base_qindex + cr->qindex_delta[2],
                                       mbs, correction_factor, cm->bit_depth));
  return estimated_bits;
}

// a corresponding delta-q (for segment 1). This function is called in the
// rc_regulate_q() to set the base qp index.
// Note: the segment map is set to either 0/CR_SEGMENT_ID_BASE (no refresh) or
// to 1/CR_SEGMENT_ID_BOOST1 (refresh) for each superblock, prior to encoding.
int vp9_cyclic_refresh_rc_bits_per_mb(const VP9_COMP *cpi, int i,
                                      double correction_factor) {
  const VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  int bits_per_mb;
  int deltaq = 0;
  if (cpi->oxcf.speed < 8)
    deltaq = compute_deltaq(cpi, i, cr->rate_ratio_qdelta);
  else
    deltaq = -(cr->max_qdelta_perc * i) / 200;

  bits_per_mb = (int)((1.0 - cr->weight_segment) *
                          vp9_rc_bits_per_mb(cm->frame_type, i,
                                             correction_factor, cm->bit_depth) +
                      cr->weight_segment *
                          vp9_rc_bits_per_mb(cm->frame_type, i + deltaq,
                                             correction_factor, cm->bit_depth));
  return bits_per_mb;
}

// check if we should reset the segment_id, and update the cyclic_refresh map
// and segmentation map.
void vp9_cyclic_refresh_update_segment(VP9_COMP *const cpi, MODE_INFO *const mi,
                                       int mi_row, int mi_col, BLOCK_SIZE bsize,
                                       int64_t rate, int64_t dist, int skip,
                                       struct macroblock_plane *const p) {
  const VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  const int bw = num_8x8_blocks_wide_lookup[bsize];
  const int bh = num_8x8_blocks_high_lookup[bsize];
  const int xmis = VPXMIN(cm->mi_cols - mi_col, bw);
  const int ymis = VPXMIN(cm->mi_rows - mi_row, bh);
  const int block_index = mi_row * cm->mi_cols + mi_col;
  int refresh_this_block = candidate_refresh_aq(cr, mi, rate, dist, bsize);

  int new_map_value = cr->map[block_index];
  int x = 0;
  int y = 0;

  int is_skin = 0;
  if (refresh_this_block == 0 && bsize <= BLOCK_16X16 &&
      cpi->use_skin_detection) {
    is_skin =
        vp9_compute_skin_block(p[0].src.buf, p[1].src.buf, p[2].src.buf,
                               p[0].src.stride, p[1].src.stride, bsize, 0, 0);
    if (is_skin) refresh_this_block = 1;
  }

  if (cpi->oxcf.rc_mode == VPX_VBR && mi->ref_frame[0] == GOLDEN_FRAME)
    refresh_this_block = 0;


  if (cpi->sf.use_nonrd_pick_mode &&
      cyclic_refresh_segment_id_boosted(mi->segment_id)) {
    mi->segment_id = refresh_this_block;

    if (skip) mi->segment_id = CR_SEGMENT_ID_BASE;
  }




  if (cyclic_refresh_segment_id_boosted(mi->segment_id)) {
    new_map_value = -cr->time_for_refresh;
  } else if (refresh_this_block) {



    if (cr->map[block_index] == 1) new_map_value = 0;
  } else {

    new_map_value = 1;
  }


  for (y = 0; y < ymis; y++)
    for (x = 0; x < xmis; x++) {
      int map_offset = block_index + y * cm->mi_cols + x;
      cr->map[map_offset] = new_map_value;
      cpi->segmentation_map[map_offset] = mi->segment_id;
    }
}

void vp9_cyclic_refresh_update_sb_postencode(VP9_COMP *const cpi,
                                             const MODE_INFO *const mi,
                                             int mi_row, int mi_col,
                                             BLOCK_SIZE bsize) {
  const VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  const int bw = num_8x8_blocks_wide_lookup[bsize];
  const int bh = num_8x8_blocks_high_lookup[bsize];
  const int xmis = VPXMIN(cm->mi_cols - mi_col, bw);
  const int ymis = VPXMIN(cm->mi_rows - mi_row, bh);
  const int block_index = mi_row * cm->mi_cols + mi_col;
  int x, y;
  for (y = 0; y < ymis; y++)
    for (x = 0; x < xmis; x++) {
      int map_offset = block_index + y * cm->mi_cols + x;




      if ((!is_inter_block(mi) || !mi->skip) &&
          mi->segment_id <= CR_SEGMENT_ID_BOOST2) {
        cr->last_coded_q_map[map_offset] =
            clamp(cm->base_qindex + cr->qindex_delta[mi->segment_id], 0, MAXQ);
      } else if (is_inter_block(mi) && mi->skip &&
                 mi->segment_id <= CR_SEGMENT_ID_BOOST2) {
        cr->last_coded_q_map[map_offset] = VPXMIN(
            clamp(cm->base_qindex + cr->qindex_delta[mi->segment_id], 0, MAXQ),
            cr->last_coded_q_map[map_offset]);
      }
    }
}

// applied the segment delta q, and the amount of low motion in the frame.
// Also check conditions for forcing golden update, or preventing golden
// update if the period is up.
void vp9_cyclic_refresh_postencode(VP9_COMP *const cpi) {
  VP9_COMMON *const cm = &cpi->common;
  MODE_INFO **mi = cm->mi_grid_visible;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  RATE_CONTROL *const rc = &cpi->rc;
  unsigned char *const seg_map = cpi->segmentation_map;
  double fraction_low = 0.0;
  int force_gf_refresh = 0;
  int low_content_frame = 0;
  int mi_row, mi_col;
  cr->actual_num_seg1_blocks = 0;
  cr->actual_num_seg2_blocks = 0;
  for (mi_row = 0; mi_row < cm->mi_rows; mi_row++) {
    for (mi_col = 0; mi_col < cm->mi_cols; mi_col++) {
      MV mv = mi[0]->mv[0].as_mv;
      int map_index = mi_row * cm->mi_cols + mi_col;
      if (cyclic_refresh_segment_id(seg_map[map_index]) == CR_SEGMENT_ID_BOOST1)
        cr->actual_num_seg1_blocks++;
      else if (cyclic_refresh_segment_id(seg_map[map_index]) ==
               CR_SEGMENT_ID_BOOST2)
        cr->actual_num_seg2_blocks++;

      if (is_inter_block(mi[0]) && abs(mv.row) < 16 && abs(mv.col) < 16)
        low_content_frame++;
      mi++;
    }
    mi += 8;
  }

  if (!cpi->use_svc && cpi->ext_refresh_frame_flags_pending == 0 &&
      !cpi->oxcf.gf_cbr_boost_pct) {


    if (cpi->resize_pending != 0) {
      vp9_cyclic_refresh_set_golden_update(cpi);
      rc->frames_till_gf_update_due = rc->baseline_gf_interval;
      if (rc->frames_till_gf_update_due > rc->frames_to_key)
        rc->frames_till_gf_update_due = rc->frames_to_key;
      cpi->refresh_golden_frame = 1;
      force_gf_refresh = 1;
    }

    fraction_low = (double)low_content_frame / (cm->mi_rows * cm->mi_cols);
    cr->low_content_avg = (fraction_low + 3 * cr->low_content_avg) / 4;
    if (!force_gf_refresh && cpi->refresh_golden_frame == 1 &&
        rc->frames_since_key > rc->frames_since_golden + 1) {



      if (fraction_low < 0.65 || cr->low_content_avg < 0.6) {
        cpi->refresh_golden_frame = 0;
      }

      cr->low_content_avg = fraction_low;
    }
  }
}

void vp9_cyclic_refresh_set_golden_update(VP9_COMP *const cpi) {
  RATE_CONTROL *const rc = &cpi->rc;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;



  if (cr->percent_refresh > 0)
    rc->baseline_gf_interval = VPXMIN(4 * (100 / cr->percent_refresh), 40);
  else
    rc->baseline_gf_interval = 40;
  if (cpi->oxcf.rc_mode == VPX_VBR) rc->baseline_gf_interval = 20;
  if (rc->avg_frame_low_motion < 50 && rc->frames_since_key > 40)
    rc->baseline_gf_interval = 10;
}

static int is_superblock_flat_static(VP9_COMP *const cpi, int sb_row_index,
                                     int sb_col_index) {
  unsigned int source_variance;
  const uint8_t *src_y = cpi->Source->y_buffer;
  const int ystride = cpi->Source->y_stride;
  unsigned int sse;
  const BLOCK_SIZE bsize = BLOCK_64X64;
  src_y += (sb_row_index << 6) * ystride + (sb_col_index << 6);
  source_variance =
      cpi->fn_ptr[bsize].vf(src_y, ystride, VP9_VAR_OFFS, 0, &sse);
  if (source_variance == 0) {
    uint64_t block_sad;
    const uint8_t *last_src_y = cpi->Last_Source->y_buffer;
    const int last_ystride = cpi->Last_Source->y_stride;
    last_src_y += (sb_row_index << 6) * ystride + (sb_col_index << 6);
    block_sad =
        cpi->fn_ptr[bsize].sdf(src_y, ystride, last_src_y, last_ystride);
    if (block_sad == 0) return 1;
  }
  return 0;
}

// refresh sb_index, and target number of blocks to be refreshed.
// The map is set to either 0/CR_SEGMENT_ID_BASE (no refresh) or to
// 1/CR_SEGMENT_ID_BOOST1 (refresh) for each superblock.
// Blocks labeled as BOOST1 may later get set to BOOST2 (during the
// encoding of the superblock).
static void cyclic_refresh_update_map(VP9_COMP *const cpi) {
  VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  unsigned char *const seg_map = cpi->segmentation_map;
  int i, block_count, bl_index, sb_rows, sb_cols, sbs_in_frame;
  int xmis, ymis, x, y;
  int consec_zero_mv_thresh = 0;
  int qindex_thresh = 0;
  int count_sel = 0;
  int count_tot = 0;
  memset(seg_map, CR_SEGMENT_ID_BASE, cm->mi_rows * cm->mi_cols);
  sb_cols = (cm->mi_cols + MI_BLOCK_SIZE - 1) / MI_BLOCK_SIZE;
  sb_rows = (cm->mi_rows + MI_BLOCK_SIZE - 1) / MI_BLOCK_SIZE;
  sbs_in_frame = sb_cols * sb_rows;

  block_count = cr->percent_refresh * cm->mi_rows * cm->mi_cols / 100;



  assert(cr->sb_index < sbs_in_frame);
  i = cr->sb_index;
  cr->target_num_seg_blocks = 0;
  if (cpi->oxcf.content != VP9E_CONTENT_SCREEN) {
    consec_zero_mv_thresh = 100;
  }
  qindex_thresh =
      cpi->oxcf.content == VP9E_CONTENT_SCREEN
          ? vp9_get_qindex(&cm->seg, CR_SEGMENT_ID_BOOST2, cm->base_qindex)
          : vp9_get_qindex(&cm->seg, CR_SEGMENT_ID_BOOST1, cm->base_qindex);

  if (cpi->noise_estimate.enabled && cpi->noise_estimate.level >= kMedium) {
    consec_zero_mv_thresh = 60;
    qindex_thresh =
        VPXMAX(vp9_get_qindex(&cm->seg, CR_SEGMENT_ID_BOOST1, cm->base_qindex),
               cm->base_qindex);
  }
  do {
    int sum_map = 0;
    int consec_zero_mv_thresh_block = consec_zero_mv_thresh;

    int sb_row_index = (i / sb_cols);
    int sb_col_index = i - sb_row_index * sb_cols;
    int mi_row = sb_row_index * MI_BLOCK_SIZE;
    int mi_col = sb_col_index * MI_BLOCK_SIZE;
    int flat_static_blocks = 0;
    int compute_content = 1;
    assert(mi_row >= 0 && mi_row < cm->mi_rows);
    assert(mi_col >= 0 && mi_col < cm->mi_cols);
#if CONFIG_VP9_HIGHBITDEPTH
    if (cpi->common.use_highbitdepth) compute_content = 0;
#endif
    if (cpi->Last_Source == NULL ||
        cpi->Last_Source->y_width != cpi->Source->y_width ||
        cpi->Last_Source->y_height != cpi->Source->y_height)
      compute_content = 0;
    bl_index = mi_row * cm->mi_cols + mi_col;

    xmis =
        VPXMIN(cm->mi_cols - mi_col, num_8x8_blocks_wide_lookup[BLOCK_64X64]);
    ymis =
        VPXMIN(cm->mi_rows - mi_row, num_8x8_blocks_high_lookup[BLOCK_64X64]);
    if (cpi->noise_estimate.enabled && cpi->noise_estimate.level >= kMedium &&
        (xmis <= 2 || ymis <= 2))
      consec_zero_mv_thresh_block = 4;
    for (y = 0; y < ymis; y++) {
      for (x = 0; x < xmis; x++) {
        const int bl_index2 = bl_index + y * cm->mi_cols + x;



        if (cr->map[bl_index2] == 0) {
          count_tot++;
          if (cr->last_coded_q_map[bl_index2] > qindex_thresh ||
              cpi->consec_zero_mv[bl_index2] < consec_zero_mv_thresh_block) {
            sum_map++;
            count_sel++;
          }
        } else if (cr->map[bl_index2] < 0) {
          cr->map[bl_index2]++;
        }
      }
    }


    if (sum_map >= xmis * ymis / 2) {




      if (compute_content && cr->skip_flat_static_blocks)
        flat_static_blocks =
            is_superblock_flat_static(cpi, sb_row_index, sb_col_index);
      if (!flat_static_blocks) {

        for (y = 0; y < ymis; y++)
          for (x = 0; x < xmis; x++) {
            seg_map[bl_index + y * cm->mi_cols + x] = CR_SEGMENT_ID_BOOST1;
          }
        cr->target_num_seg_blocks += xmis * ymis;
      }
    }
    i++;
    if (i == sbs_in_frame) {
      i = 0;
    }
  } while (cr->target_num_seg_blocks < block_count && i != cr->sb_index);
  cr->sb_index = i;
  cr->reduce_refresh = 0;
  if (cpi->oxcf.content != VP9E_CONTENT_SCREEN)
    if (count_sel<(3 * count_tot)>> 2) cr->reduce_refresh = 1;
}

void vp9_cyclic_refresh_update_parameters(VP9_COMP *const cpi) {
  const RATE_CONTROL *const rc = &cpi->rc;
  const VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  int num8x8bl = cm->MBs << 2;
  int target_refresh = 0;
  double weight_segment_target = 0;
  double weight_segment = 0;
  int thresh_low_motion = 20;
  int qp_thresh = VPXMIN((cpi->oxcf.content == VP9E_CONTENT_SCREEN) ? 35 : 20,
                         rc->best_quality << 1);
  int qp_max_thresh = 117 * MAXQ >> 7;
  cr->apply_cyclic_refresh = 1;
  if (frame_is_intra_only(cm) || cpi->svc.temporal_layer_id > 0 ||
      is_lossless_requested(&cpi->oxcf) ||
      rc->avg_frame_qindex[INTER_FRAME] < qp_thresh ||
      (cpi->use_svc &&
       cpi->svc.layer_context[cpi->svc.temporal_layer_id].is_key_frame) ||
      (!cpi->use_svc && rc->avg_frame_low_motion < thresh_low_motion &&
       rc->frames_since_key > 40) ||
      (!cpi->use_svc && rc->avg_frame_qindex[INTER_FRAME] > qp_max_thresh &&
       rc->frames_since_key > 20)) {
    cr->apply_cyclic_refresh = 0;
    return;
  }
  cr->percent_refresh = 10;
  if (cr->reduce_refresh) cr->percent_refresh = 5;
  cr->max_qdelta_perc = 60;
  cr->time_for_refresh = 0;
  cr->motion_thresh = 32;
  cr->rate_boost_fac = 15;



  if (cr->percent_refresh > 0 &&
      rc->frames_since_key <
          (4 * cpi->svc.number_temporal_layers) * (100 / cr->percent_refresh)) {
    cr->rate_ratio_qdelta = 3.0;
  } else {
    cr->rate_ratio_qdelta = 2.0;
    if (cpi->noise_estimate.enabled && cpi->noise_estimate.level >= kMedium) {

      cr->rate_ratio_qdelta = 1.7;
      cr->rate_boost_fac = 13;
    }
  }



  if (cpi->oxcf.content == VP9E_CONTENT_SCREEN) {


    if (cpi->svc.spatial_layer_id == cpi->svc.number_spatial_layers - 1)
      cr->skip_flat_static_blocks = 1;
    cr->percent_refresh = (cr->skip_flat_static_blocks) ? 5 : 10;


    if (cr->counter_encode_maxq_scene_change < 30)
      cr->percent_refresh = (cr->skip_flat_static_blocks) ? 10 : 15;
    cr->rate_ratio_qdelta = 2.0;
    cr->rate_boost_fac = 10;
  }

  if (cm->width * cm->height <= 352 * 288) {
    if (rc->avg_frame_bandwidth < 3000) {
      cr->motion_thresh = 64;
      cr->rate_boost_fac = 13;
    } else {
      cr->max_qdelta_perc = 70;
      cr->rate_ratio_qdelta = VPXMAX(cr->rate_ratio_qdelta, 2.5);
    }
  }
  if (cpi->oxcf.rc_mode == VPX_VBR) {



    cr->percent_refresh = 10;
    cr->rate_ratio_qdelta = 1.5;
    cr->rate_boost_fac = 10;
    if (cpi->refresh_golden_frame == 1) {
      cr->percent_refresh = 0;
      cr->rate_ratio_qdelta = 1.0;
    }
  }




  target_refresh = cr->percent_refresh * cm->mi_rows * cm->mi_cols / 100;
  weight_segment_target = (double)(target_refresh) / num8x8bl;
  weight_segment = (double)((target_refresh + cr->actual_num_seg1_blocks +
                             cr->actual_num_seg2_blocks) >>
                            1) /
                   num8x8bl;
  if (weight_segment_target < 7 * weight_segment / 8)
    weight_segment = weight_segment_target;



  if (cpi->oxcf.content == VP9E_CONTENT_SCREEN)
    weight_segment =
        (double)(cr->actual_num_seg1_blocks + cr->actual_num_seg2_blocks) /
        num8x8bl;
  cr->weight_segment = weight_segment;
}

void vp9_cyclic_refresh_setup(VP9_COMP *const cpi) {
  VP9_COMMON *const cm = &cpi->common;
  const RATE_CONTROL *const rc = &cpi->rc;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  struct segmentation *const seg = &cm->seg;
  int scene_change_detected =
      cpi->rc.high_source_sad ||
      (cpi->use_svc && cpi->svc.high_source_sad_superframe);
  if (cm->current_video_frame == 0) cr->low_content_avg = 0.0;

  if (cpi->resize_pending != 0) vp9_cyclic_refresh_reset_resize(cpi);
  if (!cr->apply_cyclic_refresh || (cpi->force_update_segmentation) ||
      scene_change_detected) {

    unsigned char *const seg_map = cpi->segmentation_map;
    memset(seg_map, 0, cm->mi_rows * cm->mi_cols);
    vp9_disable_segmentation(&cm->seg);
    if (cm->frame_type == KEY_FRAME || scene_change_detected) {
      memset(cr->last_coded_q_map, MAXQ,
             cm->mi_rows * cm->mi_cols * sizeof(*cr->last_coded_q_map));
      cr->sb_index = 0;
      cr->reduce_refresh = 0;
      cr->counter_encode_maxq_scene_change = 0;
    }
    return;
  } else {
    int qindex_delta = 0;
    int qindex2;
    const double q = vp9_convert_qindex_to_q(cm->base_qindex, cm->bit_depth);
    cr->counter_encode_maxq_scene_change++;
    vpx_clear_system_state();


    cr->thresh_rate_sb = ((int64_t)(rc->sb64_target_rate) << 8) << 2;



    cr->thresh_dist_sb = ((int64_t)(q * q)) << 2;


    vp9_enable_segmentation(&cm->seg);
    vp9_clearall_segfeatures(seg);

    seg->abs_delta = SEGMENT_DELTADATA;







    vp9_disable_segfeature(seg, CR_SEGMENT_ID_BASE, SEG_LVL_ALT_Q);

    vp9_enable_segfeature(seg, CR_SEGMENT_ID_BOOST1, SEG_LVL_ALT_Q);

    vp9_enable_segfeature(seg, CR_SEGMENT_ID_BOOST2, SEG_LVL_ALT_Q);

    qindex_delta = compute_deltaq(cpi, cm->base_qindex, cr->rate_ratio_qdelta);
    cr->qindex_delta[1] = qindex_delta;

    qindex2 = clamp(cm->base_qindex + cm->y_dc_delta_q + qindex_delta, 0, MAXQ);

    cr->rdmult = vp9_compute_rd_mult(cpi, qindex2);

    vp9_set_segdata(seg, CR_SEGMENT_ID_BOOST1, SEG_LVL_ALT_Q, qindex_delta);

    qindex_delta = compute_deltaq(
        cpi, cm->base_qindex,
        VPXMIN(CR_MAX_RATE_TARGET_RATIO,
               0.1 * cr->rate_boost_fac * cr->rate_ratio_qdelta));
    cr->qindex_delta[2] = qindex_delta;
    vp9_set_segdata(seg, CR_SEGMENT_ID_BOOST2, SEG_LVL_ALT_Q, qindex_delta);

    cyclic_refresh_update_map(cpi);
  }
}

int vp9_cyclic_refresh_get_rdmult(const CYCLIC_REFRESH *cr) {
  return cr->rdmult;
}

void vp9_cyclic_refresh_reset_resize(VP9_COMP *const cpi) {
  const VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;
  memset(cr->map, 0, cm->mi_rows * cm->mi_cols);
  memset(cr->last_coded_q_map, MAXQ,
         cm->mi_rows * cm->mi_cols * sizeof(*cr->last_coded_q_map));
  cr->sb_index = 0;
  cpi->refresh_golden_frame = 1;
  cpi->refresh_alt_ref_frame = 1;
  cr->counter_encode_maxq_scene_change = 0;
}

void vp9_cyclic_refresh_limit_q(const VP9_COMP *cpi, int *q) {
  CYCLIC_REFRESH *const cr = cpi->cyclic_refresh;


  if (cr->percent_refresh > 0 && cpi->rc.q_1_frame - *q > 8) {
    *q = cpi->rc.q_1_frame - 8;
  }
}
