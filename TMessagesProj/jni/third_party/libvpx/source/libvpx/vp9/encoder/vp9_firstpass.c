/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "./vpx_dsp_rtcd.h"
#include "./vpx_scale_rtcd.h"

#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/system_state.h"
#include "vpx_scale/vpx_scale.h"
#include "vpx_scale/yv12config.h"

#include "vp9/common/vp9_entropymv.h"
#include "vp9/common/vp9_quant_common.h"
#include "vp9/common/vp9_reconinter.h"  // vp9_setup_dst_planes()
#include "vp9/encoder/vp9_aq_variance.h"
#include "vp9/encoder/vp9_block.h"
#include "vp9/encoder/vp9_encodeframe.h"
#include "vp9/encoder/vp9_encodemb.h"
#include "vp9/encoder/vp9_encodemv.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_ethread.h"
#include "vp9/encoder/vp9_extend.h"
#include "vp9/encoder/vp9_firstpass.h"
#include "vp9/encoder/vp9_mcomp.h"
#include "vp9/encoder/vp9_quantize.h"
#include "vp9/encoder/vp9_rd.h"
#include "vpx_dsp/variance.h"

#define OUTPUT_FPF 0
#define ARF_STATS_OUTPUT 0
#define COMPLEXITY_STATS_OUTPUT 0

#define FIRST_PASS_Q 10.0
#define NORMAL_BOOST 100
#define MIN_ARF_GF_BOOST 250
#define MIN_DECAY_FACTOR 0.01
#define NEW_MV_MODE_PENALTY 32
#define DARK_THRESH 64
#define LOW_I_THRESH 24000

#define NCOUNT_INTRA_THRESH 8192
#define NCOUNT_INTRA_FACTOR 3

#define DOUBLE_DIVIDE_CHECK(x) ((x) < 0 ? (x)-0.000001 : (x) + 0.000001)

#if ARF_STATS_OUTPUT
unsigned int arf_count = 0;
#endif

// the current position.
static void reset_fpf_position(TWO_PASS *p, const FIRSTPASS_STATS *position) {
  p->stats_in = position;
}

static const FIRSTPASS_STATS *read_frame_stats(const TWO_PASS *p, int offset) {
  if ((offset >= 0 && p->stats_in + offset >= p->stats_in_end) ||
      (offset < 0 && p->stats_in + offset < p->stats_in_start)) {
    return NULL;
  }

  return &p->stats_in[offset];
}

static int input_stats(TWO_PASS *p, FIRSTPASS_STATS *fps) {
  if (p->stats_in >= p->stats_in_end) return EOF;

  *fps = *p->stats_in;
  ++p->stats_in;
  return 1;
}

static void output_stats(FIRSTPASS_STATS *stats) {
  (void)stats;
// TEMP debug code
#if OUTPUT_FPF
  {
    FILE *fpfile;
    fpfile = fopen("firstpass.stt", "a");

    fprintf(fpfile,
            "%12.0lf %12.4lf %12.2lf %12.2lf %12.2lf %12.0lf %12.4lf %12.4lf"
            "%12.4lf %12.4lf %12.4lf %12.4lf %12.4lf %12.4lf %12.4lf %12.4lf"
            "%12.4lf %12.4lf %12.4lf %12.4lf %12.4lf %12.0lf %12.4lf %12.0lf"
            "%12.4lf"
            "\n",
            stats->frame, stats->weight, stats->intra_error, stats->coded_error,
            stats->sr_coded_error, stats->frame_noise_energy, stats->pcnt_inter,
            stats->pcnt_motion, stats->pcnt_second_ref, stats->pcnt_neutral,
            stats->pcnt_intra_low, stats->pcnt_intra_high,
            stats->intra_skip_pct, stats->intra_smooth_pct,
            stats->inactive_zone_rows, stats->inactive_zone_cols, stats->MVr,
            stats->mvr_abs, stats->MVc, stats->mvc_abs, stats->MVrv,
            stats->MVcv, stats->mv_in_out_count, stats->count, stats->duration);
    fclose(fpfile);
  }
#endif
}

#if CONFIG_FP_MB_STATS
static void output_fpmb_stats(uint8_t *this_frame_mb_stats, VP9_COMMON *cm,
                              struct vpx_codec_pkt_list *pktlist) {
  struct vpx_codec_cx_pkt pkt;
  pkt.kind = VPX_CODEC_FPMB_STATS_PKT;
  pkt.data.firstpass_mb_stats.buf = this_frame_mb_stats;
  pkt.data.firstpass_mb_stats.sz = cm->initial_mbs * sizeof(uint8_t);
  vpx_codec_pkt_list_add(pktlist, &pkt);
}
#endif

static void zero_stats(FIRSTPASS_STATS *section) {
  section->frame = 0.0;
  section->weight = 0.0;
  section->intra_error = 0.0;
  section->coded_error = 0.0;
  section->sr_coded_error = 0.0;
  section->frame_noise_energy = 0.0;
  section->pcnt_inter = 0.0;
  section->pcnt_motion = 0.0;
  section->pcnt_second_ref = 0.0;
  section->pcnt_neutral = 0.0;
  section->intra_skip_pct = 0.0;
  section->intra_smooth_pct = 0.0;
  section->pcnt_intra_low = 0.0;
  section->pcnt_intra_high = 0.0;
  section->inactive_zone_rows = 0.0;
  section->inactive_zone_cols = 0.0;
  section->MVr = 0.0;
  section->mvr_abs = 0.0;
  section->MVc = 0.0;
  section->mvc_abs = 0.0;
  section->MVrv = 0.0;
  section->MVcv = 0.0;
  section->mv_in_out_count = 0.0;
  section->count = 0.0;
  section->duration = 1.0;
  section->spatial_layer_id = 0;
}

static void accumulate_stats(FIRSTPASS_STATS *section,
                             const FIRSTPASS_STATS *frame) {
  section->frame += frame->frame;
  section->weight += frame->weight;
  section->spatial_layer_id = frame->spatial_layer_id;
  section->intra_error += frame->intra_error;
  section->coded_error += frame->coded_error;
  section->sr_coded_error += frame->sr_coded_error;
  section->frame_noise_energy += frame->frame_noise_energy;
  section->pcnt_inter += frame->pcnt_inter;
  section->pcnt_motion += frame->pcnt_motion;
  section->pcnt_second_ref += frame->pcnt_second_ref;
  section->pcnt_neutral += frame->pcnt_neutral;
  section->intra_skip_pct += frame->intra_skip_pct;
  section->intra_smooth_pct += frame->intra_smooth_pct;
  section->pcnt_intra_low += frame->pcnt_intra_low;
  section->pcnt_intra_high += frame->pcnt_intra_high;
  section->inactive_zone_rows += frame->inactive_zone_rows;
  section->inactive_zone_cols += frame->inactive_zone_cols;
  section->MVr += frame->MVr;
  section->mvr_abs += frame->mvr_abs;
  section->MVc += frame->MVc;
  section->mvc_abs += frame->mvc_abs;
  section->MVrv += frame->MVrv;
  section->MVcv += frame->MVcv;
  section->mv_in_out_count += frame->mv_in_out_count;
  section->count += frame->count;
  section->duration += frame->duration;
}

static void subtract_stats(FIRSTPASS_STATS *section,
                           const FIRSTPASS_STATS *frame) {
  section->frame -= frame->frame;
  section->weight -= frame->weight;
  section->intra_error -= frame->intra_error;
  section->coded_error -= frame->coded_error;
  section->sr_coded_error -= frame->sr_coded_error;
  section->frame_noise_energy -= frame->frame_noise_energy;
  section->pcnt_inter -= frame->pcnt_inter;
  section->pcnt_motion -= frame->pcnt_motion;
  section->pcnt_second_ref -= frame->pcnt_second_ref;
  section->pcnt_neutral -= frame->pcnt_neutral;
  section->intra_skip_pct -= frame->intra_skip_pct;
  section->intra_smooth_pct -= frame->intra_smooth_pct;
  section->pcnt_intra_low -= frame->pcnt_intra_low;
  section->pcnt_intra_high -= frame->pcnt_intra_high;
  section->inactive_zone_rows -= frame->inactive_zone_rows;
  section->inactive_zone_cols -= frame->inactive_zone_cols;
  section->MVr -= frame->MVr;
  section->mvr_abs -= frame->mvr_abs;
  section->MVc -= frame->MVc;
  section->mvc_abs -= frame->mvc_abs;
  section->MVrv -= frame->MVrv;
  section->MVcv -= frame->MVcv;
  section->mv_in_out_count -= frame->mv_in_out_count;
  section->count -= frame->count;
  section->duration -= frame->duration;
}

// bars and partially discounts other 0 energy areas.
#define MIN_ACTIVE_AREA 0.5
#define MAX_ACTIVE_AREA 1.0
static double calculate_active_area(const FRAME_INFO *frame_info,
                                    const FIRSTPASS_STATS *this_frame) {
  double active_pct;

  active_pct =
      1.0 -
      ((this_frame->intra_skip_pct / 2) +
       ((this_frame->inactive_zone_rows * 2) / (double)frame_info->mb_rows));
  return fclamp(active_pct, MIN_ACTIVE_AREA, MAX_ACTIVE_AREA);
}

static double get_distribution_av_err(VP9_COMP *cpi, TWO_PASS *const twopass) {
  const double av_weight =
      twopass->total_stats.weight / twopass->total_stats.count;

  if (cpi->oxcf.vbr_corpus_complexity)
    return av_weight * twopass->mean_mod_score;
  else
    return (twopass->total_stats.coded_error * av_weight) /
           twopass->total_stats.count;
}

#define ACT_AREA_CORRECTION 0.5
// Calculate a modified Error used in distributing bits between easier and
// harder frames.
static double calculate_mod_frame_score(const VP9_COMP *cpi,
                                        const VP9EncoderConfig *oxcf,
                                        const FIRSTPASS_STATS *this_frame,
                                        const double av_err) {
  double modified_score =
      av_err * pow(this_frame->coded_error * this_frame->weight /
                       DOUBLE_DIVIDE_CHECK(av_err),
                   oxcf->two_pass_vbrbias / 100.0);





  modified_score *= pow(calculate_active_area(&cpi->frame_info, this_frame),
                        ACT_AREA_CORRECTION);

  return modified_score;
}

static double calc_norm_frame_score(const VP9EncoderConfig *oxcf,
                                    const FRAME_INFO *frame_info,
                                    const FIRSTPASS_STATS *this_frame,
                                    double mean_mod_score, double av_err) {
  double modified_score =
      av_err * pow(this_frame->coded_error * this_frame->weight /
                       DOUBLE_DIVIDE_CHECK(av_err),
                   oxcf->two_pass_vbrbias / 100.0);

  const double min_score = (double)(oxcf->two_pass_vbrmin_section) / 100.0;
  const double max_score = (double)(oxcf->two_pass_vbrmax_section) / 100.0;





  modified_score *=
      pow(calculate_active_area(frame_info, this_frame), ACT_AREA_CORRECTION);

  modified_score /= DOUBLE_DIVIDE_CHECK(mean_mod_score);
  return fclamp(modified_score, min_score, max_score);
}

static double calculate_norm_frame_score(const VP9_COMP *cpi,
                                         const TWO_PASS *twopass,
                                         const VP9EncoderConfig *oxcf,
                                         const FIRSTPASS_STATS *this_frame,
                                         const double av_err) {
  return calc_norm_frame_score(oxcf, &cpi->frame_info, this_frame,
                               twopass->mean_mod_score, av_err);
}

static int frame_max_bits(const RATE_CONTROL *rc,
                          const VP9EncoderConfig *oxcf) {
  int64_t max_bits = ((int64_t)rc->avg_frame_bandwidth *
                      (int64_t)oxcf->two_pass_vbrmax_section) /
                     100;
  if (max_bits < 0)
    max_bits = 0;
  else if (max_bits > rc->max_frame_bandwidth)
    max_bits = rc->max_frame_bandwidth;

  return (int)max_bits;
}

void vp9_init_first_pass(VP9_COMP *cpi) {
  zero_stats(&cpi->twopass.total_stats);
}

void vp9_end_first_pass(VP9_COMP *cpi) {
  output_stats(&cpi->twopass.total_stats);
  cpi->twopass.first_pass_done = 1;
  vpx_free(cpi->twopass.fp_mb_float_stats);
  cpi->twopass.fp_mb_float_stats = NULL;
}

static vpx_variance_fn_t get_block_variance_fn(BLOCK_SIZE bsize) {
  switch (bsize) {
    case BLOCK_8X8: return vpx_mse8x8;
    case BLOCK_16X8: return vpx_mse16x8;
    case BLOCK_8X16: return vpx_mse8x16;
    default: return vpx_mse16x16;
  }
}

static unsigned int get_prediction_error(BLOCK_SIZE bsize,
                                         const struct buf_2d *src,
                                         const struct buf_2d *ref) {
  unsigned int sse;
  const vpx_variance_fn_t fn = get_block_variance_fn(bsize);
  fn(src->buf, src->stride, ref->buf, ref->stride, &sse);
  return sse;
}

#if CONFIG_VP9_HIGHBITDEPTH
static vpx_variance_fn_t highbd_get_block_variance_fn(BLOCK_SIZE bsize,
                                                      int bd) {
  switch (bd) {
    default:
      switch (bsize) {
        case BLOCK_8X8: return vpx_highbd_8_mse8x8;
        case BLOCK_16X8: return vpx_highbd_8_mse16x8;
        case BLOCK_8X16: return vpx_highbd_8_mse8x16;
        default: return vpx_highbd_8_mse16x16;
      }
      break;
    case 10:
      switch (bsize) {
        case BLOCK_8X8: return vpx_highbd_10_mse8x8;
        case BLOCK_16X8: return vpx_highbd_10_mse16x8;
        case BLOCK_8X16: return vpx_highbd_10_mse8x16;
        default: return vpx_highbd_10_mse16x16;
      }
      break;
    case 12:
      switch (bsize) {
        case BLOCK_8X8: return vpx_highbd_12_mse8x8;
        case BLOCK_16X8: return vpx_highbd_12_mse16x8;
        case BLOCK_8X16: return vpx_highbd_12_mse8x16;
        default: return vpx_highbd_12_mse16x16;
      }
      break;
  }
}

static unsigned int highbd_get_prediction_error(BLOCK_SIZE bsize,
                                                const struct buf_2d *src,
                                                const struct buf_2d *ref,
                                                int bd) {
  unsigned int sse;
  const vpx_variance_fn_t fn = highbd_get_block_variance_fn(bsize, bd);
  fn(src->buf, src->stride, ref->buf, ref->stride, &sse);
  return sse;
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

// for first pass test.
static int get_search_range(const VP9_COMP *cpi) {
  int sr = 0;
  const int dim = VPXMIN(cpi->initial_width, cpi->initial_height);

  while ((dim << sr) < MAX_FULL_PEL_VAL) ++sr;
  return sr;
}

// this can be problematic for big videos (8K) and may cause assert failure
// (or memory violation) in mv_cost. Limits are only modified if they would
// be non-empty. Returns 1 if limits are non-empty.
static int intersect_limits_with_mv_max(MvLimits *mv_limits, const MV *ref_mv) {
  const int row_min =
      VPXMAX(mv_limits->row_min, (ref_mv->row + 7 - MV_MAX) >> 3);
  const int row_max =
      VPXMIN(mv_limits->row_max, (ref_mv->row - 1 + MV_MAX) >> 3);
  const int col_min =
      VPXMAX(mv_limits->col_min, (ref_mv->col + 7 - MV_MAX) >> 3);
  const int col_max =
      VPXMIN(mv_limits->col_max, (ref_mv->col - 1 + MV_MAX) >> 3);
  if (row_min > row_max || col_min > col_max) {
    return 0;
  }
  mv_limits->row_min = row_min;
  mv_limits->row_max = row_max;
  mv_limits->col_min = col_min;
  mv_limits->col_max = col_max;
  return 1;
}

static void first_pass_motion_search(VP9_COMP *cpi, MACROBLOCK *x,
                                     const MV *ref_mv, MV *best_mv,
                                     int *best_motion_err) {
  MACROBLOCKD *const xd = &x->e_mbd;
  MV tmp_mv = { 0, 0 };
  MV ref_mv_full = { ref_mv->row >> 3, ref_mv->col >> 3 };
  int num00, tmp_err, n;
  const BLOCK_SIZE bsize = xd->mi[0]->sb_type;
  vp9_variance_fn_ptr_t v_fn_ptr = cpi->fn_ptr[bsize];
  const int new_mv_mode_penalty = NEW_MV_MODE_PENALTY;

  int step_param = 3;
  int further_steps = (MAX_MVSEARCH_STEPS - 1) - step_param;
  const int sr = get_search_range(cpi);
  const MvLimits tmp_mv_limits = x->mv_limits;
  step_param += sr;
  further_steps -= sr;

  if (!intersect_limits_with_mv_max(&x->mv_limits, ref_mv)) {
    return;
  }

  v_fn_ptr.vf = get_block_variance_fn(bsize);
#if CONFIG_VP9_HIGHBITDEPTH
  if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
    v_fn_ptr.vf = highbd_get_block_variance_fn(bsize, xd->bd);
  }
#endif  // CONFIG_VP9_HIGHBITDEPTH

  tmp_err = cpi->diamond_search_sad(x, &cpi->ss_cfg, &ref_mv_full, &tmp_mv,
                                    step_param, x->sadperbit16, &num00,
                                    &v_fn_ptr, ref_mv);
  if (tmp_err < INT_MAX)
    tmp_err = vp9_get_mvpred_var(x, &tmp_mv, ref_mv, &v_fn_ptr, 1);
  if (tmp_err < INT_MAX - new_mv_mode_penalty) tmp_err += new_mv_mode_penalty;

  if (tmp_err < *best_motion_err) {
    *best_motion_err = tmp_err;
    *best_mv = tmp_mv;
  }

  n = num00;
  num00 = 0;

  while (n < further_steps) {
    ++n;

    if (num00) {
      --num00;
    } else {
      tmp_err = cpi->diamond_search_sad(x, &cpi->ss_cfg, &ref_mv_full, &tmp_mv,
                                        step_param + n, x->sadperbit16, &num00,
                                        &v_fn_ptr, ref_mv);
      if (tmp_err < INT_MAX)
        tmp_err = vp9_get_mvpred_var(x, &tmp_mv, ref_mv, &v_fn_ptr, 1);
      if (tmp_err < INT_MAX - new_mv_mode_penalty)
        tmp_err += new_mv_mode_penalty;

      if (tmp_err < *best_motion_err) {
        *best_motion_err = tmp_err;
        *best_mv = tmp_mv;
      }
    }
  }
  x->mv_limits = tmp_mv_limits;
}

