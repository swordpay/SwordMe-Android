/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_SPEED_FEATURES_H_
#define VPX_VP9_ENCODER_VP9_SPEED_FEATURES_H_

#include "vp9/common/vp9_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  INTRA_ALL = (1 << DC_PRED) | (1 << V_PRED) | (1 << H_PRED) | (1 << D45_PRED) |
              (1 << D135_PRED) | (1 << D117_PRED) | (1 << D153_PRED) |
              (1 << D207_PRED) | (1 << D63_PRED) | (1 << TM_PRED),
  INTRA_DC = (1 << DC_PRED),
  INTRA_DC_TM = (1 << DC_PRED) | (1 << TM_PRED),
  INTRA_DC_H_V = (1 << DC_PRED) | (1 << V_PRED) | (1 << H_PRED),
  INTRA_DC_TM_H_V =
      (1 << DC_PRED) | (1 << TM_PRED) | (1 << V_PRED) | (1 << H_PRED)
};

enum {
  INTER_ALL = (1 << NEARESTMV) | (1 << NEARMV) | (1 << ZEROMV) | (1 << NEWMV),
  INTER_NEAREST = (1 << NEARESTMV),
  INTER_NEAREST_NEW = (1 << NEARESTMV) | (1 << NEWMV),
  INTER_NEAREST_ZERO = (1 << NEARESTMV) | (1 << ZEROMV),
  INTER_NEAREST_NEW_ZERO = (1 << NEARESTMV) | (1 << ZEROMV) | (1 << NEWMV),
  INTER_NEAREST_NEAR_NEW = (1 << NEARESTMV) | (1 << NEARMV) | (1 << NEWMV),
  INTER_NEAREST_NEAR_ZERO = (1 << NEARESTMV) | (1 << NEARMV) | (1 << ZEROMV),
};

enum {
  DISABLE_ALL_INTER_SPLIT = (1 << THR_COMP_GA) | (1 << THR_COMP_LA) |
                            (1 << THR_ALTR) | (1 << THR_GOLD) | (1 << THR_LAST),

  DISABLE_ALL_SPLIT = (1 << THR_INTRA) | DISABLE_ALL_INTER_SPLIT,

  DISABLE_COMPOUND_SPLIT = (1 << THR_COMP_GA) | (1 << THR_COMP_LA),

  LAST_AND_INTRA_SPLIT_ONLY = (1 << THR_COMP_GA) | (1 << THR_COMP_LA) |
                              (1 << THR_ALTR) | (1 << THR_GOLD)
};

typedef enum {
  DIAMOND = 0,
  NSTEP = 1,
  HEX = 2,
  BIGDIA = 3,
  SQUARE = 4,
  FAST_HEX = 5,
  FAST_DIAMOND = 6,
  MESH = 7
} SEARCH_METHODS;

typedef enum {

  DISALLOW_RECODE = 0,

  ALLOW_RECODE_KFMAXBW = 1,

  ALLOW_RECODE_KFARFGF = 2,

  ALLOW_RECODE_FIRST = 3,

  ALLOW_RECODE = 4,
} RECODE_LOOP_TYPE;

typedef enum {
  SUBPEL_TREE = 0,
  SUBPEL_TREE_PRUNED = 1,           // Prunes 1/2-pel searches
  SUBPEL_TREE_PRUNED_MORE = 2,      // Prunes 1/2-pel searches more aggressively
  SUBPEL_TREE_PRUNED_EVENMORE = 3,  // Prunes 1/2- and 1/4-pel searches

} SUBPEL_SEARCH_METHODS;

typedef enum {
  NO_MOTION_THRESHOLD = 0,
  LOW_MOTION_THRESHOLD = 7
} MOTION_THRESHOLD;

typedef enum {
  USE_FULL_RD = 0,
  USE_LARGESTALL,
  USE_TX_8X8
} TX_SIZE_SEARCH_METHOD;

typedef enum {
  NOT_IN_USE = 0,
  RELAXED_NEIGHBORING_MIN_MAX = 1,
  STRICT_NEIGHBORING_MIN_MAX = 2
} AUTO_MIN_MAX_MODE;