static BLOCK_SIZE get_bsize(const VP9_COMMON *cm, int mb_row, int mb_col) {
  if (2 * mb_col + 1 < cm->mi_cols) {
    return 2 * mb_row + 1 < cm->mi_rows ? BLOCK_16X16 : BLOCK_16X8;
  } else {
    return 2 * mb_row + 1 < cm->mi_rows ? BLOCK_8X16 : BLOCK_8X8;
  }
}

static int find_fp_qindex(vpx_bit_depth_t bit_depth) {
  int i;

  for (i = 0; i < QINDEX_RANGE; ++i)
    if (vp9_convert_qindex_to_q(i, bit_depth) >= FIRST_PASS_Q) break;

  if (i == QINDEX_RANGE) i--;

  return i;
}

static void set_first_pass_params(VP9_COMP *cpi) {
  VP9_COMMON *const cm = &cpi->common;
  if (!cpi->refresh_alt_ref_frame &&
      (cm->current_video_frame == 0 || (cpi->frame_flags & FRAMEFLAGS_KEY))) {
    cm->frame_type = KEY_FRAME;
  } else {
    cm->frame_type = INTER_FRAME;
  }

  cpi->rc.frames_to_key = INT_MAX;
}

static int scale_sse_threshold(VP9_COMMON *cm, int thresh) {
  int ret_val = thresh;
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    switch (cm->bit_depth) {
      case VPX_BITS_8: ret_val = thresh; break;
      case VPX_BITS_10: ret_val = thresh << 4; break;
      default:
        assert(cm->bit_depth == VPX_BITS_12);
        ret_val = thresh << 8;
        break;
    }
  }
#else
  (void)cm;
#endif  // CONFIG_VP9_HIGHBITDEPTH
  return ret_val;
}

// the intra prediction error 0. Though the metric we test against
// is technically a sse we are mainly interested in blocks where all the pixels
// in the 8 bit domain have an error of <= 1 (where error = sse) so a
// linear scaling for 10 and 12 bit gives similar results.
#define UL_INTRA_THRESH 50
static int get_ul_intra_threshold(VP9_COMMON *cm) {
  int ret_val = UL_INTRA_THRESH;
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    switch (cm->bit_depth) {
      case VPX_BITS_8: ret_val = UL_INTRA_THRESH; break;
      case VPX_BITS_10: ret_val = UL_INTRA_THRESH << 2; break;
      default:
        assert(cm->bit_depth == VPX_BITS_12);
        ret_val = UL_INTRA_THRESH << 4;
        break;
    }
  }
#else
  (void)cm;
#endif  // CONFIG_VP9_HIGHBITDEPTH
  return ret_val;
}

#define SMOOTH_INTRA_THRESH 4000
static int get_smooth_intra_threshold(VP9_COMMON *cm) {
  int ret_val = SMOOTH_INTRA_THRESH;
#if CONFIG_VP9_HIGHBITDEPTH
  if (cm->use_highbitdepth) {
    switch (cm->bit_depth) {
      case VPX_BITS_8: ret_val = SMOOTH_INTRA_THRESH; break;
      case VPX_BITS_10: ret_val = SMOOTH_INTRA_THRESH << 4; break;
      default:
        assert(cm->bit_depth == VPX_BITS_12);
        ret_val = SMOOTH_INTRA_THRESH << 8;
        break;
    }
  }
#else
  (void)cm;
#endif  // CONFIG_VP9_HIGHBITDEPTH
  return ret_val;
}

#define FP_DN_THRESH 8
#define FP_MAX_DN_THRESH 24
#define KERNEL_SIZE 3

static uint8_t fp_dn_kernal_3[KERNEL_SIZE * KERNEL_SIZE] = { 1, 2, 1, 2, 4,
                                                             2, 1, 2, 1 };

// on the point value
static int fp_estimate_point_noise(uint8_t *src_ptr, const int stride) {
  int sum_weight = 0;
  int sum_val = 0;
  int i, j;
  int max_diff = 0;
  int diff;
  int dn_diff;
  uint8_t *tmp_ptr;
  uint8_t *kernal_ptr;
  uint8_t dn_val;
  uint8_t centre_val = *src_ptr;

  kernal_ptr = fp_dn_kernal_3;

  tmp_ptr = src_ptr - stride - 1;
  for (i = 0; i < KERNEL_SIZE; ++i) {
    for (j = 0; j < KERNEL_SIZE; ++j) {
      diff = abs((int)centre_val - (int)tmp_ptr[j]);
      max_diff = VPXMAX(max_diff, diff);
      if (diff <= FP_DN_THRESH) {
        sum_weight += *kernal_ptr;
        sum_val += (int)tmp_ptr[j] * (int)*kernal_ptr;
      }
      ++kernal_ptr;
    }
    tmp_ptr += stride;
  }

  if (max_diff < FP_MAX_DN_THRESH)

    dn_val = (sum_val + (sum_weight >> 1)) / sum_weight;
  else
    dn_val = *src_ptr;


  dn_diff = (int)*src_ptr - (int)dn_val;
  return dn_diff * dn_diff;
}
#if CONFIG_VP9_HIGHBITDEPTH
static int fp_highbd_estimate_point_noise(uint8_t *src_ptr, const int stride) {
  int sum_weight = 0;
  int sum_val = 0;
  int i, j;
  int max_diff = 0;
  int diff;
  int dn_diff;
  uint8_t *tmp_ptr;
  uint16_t *tmp_ptr16;
  uint8_t *kernal_ptr;
  uint16_t dn_val;
  uint16_t centre_val = *CONVERT_TO_SHORTPTR(src_ptr);

  kernal_ptr = fp_dn_kernal_3;

  tmp_ptr = src_ptr - stride - 1;
  for (i = 0; i < KERNEL_SIZE; ++i) {
    tmp_ptr16 = CONVERT_TO_SHORTPTR(tmp_ptr);
    for (j = 0; j < KERNEL_SIZE; ++j) {
      diff = abs((int)centre_val - (int)tmp_ptr16[j]);
      max_diff = VPXMAX(max_diff, diff);
      if (diff <= FP_DN_THRESH) {
        sum_weight += *kernal_ptr;
        sum_val += (int)tmp_ptr16[j] * (int)*kernal_ptr;
      }
      ++kernal_ptr;
    }
    tmp_ptr += stride;
  }

  if (max_diff < FP_MAX_DN_THRESH)

    dn_val = (sum_val + (sum_weight >> 1)) / sum_weight;
  else
    dn_val = *CONVERT_TO_SHORTPTR(src_ptr);


  dn_diff = (int)(*CONVERT_TO_SHORTPTR(src_ptr)) - (int)dn_val;
  return dn_diff * dn_diff;
}
#endif

static int fp_estimate_block_noise(MACROBLOCK *x, BLOCK_SIZE bsize) {
#if CONFIG_VP9_HIGHBITDEPTH
  MACROBLOCKD *xd = &x->e_mbd;
#endif
  uint8_t *src_ptr = &x->plane[0].src.buf[0];
  const int width = num_4x4_blocks_wide_lookup[bsize] * 4;
  const int height = num_4x4_blocks_high_lookup[bsize] * 4;
  int w, h;
  int stride = x->plane[0].src.stride;
  int block_noise = 0;

  for (h = 0; h < height; h += 2) {
    for (w = 0; w < width; w += 2) {
#if CONFIG_VP9_HIGHBITDEPTH
      if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH)
        block_noise += fp_highbd_estimate_point_noise(src_ptr, stride);
      else
        block_noise += fp_estimate_point_noise(src_ptr, stride);
#else
      block_noise += fp_estimate_point_noise(src_ptr, stride);
#endif
      ++src_ptr;
    }
    src_ptr += (stride - width);
  }
  return block_noise << 2;  // Scale << 2 to account for sampling.
}

// multi-threading in unit tests for bit-exactness
static void accumulate_floating_point_stats(VP9_COMP *cpi,
                                            TileDataEnc *first_tile_col) {
  VP9_COMMON *const cm = &cpi->common;
  int mb_row, mb_col;
  first_tile_col->fp_data.intra_factor = 0;
  first_tile_col->fp_data.brightness_factor = 0;
  first_tile_col->fp_data.neutral_count = 0;
  for (mb_row = 0; mb_row < cm->mb_rows; ++mb_row) {
    for (mb_col = 0; mb_col < cm->mb_cols; ++mb_col) {
      const int mb_index = mb_row * cm->mb_cols + mb_col;
      first_tile_col->fp_data.intra_factor +=
          cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_intra_factor;
      first_tile_col->fp_data.brightness_factor +=
          cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_brightness_factor;
      first_tile_col->fp_data.neutral_count +=
          cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_neutral_count;
    }
  }
}

static void first_pass_stat_calc(VP9_COMP *cpi, FIRSTPASS_STATS *fps,
                                 FIRSTPASS_DATA *fp_acc_data) {
  VP9_COMMON *const cm = &cpi->common;





  const int num_mbs = (cpi->oxcf.resize_mode != RESIZE_NONE) ? cpi->initial_mbs
                                                             : cpi->common.MBs;
  const double min_err = 200 * sqrt(num_mbs);


  if ((fp_acc_data->image_data_start_row > cm->mb_rows / 2) ||
      (fp_acc_data->image_data_start_row == INVALID_ROW)) {
    fp_acc_data->image_data_start_row = cm->mb_rows / 2;
  }

  if (fp_acc_data->image_data_start_row > 0) {
    fp_acc_data->intra_skip_count =
        VPXMAX(0, fp_acc_data->intra_skip_count -
                      (fp_acc_data->image_data_start_row * cm->mb_cols * 2));
  }

  fp_acc_data->intra_factor = fp_acc_data->intra_factor / (double)num_mbs;
  fp_acc_data->brightness_factor =
      fp_acc_data->brightness_factor / (double)num_mbs;
  fps->weight = fp_acc_data->intra_factor * fp_acc_data->brightness_factor;

  fps->frame = cm->current_video_frame;
  fps->spatial_layer_id = cpi->svc.spatial_layer_id;

  fps->coded_error =
      ((double)(fp_acc_data->coded_error >> 8) + min_err) / num_mbs;
  fps->sr_coded_error =
      ((double)(fp_acc_data->sr_coded_error >> 8) + min_err) / num_mbs;
  fps->intra_error =
      ((double)(fp_acc_data->intra_error >> 8) + min_err) / num_mbs;

  fps->frame_noise_energy =
      (double)(fp_acc_data->frame_noise_energy) / (double)num_mbs;
  fps->count = 1.0;
  fps->pcnt_inter = (double)(fp_acc_data->intercount) / num_mbs;
  fps->pcnt_second_ref = (double)(fp_acc_data->second_ref_count) / num_mbs;
  fps->pcnt_neutral = (double)(fp_acc_data->neutral_count) / num_mbs;
  fps->pcnt_intra_low = (double)(fp_acc_data->intra_count_low) / num_mbs;
  fps->pcnt_intra_high = (double)(fp_acc_data->intra_count_high) / num_mbs;
  fps->intra_skip_pct = (double)(fp_acc_data->intra_skip_count) / num_mbs;
  fps->intra_smooth_pct = (double)(fp_acc_data->intra_smooth_count) / num_mbs;
  fps->inactive_zone_rows = (double)(fp_acc_data->image_data_start_row);

  fps->inactive_zone_cols = (double)0;

  if (fp_acc_data->mvcount > 0) {
    fps->MVr = (double)(fp_acc_data->sum_mvr) / fp_acc_data->mvcount;
    fps->mvr_abs = (double)(fp_acc_data->sum_mvr_abs) / fp_acc_data->mvcount;
    fps->MVc = (double)(fp_acc_data->sum_mvc) / fp_acc_data->mvcount;
    fps->mvc_abs = (double)(fp_acc_data->sum_mvc_abs) / fp_acc_data->mvcount;
    fps->MVrv = ((double)(fp_acc_data->sum_mvrs) -
                 ((double)(fp_acc_data->sum_mvr) * (fp_acc_data->sum_mvr) /
                  fp_acc_data->mvcount)) /
                fp_acc_data->mvcount;
    fps->MVcv = ((double)(fp_acc_data->sum_mvcs) -
                 ((double)(fp_acc_data->sum_mvc) * (fp_acc_data->sum_mvc) /
                  fp_acc_data->mvcount)) /
                fp_acc_data->mvcount;
    fps->mv_in_out_count =
        (double)(fp_acc_data->sum_in_vectors) / (fp_acc_data->mvcount * 2);
    fps->pcnt_motion = (double)(fp_acc_data->mvcount) / num_mbs;
  } else {
    fps->MVr = 0.0;
    fps->mvr_abs = 0.0;
    fps->MVc = 0.0;
    fps->mvc_abs = 0.0;
    fps->MVrv = 0.0;
    fps->MVcv = 0.0;
    fps->mv_in_out_count = 0.0;
    fps->pcnt_motion = 0.0;
  }
}

static void accumulate_fp_mb_row_stat(TileDataEnc *this_tile,
                                      FIRSTPASS_DATA *fp_acc_data) {
  this_tile->fp_data.intra_factor += fp_acc_data->intra_factor;
  this_tile->fp_data.brightness_factor += fp_acc_data->brightness_factor;
  this_tile->fp_data.coded_error += fp_acc_data->coded_error;
  this_tile->fp_data.sr_coded_error += fp_acc_data->sr_coded_error;
  this_tile->fp_data.frame_noise_energy += fp_acc_data->frame_noise_energy;
  this_tile->fp_data.intra_error += fp_acc_data->intra_error;
  this_tile->fp_data.intercount += fp_acc_data->intercount;
  this_tile->fp_data.second_ref_count += fp_acc_data->second_ref_count;
  this_tile->fp_data.neutral_count += fp_acc_data->neutral_count;
  this_tile->fp_data.intra_count_low += fp_acc_data->intra_count_low;
  this_tile->fp_data.intra_count_high += fp_acc_data->intra_count_high;
  this_tile->fp_data.intra_skip_count += fp_acc_data->intra_skip_count;
  this_tile->fp_data.mvcount += fp_acc_data->mvcount;
  this_tile->fp_data.sum_mvr += fp_acc_data->sum_mvr;
  this_tile->fp_data.sum_mvr_abs += fp_acc_data->sum_mvr_abs;
  this_tile->fp_data.sum_mvc += fp_acc_data->sum_mvc;
  this_tile->fp_data.sum_mvc_abs += fp_acc_data->sum_mvc_abs;
  this_tile->fp_data.sum_mvrs += fp_acc_data->sum_mvrs;
  this_tile->fp_data.sum_mvcs += fp_acc_data->sum_mvcs;
  this_tile->fp_data.sum_in_vectors += fp_acc_data->sum_in_vectors;
  this_tile->fp_data.intra_smooth_count += fp_acc_data->intra_smooth_count;
  this_tile->fp_data.image_data_start_row =
      VPXMIN(this_tile->fp_data.image_data_start_row,
             fp_acc_data->image_data_start_row) == INVALID_ROW
          ? VPXMAX(this_tile->fp_data.image_data_start_row,
                   fp_acc_data->image_data_start_row)
          : VPXMIN(this_tile->fp_data.image_data_start_row,
                   fp_acc_data->image_data_start_row);
}

#if CONFIG_RATE_CTRL
static void store_fp_motion_vector(VP9_COMP *cpi, const MV *mv,
                                   const int mb_row, const int mb_col,
                                   MV_REFERENCE_FRAME frame_type,
                                   const int mv_idx) {
  VP9_COMMON *const cm = &cpi->common;
  const int mb_index = mb_row * cm->mb_cols + mb_col;
  MOTION_VECTOR_INFO *this_motion_vector_info =
      &cpi->fp_motion_vector_info[mb_index];
  this_motion_vector_info->ref_frame[mv_idx] = frame_type;
  if (frame_type != INTRA_FRAME) {
    this_motion_vector_info->mv[mv_idx].as_mv = *mv;
  }
}
#endif  // CONFIG_RATE_CTRL

#define NZ_MOTION_PENALTY 128
#define INTRA_MODE_PENALTY 1024
void vp9_first_pass_encode_tile_mb_row(VP9_COMP *cpi, ThreadData *td,
                                       FIRSTPASS_DATA *fp_acc_data,
                                       TileDataEnc *tile_data, MV *best_ref_mv,
                                       int mb_row) {
  int mb_col;
  MACROBLOCK *const x = &td->mb;
  VP9_COMMON *const cm = &cpi->common;
  MACROBLOCKD *const xd = &x->e_mbd;
  TileInfo tile = tile_data->tile_info;
  const int mb_col_start = ROUND_POWER_OF_TWO(tile.mi_col_start, 1);
  const int mb_col_end = ROUND_POWER_OF_TWO(tile.mi_col_end, 1);
  struct macroblock_plane *const p = x->plane;
  struct macroblockd_plane *const pd = xd->plane;
  const PICK_MODE_CONTEXT *ctx = &td->pc_root->none;
  int i, c;
  int num_mb_cols = get_num_cols(tile_data->tile_info, 1);

  int recon_yoffset, recon_uvoffset;
  const int intrapenalty = INTRA_MODE_PENALTY;
  const MV zero_mv = { 0, 0 };
  int recon_y_stride, recon_uv_stride, uv_mb_height;

  YV12_BUFFER_CONFIG *const lst_yv12 = get_ref_frame_buffer(cpi, LAST_FRAME);
  YV12_BUFFER_CONFIG *gld_yv12 = get_ref_frame_buffer(cpi, GOLDEN_FRAME);
  YV12_BUFFER_CONFIG *const new_yv12 = get_frame_new_buffer(cm);
  const YV12_BUFFER_CONFIG *first_ref_buf = lst_yv12;

  MODE_INFO mi_above, mi_left;

  double mb_intra_factor;
  double mb_brightness_factor;
  double mb_neutral_count;
  int scaled_low_intra_thresh = scale_sse_threshold(cm, LOW_I_THRESH);

  assert(new_yv12 != NULL);
  assert(frame_is_intra_only(cm) || (lst_yv12 != NULL));

  xd->mi = cm->mi_grid_visible + xd->mi_stride * (mb_row << 1) + mb_col_start;
  xd->mi[0] = cm->mi + xd->mi_stride * (mb_row << 1) + mb_col_start;

  for (i = 0; i < MAX_MB_PLANE; ++i) {
    p[i].coeff = ctx->coeff_pbuf[i][1];
    p[i].qcoeff = ctx->qcoeff_pbuf[i][1];
    pd[i].dqcoeff = ctx->dqcoeff_pbuf[i][1];
    p[i].eobs = ctx->eobs_pbuf[i][1];
  }

  recon_y_stride = new_yv12->y_stride;
  recon_uv_stride = new_yv12->uv_stride;
  uv_mb_height = 16 >> (new_yv12->y_height > new_yv12->uv_height);

  recon_yoffset = (mb_row * recon_y_stride * 16) + mb_col_start * 16;
  recon_uvoffset =
      (mb_row * recon_uv_stride * uv_mb_height) + mb_col_start * uv_mb_height;


  x->mv_limits.row_min = -((mb_row * 16) + BORDER_MV_PIXELS_B16);
  x->mv_limits.row_max =
      ((cm->mb_rows - 1 - mb_row) * 16) + BORDER_MV_PIXELS_B16;

  for (mb_col = mb_col_start, c = 0; mb_col < mb_col_end; ++mb_col, c++) {
    int this_error;
    int this_intra_error;
    const int use_dc_pred = (mb_col || mb_row) && (!mb_col || !mb_row);
    const BLOCK_SIZE bsize = get_bsize(cm, mb_row, mb_col);
    double log_intra;
    int level_sample;
    const int mb_index = mb_row * cm->mb_cols + mb_col;

#if CONFIG_FP_MB_STATS
    const int mb_index = mb_row * cm->mb_cols + mb_col;
#endif

    (*(cpi->row_mt_sync_read_ptr))(&tile_data->row_mt_sync, mb_row, c);

    x->plane[0].src.buf = cpi->Source->y_buffer +
                          mb_row * 16 * x->plane[0].src.stride + mb_col * 16;
    x->plane[1].src.buf = cpi->Source->u_buffer +
                          mb_row * uv_mb_height * x->plane[1].src.stride +
                          mb_col * uv_mb_height;
    x->plane[2].src.buf = cpi->Source->v_buffer +
                          mb_row * uv_mb_height * x->plane[1].src.stride +
                          mb_col * uv_mb_height;

    vpx_clear_system_state();

    xd->plane[0].dst.buf = new_yv12->y_buffer + recon_yoffset;
    xd->plane[1].dst.buf = new_yv12->u_buffer + recon_uvoffset;
    xd->plane[2].dst.buf = new_yv12->v_buffer + recon_uvoffset;
    xd->mi[0]->sb_type = bsize;
    xd->mi[0]->ref_frame[0] = INTRA_FRAME;
    set_mi_row_col(xd, &tile, mb_row << 1, num_8x8_blocks_high_lookup[bsize],
                   mb_col << 1, num_8x8_blocks_wide_lookup[bsize], cm->mi_rows,
                   cm->mi_cols);




    xd->above_mi = (mb_row != 0) ? &mi_above : NULL;
    xd->left_mi = ((mb_col << 1) > tile.mi_col_start) ? &mi_left : NULL;

    x->skip_encode = 0;
    x->fp_src_pred = 0;

    if (mb_col == mb_col_start && mb_col != 0) {
      xd->left_mi = &mi_left;
      x->fp_src_pred = 1;
    }
    xd->mi[0]->mode = DC_PRED;
    xd->mi[0]->tx_size =
        use_dc_pred ? (bsize >= BLOCK_16X16 ? TX_16X16 : TX_8X8) : TX_4X4;


    vp9_zero_array(x->plane[0].src_diff, 256);
    vp9_encode_intra_block_plane(x, bsize, 0, 0);
    this_error = vpx_get_mb_ss(x->plane[0].src_diff);
    this_intra_error = this_error;





    if (this_error < get_ul_intra_threshold(cm)) {
      ++(fp_acc_data->intra_skip_count);
    } else if ((mb_col > 0) &&
               (fp_acc_data->image_data_start_row == INVALID_ROW)) {
      fp_acc_data->image_data_start_row = mb_row;
    }



    if (this_error < get_smooth_intra_threshold(cm)) {
      ++(fp_acc_data->intra_smooth_count);
    }

    if (cm->current_video_frame == 0) {
      if (this_intra_error < scale_sse_threshold(cm, LOW_I_THRESH)) {
        fp_acc_data->frame_noise_energy += fp_estimate_block_noise(x, bsize);
      } else {
        fp_acc_data->frame_noise_energy += (int64_t)SECTION_NOISE_DEF;
      }
    }

#if CONFIG_VP9_HIGHBITDEPTH
    if (cm->use_highbitdepth) {
      switch (cm->bit_depth) {
        case VPX_BITS_8: break;
        case VPX_BITS_10: this_error >>= 4; break;
        default:
          assert(cm->bit_depth == VPX_BITS_12);
          this_error >>= 8;
          break;
      }
    }
#endif  // CONFIG_VP9_HIGHBITDEPTH

    vpx_clear_system_state();
    log_intra = log(this_error + 1.0);
    if (log_intra < 10.0) {
      mb_intra_factor = 1.0 + ((10.0 - log_intra) * 0.05);
      fp_acc_data->intra_factor += mb_intra_factor;
      if (cpi->row_mt_bit_exact)
        cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_intra_factor =
            mb_intra_factor;
    } else {
      fp_acc_data->intra_factor += 1.0;
      if (cpi->row_mt_bit_exact)
        cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_intra_factor = 1.0;
    }

#if CONFIG_VP9_HIGHBITDEPTH
    if (cm->use_highbitdepth)
      level_sample = CONVERT_TO_SHORTPTR(x->plane[0].src.buf)[0];
    else
      level_sample = x->plane[0].src.buf[0];
#else
    level_sample = x->plane[0].src.buf[0];
#endif
    if ((level_sample < DARK_THRESH) && (log_intra < 9.0)) {
      mb_brightness_factor = 1.0 + (0.01 * (DARK_THRESH - level_sample));
      fp_acc_data->brightness_factor += mb_brightness_factor;
      if (cpi->row_mt_bit_exact)
        cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_brightness_factor =
            mb_brightness_factor;
    } else {
      fp_acc_data->brightness_factor += 1.0;
      if (cpi->row_mt_bit_exact)
        cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_brightness_factor =
            1.0;
    }







    this_error += intrapenalty;

    fp_acc_data->intra_error += (int64_t)this_error;

#if CONFIG_FP_MB_STATS
    if (cpi->use_fp_mb_stats) {

      cpi->twopass.frame_mb_stats_buf[mb_index] = 0;
    }
#endif


    x->mv_limits.col_min = -((mb_col * 16) + BORDER_MV_PIXELS_B16);
    x->mv_limits.col_max =
        ((cm->mb_cols - 1 - mb_col) * 16) + BORDER_MV_PIXELS_B16;

    if (cm->current_video_frame > 0) {
      int tmp_err, motion_error, this_motion_error, raw_motion_error;

      MV mv = { 0, 0 }, tmp_mv = { 0, 0 };
      struct buf_2d unscaled_last_source_buf_2d;
      vp9_variance_fn_ptr_t v_fn_ptr = cpi->fn_ptr[bsize];

#if CONFIG_RATE_CTRL

      store_fp_motion_vector(cpi, &mv, mb_row, mb_col, LAST_FRAME, 0);
#endif  // CONFIG_RAGE_CTRL

      xd->plane[0].pre[0].buf = first_ref_buf->y_buffer + recon_yoffset;
#if CONFIG_VP9_HIGHBITDEPTH
      if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
        motion_error = highbd_get_prediction_error(
            bsize, &x->plane[0].src, &xd->plane[0].pre[0], xd->bd);
        this_motion_error = highbd_get_prediction_error(
            bsize, &x->plane[0].src, &xd->plane[0].pre[0], 8);
      } else {
        motion_error =
            get_prediction_error(bsize, &x->plane[0].src, &xd->plane[0].pre[0]);
        this_motion_error = motion_error;
      }
#else
      motion_error =
          get_prediction_error(bsize, &x->plane[0].src, &xd->plane[0].pre[0]);
      this_motion_error = motion_error;
#endif  // CONFIG_VP9_HIGHBITDEPTH



      unscaled_last_source_buf_2d.buf =
          cpi->unscaled_last_source->y_buffer + recon_yoffset;
      unscaled_last_source_buf_2d.stride = cpi->unscaled_last_source->y_stride;
#if CONFIG_VP9_HIGHBITDEPTH
      if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
        raw_motion_error = highbd_get_prediction_error(
            bsize, &x->plane[0].src, &unscaled_last_source_buf_2d, xd->bd);
      } else {
        raw_motion_error = get_prediction_error(bsize, &x->plane[0].src,
                                                &unscaled_last_source_buf_2d);
      }
#else
      raw_motion_error = get_prediction_error(bsize, &x->plane[0].src,
                                              &unscaled_last_source_buf_2d);
#endif  // CONFIG_VP9_HIGHBITDEPTH

      if (raw_motion_error > NZ_MOTION_PENALTY) {


        first_pass_motion_search(cpi, x, best_ref_mv, &mv, &motion_error);

        v_fn_ptr.vf = get_block_variance_fn(bsize);
#if CONFIG_VP9_HIGHBITDEPTH
        if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
          v_fn_ptr.vf = highbd_get_block_variance_fn(bsize, 8);
        }
#endif  // CONFIG_VP9_HIGHBITDEPTH
        this_motion_error =
            vp9_get_mvpred_var(x, &mv, best_ref_mv, &v_fn_ptr, 0);


        if (!is_zero_mv(best_ref_mv)) {
          tmp_err = INT_MAX;
          first_pass_motion_search(cpi, x, &zero_mv, &tmp_mv, &tmp_err);

          if (tmp_err < motion_error) {
            motion_error = tmp_err;
            mv = tmp_mv;
            this_motion_error =
                vp9_get_mvpred_var(x, &tmp_mv, &zero_mv, &v_fn_ptr, 0);
          }
        }
#if CONFIG_RATE_CTRL
        store_fp_motion_vector(cpi, &mv, mb_row, mb_col, LAST_FRAME, 0);
#endif  // CONFIG_RAGE_CTRL

        if ((cm->current_video_frame > 1) && gld_yv12 != NULL) {

          int gf_motion_error;

          xd->plane[0].pre[0].buf = gld_yv12->y_buffer + recon_yoffset;
#if CONFIG_VP9_HIGHBITDEPTH
          if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
            gf_motion_error = highbd_get_prediction_error(
                bsize, &x->plane[0].src, &xd->plane[0].pre[0], xd->bd);
          } else {
            gf_motion_error = get_prediction_error(bsize, &x->plane[0].src,
                                                   &xd->plane[0].pre[0]);
          }
#else
          gf_motion_error = get_prediction_error(bsize, &x->plane[0].src,
                                                 &xd->plane[0].pre[0]);
#endif  // CONFIG_VP9_HIGHBITDEPTH

          first_pass_motion_search(cpi, x, &zero_mv, &tmp_mv, &gf_motion_error);
#if CONFIG_RATE_CTRL
          store_fp_motion_vector(cpi, &tmp_mv, mb_row, mb_col, GOLDEN_FRAME, 1);
#endif  // CONFIG_RAGE_CTRL

          if (gf_motion_error < motion_error && gf_motion_error < this_error)
            ++(fp_acc_data->second_ref_count);

          xd->plane[0].pre[0].buf = first_ref_buf->y_buffer + recon_yoffset;
          xd->plane[1].pre[0].buf = first_ref_buf->u_buffer + recon_uvoffset;
          xd->plane[2].pre[0].buf = first_ref_buf->v_buffer + recon_uvoffset;




          if (gf_motion_error < this_error)
            fp_acc_data->sr_coded_error += gf_motion_error;
          else
            fp_acc_data->sr_coded_error += this_error;
        } else {
          fp_acc_data->sr_coded_error += motion_error;
        }
      } else {
        fp_acc_data->sr_coded_error += motion_error;
      }

      best_ref_mv->row = 0;
      best_ref_mv->col = 0;

#if CONFIG_FP_MB_STATS
      if (cpi->use_fp_mb_stats) {

        cpi->twopass.frame_mb_stats_buf[mb_index] = 0;
        cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_DCINTRA_MASK;
        cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_MOTION_ZERO_MASK;
        if (this_error > FPMB_ERROR_LARGE_TH) {
          cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_ERROR_LARGE_MASK;
        } else if (this_error < FPMB_ERROR_SMALL_TH) {
          cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_ERROR_SMALL_MASK;
        }
      }
#endif

      if (motion_error <= this_error) {
        vpx_clear_system_state();



        if (((this_error - intrapenalty) * 9 <= motion_error * 10) &&
            (this_error < (2 * intrapenalty))) {
          fp_acc_data->neutral_count += 1.0;
          if (cpi->row_mt_bit_exact)
            cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_neutral_count =
                1.0;


        } else if ((this_error > NCOUNT_INTRA_THRESH) &&
                   (this_error < (NCOUNT_INTRA_FACTOR * motion_error))) {
          mb_neutral_count =
              (double)motion_error / DOUBLE_DIVIDE_CHECK((double)this_error);
          fp_acc_data->neutral_count += mb_neutral_count;
          if (cpi->row_mt_bit_exact)
            cpi->twopass.fp_mb_float_stats[mb_index].frame_mb_neutral_count =
                mb_neutral_count;
        }

        mv.row *= 8;
        mv.col *= 8;
        this_error = motion_error;
        xd->mi[0]->mode = NEWMV;
        xd->mi[0]->mv[0].as_mv = mv;
        xd->mi[0]->tx_size = TX_4X4;
        xd->mi[0]->ref_frame[0] = LAST_FRAME;
        xd->mi[0]->ref_frame[1] = NONE;
        vp9_build_inter_predictors_sby(xd, mb_row << 1, mb_col << 1, bsize);
        vp9_encode_sby_pass1(x, bsize);
        fp_acc_data->sum_mvr += mv.row;
        fp_acc_data->sum_mvr_abs += abs(mv.row);
        fp_acc_data->sum_mvc += mv.col;
        fp_acc_data->sum_mvc_abs += abs(mv.col);
        fp_acc_data->sum_mvrs += mv.row * mv.row;
        fp_acc_data->sum_mvcs += mv.col * mv.col;
        ++(fp_acc_data->intercount);

        *best_ref_mv = mv;

#if CONFIG_FP_MB_STATS
        if (cpi->use_fp_mb_stats) {

          cpi->twopass.frame_mb_stats_buf[mb_index] = 0;
          cpi->twopass.frame_mb_stats_buf[mb_index] &= ~FPMB_DCINTRA_MASK;
          cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_MOTION_ZERO_MASK;
          if (this_error > FPMB_ERROR_LARGE_TH) {
            cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_ERROR_LARGE_MASK;
          } else if (this_error < FPMB_ERROR_SMALL_TH) {
            cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_ERROR_SMALL_MASK;
          }
        }
#endif

        if (!is_zero_mv(&mv)) {
          ++(fp_acc_data->mvcount);

#if CONFIG_FP_MB_STATS
          if (cpi->use_fp_mb_stats) {
            cpi->twopass.frame_mb_stats_buf[mb_index] &= ~FPMB_MOTION_ZERO_MASK;

            if (mv.as_mv.col > 0 && mv.as_mv.col >= abs(mv.as_mv.row)) {

              cpi->twopass.frame_mb_stats_buf[mb_index] |=
                  FPMB_MOTION_RIGHT_MASK;
            } else if (mv.as_mv.row < 0 &&
                       abs(mv.as_mv.row) >= abs(mv.as_mv.col)) {

              cpi->twopass.frame_mb_stats_buf[mb_index] |= FPMB_MOTION_UP_MASK;
            } else if (mv.as_mv.col < 0 &&
                       abs(mv.as_mv.col) >= abs(mv.as_mv.row)) {

              cpi->twopass.frame_mb_stats_buf[mb_index] |=
                  FPMB_MOTION_LEFT_MASK;
            } else {

              cpi->twopass.frame_mb_stats_buf[mb_index] |=
                  FPMB_MOTION_DOWN_MASK;
            }
          }
#endif

          if (mb_row < cm->mb_rows / 2) {
            if (mv.row > 0)
              --(fp_acc_data->sum_in_vectors);
            else if (mv.row < 0)
              ++(fp_acc_data->sum_in_vectors);
          } else if (mb_row > cm->mb_rows / 2) {
            if (mv.row > 0)
              ++(fp_acc_data->sum_in_vectors);
            else if (mv.row < 0)
              --(fp_acc_data->sum_in_vectors);
          }

          if (mb_col < cm->mb_cols / 2) {
            if (mv.col > 0)
              --(fp_acc_data->sum_in_vectors);
            else if (mv.col < 0)
              ++(fp_acc_data->sum_in_vectors);
          } else if (mb_col > cm->mb_cols / 2) {
            if (mv.col > 0)
              ++(fp_acc_data->sum_in_vectors);
            else if (mv.col < 0)
              --(fp_acc_data->sum_in_vectors);
          }
        }
        if (this_intra_error < scaled_low_intra_thresh) {
          fp_acc_data->frame_noise_energy += fp_estimate_block_noise(x, bsize);
        } else {
          fp_acc_data->frame_noise_energy += (int64_t)SECTION_NOISE_DEF;
        }
      } else {  // Intra < inter error
        if (this_intra_error < scaled_low_intra_thresh) {
          fp_acc_data->frame_noise_energy += fp_estimate_block_noise(x, bsize);
          if (this_motion_error < scaled_low_intra_thresh) {
            fp_acc_data->intra_count_low += 1.0;
          } else {
            fp_acc_data->intra_count_high += 1.0;
          }
        } else {
          fp_acc_data->frame_noise_energy += (int64_t)SECTION_NOISE_DEF;
          fp_acc_data->intra_count_high += 1.0;
        }
      }
    } else {
      fp_acc_data->sr_coded_error += (int64_t)this_error;
#if CONFIG_RATE_CTRL
      store_fp_motion_vector(cpi, NULL, mb_row, mb_col, INTRA_FRAME, 0);
#endif  // CONFIG_RAGE_CTRL
    }
    fp_acc_data->coded_error += (int64_t)this_error;

    recon_yoffset += 16;
    recon_uvoffset += uv_mb_height;

    if (cpi->row_mt && mb_col == mb_col_end - 1)
      accumulate_fp_mb_row_stat(tile_data, fp_acc_data);

    (*(cpi->row_mt_sync_write_ptr))(&tile_data->row_mt_sync, mb_row, c,
                                    num_mb_cols);
  }
  vpx_clear_system_state();
}