typedef enum {

  LPF_PICK_FROM_FULL_IMAGE,

  LPF_PICK_FROM_SUBIMAGE,

  LPF_PICK_FROM_Q,

  LPF_PICK_MINIMAL_LPF
} LPF_PICK_METHOD;

typedef enum {


  FLAG_EARLY_TERMINATE = 1 << 0,

  FLAG_SKIP_COMP_BESTINTRA = 1 << 1,

  FLAG_SKIP_INTRA_BESTINTER = 1 << 3,


  FLAG_SKIP_INTRA_DIRMISMATCH = 1 << 4,

  FLAG_SKIP_INTRA_LOWVAR = 1 << 5,
} MODE_SEARCH_SKIP_LOGIC;

typedef enum {
  FLAG_SKIP_EIGHTTAP = 1 << EIGHTTAP,
  FLAG_SKIP_EIGHTTAP_SMOOTH = 1 << EIGHTTAP_SMOOTH,
  FLAG_SKIP_EIGHTTAP_SHARP = 1 << EIGHTTAP_SHARP,
} INTERP_FILTER_MASK;

typedef enum {

  SEARCH_PARTITION,

  FIXED_PARTITION,

  REFERENCE_PARTITION,


  VAR_BASED_PARTITION,

  SOURCE_VAR_BASED_PARTITION,

  ML_BASED_PARTITION
} PARTITION_SEARCH_TYPE;

typedef enum {


  TWO_LOOP = 0,


  ONE_LOOP_REDUCED = 1
} FAST_COEFF_UPDATE;

typedef enum { EIGHTH_PEL, QUARTER_PEL, HALF_PEL, FULL_PEL } SUBPEL_FORCE_STOP;

typedef struct ADAPT_SUBPEL_FORCE_STOP {

  int mv_thresh;

  SUBPEL_FORCE_STOP force_stop_below;

  SUBPEL_FORCE_STOP force_stop_above;
} ADAPT_SUBPEL_FORCE_STOP;

typedef struct MV_SPEED_FEATURES {

  SEARCH_METHODS search_method;


  int reduce_first_step_size;


  int auto_mv_step_size;




  SUBPEL_SEARCH_METHODS subpel_search_method;


  int subpel_search_level;

  SUBPEL_FORCE_STOP subpel_force_stop;

  int enable_adaptive_subpel_force_stop;

  ADAPT_SUBPEL_FORCE_STOP adapt_subpel_force_stop;

  int fullpel_search_step_param;
} MV_SPEED_FEATURES;

typedef struct PARTITION_SEARCH_BREAKOUT_THR {
  int64_t dist;
  int rate;
} PARTITION_SEARCH_BREAKOUT_THR;

#define MAX_MESH_STEP 4

typedef struct MESH_PATTERN {
  int range;
  int interval;
} MESH_PATTERN;

typedef enum {

  NO_DETECTION = 0,


  FAST_DETECTION_MAXQ = 1,




  RE_ENCODE_MAXQ = 2
} OVERSHOOT_DETECTION_CBR_RT;

typedef enum {
  USE_2_TAPS = 0,
  USE_4_TAPS,
  USE_8_TAPS,
  USE_8_TAPS_SHARP,
} SUBPEL_SEARCH_TYPE;