static void first_pass_encode(VP9_COMP *cpi, FIRSTPASS_DATA *fp_acc_data) {
  VP9_COMMON *const cm = &cpi->common;
  int mb_row;
  TileDataEnc tile_data;
  TileInfo *tile = &tile_data.tile_info;
  MV zero_mv = { 0, 0 };
  MV best_ref_mv;

  vp9_tile_init(tile, cm, 0, 0);

#if CONFIG_RATE_CTRL
  fp_motion_vector_info_reset(cpi->frame_info.frame_width,
                              cpi->frame_info.frame_height,
                              cpi->fp_motion_vector_info);
#endif

  for (mb_row = 0; mb_row < cm->mb_rows; ++mb_row) {
    best_ref_mv = zero_mv;
    vp9_first_pass_encode_tile_mb_row(cpi, &cpi->td, fp_acc_data, &tile_data,
                                      &best_ref_mv, mb_row);
  }
}

void vp9_first_pass(VP9_COMP *cpi, const struct lookahead_entry *source) {
  MACROBLOCK *const x = &cpi->td.mb;
  VP9_COMMON *const cm = &cpi->common;
  MACROBLOCKD *const xd = &x->e_mbd;
  TWO_PASS *twopass = &cpi->twopass;

  YV12_BUFFER_CONFIG *const lst_yv12 = get_ref_frame_buffer(cpi, LAST_FRAME);
  YV12_BUFFER_CONFIG *gld_yv12 = get_ref_frame_buffer(cpi, GOLDEN_FRAME);
  YV12_BUFFER_CONFIG *const new_yv12 = get_frame_new_buffer(cm);
  const YV12_BUFFER_CONFIG *first_ref_buf = lst_yv12;

  BufferPool *const pool = cm->buffer_pool;

  FIRSTPASS_DATA fp_temp_data;
  FIRSTPASS_DATA *fp_acc_data = &fp_temp_data;

  vpx_clear_system_state();
  vp9_zero(fp_temp_data);
  fp_acc_data->image_data_start_row = INVALID_ROW;

  assert(new_yv12 != NULL);
  assert(frame_is_intra_only(cm) || (lst_yv12 != NULL));

#if CONFIG_FP_MB_STATS
  if (cpi->use_fp_mb_stats) {
    vp9_zero_array(cpi->twopass.frame_mb_stats_buf, cm->initial_mbs);
  }
#endif

  set_first_pass_params(cpi);
  vp9_set_quantizer(cpi, find_fp_qindex(cm->bit_depth));

  vp9_setup_block_planes(&x->e_mbd, cm->subsampling_x, cm->subsampling_y);

  vp9_setup_src_planes(x, cpi->Source, 0, 0);
  vp9_setup_dst_planes(xd->plane, new_yv12, 0, 0);

  if (!frame_is_intra_only(cm)) {
    vp9_setup_pre_planes(xd, 0, first_ref_buf, 0, 0, NULL);
  }

  xd->mi = cm->mi_grid_visible;
  xd->mi[0] = cm->mi;

  vp9_frame_init_quantizer(cpi);

  x->skip_recode = 0;

  vp9_init_mv_probs(cm);
  vp9_initialize_rd_consts(cpi);

  cm->log2_tile_rows = 0;

  if (cpi->row_mt_bit_exact && cpi->twopass.fp_mb_float_stats == NULL)
    CHECK_MEM_ERROR(
        cm, cpi->twopass.fp_mb_float_stats,
        vpx_calloc(cm->MBs * sizeof(*cpi->twopass.fp_mb_float_stats), 1));

  {
    FIRSTPASS_STATS fps;
    TileDataEnc *first_tile_col;
    if (!cpi->row_mt) {
      cm->log2_tile_cols = 0;
      cpi->row_mt_sync_read_ptr = vp9_row_mt_sync_read_dummy;
      cpi->row_mt_sync_write_ptr = vp9_row_mt_sync_write_dummy;
      first_pass_encode(cpi, fp_acc_data);
      first_pass_stat_calc(cpi, &fps, fp_acc_data);
    } else {
      cpi->row_mt_sync_read_ptr = vp9_row_mt_sync_read;
      cpi->row_mt_sync_write_ptr = vp9_row_mt_sync_write;
      if (cpi->row_mt_bit_exact) {
        cm->log2_tile_cols = 0;
        vp9_zero_array(cpi->twopass.fp_mb_float_stats, cm->MBs);
      }
      vp9_encode_fp_row_mt(cpi);
      first_tile_col = &cpi->tile_data[0];
      if (cpi->row_mt_bit_exact)
        accumulate_floating_point_stats(cpi, first_tile_col);
      first_pass_stat_calc(cpi, &fps, &(first_tile_col->fp_data));
    }


    fps.duration = VPXMAX(1.0, (double)(source->ts_end - source->ts_start));

    twopass->this_frame_stats = fps;
    output_stats(&twopass->this_frame_stats);
    accumulate_stats(&twopass->total_stats, &fps);

#if CONFIG_FP_MB_STATS
    if (cpi->use_fp_mb_stats) {
      output_fpmb_stats(twopass->frame_mb_stats_buf, cm, cpi->output_pkt_list);
    }
#endif
  }


  if ((twopass->sr_update_lag > 3) ||
      ((cm->current_video_frame > 0) &&
       (twopass->this_frame_stats.pcnt_inter > 0.20) &&
       ((twopass->this_frame_stats.intra_error /
         DOUBLE_DIVIDE_CHECK(twopass->this_frame_stats.coded_error)) > 2.0))) {
    if (gld_yv12 != NULL) {
      ref_cnt_fb(pool->frame_bufs, &cm->ref_frame_map[cpi->gld_fb_idx],
                 cm->ref_frame_map[cpi->lst_fb_idx]);
    }
    twopass->sr_update_lag = 1;
  } else {
    ++twopass->sr_update_lag;
  }

  vpx_extend_frame_borders(new_yv12);

  ref_cnt_fb(pool->frame_bufs, &cm->ref_frame_map[cpi->lst_fb_idx],
             cm->new_fb_idx);


  if (cm->current_video_frame == 0 && cpi->gld_fb_idx != INVALID_IDX) {
    ref_cnt_fb(pool->frame_bufs, &cm->ref_frame_map[cpi->gld_fb_idx],
               cm->ref_frame_map[cpi->lst_fb_idx]);
  }

  if (0) {
    char filename[512];
    FILE *recon_file;
    snprintf(filename, sizeof(filename), "enc%04d.yuv",
             (int)cm->current_video_frame);

    if (cm->current_video_frame == 0)
      recon_file = fopen(filename, "wb");
    else
      recon_file = fopen(filename, "ab");

    (void)fwrite(lst_yv12->buffer_alloc, lst_yv12->frame_size, 1, recon_file);
    fclose(recon_file);
  }

  update_frame_indexes(cm, /*show_frame=*/1);
  if (cpi->use_svc) vp9_inc_frame_in_layer(cpi);
}

static const double q_pow_term[(QINDEX_RANGE >> 5) + 1] = { 0.65, 0.70, 0.75,
                                                            0.85, 0.90, 0.90,
                                                            0.90, 1.00, 1.25 };

static double calc_correction_factor(double err_per_mb, double err_divisor,
                                     int q) {
  const double error_term = err_per_mb / DOUBLE_DIVIDE_CHECK(err_divisor);
  const int index = q >> 5;
  double power_term;

  assert((index >= 0) && (index < (QINDEX_RANGE >> 5)));

  power_term =
      q_pow_term[index] +
      (((q_pow_term[index + 1] - q_pow_term[index]) * (q % 32)) / 32.0);

  if (power_term < 1.0) assert(error_term >= 0.0);

  return fclamp(pow(error_term, power_term), 0.05, 5.0);
}

static double wq_err_divisor(VP9_COMP *cpi) {
  const VP9_COMMON *const cm = &cpi->common;
  unsigned int screen_area = (cm->width * cm->height);


  if (screen_area <= 640 * 360) {
    return 115.0;
  } else if (screen_area < 1280 * 720) {
    return 125.0;
  } else if (screen_area <= 1920 * 1080) {
    return 130.0;
  } else if (screen_area < 3840 * 2160) {
    return 150.0;
  }

  return 200.0;
}

#define NOISE_FACTOR_MIN 0.9
#define NOISE_FACTOR_MAX 1.1
static int get_twopass_worst_quality(VP9_COMP *cpi, const double section_err,
                                     double inactive_zone, double section_noise,
                                     int section_target_bandwidth) {
  const RATE_CONTROL *const rc = &cpi->rc;
  const VP9EncoderConfig *const oxcf = &cpi->oxcf;
  TWO_PASS *const twopass = &cpi->twopass;
  double last_group_rate_err;

  const int target_rate =
      vp9_rc_clamp_pframe_target_size(cpi, section_target_bandwidth);
  double noise_factor = pow((section_noise / SECTION_NOISE_DEF), 0.5);
  noise_factor = fclamp(noise_factor, NOISE_FACTOR_MIN, NOISE_FACTOR_MAX);
  inactive_zone = fclamp(inactive_zone, 0.0, 1.0);

// well tested.
#if CONFIG_ALWAYS_ADJUST_BPM

  last_group_rate_err =
      (double)twopass->rolling_arf_group_actual_bits /
      DOUBLE_DIVIDE_CHECK((double)twopass->rolling_arf_group_target_bits);
  last_group_rate_err = VPXMAX(0.25, VPXMIN(4.0, last_group_rate_err));
  twopass->bpm_factor *= (3.0 + last_group_rate_err) / 4.0;
  twopass->bpm_factor = VPXMAX(0.25, VPXMIN(4.0, twopass->bpm_factor));
#endif

  if (target_rate <= 0) {
    return rc->worst_quality;  // Highest value allowed
  } else {
    const int num_mbs = (cpi->oxcf.resize_mode != RESIZE_NONE)
                            ? cpi->initial_mbs
                            : cpi->common.MBs;
    const double active_pct = VPXMAX(0.01, 1.0 - inactive_zone);
    const int active_mbs = (int)VPXMAX(1, (double)num_mbs * active_pct);
    const double av_err_per_mb = section_err / active_pct;
    const double speed_term = 1.0 + 0.04 * oxcf->speed;
    const int target_norm_bits_per_mb =
        (int)(((uint64_t)target_rate << BPER_MB_NORMBITS) / active_mbs);
    int q;

// well tested.
#if !CONFIG_ALWAYS_ADJUST_BPM

    last_group_rate_err =
        (double)twopass->rolling_arf_group_actual_bits /
        DOUBLE_DIVIDE_CHECK((double)twopass->rolling_arf_group_target_bits);
    last_group_rate_err = VPXMAX(0.25, VPXMIN(4.0, last_group_rate_err));
    twopass->bpm_factor *= (3.0 + last_group_rate_err) / 4.0;
    twopass->bpm_factor = VPXMAX(0.25, VPXMIN(4.0, twopass->bpm_factor));
#endif


    for (q = rc->best_quality; q < rc->worst_quality; ++q) {
      const double factor =
          calc_correction_factor(av_err_per_mb, wq_err_divisor(cpi), q);
      const int bits_per_mb = vp9_rc_bits_per_mb(
          INTER_FRAME, q,
          factor * speed_term * cpi->twopass.bpm_factor * noise_factor,
          cpi->common.bit_depth);
      if (bits_per_mb <= target_norm_bits_per_mb) break;
    }

    if (cpi->oxcf.rc_mode == VPX_CQ) q = VPXMAX(q, oxcf->cq_level);
    return q;
  }
}

static void setup_rf_level_maxq(VP9_COMP *cpi) {
  int i;
  RATE_CONTROL *const rc = &cpi->rc;
  for (i = INTER_NORMAL; i < RATE_FACTOR_LEVELS; ++i) {
    int qdelta = vp9_frame_type_qdelta(cpi, i, rc->worst_quality);
    rc->rf_level_maxq[i] = VPXMAX(rc->worst_quality + qdelta, rc->best_quality);
  }
}

static void init_subsampling(VP9_COMP *cpi) {
  const VP9_COMMON *const cm = &cpi->common;
  RATE_CONTROL *const rc = &cpi->rc;
  const int w = cm->width;
  const int h = cm->height;
  int i;

  for (i = 0; i < FRAME_SCALE_STEPS; ++i) {

    rc->frame_width[i] = (w * 16) / frame_scale_factor[i];
    rc->frame_height[i] = (h * 16) / frame_scale_factor[i];
  }

  setup_rf_level_maxq(cpi);
}

void calculate_coded_size(VP9_COMP *cpi, int *scaled_frame_width,
                          int *scaled_frame_height) {
  RATE_CONTROL *const rc = &cpi->rc;
  *scaled_frame_width = rc->frame_width[rc->frame_size_selector];
  *scaled_frame_height = rc->frame_height[rc->frame_size_selector];
}

void vp9_init_second_pass(VP9_COMP *cpi) {
  VP9EncoderConfig *const oxcf = &cpi->oxcf;
  RATE_CONTROL *const rc = &cpi->rc;
  TWO_PASS *const twopass = &cpi->twopass;
  double frame_rate;
  FIRSTPASS_STATS *stats;

  zero_stats(&twopass->total_stats);
  zero_stats(&twopass->total_left_stats);

  if (!twopass->stats_in_end) return;

  stats = &twopass->total_stats;

  *stats = *twopass->stats_in_end;
  twopass->total_left_stats = *stats;




  {
    double modified_score_total = 0.0;
    const FIRSTPASS_STATS *s = twopass->stats_in;
    double av_err;

    if (oxcf->vbr_corpus_complexity) {
      twopass->mean_mod_score = (double)oxcf->vbr_corpus_complexity / 10.0;
      av_err = get_distribution_av_err(cpi, twopass);
    } else {
      av_err = get_distribution_av_err(cpi, twopass);

      while (s < twopass->stats_in_end) {
        modified_score_total += calculate_mod_frame_score(cpi, oxcf, s, av_err);
        ++s;
      }


      twopass->mean_mod_score =
          modified_score_total / DOUBLE_DIVIDE_CHECK(stats->count);
    }



    modified_score_total = 0.0;
    s = twopass->stats_in;
    while (s < twopass->stats_in_end) {
      modified_score_total +=
          calculate_norm_frame_score(cpi, twopass, oxcf, s, av_err);
      ++s;
    }
    twopass->normalized_score_left = modified_score_total;


    if (oxcf->vbr_corpus_complexity) {
      oxcf->target_bandwidth =
          (int64_t)((double)oxcf->target_bandwidth *
                    (twopass->normalized_score_left / stats->count));
    }

#if COMPLEXITY_STATS_OUTPUT
    {
      FILE *compstats;
      compstats = fopen("complexity_stats.stt", "a");
      fprintf(compstats, "%10.3lf\n",
              twopass->normalized_score_left / stats->count);
      fclose(compstats);
    }
#endif
  }

  frame_rate = 10000000.0 * stats->count / stats->duration;





  vp9_new_framerate(cpi, frame_rate);
  twopass->bits_left =
      (int64_t)(stats->duration * oxcf->target_bandwidth / 10000000.0);

  twopass->sr_update_lag = 1;

  rc->vbr_bits_off_target = 0;
  rc->vbr_bits_off_target_fast = 0;
  rc->rate_error_estimate = 0;

  twopass->kf_zeromotion_pct = 100;
  twopass->last_kfgroup_zeromotion_pct = 100;

  twopass->bpm_factor = 1.0;


  twopass->rolling_arf_group_target_bits = 1;
  twopass->rolling_arf_group_actual_bits = 1;

  if (oxcf->resize_mode != RESIZE_NONE) {
    init_subsampling(cpi);
  }

  twopass->arnr_strength_adjustment = 0;
}

#define SR_DIFF_PART 0.0015
#define INTRA_PART 0.005
#define DEFAULT_DECAY_LIMIT 0.75
#define LOW_SR_DIFF_TRHESH 0.1
#define SR_DIFF_MAX 128.0
#define LOW_CODED_ERR_PER_MB 10.0
#define NCOUNT_FRAME_II_THRESH 6.0

static double get_sr_decay_rate(const FRAME_INFO *frame_info,
                                const FIRSTPASS_STATS *frame) {
  double sr_diff = (frame->sr_coded_error - frame->coded_error);
  double sr_decay = 1.0;
  double modified_pct_inter;
  double modified_pcnt_intra;
  const double motion_amplitude_part =
      frame->pcnt_motion *
      ((frame->mvc_abs + frame->mvr_abs) /
       (frame_info->frame_height + frame_info->frame_width));

  modified_pct_inter = frame->pcnt_inter;
  if ((frame->coded_error > LOW_CODED_ERR_PER_MB) &&
      ((frame->intra_error / DOUBLE_DIVIDE_CHECK(frame->coded_error)) <
       (double)NCOUNT_FRAME_II_THRESH)) {
    modified_pct_inter =
        frame->pcnt_inter + frame->pcnt_intra_low - frame->pcnt_neutral;
  }
  modified_pcnt_intra = 100 * (1.0 - modified_pct_inter);

  if ((sr_diff > LOW_SR_DIFF_TRHESH)) {
    sr_diff = VPXMIN(sr_diff, SR_DIFF_MAX);
    sr_decay = 1.0 - (SR_DIFF_PART * sr_diff) - motion_amplitude_part -
               (INTRA_PART * modified_pcnt_intra);
  }
  return VPXMAX(sr_decay, DEFAULT_DECAY_LIMIT);
}

// quality is decaying from frame to frame.
static double get_zero_motion_factor(const FRAME_INFO *frame_info,
                                     const FIRSTPASS_STATS *frame_stats) {
  const double zero_motion_pct =
      frame_stats->pcnt_inter - frame_stats->pcnt_motion;
  double sr_decay = get_sr_decay_rate(frame_info, frame_stats);
  return VPXMIN(sr_decay, zero_motion_pct);
}

#define ZM_POWER_FACTOR 0.75

static double get_prediction_decay_rate(const FRAME_INFO *frame_info,
                                        const FIRSTPASS_STATS *frame_stats) {
  const double sr_decay_rate = get_sr_decay_rate(frame_info, frame_stats);
  const double zero_motion_factor =
      (0.95 * pow((frame_stats->pcnt_inter - frame_stats->pcnt_motion),
                  ZM_POWER_FACTOR));

  return VPXMAX(zero_motion_factor,
                (sr_decay_rate + ((1.0 - sr_decay_rate) * zero_motion_factor)));
}

static int get_show_idx(const TWO_PASS *twopass) {
  return (int)(twopass->stats_in - twopass->stats_in_start);
}
// Function to test for a condition where a complex transition is followed
// by a static section. For example in slide shows where there is a fade
// between slides. This is to help with more optimal kf and gf positioning.
static int check_transition_to_still(const FIRST_PASS_INFO *first_pass_info,
                                     int show_idx, int still_interval) {
  int j;
  int num_frames = fps_get_num_frames(first_pass_info);
  if (show_idx + still_interval > num_frames) {
    return 0;
  }

  for (j = 0; j < still_interval; ++j) {
    const FIRSTPASS_STATS *stats =
        fps_get_frame_stats(first_pass_info, show_idx + j);
    if (stats->pcnt_inter - stats->pcnt_motion < 0.999) break;
  }

  return j == still_interval;
}

// score in the frame following a flash frame. The offset passed in should
// reflect this.
static int detect_flash_from_frame_stats(const FIRSTPASS_STATS *frame_stats) {






  if (frame_stats == NULL) {
    return 0;
  }
  return (frame_stats->sr_coded_error < frame_stats->coded_error) ||
         ((frame_stats->pcnt_second_ref > frame_stats->pcnt_inter) &&
          (frame_stats->pcnt_second_ref >= 0.5));
}

static int detect_flash(const TWO_PASS *twopass, int offset) {
  const FIRSTPASS_STATS *const next_frame = read_frame_stats(twopass, offset);
  return detect_flash_from_frame_stats(next_frame);
}

static void accumulate_frame_motion_stats(const FIRSTPASS_STATS *stats,
                                          double *mv_in_out,
                                          double *mv_in_out_accumulator,
                                          double *abs_mv_in_out_accumulator,
                                          double *mv_ratio_accumulator) {
  const double pct = stats->pcnt_motion;

  *mv_in_out = stats->mv_in_out_count * pct;
  *mv_in_out_accumulator += *mv_in_out;
  *abs_mv_in_out_accumulator += fabs(*mv_in_out);


  if (pct > 0.05) {
    const double mvr_ratio =
        fabs(stats->mvr_abs) / DOUBLE_DIVIDE_CHECK(fabs(stats->MVr));
    const double mvc_ratio =
        fabs(stats->mvc_abs) / DOUBLE_DIVIDE_CHECK(fabs(stats->MVc));

    *mv_ratio_accumulator +=
        pct * (mvr_ratio < stats->mvr_abs ? mvr_ratio : stats->mvr_abs);
    *mv_ratio_accumulator +=
        pct * (mvc_ratio < stats->mvc_abs ? mvc_ratio : stats->mvc_abs);
  }
}

#define BASELINE_ERR_PER_MB 12500.0
#define GF_MAX_BOOST 96.0
static double calc_frame_boost(const FRAME_INFO *frame_info,
                               const FIRSTPASS_STATS *this_frame,
                               int avg_frame_qindex,
                               double this_frame_mv_in_out) {
  double frame_boost;
  const double lq =
      vp9_convert_qindex_to_q(avg_frame_qindex, frame_info->bit_depth);
  const double boost_q_correction = VPXMIN((0.5 + (lq * 0.015)), 1.5);
  const double active_area = calculate_active_area(frame_info, this_frame);

  frame_boost = (BASELINE_ERR_PER_MB * active_area) /
                DOUBLE_DIVIDE_CHECK(this_frame->coded_error);

  if (this_frame_mv_in_out > 0.0)
    frame_boost += frame_boost * (this_frame_mv_in_out * 2.0);

  frame_boost = frame_boost * boost_q_correction;

  return VPXMIN(frame_boost, GF_MAX_BOOST * boost_q_correction);
}

static double kf_err_per_mb(VP9_COMP *cpi) {
  const VP9_COMMON *const cm = &cpi->common;
  unsigned int screen_area = (cm->width * cm->height);


  if (screen_area < 1280 * 720) {
    return 2000.0;
  } else if (screen_area < 1920 * 1080) {
    return 500.0;
  }
  return 250.0;
}

static double calc_kf_frame_boost(VP9_COMP *cpi,
                                  const FIRSTPASS_STATS *this_frame,
                                  double *sr_accumulator,
                                  double this_frame_mv_in_out,
                                  double max_boost) {
  double frame_boost;
  const double lq = vp9_convert_qindex_to_q(
      cpi->rc.avg_frame_qindex[INTER_FRAME], cpi->common.bit_depth);
  const double boost_q_correction = VPXMIN((0.50 + (lq * 0.015)), 2.00);
  const double active_area =
      calculate_active_area(&cpi->frame_info, this_frame);

  frame_boost = (kf_err_per_mb(cpi) * active_area) /
                DOUBLE_DIVIDE_CHECK(this_frame->coded_error + *sr_accumulator);



  *sr_accumulator += (this_frame->sr_coded_error - this_frame->coded_error);
  *sr_accumulator = VPXMAX(0.0, *sr_accumulator);

  if (this_frame_mv_in_out > 0.0)
    frame_boost += frame_boost * (this_frame_mv_in_out * 2.0);




  frame_boost = ((frame_boost + 40.0) * boost_q_correction);

  return VPXMIN(frame_boost, max_boost * boost_q_correction);
}

static int compute_arf_boost(const FRAME_INFO *frame_info,
                             const FIRST_PASS_INFO *first_pass_info,
                             int arf_show_idx, int f_frames, int b_frames,
                             int avg_frame_qindex) {
  int i;
  double boost_score = 0.0;
  double mv_ratio_accumulator = 0.0;
  double decay_accumulator = 1.0;
  double this_frame_mv_in_out = 0.0;
  double mv_in_out_accumulator = 0.0;
  double abs_mv_in_out_accumulator = 0.0;
  int arf_boost;
  int flash_detected = 0;

  for (i = 0; i < f_frames; ++i) {
    const FIRSTPASS_STATS *this_frame =
        fps_get_frame_stats(first_pass_info, arf_show_idx + i);
    const FIRSTPASS_STATS *next_frame =
        fps_get_frame_stats(first_pass_info, arf_show_idx + i + 1);
    if (this_frame == NULL) break;

    accumulate_frame_motion_stats(
        this_frame, &this_frame_mv_in_out, &mv_in_out_accumulator,
        &abs_mv_in_out_accumulator, &mv_ratio_accumulator);


    flash_detected = detect_flash_from_frame_stats(this_frame) ||
                     detect_flash_from_frame_stats(next_frame);

    if (!flash_detected) {
      decay_accumulator *= get_prediction_decay_rate(frame_info, this_frame);
      decay_accumulator = decay_accumulator < MIN_DECAY_FACTOR
                              ? MIN_DECAY_FACTOR
                              : decay_accumulator;
    }
    boost_score += decay_accumulator * calc_frame_boost(frame_info, this_frame,
                                                        avg_frame_qindex,
                                                        this_frame_mv_in_out);
  }

  arf_boost = (int)boost_score;

  boost_score = 0.0;
  mv_ratio_accumulator = 0.0;
  decay_accumulator = 1.0;
  this_frame_mv_in_out = 0.0;
  mv_in_out_accumulator = 0.0;
  abs_mv_in_out_accumulator = 0.0;

  for (i = -1; i >= -b_frames; --i) {
    const FIRSTPASS_STATS *this_frame =
        fps_get_frame_stats(first_pass_info, arf_show_idx + i);
    const FIRSTPASS_STATS *next_frame =
        fps_get_frame_stats(first_pass_info, arf_show_idx + i + 1);
    if (this_frame == NULL) break;

    accumulate_frame_motion_stats(
        this_frame, &this_frame_mv_in_out, &mv_in_out_accumulator,
        &abs_mv_in_out_accumulator, &mv_ratio_accumulator);


    flash_detected = detect_flash_from_frame_stats(this_frame) ||
                     detect_flash_from_frame_stats(next_frame);

    if (!flash_detected) {
      decay_accumulator *= get_prediction_decay_rate(frame_info, this_frame);
      decay_accumulator = decay_accumulator < MIN_DECAY_FACTOR
                              ? MIN_DECAY_FACTOR
                              : decay_accumulator;
    }
    boost_score += decay_accumulator * calc_frame_boost(frame_info, this_frame,
                                                        avg_frame_qindex,
                                                        this_frame_mv_in_out);
  }
  arf_boost += (int)boost_score;

  if (arf_boost < ((b_frames + f_frames) * 40))
    arf_boost = ((b_frames + f_frames) * 40);
  arf_boost = VPXMAX(arf_boost, MIN_ARF_GF_BOOST);

  return arf_boost;
}

static int calc_arf_boost(VP9_COMP *cpi, int f_frames, int b_frames) {
  const FRAME_INFO *frame_info = &cpi->frame_info;
  TWO_PASS *const twopass = &cpi->twopass;
  const int avg_inter_frame_qindex = cpi->rc.avg_frame_qindex[INTER_FRAME];
  int arf_show_idx = get_show_idx(twopass);
  return compute_arf_boost(frame_info, &twopass->first_pass_info, arf_show_idx,
                           f_frames, b_frames, avg_inter_frame_qindex);
}

static int calculate_section_intra_ratio(const FIRSTPASS_STATS *begin,
                                         const FIRSTPASS_STATS *end,
                                         int section_length) {
  const FIRSTPASS_STATS *s = begin;
  double intra_error = 0.0;
  double coded_error = 0.0;
  int i = 0;

  while (s < end && i < section_length) {
    intra_error += s->intra_error;
    coded_error += s->coded_error;
    ++s;
    ++i;
  }

  return (int)(intra_error / DOUBLE_DIVIDE_CHECK(coded_error));
}

static int64_t calculate_total_gf_group_bits(VP9_COMP *cpi,
                                             double gf_group_err) {
  VP9_COMMON *const cm = &cpi->common;
  const RATE_CONTROL *const rc = &cpi->rc;
  const TWO_PASS *const twopass = &cpi->twopass;
  const int max_bits = frame_max_bits(rc, &cpi->oxcf);
  int64_t total_group_bits;
  const int is_key_frame = frame_is_intra_only(cm);
  const int arf_active_or_kf = is_key_frame || rc->source_alt_ref_active;
  int gop_frames =
      rc->baseline_gf_interval + rc->source_alt_ref_pending - arf_active_or_kf;

  if ((twopass->kf_group_bits > 0) && (twopass->kf_group_error_left > 0.0)) {
    int key_frame_interval = rc->frames_since_key + rc->frames_to_key;
    int distance_from_next_key_frame =
        rc->frames_to_key -
        (rc->baseline_gf_interval + rc->source_alt_ref_pending);
    int max_gf_bits_bias = rc->avg_frame_bandwidth;
    double gf_interval_bias_bits_normalize_factor =
        (double)rc->baseline_gf_interval / 16;
    total_group_bits = (int64_t)(twopass->kf_group_bits *
                                 (gf_group_err / twopass->kf_group_error_left));

    total_group_bits +=
        (int64_t)((double)distance_from_next_key_frame / key_frame_interval *
                  max_gf_bits_bias * gf_interval_bias_bits_normalize_factor);
  } else {
    total_group_bits = 0;
  }

  total_group_bits = (total_group_bits < 0)
                         ? 0
                         : (total_group_bits > twopass->kf_group_bits)
                               ? twopass->kf_group_bits
                               : total_group_bits;

  if (total_group_bits > (int64_t)max_bits * gop_frames)
    total_group_bits = (int64_t)max_bits * gop_frames;

  return total_group_bits;
}

static int calculate_boost_bits(int frame_count, int boost,
                                int64_t total_group_bits) {
  int allocation_chunks;

  if (!boost || (total_group_bits <= 0) || (frame_count < 0)) return 0;

  allocation_chunks = (frame_count * NORMAL_BOOST) + boost;

  if (boost > 1023) {
    int divisor = boost >> 10;
    boost /= divisor;
    allocation_chunks /= divisor;
  }

  return VPXMAX((int)(((int64_t)boost * total_group_bits) / allocation_chunks),
                0);
}

// for a given number of frames starting at the current position in the stats
// file.
static double calculate_group_score(VP9_COMP *cpi, double av_score,
                                    int frame_count) {
  VP9EncoderConfig *const oxcf = &cpi->oxcf;
  TWO_PASS *const twopass = &cpi->twopass;
  const FIRSTPASS_STATS *s = twopass->stats_in;
  double score_total = 0.0;
  int i = 0;

  if (frame_count == 0) return 1.0;

  while ((i < frame_count) && (s < twopass->stats_in_end)) {
    score_total += calculate_norm_frame_score(cpi, twopass, oxcf, s, av_score);
    ++s;
    ++i;
  }

  return score_total;
}

static void find_arf_order(VP9_COMP *cpi, GF_GROUP *gf_group,
                           int *index_counter, int depth, int start, int end) {
  TWO_PASS *twopass = &cpi->twopass;
  const FIRSTPASS_STATS *const start_pos = twopass->stats_in;
  FIRSTPASS_STATS fpf_frame;
  const int mid = (start + end + 1) >> 1;
  const int min_frame_interval = 2;
  int idx;

  if ((end - start < min_frame_interval) ||
      (depth > gf_group->allowed_max_layer_depth)) {
    for (idx = start; idx <= end; ++idx) {
      gf_group->update_type[*index_counter] = LF_UPDATE;
      gf_group->arf_src_offset[*index_counter] = 0;
      gf_group->frame_gop_index[*index_counter] = idx;
      gf_group->rf_level[*index_counter] = INTER_NORMAL;
      gf_group->layer_depth[*index_counter] = depth;
      gf_group->gfu_boost[*index_counter] = NORMAL_BOOST;
      ++(*index_counter);
    }
    gf_group->max_layer_depth = VPXMAX(gf_group->max_layer_depth, depth);
    return;
  }

  assert(abs(mid - start) >= 1 && abs(mid - end) >= 1);

  gf_group->layer_depth[*index_counter] = depth;
  gf_group->update_type[*index_counter] = ARF_UPDATE;
  gf_group->arf_src_offset[*index_counter] = mid - start;
  gf_group->frame_gop_index[*index_counter] = mid;
  gf_group->rf_level[*index_counter] = GF_ARF_LOW;

  for (idx = 0; idx <= mid; ++idx)
    if (EOF == input_stats(twopass, &fpf_frame)) break;

  gf_group->gfu_boost[*index_counter] =
      VPXMAX(MIN_ARF_GF_BOOST,
             calc_arf_boost(cpi, end - mid + 1, mid - start) >> depth);

  reset_fpf_position(twopass, start_pos);

  ++(*index_counter);

  find_arf_order(cpi, gf_group, index_counter, depth + 1, start, mid - 1);

  gf_group->update_type[*index_counter] = USE_BUF_FRAME;
  gf_group->arf_src_offset[*index_counter] = 0;
  gf_group->frame_gop_index[*index_counter] = mid;
  gf_group->rf_level[*index_counter] = INTER_NORMAL;
  gf_group->layer_depth[*index_counter] = depth;
  ++(*index_counter);

  find_arf_order(cpi, gf_group, index_counter, depth + 1, mid + 1, end);
}

static INLINE void set_gf_overlay_frame_type(GF_GROUP *gf_group,
                                             int frame_index,
                                             int source_alt_ref_active) {
  if (source_alt_ref_active) {
    gf_group->update_type[frame_index] = OVERLAY_UPDATE;
    gf_group->rf_level[frame_index] = INTER_NORMAL;
    gf_group->layer_depth[frame_index] = MAX_ARF_LAYERS - 1;
    gf_group->gfu_boost[frame_index] = NORMAL_BOOST;
  } else {
    gf_group->update_type[frame_index] = GF_UPDATE;
    gf_group->rf_level[frame_index] = GF_ARF_STD;
    gf_group->layer_depth[frame_index] = 0;
  }
}

static void define_gf_group_structure(VP9_COMP *cpi) {
  RATE_CONTROL *const rc = &cpi->rc;
  TWO_PASS *const twopass = &cpi->twopass;
  GF_GROUP *const gf_group = &twopass->gf_group;
  int frame_index = 0;
  int key_frame = cpi->common.frame_type == KEY_FRAME;
  int layer_depth = 1;
  int gop_frames =
      rc->baseline_gf_interval - (key_frame || rc->source_alt_ref_pending);

  gf_group->frame_start = cpi->common.current_video_frame;
  gf_group->frame_end = gf_group->frame_start + rc->baseline_gf_interval;
  gf_group->max_layer_depth = 0;
  gf_group->allowed_max_layer_depth = 0;



  if (!key_frame)
    set_gf_overlay_frame_type(gf_group, frame_index, rc->source_alt_ref_active);

  ++frame_index;

  if (rc->source_alt_ref_pending) {
    gf_group->update_type[frame_index] = ARF_UPDATE;
    gf_group->rf_level[frame_index] = GF_ARF_STD;
    gf_group->layer_depth[frame_index] = layer_depth;
    gf_group->arf_src_offset[frame_index] =
        (unsigned char)(rc->baseline_gf_interval - 1);
    gf_group->frame_gop_index[frame_index] = rc->baseline_gf_interval;
    gf_group->max_layer_depth = 1;
    ++frame_index;
    ++layer_depth;
    gf_group->allowed_max_layer_depth = cpi->oxcf.enable_auto_arf;
  }

  find_arf_order(cpi, gf_group, &frame_index, layer_depth, 1, gop_frames);

  set_gf_overlay_frame_type(gf_group, frame_index, rc->source_alt_ref_pending);
  gf_group->arf_src_offset[frame_index] = 0;
  gf_group->frame_gop_index[frame_index] = rc->baseline_gf_interval;

  gf_group->gf_group_size = frame_index;
}