typedef struct SPEED_FEATURES {
  MV_SPEED_FEATURES mv;

  int frame_parameter_update;

  RECODE_LOOP_TYPE recode_loop;

  int optimize_coefficients;





  int static_segmentation;




  BLOCK_SIZE comp_inter_joint_search_thresh;



  int adaptive_rd_thresh;


  int adaptive_rd_thresh_row_mt;



  int skip_encode_sb;
  int skip_encode_frame;


  int allow_skip_recode;

  int coeff_prob_appx_step;

  int allow_quant_coeff_opt;
  double quant_opt_thresh;


  int allow_acl;

  int enable_tpl_model;


  int allow_txfm_domain_distortion;
  double tx_domain_thresh;


  MOTION_THRESHOLD lf_motion_threshold;



  TX_SIZE_SEARCH_METHOD tx_size_search_method;

  int tx_size_search_depth;


  int use_lp32x32fdct;



  int mode_skip_start;

  int reference_masking;

  PARTITION_SEARCH_TYPE partition_search_type;

  BLOCK_SIZE always_this_block_size;


  int less_rectangular_check;


  int use_square_partition_only;
  BLOCK_SIZE use_square_only_thresh_high;
  BLOCK_SIZE use_square_only_thresh_low;

  int prune_ref_frame_for_rect_partitions;


  AUTO_MIN_MAX_MODE auto_min_max_partition_size;


  BLOCK_SIZE rd_auto_partition_min_limit;


  BLOCK_SIZE default_min_partition_size;
  BLOCK_SIZE default_max_partition_size;


  int adjust_partitioning_from_last_frame;


  int last_partitioning_redo_frequency;



  int disable_split_mask;



  int adaptive_motion_search;

  int enhanced_full_pixel_motion_search;

  int exhaustive_searches_thresh;

  MESH_PATTERN mesh_patterns[MAX_MESH_STEP];

  int schedule_mode_search;




  int adaptive_pred_interp_filter;

  int adaptive_mode_search;

  int cb_pred_filter_search;

  int cb_partition_search;

  int motion_field_mode_search;

  int alt_ref_search_fp;

  int use_quant_fp;


  int force_frame_boost;

  int max_delta_qindex;



  unsigned int mode_search_skip_flags;


  unsigned int disable_filter_search_var_thresh;


  int intra_y_mode_mask[TX_SIZES];
  int intra_uv_mode_mask[TX_SIZES];


  int intra_y_mode_bsize_mask[BLOCK_SIZES];



  int use_rd_breakout;



  int use_uv_intra_rd_estimate;

  LPF_PICK_METHOD lpf_pick;


  FAST_COEFF_UPDATE use_fast_coef_updates;

  int use_nonrd_pick_mode;


  int inter_mode_mask[BLOCK_SIZES];


  int use_fast_coef_costing;


  int recode_tolerance_low;
  int recode_tolerance_high;



  BLOCK_SIZE max_intra_bsize;


  int search_type_check_frequency;



  int reuse_inter_pred_sby;


  int encode_breakout_thresh;

  INTERP_FILTER default_interp_filter;


  int tx_size_search_breakout;

  int adaptive_interp_filter_search;

  INTERP_FILTER_MASK interp_filter_search_mask;

  PARTITION_SEARCH_BREAKOUT_THR partition_search_breakout_thr;

  struct {

    int search_breakout;


    float search_breakout_thresh[3];

    int search_early_termination;


    int var_pruning;





    int prune_rect_thresh[4];
  } rd_ml_partition;

  int allow_partition_search_skip;

  int simple_model_rd_from_var;


  int short_circuit_flat_blocks;








  int short_circuit_low_temp_var;


  int limit_newmv_early_exit;

  int bias_golden;


  int base_mv_aggressive;

  int copy_partition_flag;


  int use_source_sad;

  int use_simple_block_yrd;



  int adapt_partition_source_sad;
  int adapt_partition_thresh;

  int use_altref_onepass;

  int use_compound_nonrd_pickmode;

  int nonrd_keyframe;

  int svc_use_lowres_part;


  OVERSHOOT_DETECTION_CBR_RT overshoot_detection_cbr_rt;

  int disable_16x16part_nonkey;

  int disable_golden_ref;


  SUBPEL_SEARCH_TYPE use_accurate_subpel_search;

  SEARCH_METHODS temporal_filter_search_method;

  int nonrd_use_ml_partition;

  int variance_part_thresh_mult;

  int force_smooth_interpol;


  int rt_intra_dc_only_low_content;
} SPEED_FEATURES;

struct VP9_COMP;

void vp9_set_speed_features_framesize_independent(struct VP9_COMP *cpi,
                                                  int speed);
void vp9_set_speed_features_framesize_dependent(struct VP9_COMP *cpi,
                                                int speed);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VP9_ENCODER_VP9_SPEED_FEATURES_H_