static void allocate_gf_group_bits(VP9_COMP *cpi, int64_t gf_group_bits,
                                   int gf_arf_bits) {
  VP9EncoderConfig *const oxcf = &cpi->oxcf;
  RATE_CONTROL *const rc = &cpi->rc;
  TWO_PASS *const twopass = &cpi->twopass;
  GF_GROUP *const gf_group = &twopass->gf_group;
  FIRSTPASS_STATS frame_stats;
  int i;
  int frame_index = 0;
  int target_frame_size;
  int key_frame;
  const int max_bits = frame_max_bits(&cpi->rc, oxcf);
  int64_t total_group_bits = gf_group_bits;
  int mid_frame_idx;
  int normal_frames;
  int normal_frame_bits;
  int last_frame_reduction = 0;
  double av_score = 1.0;
  double tot_norm_frame_score = 1.0;
  double this_frame_score = 1.0;

  int gop_frames = gf_group->gf_group_size;

  key_frame = cpi->common.frame_type == KEY_FRAME;



  if (!key_frame) {
    gf_group->bit_allocation[frame_index] =
        rc->source_alt_ref_active ? 0 : gf_arf_bits;
  }


  if (rc->source_alt_ref_pending || !key_frame) total_group_bits -= gf_arf_bits;

  ++frame_index;


  if (rc->source_alt_ref_pending) {
    gf_group->bit_allocation[frame_index] = gf_arf_bits;

    ++frame_index;
  }

  mid_frame_idx = frame_index + (rc->baseline_gf_interval >> 1) - 1;

  normal_frames = (rc->baseline_gf_interval - 1);
  if (normal_frames > 1)
    normal_frame_bits = (int)(total_group_bits / normal_frames);
  else
    normal_frame_bits = (int)total_group_bits;

  gf_group->gfu_boost[1] = rc->gfu_boost;

  if (cpi->multi_layer_arf) {
    int idx;
    int arf_depth_bits[MAX_ARF_LAYERS] = { 0 };
    int arf_depth_count[MAX_ARF_LAYERS] = { 0 };
    int arf_depth_boost[MAX_ARF_LAYERS] = { 0 };
    int total_arfs = 1;  // Account for the base layer ARF.

    for (idx = 0; idx < gop_frames; ++idx) {
      if (gf_group->update_type[idx] == ARF_UPDATE) {
        arf_depth_boost[gf_group->layer_depth[idx]] += gf_group->gfu_boost[idx];
        ++arf_depth_count[gf_group->layer_depth[idx]];
      }
    }

    for (idx = 2; idx < MAX_ARF_LAYERS; ++idx) {
      if (arf_depth_boost[idx] == 0) break;
      arf_depth_bits[idx] = calculate_boost_bits(
          rc->baseline_gf_interval - total_arfs - arf_depth_count[idx],
          arf_depth_boost[idx], total_group_bits);

      total_group_bits -= arf_depth_bits[idx];
      total_arfs += arf_depth_count[idx];
    }

    normal_frames -= (total_arfs - 1);
    if (normal_frames > 1)
      normal_frame_bits = (int)(total_group_bits / normal_frames);
    else
      normal_frame_bits = (int)total_group_bits;

    target_frame_size = normal_frame_bits;
    target_frame_size =
        clamp(target_frame_size, 0, VPXMIN(max_bits, (int)total_group_bits));

    for (idx = frame_index; idx < gop_frames; ++idx) {
      switch (gf_group->update_type[idx]) {
        case ARF_UPDATE:
          gf_group->bit_allocation[idx] =
              (int)(((int64_t)arf_depth_bits[gf_group->layer_depth[idx]] *
                     gf_group->gfu_boost[idx]) /
                    arf_depth_boost[gf_group->layer_depth[idx]]);
          break;
        case USE_BUF_FRAME: gf_group->bit_allocation[idx] = 0; break;
        default: gf_group->bit_allocation[idx] = target_frame_size; break;
      }
    }
    gf_group->bit_allocation[idx] = 0;

    return;
  }

  if (oxcf->vbr_corpus_complexity) {
    av_score = get_distribution_av_err(cpi, twopass);
    tot_norm_frame_score = calculate_group_score(cpi, av_score, normal_frames);
  }

  for (i = 0; i < normal_frames; ++i) {
    if (EOF == input_stats(twopass, &frame_stats)) break;
    if (oxcf->vbr_corpus_complexity) {
      this_frame_score = calculate_norm_frame_score(cpi, twopass, oxcf,
                                                    &frame_stats, av_score);
      normal_frame_bits = (int)((double)total_group_bits *
                                (this_frame_score / tot_norm_frame_score));
    }

    target_frame_size = normal_frame_bits;
    if ((i == (normal_frames - 1)) && (i >= 1)) {
      last_frame_reduction = normal_frame_bits / 16;
      target_frame_size -= last_frame_reduction;
    }

    target_frame_size =
        clamp(target_frame_size, 0, VPXMIN(max_bits, (int)total_group_bits));

    gf_group->bit_allocation[frame_index] = target_frame_size;
    ++frame_index;
  }

  gf_group->bit_allocation[mid_frame_idx] += last_frame_reduction;




}

static void adjust_group_arnr_filter(VP9_COMP *cpi, double section_noise,
                                     double section_inter,
                                     double section_motion) {
  TWO_PASS *const twopass = &cpi->twopass;
  double section_zeromv = section_inter - section_motion;

  twopass->arnr_strength_adjustment = 0;

  if (section_noise < 150) {
    twopass->arnr_strength_adjustment -= 1;
    if (section_noise < 75) twopass->arnr_strength_adjustment -= 1;
  } else if (section_noise > 250)
    twopass->arnr_strength_adjustment += 1;

  if (section_zeromv > 0.50) twopass->arnr_strength_adjustment += 1;
}

#define ARF_ABS_ZOOM_THRESH 4.0

#define MAX_GF_BOOST 5400

typedef struct RANGE {
  int min;
  int max;
} RANGE;

/* get_gop_coding_frame_num() depends on several fields in RATE_CONTROL *rc as
 * follows.
 * Static fields:
 * (The following fields will remain unchanged after initialization of encoder.)
 *   rc->static_scene_max_gf_interval
 *   rc->min_gf_interval
 *
 * Dynamic fields:
 * (The following fields will be updated before or after coding each frame.)
 *   rc->frames_to_key
 *   rc->frames_since_key
 *   rc->source_alt_ref_active
 *
 * Special case: if CONFIG_RATE_CTRL is true, the external arf indexes will
 * determine the arf position.
 *
 * TODO(angiebird): Separate the dynamic fields and static fields into two
 * structs.
 */
static int get_gop_coding_frame_num(
    int *use_alt_ref, const FRAME_INFO *frame_info,
    const FIRST_PASS_INFO *first_pass_info, const RATE_CONTROL *rc,
    int gf_start_show_idx, const RANGE *active_gf_interval,
    double gop_intra_factor, int lag_in_frames) {
  double loop_decay_rate = 1.00;
  double mv_ratio_accumulator = 0.0;
  double this_frame_mv_in_out = 0.0;
  double mv_in_out_accumulator = 0.0;
  double abs_mv_in_out_accumulator = 0.0;
  double sr_accumulator = 0.0;

  double mv_ratio_accumulator_thresh =
      (frame_info->frame_height + frame_info->frame_width) / 4.0;
  double zero_motion_accumulator = 1.0;
  int gop_coding_frames;

  *use_alt_ref = 1;
  gop_coding_frames = 0;
  while (gop_coding_frames < rc->static_scene_max_gf_interval &&
         gop_coding_frames < rc->frames_to_key) {
    const FIRSTPASS_STATS *next_next_frame;
    const FIRSTPASS_STATS *next_frame;
    int flash_detected;
    ++gop_coding_frames;

    next_frame = fps_get_frame_stats(first_pass_info,
                                     gf_start_show_idx + gop_coding_frames);
    if (next_frame == NULL) {
      break;
    }


    next_next_frame = fps_get_frame_stats(
        first_pass_info, gf_start_show_idx + gop_coding_frames + 1);
    flash_detected = detect_flash_from_frame_stats(next_next_frame);

    accumulate_frame_motion_stats(
        next_frame, &this_frame_mv_in_out, &mv_in_out_accumulator,
        &abs_mv_in_out_accumulator, &mv_ratio_accumulator);

    if ((rc->frames_since_key + gop_coding_frames - 1) > 1) {
      zero_motion_accumulator =
          VPXMIN(zero_motion_accumulator,
                 get_zero_motion_factor(frame_info, next_frame));
    }

    if (!flash_detected) {
      double last_loop_decay_rate = loop_decay_rate;
      loop_decay_rate = get_prediction_decay_rate(frame_info, next_frame);


      if (gop_coding_frames > rc->min_gf_interval && loop_decay_rate >= 0.999 &&
          last_loop_decay_rate < 0.9) {
        int still_interval = 5;
        if (check_transition_to_still(first_pass_info,
                                      gf_start_show_idx + gop_coding_frames,
                                      still_interval)) {
          *use_alt_ref = 0;
          break;
        }
      }



      if (gop_coding_frames == 1) {
        sr_accumulator += next_frame->coded_error;
      } else {
        sr_accumulator +=
            (next_frame->sr_coded_error - next_frame->coded_error);
      }
    }












    if ((gop_coding_frames >= active_gf_interval->max) &&
        ((zero_motion_accumulator < 0.995) || (rc->source_alt_ref_active))) {
      break;
    }
    if (

        (gop_coding_frames >= active_gf_interval->min) &&

        ((rc->frames_to_key - gop_coding_frames) >= rc->min_gf_interval) &&
        (gop_coding_frames & 0x01) && (!flash_detected) &&
        ((mv_ratio_accumulator > mv_ratio_accumulator_thresh) ||
         (abs_mv_in_out_accumulator > ARF_ABS_ZOOM_THRESH) ||
         (sr_accumulator > gop_intra_factor * next_frame->intra_error))) {
      break;
    }
  }
  *use_alt_ref &= zero_motion_accumulator < 0.995;
  *use_alt_ref &= gop_coding_frames < lag_in_frames;
  *use_alt_ref &= gop_coding_frames >= rc->min_gf_interval;
  return gop_coding_frames;
}

static RANGE get_active_gf_inverval_range(
    const FRAME_INFO *frame_info, const RATE_CONTROL *rc, int arf_active_or_kf,
    int gf_start_show_idx, int active_worst_quality, int last_boosted_qindex) {
  RANGE active_gf_interval;
#if CONFIG_RATE_CTRL
  (void)frame_info;
  (void)gf_start_show_idx;
  (void)active_worst_quality;
  (void)last_boosted_qindex;
  active_gf_interval.min = rc->min_gf_interval + arf_active_or_kf + 2;

  active_gf_interval.max = 16 + arf_active_or_kf;

  if ((active_gf_interval.max <= rc->frames_to_key) &&
      (active_gf_interval.max >= (rc->frames_to_key - rc->min_gf_interval))) {
    active_gf_interval.min = rc->frames_to_key / 2;
    active_gf_interval.max = rc->frames_to_key / 2;
  }
#else
  int int_max_q = (int)(vp9_convert_qindex_to_q(active_worst_quality,
                                                frame_info->bit_depth));
  int q_term = (gf_start_show_idx == 0)
                   ? int_max_q / 32
                   : (int)(vp9_convert_qindex_to_q(last_boosted_qindex,
                                                   frame_info->bit_depth) /
                           6);
  active_gf_interval.min =
      rc->min_gf_interval + arf_active_or_kf + VPXMIN(2, int_max_q / 200);
  active_gf_interval.min =
      VPXMIN(active_gf_interval.min, rc->max_gf_interval + arf_active_or_kf);




  active_gf_interval.max = 11 + arf_active_or_kf + VPXMIN(5, q_term);

  active_gf_interval.max = active_gf_interval.max | 0x01;


  if (active_gf_interval.max < active_gf_interval.min) {
    active_gf_interval.max = active_gf_interval.min;
  } else {
    active_gf_interval.max =
        VPXMIN(active_gf_interval.max, rc->max_gf_interval + arf_active_or_kf);
  }

  if ((active_gf_interval.max <= rc->frames_to_key) &&
      (active_gf_interval.max >= (rc->frames_to_key - rc->min_gf_interval))) {
    active_gf_interval.max = rc->frames_to_key / 2;
  }
  active_gf_interval.max =
      VPXMAX(active_gf_interval.max, active_gf_interval.min);
#endif
  return active_gf_interval;
}

static int get_arf_layers(int multi_layer_arf, int max_layers,
                          int coding_frame_num) {
  assert(max_layers <= MAX_ARF_LAYERS);
  if (multi_layer_arf) {
    int layers = 0;
    int i;
    for (i = coding_frame_num; i > 0; i >>= 1) {
      ++layers;
    }
    layers = VPXMIN(max_layers, layers);
    return layers;
  } else {
    return 1;
  }
}

static void define_gf_group(VP9_COMP *cpi, int gf_start_show_idx) {
  VP9_COMMON *const cm = &cpi->common;
  RATE_CONTROL *const rc = &cpi->rc;
  VP9EncoderConfig *const oxcf = &cpi->oxcf;
  TWO_PASS *const twopass = &cpi->twopass;
  const FRAME_INFO *frame_info = &cpi->frame_info;
  const FIRST_PASS_INFO *first_pass_info = &twopass->first_pass_info;
  const FIRSTPASS_STATS *const start_pos = twopass->stats_in;
  int gop_coding_frames;

  double gf_group_err = 0.0;
  double gf_group_raw_error = 0.0;
  double gf_group_noise = 0.0;
  double gf_group_skip_pct = 0.0;
  double gf_group_inactive_zone_rows = 0.0;
  double gf_group_inter = 0.0;
  double gf_group_motion = 0.0;

  int allow_alt_ref = is_altref_enabled(cpi);
  int use_alt_ref;

  int64_t gf_group_bits;
  int gf_arf_bits;
  const int is_key_frame = frame_is_intra_only(cm);


  const int arf_active_or_kf = is_key_frame || rc->source_alt_ref_active;
  int is_alt_ref_flash = 0;

  double gop_intra_factor;
  int gop_frames;
  RANGE active_gf_interval;


  if (is_key_frame == 0) {
    vp9_zero(twopass->gf_group);
  }

  vpx_clear_system_state();

  active_gf_interval = get_active_gf_inverval_range(
      frame_info, rc, arf_active_or_kf, gf_start_show_idx,
      twopass->active_worst_quality, rc->last_boosted_qindex);

  if (cpi->multi_layer_arf) {
    int arf_layers = get_arf_layers(cpi->multi_layer_arf, oxcf->enable_auto_arf,
                                    active_gf_interval.max);
    gop_intra_factor = 1.0 + 0.25 * arf_layers;
  } else {
    gop_intra_factor = 1.0;
  }

#if CONFIG_RATE_CTRL
  {
    const GOP_COMMAND *gop_command = &cpi->encode_command.gop_command;
    assert(allow_alt_ref == 1);
    if (gop_command->use) {
      gop_coding_frames = gop_command_coding_frame_count(gop_command);
      use_alt_ref = gop_command->use_alt_ref;
    } else {
      gop_coding_frames = get_gop_coding_frame_num(
          &use_alt_ref, frame_info, first_pass_info, rc, gf_start_show_idx,
          &active_gf_interval, gop_intra_factor, cpi->oxcf.lag_in_frames);
      use_alt_ref &= allow_alt_ref;
    }
  }
#else
  gop_coding_frames = get_gop_coding_frame_num(
      &use_alt_ref, frame_info, first_pass_info, rc, gf_start_show_idx,
      &active_gf_interval, gop_intra_factor, cpi->oxcf.lag_in_frames);
  use_alt_ref &= allow_alt_ref;
#endif

  rc->constrained_gf_group = (gop_coding_frames >= rc->frames_to_key) ? 1 : 0;

  if (use_alt_ref) {
    const int f_frames =
        (rc->frames_to_key - gop_coding_frames >= gop_coding_frames - 1)
            ? gop_coding_frames - 1
            : VPXMAX(0, rc->frames_to_key - gop_coding_frames);
    const int b_frames = gop_coding_frames - 1;
    const int avg_inter_frame_qindex = rc->avg_frame_qindex[INTER_FRAME];

    const int arf_show_idx = VPXMIN(gf_start_show_idx + gop_coding_frames + 1,
                                    fps_get_num_frames(first_pass_info));

    rc->gfu_boost =
        compute_arf_boost(frame_info, first_pass_info, arf_show_idx, f_frames,
                          b_frames, avg_inter_frame_qindex);
    rc->source_alt_ref_pending = 1;
  } else {
    const int f_frames = gop_coding_frames - 1;
    const int b_frames = 0;
    const int avg_inter_frame_qindex = rc->avg_frame_qindex[INTER_FRAME];

    const int gld_show_idx =
        VPXMIN(gf_start_show_idx + 1, fps_get_num_frames(first_pass_info));
    const int arf_boost =
        compute_arf_boost(frame_info, first_pass_info, gld_show_idx, f_frames,
                          b_frames, avg_inter_frame_qindex);
    rc->gfu_boost = VPXMIN(MAX_GF_BOOST, arf_boost);
    rc->source_alt_ref_pending = 0;
  }

#define LAST_ALR_ACTIVE_BEST_QUALITY_ADJUSTMENT_FACTOR 0.2
  rc->arf_active_best_quality_adjustment_factor = 1.0;
  rc->arf_increase_active_best_quality = 0;

  if (!is_lossless_requested(&cpi->oxcf)) {
    if (rc->frames_since_key >= rc->frames_to_key) {


      rc->arf_active_best_quality_adjustment_factor =
          LAST_ALR_ACTIVE_BEST_QUALITY_ADJUSTMENT_FACTOR +
          (1.0 - LAST_ALR_ACTIVE_BEST_QUALITY_ADJUSTMENT_FACTOR) *
              (rc->frames_to_key - gop_coding_frames) /
              (VPXMAX(1, ((rc->frames_to_key + rc->frames_since_key) / 2 -
                          gop_coding_frames)));
      rc->arf_increase_active_best_quality = 1;
    } else if ((rc->frames_to_key - gop_coding_frames) > 0) {

      rc->arf_active_best_quality_adjustment_factor =
          LAST_ALR_ACTIVE_BEST_QUALITY_ADJUSTMENT_FACTOR +
          (1.0 - LAST_ALR_ACTIVE_BEST_QUALITY_ADJUSTMENT_FACTOR) *
              (rc->frames_since_key + gop_coding_frames) /
              (VPXMAX(1, (rc->frames_to_key + rc->frames_since_key) / 2 +
                             gop_coding_frames));
      rc->arf_increase_active_best_quality = -1;
    }
  }

#ifdef AGGRESSIVE_VBR

  rc->gfu_boost = VPXMIN((int)rc->gfu_boost, gop_coding_frames * 140);
#else
  rc->gfu_boost = VPXMIN((int)rc->gfu_boost, gop_coding_frames * 200);
#endif




  if (oxcf->aq_mode == PERCEPTUAL_AQ)
    rc->gfu_boost = VPXMIN(rc->gfu_boost, MIN_ARF_GF_BOOST);

  rc->baseline_gf_interval = gop_coding_frames - rc->source_alt_ref_pending;

  if (rc->source_alt_ref_pending)
    is_alt_ref_flash = detect_flash(twopass, rc->baseline_gf_interval);

  {
    const double av_err = get_distribution_av_err(cpi, twopass);
    const double mean_mod_score = twopass->mean_mod_score;


    int start_idx = arf_active_or_kf ? 1 : 0;
    int j;
    for (j = start_idx; j < gop_coding_frames; ++j) {
      int show_idx = gf_start_show_idx + j;
      const FIRSTPASS_STATS *frame_stats =
          fps_get_frame_stats(first_pass_info, show_idx);

      gf_group_err += calc_norm_frame_score(oxcf, frame_info, frame_stats,
                                            mean_mod_score, av_err);
      gf_group_raw_error += frame_stats->coded_error;
      gf_group_noise += frame_stats->frame_noise_energy;
      gf_group_skip_pct += frame_stats->intra_skip_pct;
      gf_group_inactive_zone_rows += frame_stats->inactive_zone_rows;
      gf_group_inter += frame_stats->pcnt_inter;
      gf_group_motion += frame_stats->pcnt_motion;
    }
  }

  gf_group_bits = calculate_total_gf_group_bits(cpi, gf_group_err);

  gop_frames =
      rc->baseline_gf_interval + rc->source_alt_ref_pending - arf_active_or_kf;



  if (gop_frames > 0)
    twopass->gf_group.group_noise_energy = (int)(gf_group_noise / gop_frames);
  else
    twopass->gf_group.group_noise_energy = 0;





  if ((cpi->oxcf.rc_mode != VPX_Q) && (rc->baseline_gf_interval > 1)) {
    const int vbr_group_bits_per_frame = (int)(gf_group_bits / gop_frames);
    const double group_av_err = gf_group_raw_error / gop_frames;
    const double group_av_noise = gf_group_noise / gop_frames;
    const double group_av_skip_pct = gf_group_skip_pct / gop_frames;
    const double group_av_inactive_zone = ((gf_group_inactive_zone_rows * 2) /
                                           (gop_frames * (double)cm->mb_rows));
    int tmp_q = get_twopass_worst_quality(
        cpi, group_av_err, (group_av_skip_pct + group_av_inactive_zone),
        group_av_noise, vbr_group_bits_per_frame);
    twopass->active_worst_quality =
        (tmp_q + (twopass->active_worst_quality * 3)) >> 2;

#if CONFIG_ALWAYS_ADJUST_BPM

    twopass->rolling_arf_group_target_bits = 0;
    twopass->rolling_arf_group_actual_bits = 0;
#endif
  }

  if (rc->baseline_gf_interval > 1) {
    adjust_group_arnr_filter(cpi, (gf_group_noise / gop_frames),
                             (gf_group_inter / gop_frames),
                             (gf_group_motion / gop_frames));
  } else {
    twopass->arnr_strength_adjustment = 0;
  }

  gf_arf_bits = calculate_boost_bits((rc->baseline_gf_interval - 1),
                                     rc->gfu_boost, gf_group_bits);

  twopass->kf_group_error_left -= gf_group_err;

  define_gf_group_structure(cpi);

  allocate_gf_group_bits(cpi, gf_group_bits, gf_arf_bits);

  reset_fpf_position(twopass, start_pos);

  twopass->section_intra_rating = calculate_section_intra_ratio(
      start_pos, twopass->stats_in_end, rc->baseline_gf_interval);

  if (oxcf->resize_mode == RESIZE_DYNAMIC) {

    cpi->rc.next_frame_size_selector = UNSCALED;
  }
#if !CONFIG_ALWAYS_ADJUST_BPM

  twopass->rolling_arf_group_target_bits = 0;
  twopass->rolling_arf_group_actual_bits = 0;
#endif
  rc->preserve_arf_as_gld = rc->preserve_next_arf_as_gld;
  rc->preserve_next_arf_as_gld = 0;

  if (!is_lossless_requested(&cpi->oxcf) && !cpi->use_svc &&
      cpi->oxcf.aq_mode == NO_AQ && cpi->multi_layer_arf && !is_alt_ref_flash)
    rc->preserve_next_arf_as_gld = 1;
}

#define VERY_LOW_II 1.5
// Clean slide transitions we expect a sharp single frame spike in error.
#define ERROR_SPIKE 5.0

// Tests for case where there is very low error either side of the current frame
// but much higher just for this frame. This can help detect key frames in
// slide shows even where the slides are pictures of different sizes.
// Also requires that intra and inter errors are very similar to help eliminate
// harmful false positives.
// It will not help if the transition is a fade or other multi-frame effect.
static int slide_transition(const FIRSTPASS_STATS *this_frame,
                            const FIRSTPASS_STATS *last_frame,
                            const FIRSTPASS_STATS *next_frame) {
  return (this_frame->intra_error < (this_frame->coded_error * VERY_LOW_II)) &&
         (this_frame->coded_error > (last_frame->coded_error * ERROR_SPIKE)) &&
         (this_frame->coded_error > (next_frame->coded_error * ERROR_SPIKE));
}

// related to the previous and next frame as an indicator for coding a key
// frame. This test serves to detect some additional scene cuts,
// especially in lowish motion and low contrast sections, that are missed
// by the other tests.
static int intra_step_transition(const FIRSTPASS_STATS *this_frame,
                                 const FIRSTPASS_STATS *last_frame,
                                 const FIRSTPASS_STATS *next_frame) {
  double last_ii_ratio;
  double this_ii_ratio;
  double next_ii_ratio;
  double last_pcnt_intra = 1.0 - last_frame->pcnt_inter;
  double this_pcnt_intra = 1.0 - this_frame->pcnt_inter;
  double next_pcnt_intra = 1.0 - next_frame->pcnt_inter;
  double mod_this_intra = this_pcnt_intra + this_frame->pcnt_neutral;

  last_ii_ratio =
      last_frame->intra_error / DOUBLE_DIVIDE_CHECK(last_frame->coded_error);
  this_ii_ratio =
      this_frame->intra_error / DOUBLE_DIVIDE_CHECK(this_frame->coded_error);
  next_ii_ratio =
      next_frame->intra_error / DOUBLE_DIVIDE_CHECK(next_frame->coded_error);



  if ((this_ii_ratio < 2.0) && (last_ii_ratio > 2.25) &&
      (next_ii_ratio > 2.25) && (this_pcnt_intra > (3 * last_pcnt_intra)) &&
      (this_pcnt_intra > (3 * next_pcnt_intra)) &&
      ((this_pcnt_intra > 0.075) || (mod_this_intra > 0.85))) {
    return 1;


  } else if ((this_ii_ratio < 1.25) && (mod_this_intra > 0.85) &&
             (this_ii_ratio < last_ii_ratio * 0.9) &&
             (this_ii_ratio < next_ii_ratio * 0.9)) {
    return 1;
  } else {
    return 0;
  }
}

#define MIN_INTRA_LEVEL 0.25
// Threshold for use of the lagging second reference frame. Scene cuts do not
// usually have a high second ref useage.
#define SECOND_REF_USEAGE_THRESH 0.2
// Hard threshold where the first pass chooses intra for almost all blocks.
// In such a case even if the frame is not a scene cut coding a key frame
// may be a good option.
#define VERY_LOW_INTER_THRESH 0.05
// Maximum threshold for the relative ratio of intra error score vs best
// inter error score.
#define KF_II_ERR_THRESHOLD 2.5
#define KF_II_MAX 128.0
#define II_FACTOR 12.5
// Test for very low intra complexity which could cause false key frames
#define V_LOW_INTRA 0.5

static int test_candidate_kf(const FIRST_PASS_INFO *first_pass_info,
                             int show_idx) {
  const FIRSTPASS_STATS *last_frame =
      fps_get_frame_stats(first_pass_info, show_idx - 1);
  const FIRSTPASS_STATS *this_frame =
      fps_get_frame_stats(first_pass_info, show_idx);
  const FIRSTPASS_STATS *next_frame =
      fps_get_frame_stats(first_pass_info, show_idx + 1);
  int is_viable_kf = 0;
  double pcnt_intra = 1.0 - this_frame->pcnt_inter;



  detect_flash_from_frame_stats(next_frame);
  if (!detect_flash_from_frame_stats(this_frame) &&
      !detect_flash_from_frame_stats(next_frame) &&
      (this_frame->pcnt_second_ref < SECOND_REF_USEAGE_THRESH) &&
      ((this_frame->pcnt_inter < VERY_LOW_INTER_THRESH) ||
       (slide_transition(this_frame, last_frame, next_frame)) ||
       (intra_step_transition(this_frame, last_frame, next_frame)) ||
       (((this_frame->coded_error > (next_frame->coded_error * 1.2)) &&
         (this_frame->coded_error > (last_frame->coded_error * 1.2))) &&
        (pcnt_intra > MIN_INTRA_LEVEL) &&
        ((pcnt_intra + this_frame->pcnt_neutral) > 0.5) &&
        ((this_frame->intra_error /
          DOUBLE_DIVIDE_CHECK(this_frame->coded_error)) <
         KF_II_ERR_THRESHOLD)))) {
    int i;
    double boost_score = 0.0;
    double old_boost_score = 0.0;
    double decay_accumulator = 1.0;

    for (i = 0; i < 16; ++i) {
      const FIRSTPASS_STATS *frame_stats =
          fps_get_frame_stats(first_pass_info, show_idx + 1 + i);
      double next_iiratio = (II_FACTOR * frame_stats->intra_error /
                             DOUBLE_DIVIDE_CHECK(frame_stats->coded_error));

      if (next_iiratio > KF_II_MAX) next_iiratio = KF_II_MAX;

      if (frame_stats->pcnt_inter > 0.85)
        decay_accumulator *= frame_stats->pcnt_inter;
      else
        decay_accumulator *= (0.85 + frame_stats->pcnt_inter) / 2.0;

      boost_score += (decay_accumulator * next_iiratio);

      if ((frame_stats->pcnt_inter < 0.05) || (next_iiratio < 1.5) ||
          (((frame_stats->pcnt_inter - frame_stats->pcnt_neutral) < 0.20) &&
           (next_iiratio < 3.0)) ||
          ((boost_score - old_boost_score) < 3.0) ||
          (frame_stats->intra_error < V_LOW_INTRA)) {
        break;
      }

      old_boost_score = boost_score;

      if (show_idx + 1 + i == fps_get_num_frames(first_pass_info) - 1) break;
    }


    if (boost_score > 30.0 && (i > 3)) {
      is_viable_kf = 1;
    } else {
      is_viable_kf = 0;
    }
  }

  return is_viable_kf;
}

#define FRAMES_TO_CHECK_DECAY 8
#define MIN_KF_TOT_BOOST 300
#define DEFAULT_SCAN_FRAMES_FOR_KF_BOOST 32
#define MAX_SCAN_FRAMES_FOR_KF_BOOST 48
#define MIN_SCAN_FRAMES_FOR_KF_BOOST 32
#define KF_ABS_ZOOM_THRESH 6.0

#ifdef AGGRESSIVE_VBR
#define KF_MAX_FRAME_BOOST 80.0
#define MAX_KF_TOT_BOOST 4800
#else
#define KF_MAX_FRAME_BOOST 96.0
#define MAX_KF_TOT_BOOST 5400
#endif

int vp9_get_frames_to_next_key(const VP9EncoderConfig *oxcf,
                               const FRAME_INFO *frame_info,
                               const FIRST_PASS_INFO *first_pass_info,
                               int kf_show_idx, int min_gf_interval) {
  double recent_loop_decay[FRAMES_TO_CHECK_DECAY];
  int j;
  int frames_to_key;
  int max_frames_to_key = first_pass_info->num_frames - kf_show_idx;
  max_frames_to_key = VPXMIN(max_frames_to_key, oxcf->key_freq);

  for (j = 0; j < FRAMES_TO_CHECK_DECAY; ++j) recent_loop_decay[j] = 1.0;

  if (!oxcf->auto_key) {
    frames_to_key = max_frames_to_key;
  } else {
    frames_to_key = 1;
    while (frames_to_key < max_frames_to_key) {

      if (kf_show_idx + frames_to_key + 1 < first_pass_info->num_frames) {
        double loop_decay_rate;
        double decay_accumulator;
        const FIRSTPASS_STATS *next_frame = fps_get_frame_stats(
            first_pass_info, kf_show_idx + frames_to_key + 1);

        if (test_candidate_kf(first_pass_info, kf_show_idx + frames_to_key))
          break;

        loop_decay_rate = get_prediction_decay_rate(frame_info, next_frame);



        recent_loop_decay[(frames_to_key - 1) % FRAMES_TO_CHECK_DECAY] =
            loop_decay_rate;
        decay_accumulator = 1.0;
        for (j = 0; j < FRAMES_TO_CHECK_DECAY; ++j)
          decay_accumulator *= recent_loop_decay[j];


        if ((frames_to_key - 1) > min_gf_interval && loop_decay_rate >= 0.999 &&
            decay_accumulator < 0.9) {
          int still_interval = oxcf->key_freq - (frames_to_key - 1);

          int show_idx = kf_show_idx + frames_to_key;
          if (check_transition_to_still(first_pass_info, show_idx,
                                        still_interval)) {
            break;
          }
        }
      }
      ++frames_to_key;
    }
  }
  return frames_to_key;
}

static void find_next_key_frame(VP9_COMP *cpi, int kf_show_idx) {
  int i;
  RATE_CONTROL *const rc = &cpi->rc;
  TWO_PASS *const twopass = &cpi->twopass;
  GF_GROUP *const gf_group = &twopass->gf_group;
  const VP9EncoderConfig *const oxcf = &cpi->oxcf;
  const FIRST_PASS_INFO *first_pass_info = &twopass->first_pass_info;
  const FRAME_INFO *frame_info = &cpi->frame_info;
  const FIRSTPASS_STATS *const start_position = twopass->stats_in;
  const FIRSTPASS_STATS *keyframe_stats =
      fps_get_frame_stats(first_pass_info, kf_show_idx);
  FIRSTPASS_STATS next_frame;
  int kf_bits = 0;
  int64_t max_kf_bits;
  double zero_motion_accumulator = 1.0;
  double zero_motion_sum = 0.0;
  double zero_motion_avg;
  double motion_compensable_sum = 0.0;
  double motion_compensable_avg;
  int num_frames = 0;
  int kf_boost_scan_frames = DEFAULT_SCAN_FRAMES_FOR_KF_BOOST;
  double boost_score = 0.0;
  double kf_mod_err = 0.0;
  double kf_raw_err = 0.0;
  double kf_group_err = 0.0;
  double sr_accumulator = 0.0;
  double abs_mv_in_out_accumulator = 0.0;
  const double av_err = get_distribution_av_err(cpi, twopass);
  const double mean_mod_score = twopass->mean_mod_score;
  vp9_zero(next_frame);

  cpi->common.frame_type = KEY_FRAME;
  rc->frames_since_key = 0;

  vp9_zero(*gf_group);

  rc->this_key_frame_forced = rc->next_key_frame_forced;


  rc->source_alt_ref_active = 0;

  rc->frames_till_gf_update_due = 0;

  rc->frames_to_key = 1;

  twopass->kf_group_bits = 0;          // Total bits available to kf group
  twopass->kf_group_error_left = 0.0;  // Group modified error score.

  kf_raw_err = keyframe_stats->intra_error;
  kf_mod_err = calc_norm_frame_score(oxcf, frame_info, keyframe_stats,
                                     mean_mod_score, av_err);

  rc->frames_to_key = vp9_get_frames_to_next_key(
      oxcf, frame_info, first_pass_info, kf_show_idx, rc->min_gf_interval);




  if (rc->frames_to_key >= cpi->oxcf.key_freq) {
    rc->next_key_frame_forced = 1;
  } else {
    rc->next_key_frame_forced = 0;
  }

  for (i = 0; i < rc->frames_to_key; ++i) {
    const FIRSTPASS_STATS *frame_stats =
        fps_get_frame_stats(first_pass_info, kf_show_idx + i);

    kf_group_err += calc_norm_frame_score(oxcf, frame_info, frame_stats,
                                          mean_mod_score, av_err);
  }

  if (twopass->bits_left > 0 && twopass->normalized_score_left > 0.0) {

    const int max_bits = frame_max_bits(rc, &cpi->oxcf);

    int64_t max_grp_bits;


    twopass->kf_group_bits = (int64_t)(
        twopass->bits_left * (kf_group_err / twopass->normalized_score_left));

    max_grp_bits = (int64_t)max_bits * (int64_t)rc->frames_to_key;
    if (twopass->kf_group_bits > max_grp_bits)
      twopass->kf_group_bits = max_grp_bits;
  } else {
    twopass->kf_group_bits = 0;
  }
  twopass->kf_group_bits = VPXMAX(0, twopass->kf_group_bits);


  boost_score = 0.0;

  for (i = 0; i < VPXMIN(MAX_SCAN_FRAMES_FOR_KF_BOOST, (rc->frames_to_key - 1));
       ++i) {
    if (EOF == input_stats(twopass, &next_frame)) break;

    zero_motion_sum += next_frame.pcnt_inter - next_frame.pcnt_motion;
    motion_compensable_sum +=
        1 - (double)next_frame.coded_error / next_frame.intra_error;
    num_frames++;
  }

  if (num_frames >= MIN_SCAN_FRAMES_FOR_KF_BOOST) {
    zero_motion_avg = zero_motion_sum / num_frames;
    motion_compensable_avg = motion_compensable_sum / num_frames;
    kf_boost_scan_frames = (int)(VPXMAX(64 * zero_motion_avg - 16,
                                        160 * motion_compensable_avg - 112));
    kf_boost_scan_frames =
        VPXMAX(VPXMIN(kf_boost_scan_frames, MAX_SCAN_FRAMES_FOR_KF_BOOST),
               MIN_SCAN_FRAMES_FOR_KF_BOOST);
  }
  reset_fpf_position(twopass, start_position);

  for (i = 0; i < (rc->frames_to_key - 1); ++i) {
    if (EOF == input_stats(twopass, &next_frame)) break;



    if ((i <= kf_boost_scan_frames) || (zero_motion_accumulator >= 0.99)) {
      double frame_boost;
      double zm_factor;


      if (i > 0) {
        zero_motion_accumulator =
            VPXMIN(zero_motion_accumulator,
                   get_zero_motion_factor(&cpi->frame_info, &next_frame));
      } else {
        zero_motion_accumulator =
            next_frame.pcnt_inter - next_frame.pcnt_motion;
      }

      zm_factor = (0.75 + (zero_motion_accumulator / 2.0));




      if (i < 2) sr_accumulator = 0.0;
      frame_boost = calc_kf_frame_boost(cpi, &next_frame, &sr_accumulator, 0,
                                        KF_MAX_FRAME_BOOST * zm_factor);

      boost_score += frame_boost;

      abs_mv_in_out_accumulator +=
          fabs(next_frame.mv_in_out_count * next_frame.pcnt_motion);

      if ((frame_boost < 25.00) ||
          (abs_mv_in_out_accumulator > KF_ABS_ZOOM_THRESH) ||
          (sr_accumulator > (kf_raw_err * 1.50)))
        break;
    } else {
      break;
    }
  }

  reset_fpf_position(twopass, start_position);

  twopass->kf_zeromotion_pct = (int)(zero_motion_accumulator * 100.0);

  twopass->key_frame_section_intra_rating = calculate_section_intra_ratio(
      start_position, twopass->stats_in_end, rc->frames_to_key);


  if ((zero_motion_accumulator > 0.99) && (rc->frames_to_key > 8)) {
    rc->kf_boost = MAX_KF_TOT_BOOST;
  } else {

    rc->kf_boost = VPXMAX((int)boost_score, (rc->frames_to_key * 3));
    rc->kf_boost = VPXMAX(rc->kf_boost, MIN_KF_TOT_BOOST);
    rc->kf_boost = VPXMIN(rc->kf_boost, MAX_KF_TOT_BOOST);
  }

  kf_bits = calculate_boost_bits((rc->frames_to_key - 1), rc->kf_boost,
                                 twopass->kf_group_bits);

  kf_bits +=
      (int)((twopass->kf_group_bits - kf_bits) * (kf_mod_err / kf_group_err));
  max_kf_bits =
      twopass->kf_group_bits - (rc->frames_to_key - 1) * FRAME_OVERHEAD_BITS;
  max_kf_bits = lclamp(max_kf_bits, 0, INT_MAX);
  kf_bits = VPXMIN(kf_bits, (int)max_kf_bits);

  twopass->kf_group_bits -= kf_bits;

  gf_group->bit_allocation[0] = kf_bits;
  gf_group->update_type[0] = KF_UPDATE;
  gf_group->rf_level[0] = KF_STD;
  gf_group->layer_depth[0] = 0;

  twopass->kf_group_error_left = (kf_group_err - kf_mod_err);



  twopass->normalized_score_left -= kf_group_err;

  if (oxcf->resize_mode == RESIZE_DYNAMIC) {

    cpi->rc.next_frame_size_selector = UNSCALED;
  }
}

static int is_skippable_frame(const VP9_COMP *cpi) {




  const TWO_PASS *const twopass = &cpi->twopass;

  return (!frame_is_intra_only(&cpi->common) &&
          twopass->stats_in - 2 > twopass->stats_in_start &&
          twopass->stats_in < twopass->stats_in_end &&
          (twopass->stats_in - 1)->pcnt_inter -
                  (twopass->stats_in - 1)->pcnt_motion ==
              1 &&
          (twopass->stats_in - 2)->pcnt_inter -
                  (twopass->stats_in - 2)->pcnt_motion ==
              1 &&
          twopass->stats_in->pcnt_inter - twopass->stats_in->pcnt_motion == 1);
}

void vp9_rc_get_second_pass_params(VP9_COMP *cpi) {
  VP9_COMMON *const cm = &cpi->common;
  RATE_CONTROL *const rc = &cpi->rc;
  TWO_PASS *const twopass = &cpi->twopass;
  GF_GROUP *const gf_group = &twopass->gf_group;
  FIRSTPASS_STATS this_frame;
  const int show_idx = cm->current_video_frame;

  if (!twopass->stats_in) return;


  if (gf_group->update_type[gf_group->index] == ARF_UPDATE) {
    int target_rate;

    vp9_zero(this_frame);
    this_frame =
        cpi->twopass.stats_in_start[cm->current_video_frame +
                                    gf_group->arf_src_offset[gf_group->index]];

    vp9_configure_buffer_updates(cpi, gf_group->index);

    target_rate = gf_group->bit_allocation[gf_group->index];
    target_rate = vp9_rc_clamp_pframe_target_size(cpi, target_rate);
    rc->base_frame_target = target_rate;

    cm->frame_type = INTER_FRAME;


    if (cpi->sf.allow_partition_search_skip && cpi->oxcf.pass == 2 &&
        !cpi->use_svc) {
      cpi->partition_search_skippable_frame = is_skippable_frame(cpi);
    }


    twopass->mb_av_energy = log((this_frame.intra_error * 256.0) + 1.0);
    twopass->mb_smooth_pct = this_frame.intra_smooth_pct;

    return;
  }

  vpx_clear_system_state();

  if (cpi->oxcf.rc_mode == VPX_Q) {
    twopass->active_worst_quality = cpi->oxcf.cq_level;
  } else if (cm->current_video_frame == 0) {
    const int frames_left =
        (int)(twopass->total_stats.count - cm->current_video_frame);

    const int section_target_bandwidth =
        (int)(twopass->bits_left / frames_left);
    const double section_length = twopass->total_left_stats.count;
    const double section_error =
        twopass->total_left_stats.coded_error / section_length;
    const double section_intra_skip =
        twopass->total_left_stats.intra_skip_pct / section_length;
    const double section_inactive_zone =
        (twopass->total_left_stats.inactive_zone_rows * 2) /
        ((double)cm->mb_rows * section_length);
    const double section_noise =
        twopass->total_left_stats.frame_noise_energy / section_length;
    int tmp_q;

    tmp_q = get_twopass_worst_quality(
        cpi, section_error, section_intra_skip + section_inactive_zone,
        section_noise, section_target_bandwidth);

    twopass->active_worst_quality = tmp_q;
    twopass->baseline_active_worst_quality = tmp_q;
    rc->ni_av_qi = tmp_q;
    rc->last_q[INTER_FRAME] = tmp_q;
    rc->avg_q = vp9_convert_qindex_to_q(tmp_q, cm->bit_depth);
    rc->avg_frame_qindex[INTER_FRAME] = tmp_q;
    rc->last_q[KEY_FRAME] = (tmp_q + cpi->oxcf.best_allowed_q) / 2;
    rc->avg_frame_qindex[KEY_FRAME] = rc->last_q[KEY_FRAME];
  }
  vp9_zero(this_frame);
  if (EOF == input_stats(twopass, &this_frame)) return;

  if (this_frame.intra_skip_pct >= FC_ANIMATION_THRESH)
    twopass->fr_content_type = FC_GRAPHICS_ANIMATION;
  else
    twopass->fr_content_type = FC_NORMAL;

  if (rc->frames_to_key == 0 || (cpi->frame_flags & FRAMEFLAGS_KEY)) {

    find_next_key_frame(cpi, show_idx);
  } else {
    cm->frame_type = INTER_FRAME;
  }

  if (rc->frames_till_gf_update_due == 0) {
    define_gf_group(cpi, show_idx);

    rc->frames_till_gf_update_due = rc->baseline_gf_interval;

#if ARF_STATS_OUTPUT
    {
      FILE *fpfile;
      fpfile = fopen("arf.stt", "a");
      ++arf_count;
      fprintf(fpfile, "%10d %10ld %10d %10d %10ld %10ld\n",
              cm->current_video_frame, rc->frames_till_gf_update_due,
              rc->kf_boost, arf_count, rc->gfu_boost, cm->frame_type);

      fclose(fpfile);
    }
#endif
  }

  vp9_configure_buffer_updates(cpi, gf_group->index);


  if (cpi->sf.allow_partition_search_skip && cpi->oxcf.pass == 2 &&
      !cpi->use_svc) {
    cpi->partition_search_skippable_frame = is_skippable_frame(cpi);
  }

  rc->base_frame_target = gf_group->bit_allocation[gf_group->index];


  twopass->mb_av_energy = log((this_frame.intra_error * 256.0) + 1.0);
  twopass->mb_smooth_pct = this_frame.intra_smooth_pct;

  subtract_stats(&twopass->total_left_stats, &this_frame);
}

#define MINQ_ADJ_LIMIT 48
#define MINQ_ADJ_LIMIT_CQ 20
#define HIGH_UNDERSHOOT_RATIO 2
void vp9_twopass_postencode_update(VP9_COMP *cpi) {
  TWO_PASS *const twopass = &cpi->twopass;
  RATE_CONTROL *const rc = &cpi->rc;
  VP9_COMMON *const cm = &cpi->common;
  const int bits_used = rc->base_frame_target;





  rc->vbr_bits_off_target += rc->base_frame_target - rc->projected_frame_size;
  twopass->bits_left = VPXMAX(twopass->bits_left - bits_used, 0);

  twopass->rolling_arf_group_target_bits += rc->this_frame_target;
  twopass->rolling_arf_group_actual_bits += rc->projected_frame_size;

  if (rc->total_actual_bits) {
    rc->rate_error_estimate =
        (int)((rc->vbr_bits_off_target * 100) / rc->total_actual_bits);
    rc->rate_error_estimate = clamp(rc->rate_error_estimate, -100, 100);
  } else {
    rc->rate_error_estimate = 0;
  }

  if (cpi->common.frame_type != KEY_FRAME) {
    twopass->kf_group_bits -= bits_used;
    twopass->last_kfgroup_zeromotion_pct = twopass->kf_zeromotion_pct;
  }
  twopass->kf_group_bits = VPXMAX(twopass->kf_group_bits, 0);

  ++twopass->gf_group.index;

  if ((cpi->oxcf.rc_mode != VPX_Q) && !cpi->rc.is_src_frame_alt_ref) {
    const int maxq_adj_limit =
        rc->worst_quality - twopass->active_worst_quality;
    const int minq_adj_limit =
        (cpi->oxcf.rc_mode == VPX_CQ ? MINQ_ADJ_LIMIT_CQ : MINQ_ADJ_LIMIT);
    int aq_extend_min = 0;
    int aq_extend_max = 0;


    if (cpi->oxcf.aq_mode != NO_AQ && cpi->oxcf.aq_mode != PSNR_AQ &&
        cpi->oxcf.aq_mode != PERCEPTUAL_AQ) {
      if (cm->seg.aq_av_offset < 0) {

        aq_extend_min = 0;
        aq_extend_max = VPXMIN(maxq_adj_limit, -cm->seg.aq_av_offset);
      } else {

        aq_extend_min = VPXMIN(minq_adj_limit, cm->seg.aq_av_offset);
        aq_extend_max = 0;
      }
    }

    if (rc->rate_error_estimate > cpi->oxcf.under_shoot_pct) {
      --twopass->extend_maxq;
      if (rc->rolling_target_bits >= rc->rolling_actual_bits)
        ++twopass->extend_minq;

    } else if (rc->rate_error_estimate < -cpi->oxcf.over_shoot_pct) {
      --twopass->extend_minq;
      if (rc->rolling_target_bits < rc->rolling_actual_bits)
        ++twopass->extend_maxq;
    } else {

      if (rc->projected_frame_size > (2 * rc->base_frame_target) &&
          rc->projected_frame_size > (2 * rc->avg_frame_bandwidth))
        ++twopass->extend_maxq;

      if (rc->rolling_target_bits < rc->rolling_actual_bits)
        --twopass->extend_minq;
      else if (rc->rolling_target_bits > rc->rolling_actual_bits)
        --twopass->extend_maxq;
    }

    twopass->extend_minq =
        clamp(twopass->extend_minq, aq_extend_min, minq_adj_limit);
    twopass->extend_maxq =
        clamp(twopass->extend_maxq, aq_extend_max, maxq_adj_limit);




    if (!frame_is_kf_gf_arf(cpi) && !cpi->rc.is_src_frame_alt_ref) {
      int fast_extra_thresh = rc->base_frame_target / HIGH_UNDERSHOOT_RATIO;
      if (rc->projected_frame_size < fast_extra_thresh) {
        rc->vbr_bits_off_target_fast +=
            fast_extra_thresh - rc->projected_frame_size;
        rc->vbr_bits_off_target_fast =
            VPXMIN(rc->vbr_bits_off_target_fast, (4 * rc->avg_frame_bandwidth));

        if (rc->avg_frame_bandwidth) {
          twopass->extend_minq_fast =
              (int)(rc->vbr_bits_off_target_fast * 8 / rc->avg_frame_bandwidth);
        }
        twopass->extend_minq_fast = VPXMIN(
            twopass->extend_minq_fast, minq_adj_limit - twopass->extend_minq);
      } else if (rc->vbr_bits_off_target_fast) {
        twopass->extend_minq_fast = VPXMIN(
            twopass->extend_minq_fast, minq_adj_limit - twopass->extend_minq);
      } else {
        twopass->extend_minq_fast = 0;
      }
    }
  }
}

#if CONFIG_RATE_CTRL
void vp9_get_next_group_of_picture(const VP9_COMP *cpi, int *first_is_key_frame,
                                   int *use_alt_ref, int *coding_frame_count,
                                   int *first_show_idx,
                                   int *last_gop_use_alt_ref) {
  const GOP_COMMAND *gop_command = &cpi->encode_command.gop_command;



  RATE_CONTROL rc = cpi->rc;
  const int multi_layer_arf = 0;
  const int allow_alt_ref = 1;




  *first_show_idx = cpi->common.current_video_frame;
  *last_gop_use_alt_ref = rc.source_alt_ref_active;

  *first_is_key_frame = 0;
  if (rc.frames_to_key == 0) {
    rc.frames_to_key = vp9_get_frames_to_next_key(
        &cpi->oxcf, &cpi->frame_info, &cpi->twopass.first_pass_info,
        *first_show_idx, rc.min_gf_interval);
    rc.frames_since_key = 0;
    *first_is_key_frame = 1;
  }

  if (gop_command->use) {
    *coding_frame_count = gop_command_coding_frame_count(gop_command);
    *use_alt_ref = gop_command->use_alt_ref;
    assert(*coding_frame_count < rc.frames_to_key);
  } else {
    *coding_frame_count = vp9_get_gop_coding_frame_count(
        &cpi->oxcf, &cpi->frame_info, &cpi->twopass.first_pass_info, &rc,
        *first_show_idx, multi_layer_arf, allow_alt_ref, *first_is_key_frame,
        *last_gop_use_alt_ref, use_alt_ref);
  }
}

int vp9_get_gop_coding_frame_count(const VP9EncoderConfig *oxcf,
                                   const FRAME_INFO *frame_info,
                                   const FIRST_PASS_INFO *first_pass_info,
                                   const RATE_CONTROL *rc, int show_idx,
                                   int multi_layer_arf, int allow_alt_ref,
                                   int first_is_key_frame,
                                   int last_gop_use_alt_ref, int *use_alt_ref) {
  int frame_count;
  double gop_intra_factor;
  const int arf_active_or_kf = last_gop_use_alt_ref || first_is_key_frame;
  RANGE active_gf_interval = get_active_gf_inverval_range(
      frame_info, rc, arf_active_or_kf, show_idx, /*active_worst_quality=*/0,
      /*last_boosted_qindex=*/0);

  const int arf_layers = get_arf_layers(multi_layer_arf, oxcf->enable_auto_arf,
                                        active_gf_interval.max);
  if (multi_layer_arf) {
    gop_intra_factor = 1.0 + 0.25 * arf_layers;
  } else {
    gop_intra_factor = 1.0;
  }

  frame_count = get_gop_coding_frame_num(
      use_alt_ref, frame_info, first_pass_info, rc, show_idx,
      &active_gf_interval, gop_intra_factor, oxcf->lag_in_frames);
  *use_alt_ref &= allow_alt_ref;
  return frame_count;
}

// coding frames (including show frame and alt ref) can be determined.
int vp9_get_coding_frame_num(const VP9EncoderConfig *oxcf,
                             const FRAME_INFO *frame_info,
                             const FIRST_PASS_INFO *first_pass_info,
                             int multi_layer_arf, int allow_alt_ref) {
  int coding_frame_num = 0;
  RATE_CONTROL rc;
  int gop_coding_frame_count;
  int gop_show_frames;
  int show_idx = 0;
  int last_gop_use_alt_ref = 0;
  vp9_rc_init(oxcf, 1, &rc);

  while (show_idx < first_pass_info->num_frames) {
    int use_alt_ref;
    int first_is_key_frame = 0;
    if (rc.frames_to_key == 0) {
      rc.frames_to_key = vp9_get_frames_to_next_key(
          oxcf, frame_info, first_pass_info, show_idx, rc.min_gf_interval);
      rc.frames_since_key = 0;
      first_is_key_frame = 1;
    }

    gop_coding_frame_count = vp9_get_gop_coding_frame_count(
        oxcf, frame_info, first_pass_info, &rc, show_idx, multi_layer_arf,
        allow_alt_ref, first_is_key_frame, last_gop_use_alt_ref, &use_alt_ref);

    rc.source_alt_ref_active = use_alt_ref;
    last_gop_use_alt_ref = use_alt_ref;
    gop_show_frames = gop_coding_frame_count - use_alt_ref;
    rc.frames_to_key -= gop_show_frames;
    rc.frames_since_key += gop_show_frames;
    show_idx += gop_show_frames;
    coding_frame_num += gop_show_frames + use_alt_ref;
  }
  return coding_frame_num;
}

void vp9_get_key_frame_map(const VP9EncoderConfig *oxcf,
                           const FRAME_INFO *frame_info,
                           const FIRST_PASS_INFO *first_pass_info,
                           int *key_frame_map) {
  int show_idx = 0;
  RATE_CONTROL rc;
  vp9_rc_init(oxcf, 1, &rc);



  memset(key_frame_map, 0,
         sizeof(*key_frame_map) * first_pass_info->num_frames);
  while (show_idx < first_pass_info->num_frames) {
    int key_frame_group_size;
    key_frame_map[show_idx] = 1;
    key_frame_group_size = vp9_get_frames_to_next_key(
        oxcf, frame_info, first_pass_info, show_idx, rc.min_gf_interval);
    assert(key_frame_group_size > 0);
    show_idx += key_frame_group_size;
  }
  assert(show_idx == first_pass_info->num_frames);
}
#endif  // CONFIG_RATE_CTRL

FIRSTPASS_STATS vp9_get_frame_stats(const TWO_PASS *twopass) {
  return twopass->this_frame_stats;
}
FIRSTPASS_STATS vp9_get_total_stats(const TWO_PASS *twopass) {
  return twopass->total_stats;
}
